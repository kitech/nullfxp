/* utils.h --- 
 * 
 * Filename: utils.h
 * Description: 
 * Author: liuguangzhao
 * Maintainer: 
 * Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sourceforge.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 日  6月  1 09:58:33 2008 (CST)
 * Version: 
 * Last-Updated: 
 *           By: 
 *     Update #: 0
 * URL: 
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change log:
 * 
 * 
 */


#include <vector>
#include <map>
#include <string>
#include <dirent.h>

#include <QMap>
#include <QVector>
#include <QPair>
#include <QtCore>

// #ifndef S_ISDIR
// # define S_ISDIR(mode)	(((mode) & (_S_IFMT)) == (_S_IFDIR))
// #endif /* S_ISDIR */
// 
// #ifndef S_ISREG
// # define S_ISREG(mode)	(((mode) & (_S_IFMT)) == (_S_IFREG))
// #endif /* S_ISREG */
// 
// #ifndef S_ISLNK
// # define S_ISLNK(mode)	(((mode) & S_IFMT) == S_IFLNK)
// #endif /* S_ISLNK */
#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplush
extern "C"{
#endif
    
void strmode(int mode, char *p);

int is_reg(char *path);

int     is_dir(char *path) ;

void  fxp_local_do_ls( QString args , QVector<QMap<char, QString> > & fileinfos  );

int  fxp_local_do_mkdir(const char * path );

long fxp_getpid();

/*
 * Sets a socket to non-blocking operation.
 */
int set_nonblock (int sock);


#ifdef __cplusplush
};
#endif

#endif

