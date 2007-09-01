
#include "includes.h"

#include <sys/types.h>
#include <sys/ioctl.h>
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <errno.h>

#ifdef HAVE_PATHS_H
# include <paths.h>
#endif
#ifdef USE_LIBEDIT
#include <histedit.h>
#else
typedef void EditLine;
#endif
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <dirent.h>
#include "glob.h"

#include "xmalloc.h"
#include "log.h"
#include "pathnames.h"
#include "misc.h"

#include "sftp.h"
#include "buffer.h"

#include "sftp-common.h"
#include "sftp-client.h"

#include "sftp-operation.h"

#include "sftp-wrapper.h"


/* File to read commands from */
extern FILE* infile;

/* Are we in batchfile mode? */
extern int batchmode /*= 0*/;

/* Size of buffer used when copying files */
extern size_t copy_buffer_len /*= 32768*/;

/* Number of concurrent outstanding requests */
extern size_t num_requests /*= 16*/;

/* PID of ssh transport process */
extern /*static*/ pid_t sshpid /*= -1*/;

/* This is set to 0 if the progressmeter is not desired. */
extern int showprogress /*= 1*/;

/* SIGINT received during command processing */
extern volatile sig_atomic_t interrupted /*= 0*/;

/* I wish qsort() took a separate ctx for the comparison function...*/
extern int sort_flag;

int remote_glob(struct sftp_conn *, const char *, int,
   int (*)(const char *, int), glob_t *); /* proto for sftp-glob.c */

extern char *__progname;



//static 
void
        fxp_local_do_shell(const char *args)
{
    int status;
    char *shell;
    pid_t pid;

    if (!*args)
        args = NULL;

    if ((shell = getenv("SHELL")) == NULL)
        shell = _PATH_BSHELL;

    if ((pid = fork()) == -1)
    {
        fatal("Couldn't fork: %s", strerror(errno));
    }

    if (pid == 0) {
        /* XXX: child has pipe fds to ssh subproc open - issue? */
        if (args) {
            debug3("Executing %s -c \"%s\"", shell, args);
            execl(shell, shell, "-c", args, (char *)NULL);
        } else {
            debug3("Executing %s", shell);
            execl(shell, shell, (char *)NULL);
        }
        fprintf(stderr, "Couldn't execute \"%s\": %s\n", shell,
                strerror(errno));
        _exit(1);
    }
    while (waitpid(pid, &status, 0) == -1)
        if (errno != EINTR)
    {
        fatal("Couldn't wait for child: %s", strerror(errno));
    }
    if (!WIFEXITED(status))
    {
        error("Shell exited abnormally");
    }
    else if (WEXITSTATUS(status))
    {
        error("Shell exited with status %d", WEXITSTATUS(status));
    }
}

//static 
void
        fxp_local_do_ls(const char *args ,std::vector<std::map<char, std::string> > & fileinfos  )
{
    /*
    if (!args || !*args)
        local_do_shell(_PATH_LS);
    else {
        int len = strlen(_PATH_LS " ") + strlen(args) + 1;
        char *buf = (char*)xmalloc(len);

        //// XXX: quoting - rip quoting code from ftp? 
        snprintf(buf, len, _PATH_LS " %s", args);
        local_do_shell(buf);
        xfree(buf);
    }
    */
    int sz ;
    std::map<char,std::string> thefile;
    char file_size[32];
    char file_date[64];
    char file_type[32];
    char fname[PATH_MAX+1];
    char the_path[PATH_MAX+1];
    
    struct tm *ltime;
    struct stat thestat ;    
    debug3("open dir %s",args);
    
    DIR * dh = opendir(args);
    struct dirent * entry = NULL ;
    fileinfos.clear();
    
    while( ( entry = readdir(dh)) != NULL )
    {
        thefile.clear();
        memset(&thestat,0,sizeof(thestat));
        strcpy(the_path,args);
        strcat(the_path,"/");
        strcat(the_path,entry->d_name);
        if(strcmp(entry->d_name,".") == 0) goto out_point;
        if(strcmp(entry->d_name,"..") == 0) goto out_point ;
        
        debug3("stat file %s",the_path);
        
        if(stat(the_path,&thestat) != 0 ) continue;
        ltime = localtime(&thestat.st_mtime);
        
        sprintf(file_size,"%llu", thestat.st_size);
        strmode(thestat.st_mode,file_type);
        if (ltime != NULL) {
            if (time(NULL) - thestat.st_mtime < (365*24*60*60)/2)
                sz = strftime(file_date, sizeof file_date, "%Y/%m/%d %H:%M:%S", ltime);
            else
                sz = strftime(file_date, sizeof file_date, "%Y/%m/%d %H:%M:%S", ltime);
        } 
        strcpy(fname,entry->d_name);
        thefile.insert(std::make_pair('N',std::string(fname)));
        thefile.insert(std::make_pair('T',std::string(file_type)));
        thefile.insert(std::make_pair('S',std::string(file_size)));
        thefile.insert(std::make_pair('D',std::string( file_date )));
        
        fileinfos.push_back(thefile);
        
        out_point:
        //debug3("befer free dir entry ...");
        //free(entry);entry = NULL ;
        //debug3("after free dir entry ...");
                continue ;
    }
    closedir(dh);
}

