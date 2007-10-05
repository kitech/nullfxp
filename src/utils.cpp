#include "utils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "globaloption.h"

#ifdef WIN32

//#define	_IFMT		0170000	/* type of file */
//#define		_IFDIR	0040000	/* directory */
//#define		_IFCHR	0020000	/* character special */
#define		_IFBLK	0060000	/* block special */
//#define		_IFREG	0100000	/* regular */
#define		_IFLNK	0120000	/* symbolic link */
#define		_IFSOCK	0140000	/* socket */
//#define		_IFIFO	0010000	/* fifo */

#define 	S_BLKSIZE  1024 /* size of a block */

#define	S_ISUID		0004000	/* set user id on execution */
#define	S_ISGID		0002000	/* set group id on execution */

#define	S_ISVTX		0001000	/* save swapped text even after use */
//#define	S_IREAD		0000400	/* read permission, owner */
//#define	S_IWRITE 	0000200	/* write permission, owner */
//#define	S_IEXEC		0000100	/* execute/search permission, owner */
#define	S_ENFMT 	0002000	/* enforcement-mode locking */

//#define	S_IFMT		_IFMT
//#define	S_IFDIR		_IFDIR
//#define	S_IFCHR		_IFCHR
//#define	S_IFBLK		_IFBLK
//#define	S_IFREG		_IFREG
#define	S_IFLNK		_IFLNK
#define	S_IFSOCK	_IFSOCK
//#define	S_IFIFO		_IFIFO

#define link(from, to) 0


/* The Windows header files define _S_ forms of these, so we do too
   for easier portability.  */
//#define _S_IFMT		_IFMT
//#define _S_IFDIR	_IFDIR
//#define _S_IFCHR	_IFCHR
//#define _S_IFIFO	_IFIFO
//#define _S_IFREG	_IFREG
//#define _S_IREAD	0000400
//#define _S_IWRITE	0000200
//#define _S_IEXEC	0000100

#define		S_IWGRP	0000020	/* write permission, grougroup */
#define		S_IXGRP 0000010/* execute/search permission, group */
#define		S_IRGRP	0000040	/* read permission, group */
#define		S_IROTH	0000004	/* read permission, other */
#define		S_IWOTH	0000002	/* write permission, other */
#define		S_IXOTH 0000001/* execute/search permission, other */


#endif

/* XXX mode should be mode_t */

