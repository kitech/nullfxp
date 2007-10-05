/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   liuguangzhao@users.sourceforge.net   *
 *
 *   http://www.qtchina.net                                                *
 *   http://nullget.sourceforge.net                                        *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

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

void strmode(int mode, char *p);

int is_reg(char *path);

int     is_dir(char *path) ;

void  fxp_local_do_ls( QString args , QVector<QMap<char, QString> > & fileinfos  );

int  fxp_local_do_mkdir(const char * path );

#endif

