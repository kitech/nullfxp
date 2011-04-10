/* utils.h --- 
 * 
 * Author: liuguangzhao
 * Copyright (C) 2007-2010 liuguangzhao@users.sf.net
 * URL: http://www.qtchina.net http://nullget.sourceforge.net
 * Created: 2008-06-01 09:58:33 +0800
 * Version: $Id$
 */

#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <map>
#include <string>
#include <assert.h>

#ifndef _MSC_VER
#include <dirent.h>
#else
#include <sys/stat.h>
#define PATH_MAX 4096
#endif

#ifdef WIN32
//#include <windows.h>
#include <winsock2.h>
#endif

#include <QMap>
#include <QVector>
#include <QPair>
#include <QtCore>

#if defined(__WIN32__) || defined(_WIN32)
//#       include <winsock.h>
        // Stuff for Visual C++ only
#       if defined(_MSC_VER)
#               define snprintf _snprintf
#       endif
#else
#       include <stdlib.h>
#endif


#ifdef WIN32

#ifndef S_ISLNK
# define S_ISLNK(mode)	(((mode) & S_IFMT) == S_IFLNK)
#endif /* S_ISLNK */

#define		_IFBLK	0060000	/* block special */
#define		_IFLNK	0120000	/* symbolic link */
#define		_IFSOCK	0140000	/* socket */
#define		_IFIFO	0010000	/* fifo */

#define 	S_BLKSIZE  1024 /* size of a block */

#define	S_ISUID		0004000	/* set user id on execution */
#define	S_ISGID		0002000	/* set group id on execution */

#define	S_ISVTX		0001000	/* save swapped text even after use */
#define	S_ENFMT 	0002000	/* enforcement-mode locking */

#define		S_IWGRP	0000020	/* write permission, grougroup */
#define		S_IXGRP 0000010/* execute/search permission, group */
#define		S_IRGRP	0000040	/* read permission, group */
#define		S_IROTH	0000004	/* read permission, other */
#define		S_IWOTH	0000002	/* write permission, other */
#define		S_IXOTH 0000001/* execute/search permission, other */

#endif

#ifdef _MSC_VER
 #ifndef S_ISDIR
 # define S_ISDIR(mode)	(((mode) & (_S_IFMT)) == (_S_IFDIR))
 #endif /* S_ISDIR */
 
 #ifndef S_ISREG
 # define S_ISREG(mode)	(((mode) & (_S_IFMT)) == (_S_IFREG))
 #endif /* S_ISREG */
 

 #define S_IFBLK _IFBLK
 #define	S_IFSOCK	_IFSOCK
 #define	S_IFLNK		_IFLNK
 #define    S_IFIFO     _IFIFO
#define S_IRUSR _S_IREAD
 #define S_IWUSR _S_IWRITE
 #define S_IXUSR _S_IEXEC
#endif

#ifdef __cplusplush
extern "C"{
#endif
    
    void strmode(int mode, char *p);
    QString digit_mode(int mode);

    int is_reg(char *path);

    void fxp_local_do_ls(QString args , QVector<QMap<char, QString> > & fileinfos);

    long fxp_getpid();

    /*
     * Sets a socket to non-blocking operation.
     */
    int set_sock_nonblock (int sock);
    int set_sock_block(int sock);


/*******************/
#define log_printf(ls, fn, ln, yorn, ... ) do { \
    char log[2560] = {0}; \
    snprintf(log, sizeof(log), __VA_ARGS__ ); \
    fn != NULL ? fprintf(stdout, "%s %s at %s on line %d.\n %s\n", ls, __FUNCTION__, fn , ln, log) : fprintf(stdout, "%s %s\n %s\n", ls, __FUNCTION__, log) ; \
    yorn == 'y' ? (1==1) : (1==1) ; \
    }while(0);
 
#define log_error( ... ) log_printf("Error:", __FILE__, __LINE__, 'n', __VA_ARGS__ )
#ifndef NDEBUG
#define log_debug( ... ) log_printf("Debug:", __FILE__, __LINE__, 'n', __VA_ARGS__ )
#else
#define log_debug( ... ) do {} while(0);
#endif
#define log_fetal( ... ) log_printf("Fetal:", __FILE__, __LINE__, 'y', __VA_ARGS__ )
#define qlog( ... ) log_printf("Info:", (char*)0, 0, 'n', __VA_ARGS__ )

#define q_debug()  qDebug()<<"DEBUG: "<<__FUNCTION__<<" at "<<__FILE__<<" on "<<__LINE__<<"\n    "

#define q_debug()  qDebug()<<"DEBUG: "<<__FUNCTION__<<" at "<<__FILE__<<" on "<<__LINE__<<"\n    "

#define q_warning()  qWarning()<<"DEBUG: "<<__FUNCTION__<<" at "<<__FILE__<<" on "<<__LINE__<<"\n    "

#define q_cretical()  qCretical()<<"DEBUG: "<<__FUNCTION__<<" at "<<__FILE__<<" on "<<__LINE__<<"\n    "

#define q_fetal()  qFetal()<<"DEBUG: "<<__FUNCTION__<<" at "<<__FILE__<<" on "<<__LINE__<<"\n    "


/************* log end ********/

#ifdef __cplusplush
};
#endif

#endif

