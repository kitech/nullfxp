# nullfxp.pro --- 
# 
# Author: liuguangzhao
# Copyright (C) 2007-2010 liuguangzhao@users.sf.net
# URL: http://www.qtchina.net http://nullget.sourceforge.net
# Created: 2009-05-18 22:02:53 +0800
# Version: $Id$
# 

SUBDIRS +=  \
 src/libssh2 \
 src \
 src/unitest

TEMPLATE = subdirs 
CONFIG = ordered
DESTDIR = .