void
strmode ( int mode, char *p )
{
	/* print type */
	switch ( mode & S_IFMT )
	{
		case S_IFDIR:			/* directory */
			*p++ = 'd';
			break;
		case S_IFCHR:			/* character special */
			*p++ = 'c';
			break;
		case S_IFBLK:			/* block special */
			*p++ = 'b';
			break;
		case S_IFREG:			/* regular */
			*p++ = '-';
			break;
		case S_IFLNK:			/* symbolic link */
			*p++ = 'l';
			break;
//#ifdef S_IFSOCK
		case S_IFSOCK:			/* socket */
			*p++ = 's';
			break;
//#endif
//#ifdef S_IFIFO
		case S_IFIFO:			/* fifo */
			*p++ = 'p';
			break;
//#endif
		default:			/* unknown */
			*p++ = '?';
			break;
	}
	/* usr */
	if ( mode & S_IRUSR )
		*p++ = 'r';
	else
		*p++ = '-';
	if ( mode & S_IWUSR )
		*p++ = 'w';
	else
		*p++ = '-';
	switch ( mode & ( S_IXUSR | S_ISUID ) )
	{
		case 0:
			*p++ = '-';
			break;
		case S_IXUSR:
			*p++ = 'x';
			break;
		case S_ISUID:
			*p++ = 'S';
			break;
		case S_IXUSR | S_ISUID:
			*p++ = 's';
			break;
	}
	/* group */
	if ( mode & S_IRGRP )
		*p++ = 'r';
	else
		*p++ = '-';
	if ( mode & S_IWGRP )
		*p++ = 'w';
	else
		*p++ = '-';
	switch ( mode & ( S_IXGRP | S_ISGID ) )
	{
		case 0:
			*p++ = '-';
			break;
		case S_IXGRP:
			*p++ = 'x';
			break;
		case S_ISGID:
			*p++ = 'S';
			break;
		case S_IXGRP | S_ISGID:
			*p++ = 's';
			break;
	}
	/* other */
	if ( mode & S_IROTH )
		*p++ = 'r';
	else
		*p++ = '-';
	if ( mode & S_IWOTH )
		*p++ = 'w';
	else
		*p++ = '-';
	switch ( mode & ( S_IXOTH | S_ISVTX ) )
	{
		case 0:
			*p++ = '-';
			break;
		case S_IXOTH:
			*p++ = 'x';
			break;
		case S_ISVTX:
			*p++ = 'T';
			break;
		case S_IXOTH | S_ISVTX:
			*p++ = 't';
			break;
	}
	*p++ = ' ';		/* will be a '+' if ACL's implemented */
	*p = '\0';
}
//假设这个path的编码方式是本地系统上所用的编码方式
int     is_dir(char *path)
{
    struct stat sb;

    /* XXX: report errors? */
    if (stat(path, &sb) == -1)
	{
		fprintf(stderr, " is reg : %d %s %s \n" , errno,strerror(errno),path );
		return(0);
	}

    return(S_ISDIR(sb.st_mode));
}
//假设这个path的编码方式是本地系统上所用的编码方式
int is_reg(char *path)
{
    struct stat sb;

    if (stat(path, &sb) == -1)
    {
		fprintf(stderr, " is reg : %d %s %s \n" , errno,strerror(errno),path );
        return (0);
    }
    return(S_ISREG(sb.st_mode));
}
//假设这个path的编码方式是本地系统上所用的编码方式
void  fxp_local_do_ls( QString args , QVector<QMap<char, QString> > & fileinfos )
{
    int sz ;
    QMap<char,QString> thefile;
    char file_size[32];
    char file_date[64];
    char file_type[32];
    char fname[PATH_MAX+1];
    //char the_path[PATH_MAX+1];
	QString the_path ;
    
    struct tm *ltime;
    struct stat thestat ;    
    
	DIR * dh = opendir( GlobalOption::instance()->locale_codec->fromUnicode(args) ) ;
    struct dirent * entry = NULL ;
    fileinfos.clear();
    
    while( ( entry = readdir(dh)) != NULL )
    {
        thefile.clear();
        memset(&thestat,0,sizeof(thestat));
        //strcpy(the_path,args);
        //strcat(the_path,"/");
        //strcat(the_path,entry->d_name);
		the_path = args + "/"  + GlobalOption::instance()->locale_codec->toUnicode(entry->d_name);
        if(strcmp(entry->d_name,".") == 0) goto out_point;
        if(strcmp(entry->d_name,"..") == 0) goto out_point ;

		if(stat( GlobalOption::instance()->locale_codec->fromUnicode(the_path ) , &thestat) != 0 ) continue;
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
		thefile.insert( 'N',GlobalOption::instance()->locale_codec->toUnicode(fname) );
        thefile.insert( 'T',QString(file_type) );
        thefile.insert( 'S',QString(file_size ) );
        thefile.insert( 'D',QString( file_date ) );
        
        fileinfos.push_back(thefile);
        
        out_point:

                continue ;
    }
    closedir(dh);
}
//假设这个path的编码方式是本地系统上所用的编码方式
int     fxp_local_do_mkdir(const char * path )
{
    int ret = 0 ;
    
    #ifdef WIN32
    ret = mkdir(path);
    #else
    ret = mkdir(path,0777);
    #endif
    if( ret == -1 )
	{
		fprintf(stderr, " fxp_local_do_mkdir : %d %s %s \n" , errno,strerror(errno),path );
	}
    return ret ;
}
