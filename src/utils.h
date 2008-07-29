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
    const char *digit_mode(int mode);

int is_reg(char *path);

int     is_dir(char *path) ;

void  fxp_local_do_ls( QString args , QVector<QMap<char, QString> > & fileinfos  );

int  fxp_local_do_mkdir(const char * path );

long fxp_getpid();

/*
 * Sets a socket to non-blocking operation.
 */
int set_nonblock (int sock);


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

#define q_debug()  qDebug()<<"DEBUG: "<<__FILE__<<" on "<<__LINE__<<"\n    "

/************* log end ********/

#ifdef __cplusplush
};
#endif

#endif