int     fxp_local_do_mkdir(const char * path )
{
    int ret = 0 ;
    
    ret = mkdir(path,0777);
    
    return ret ;
}

/* sftp ls.1 replacement for directories */
//static 
int
        fxp_do_ls_dir(struct sftp_conn *conn, char *path, char *strip_path, int lflag,std::vector<std::map<char, std::string> > & fileinfos )
{
    int n;
    u_int c = 1, colspace = 0, columns = 1;
    SFTP_DIRENT **d;

    if ((n = do_readdir(conn, path, &d)) != 0)
        return (n);

    if (!(lflag & LS_SHORT_VIEW)) {
        u_int m = 0, width = 80;
        struct winsize ws;
        char *tmp;

        /* Count entries for sort and find longest filename */
        for (n = 0; d[n] != NULL; n++) {
            if (d[n]->filename[0] != '.' || (lflag & LS_SHOW_ALL))
                m = MAX(m, strlen(d[n]->filename));
        }

        /* Add any subpath that also needs to be counted */
        tmp = path_strip(path, strip_path);
        m += strlen(tmp);
        xfree(tmp);

        if (ioctl(fileno(stdin), TIOCGWINSZ, &ws) != -1)
            width = ws.ws_col;

        columns = width / (m + 2);
        columns = MAX(columns, 1);
        colspace = width / columns;
        colspace = MIN(colspace, width);
    }

    if (lflag & SORT_FLAGS) {
        for (n = 0; d[n] != NULL; n++)
            ;	/* count entries */
        sort_flag = lflag & (SORT_FLAGS|LS_REVERSE_SORT);
        qsort(d, n, sizeof(*d), sdirent_comp);
    }

    for (n = 0; d[n] != NULL && !interrupted; n++) {
        char *tmp, *fname;

        if (d[n]->filename[0] == '.' && !(lflag & LS_SHOW_ALL))
            continue;

        tmp = path_append(path, d[n]->filename);
        fname = path_strip(tmp, strip_path);
        xfree(tmp);

        if (lflag & LS_LONG_VIEW) {
            if (lflag & LS_NUMERIC_VIEW) {
                char *lname;
                struct stat sb;

                memset(&sb, 0, sizeof(sb));
                attrib_to_stat(&d[n]->a, &sb);
                lname = ls_file(fname, &sb, 1);
                printf("%s\n", lname);

                xfree(lname);
            } else
            {
                //char *lname;
                struct stat sb;
                memset(&sb, 0, sizeof(sb));
                attrib_to_stat(&d[n]->a, &sb);
                //lname = ls_file(fname, &sb, 1);
                //printf("%s\n", lname);
                //xfree(lname);
                struct tm *ltime = localtime(&sb.st_mtime);
                int sz = 0 ;                
                
                printf("%s\n", d[n]->longname);
                std::map<char,std::string> thefile;
                char file_size[32];
                char file_date[64];
                char file_type[32];
                memset(file_size,0,sizeof(file_size));
                memset(file_date,0,sizeof(file_date));
                memset(file_type,0,sizeof(file_type));
                
                sprintf(file_size,"%llu",d[n]->a.size);
                strmode(sb.st_mode,file_type);
                if (ltime != NULL) {
                    if (time(NULL) - sb.st_mtime < (365*24*60*60)/2)
                        sz = strftime(file_date, sizeof file_date, "%Y/%m/%d %H:%M:%S", ltime);
                    else
                        sz = strftime(file_date, sizeof file_date, "%Y/%m/%d %H:%M:%S", ltime);
                }                
                
                thefile.insert(std::make_pair('N',std::string(fname)));
                thefile.insert(std::make_pair('T',std::string(file_type)));
                thefile.insert(std::make_pair('S',std::string(file_size)));
                thefile.insert(std::make_pair('D',std::string( file_date )));
            
                fileinfos.push_back(thefile);                              
            }
        } else {
            printf("%-*s", colspace, fname);
            if (c >= columns) {
                printf("\n");
                c = 1;
            } else
                c++;
        }

        xfree(fname);
    }

    if (!(lflag & LS_LONG_VIEW) && (c != 1))
        printf("\n");

    free_sftp_dirents(d);
    return (0);
}


