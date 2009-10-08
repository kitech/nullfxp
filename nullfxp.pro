# nullfxp.pro --- 
# 
# Author: liuguangzhao
# Copyright (C) 2007-2010 liuguangzhao@users.sf.net
# URL: http://www.qtchina.net http://nullget.sourceforge.net
# Created: 2009-05-18 22:02:53 +0800
# Version: $Id$
# 

# simple check qt version
QMAKEVERSION = $$[QMAKE_VERSION]
ISQT4 = $$find(QMAKEVERSION, ^[2-9])

isEmpty( ISQT4 ) {
    error("Use the qmake include with Qt4.4 or greater, on Debian that is qmake-qt4");
}

SUBDIRS +=  \
 src/libssh2 \
 src \
 src/unitest

TEMPLATE = subdirs 
CONFIG = ordered
DESTDIR = .


