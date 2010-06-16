// utils.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-06-01 09:58:24 +0800
// Version: $Id$
// 

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#ifndef _MSC_VER 
#include <unistd.h>
#include <fcntl.h>
#endif

#include "utils.h"
#include "globaloption.h"

/* XXX mode should be mode_t */
void strmode ( int mode, char *p )
{
    /* print type */
    switch (mode & S_IFMT)
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
    switch (mode & (S_IXUSR | S_ISUID))
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
    switch (mode & (S_IXGRP | S_ISGID))
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
    switch (mode & (S_IXOTH | S_ISVTX))
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
QString digit_mode(int mode)
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
    char dmode[16] = {0};
    int i = 0, v = 0;;
    for (i =0 ;i < 4 ; i++) {
        v = 0;
        if (mode & keys[i*3]) v += 4;
        if (mode & keys[i*3+1]) v += 2;
        if (mode & keys[i*3+2]) v += 1;
        sprintf(dmode+i, "%d", v);
        dmode[i+1] = '\0';
    }
    qDebug()<<"File digit mode: "<<QString(dmode);
    return QString(dmode);
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
#ifdef _MSC_VER
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
	QStringList entries;
	QDir dh(args);

	fileinfos.clear();
	entries = dh.entryList();
	for (int i = 0 ; i < entries.count(); i++) {
        thefile.clear();
		the_path = entries.at(i);
		if (the_path == "." || the_path == "..") {
			goto out_point;
		}
		the_path = args + "/"  + the_path;
		if (QFile::exists(the_path)) {
			QFileInfo fi(the_path);
			sprintf(file_size, "%llu", fi.size());
			strcpy(file_type, "-rwxrwxrwx-");
			sprintf(file_date, "%s", fi.lastModified().toString("yyyy/MM/dd hh:mm:ss"));

			thefile.insert('N', (entries.at(i)));
		    thefile.insert('T', QString(file_type) );
			thefile.insert('S', QString(file_size ) );
	        thefile.insert('D', QString( file_date ) );
        
		    fileinfos.push_back(thefile);
		}
    out_point:	continue;
	}
}
#else
void fxp_local_do_ls( QString args , QVector<QMap<char, QString> > & fileinfos)
{
    int sz;
    QMap<char,QString> thefile;
    char file_size[32];
    char file_date[64];
    char file_type[32];
    char fname[PATH_MAX+1] = {0};
    //char the_path[PATH_MAX+1];
    QString the_path ;
    
    struct tm *ltime;
    struct stat thestat ;    
    
    DIR * dh = opendir( GlobalOption::instance()->locale_codec->fromUnicode(args) ) ;
    struct dirent * entry = NULL ;
    fileinfos.clear();
    
    while ((entry = readdir(dh)) != NULL) {
        thefile.clear();
        memset(&thestat,0,sizeof(thestat));
        //strcpy(the_path,args);
        //strcat(the_path,"/");
        //strcat(the_path,entry->d_name);
        the_path = args + "/"  + GlobalOption::instance()->locale_codec->toUnicode(entry->d_name);
        if (strcmp(entry->d_name, ".") == 0) goto out_point;
        if (strcmp(entry->d_name, "..") == 0) goto out_point;

        if (stat(GlobalOption::instance()->locale_codec->fromUnicode(the_path), &thestat) != 0) continue;
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
        
    out_point: continue ;
    }
    closedir(dh);
}
#endif

long fxp_getpid()
{
    long pid = 0;
    
#ifdef WIN32
    pid = ::GetCurrentProcessId();
#else
    pid = ::getpid();
#endif
    
    return pid;
}

int set_nonblock (int sock)
{
    unsigned long flags = 0;
#ifdef WIN32
    flags = 1;
    return (ioctlsocket(sock, FIONBIO, &flags) != SOCKET_ERROR);
#else
    flags = fcntl(sock, F_GETFL, NULL);
    flags |= O_NONBLOCK;
    return (fcntl(sock, F_SETFL, flags) >= 0);
#endif
}

int set_sock_block(int sock)
{
    unsigned long flags = 0;
#ifdef WIN32

#else
    flags = fcntl(sock, F_GETFL, NULL);
    assert(flags >= 0);
    flags |= (~O_NONBLOCK);
    return (fcntl(sock, F_SETFL, flags) >=0);
#endif
    return 1;
}