/* sftp ls.1 replacement which handles path globs */
//static
int
fxp_do_globbed_ls ( struct sftp_conn *conn, char *path, char *strip_path,
                    int lflag , std::vector<std::map<char, std::string> > & fileinfos  )
{
	glob_t g;
	u_int i, c = 1, colspace = 0, columns = 1;
	Attrib *a = NULL;

	memset ( &g, 0, sizeof ( g ) );

	if ( remote_glob ( conn, path, GLOB_MARK|GLOB_NOCHECK|GLOB_BRACE,
	                   NULL, &g ) || ( g.gl_pathc && !g.gl_matchc ) )
	{
		if ( g.gl_pathc )
			globfree ( &g );
		error ( "Can't ls: \"%s\" not found", path );
		return ( -1 );
	}

	if ( interrupted )
		goto out;

	/*
	       * If the glob returns a single match and it is a directory,
	       * then just list its contents.
	   */
	if ( g.gl_matchc == 1 )
	{
		if ( ( a = do_lstat ( conn, g.gl_pathv[0], 1 ) ) == NULL )
		{
			globfree ( &g );
			return ( -1 );
		}
		if ( ( a->flags & SSH2_FILEXFER_ATTR_PERMISSIONS ) &&
		        S_ISDIR ( a->perm ) )
		{
			int err;
            
			err = fxp_do_ls_dir ( conn, g.gl_pathv[0], strip_path, lflag,fileinfos );
            //printf("g.gl_matchc == 1 \n");
			globfree ( &g );
			return ( err );
		}
	}

	if ( ! ( lflag & LS_SHORT_VIEW ) )
	{
		u_int m = 0, width = 80;
		struct winsize ws;

		/* Count entries for sort and find longest filename */
		for ( i = 0; g.gl_pathv[i]; i++ )
			m = MAX ( m, strlen ( g.gl_pathv[i] ) );

		if ( ioctl ( fileno ( stdin ), TIOCGWINSZ, &ws ) != -1 )
			width = ws.ws_col;

		columns = width / ( m + 2 );
		columns = MAX ( columns, 1 );
		colspace = width / columns;
	}

	for ( i = 0; g.gl_pathv[i] && !interrupted; i++, a = NULL )
	{
		char *fname;

		fname = path_strip ( g.gl_pathv[i], strip_path );

		if ( lflag & LS_LONG_VIEW )
		{
			char *lname;
			struct stat sb;

			/*
			             * XXX: this is slow - 1 roundtrip per path
			             * A solution to this is to fork glob() and
			             * build a sftp specific version which keeps the
			             * attribs (which currently get thrown away)
			             * that the server returns as well as the filenames.
			         */
			memset ( &sb, 0, sizeof ( sb ) );
			if ( a == NULL )
				a = do_lstat ( conn, g.gl_pathv[i], 1 );
			if ( a != NULL )
				attrib_to_stat ( a, &sb );
			lname = ls_file ( fname, &sb, 1 );
			printf ( "%s\n", lname );
            std::map<char,std::string> thefile;
            thefile.insert(std::make_pair('N',std::string(fname)));
            thefile.insert(std::make_pair('T',std::string("tttt")));
            thefile.insert(std::make_pair('S',std::string("sss")));
            thefile.insert(std::make_pair('D',std::string("ddddd")));
            
            fileinfos.push_back(thefile);
            
            printf("fileinfo =====%d\n",fileinfos.size());
            
			xfree ( lname );
		}
		else
		{
			printf ( "%-*s", colspace, fname );
			if ( c >= columns )
			{
				printf ( "\n" );
				c = 1;
			}
			else
				c++;
		}
		xfree ( fname );
	}

	if ( ! ( lflag & LS_LONG_VIEW ) && ( c != 1 ) )
		printf ( "\n" );

out:
	if ( g.gl_pathc )
		globfree ( &g );

	return ( 0 );
}


