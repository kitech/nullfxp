# nscp.pro --- 
# 
# Author: liuguangzhao
# Copyright (C) 2007-2010 liuguangzhao@users.sf.net
# URL: http://www.qtchina.net http://nullget.sourceforge.net
# Created: 2009-05-19 20:50:18 +0800
# Last-Updated: 
# Version: $Id$
# 

TEMPLATE = app
CONFIG += qt console warn_on ordered  
TARGET = nscp
DESTDIR = ../../bin

SOURCES = nscp.cpp nscp_p.cpp

INCLUDEPATH += . .. ../libssh2/include

win32 {
    win32-g++ {
	    debug {
		LIBPATH += ./libssh2/src/debug
		LIBPATH += Z:/librarys/mw-ssl/lib
	    }
	    release {
		LIBPATH += ./libssh2/src/release 
		LIBPATH += Z:/librarys/mw-ssl/lib
	    }
	    LIBS += -lssl -lcrypto -lws2_32  -lgdi32 
    } else {
		LIBPATH += ./libssh2/src/release 
		LIBPATH += Z:/librarys/vc-ssl/lib Z:/librarys/vc-zlib/lib
		LIBS += -lzlib -llibeay32 -lssleay32 -ladvapi32 -luser32 -lwsock32
    }
    LIBS += -lssh2 -lws2_32  -lgdi32 
    #-lgcrypt -lgpg-error 
} else {
    LIBS += ../libssh2/src/libssh2.a -lssl -lcrypto
}

