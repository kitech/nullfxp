# install.pri --- 
# 
# Author: liuguangzhao
# Copyright (C) 2007-2010 liuguangzhao@users.sf.net
# URL: 
# Created: 2009-10-08 23:41:35 +0800
# Version: $Id$
# 

unix {
    isEmpty(PREFIX) {
        PREFIX = /opt/nullfxp
    }
    BINDIR = $$PREFIX/bin

#    INSTALLS += target
#    target.path = $$BINDIR

    DATADIR = $$PREFIX/share
    PKGDATADIR = $$PREFIX/share/nullfxp
}

