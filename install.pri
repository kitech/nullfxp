# install.pri --- 
# 
# Author: liuguangzhao
# Copyright (C) 2007-2010 liuguangzhao@users.sf.net
# URL: 
# Created: 2009-10-08 23:41:35 +0800
# Version: $Id$
# 

# cmdline: qmake PREFIX=/usr -r 
# or qmake "PREFIX=C:/Program Files/nullfxp" -r

win32 {
      isEmpty(PREFIX) {
         PREFIX = "C:/Program Files/nullfxp"
      }
} else {
#    message($$PREFIX)     
    isEmpty(PREFIX) {
        PREFIX = /opt/nullfxp
    }
}

    BINDIR = $$PREFIX/bin
    LIBDIR = $$PREFIX/lib

#    INSTALLS += target
#    target.path = $$BINDIR

    DATADIR = $$PREFIX/share
    PKGDATADIR = $$PREFIX/share/nullfxp

