// utils.cpp --- 
// 
// Filename: utils.cpp
// Description: 
// Author: liuguangzhao
// Maintainer: 
// Copyright (C) 2007-2010 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 日  6月  1 09:58:24 2008 (CST)
// Version: 
// Last-Updated: 
//           By: 
//     Update #: 0
// URL: 
// Keywords: 
// Compatibility: 
// 
// 

// Commentary: 
// 
// 
// 
// 

// Change log:
// 
// 
// 

#include "utils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "globaloption.h"



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
//
int     is_dir(char *path)
{
    struct stat sb;

    /* XXX: report errors? */
    if (stat(path, &sb) == -1)
    {
	fprintf(stderr, " is dir : %d %s %s \n" , errno,strerror(errno),path );
	return(0);
    }

    return(S_ISDIR(sb.st_mode));
}
const char *digit_mode(int mode)
{
    int keys[] = {
        S_ISUID,
        S_ISGID,
        S_ISVTX,
        S_IRUSR,
        S_IWUSR,
        S_IXUSR,
        S_IRGRP,
        S_IWGRP,
        S_IXGRP,
        S_IROTH,
        S_IWOTH,
        S_IXOTH,
        NULL
    };
    char dmode[5] = {0};
    int i = 0, v = 0;;
    for(i =0 ;i < 4 ; i++) {
        v = 0;
        if(mode & keys[i*3]) v += 4;
        if(mode & keys[i*3+1]) v += 2;
        if(mode & keys[i*3+2]) v += 1;
        sprintf(dmode+strlen(dmode), "%d", v);
    }
    return dmode;
}

//
int is_reg(char *path)
{
    struct stat sb;

    if (stat(path, &sb) == -1) {
        fprintf(stderr, " is reg : %d %s %s \n" , errno,strerror(errno),path );
        return (0);
    }
    return(S_ISREG(sb.st_mode));
}
//
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
//
int     fxp_local_do_mkdir(const char * path )
{
    int ret = 0 ;
    const char * ptr =0;

#ifdef WIN32
    if(path[2] == ':'){
	ptr = path + 1;
	ret = mkdir(ptr);
    }else{
	ret = mkdir(path);
    }
#else
    ret = mkdir(path,0777);
#endif
    if( ret == -1 )
    {
	fprintf(stderr, " fxp_local_do_mkdir : %d %s %s \n" , errno,strerror(errno),path );
    }
    return ret ;
}
long fxp_getpid()
{
    long pid = 0 ;
    
#ifdef WIN32
    pid = ::GetCurrentProcessId();
#else
    pid = ::getpid();
#endif
    
    return pid ;
}

int set_nonblock (int sock)
{
#ifdef WIN32
    unsigned long flags = 1;
    return (ioctlsocket(sock, FIONBIO, &flags) != SOCKET_ERROR);
#else
    return (fcntl(sock, F_SETFL, O_NONBLOCK) != -1);
#endif
}

