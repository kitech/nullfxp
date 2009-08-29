# src.pro --- 
# 
# Author: liuguangzhao
# Copyright (C) 2007-2010 liuguangzhao@users.sf.net
# URL: http://www.qtchina.net http://nullget.sourceforge.net
# Created: 2009-05-18 22:03:43 +0800
# Version: $Id$
# 

DESTDIR = .
TEMPLATE = lib
TARGET = ssh2

SOURCES += channel.c \
        comp.c \
        crypt.c \
        hostkey.c \
        knownhost.c \
        kex.c \
        mac.c \
        misc.c \
        #openssl.c \
        #libgcrypt.c \
        packet.c \
        pem.c \
        publickey.c \
        scp.c \
        session.c \
        sftp.c \
        transport.c \
        userauth.c \
        version.c \
        info.c \
        gettimeofday.c

HEADERS += libgcrypt.h \
        libssh2_config.h \
        libssh2_priv.h \
        openssl.h \
        info.h

win32 {
	#SOURCES += libgcrypt.c
	SOURCES += openssl.c	
} else {
	SOURCES += openssl.c
}

CONFIG += staticlib console 
CONFIG -= qt 

win32 {
	CONFIG += release
} else {
	CONFIG += debug release
}

DEFINES += HAVE_CONFIG_H LIBSSH2_MD5=1

# controll if show libssh debug message, using this line
# DEFINES += LIBSSH2DEBUG=1 

win32 {
    !win32-g++ {
        DEFINES += LIBSSH2_WIN32 _CRT_SECURE_NO_DEPRECATE
        INCLUDEPATH += Z:/librarys/vc-ssl/include Z:/librarys/vc-zlib/include
    }
}

QMAKE_CXXFLAGS_DEBUG += -g
QMAKE_CXXFLAGS_RELEASE += -g

INCLUDEPATH += ../include/

