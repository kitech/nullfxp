# src.pro --- 
# 
# Author: liuguangzhao
# Copyright (C) 2007-2010 liuguangzhao@users.sf.net
# URL: http://www.qtchina.net http://nullget.sourceforge.net
# Created: 2009-05-18 22:03:43 +0800
# Version: $Id: src.pro 613 2010-04-14 04:11:45Z liuguangzhao $
# 

DESTDIR = .
TEMPLATE = lib
TARGET = ssh

CONFIG += dll console #staticlib
CONFIG -= qt 

VERSION = 0.4.6

win32 {
	CONFIG += release
} else {
	CONFIG += debug release
}

DEFINES += HAVE_CONFIG_H

SOURCES += agent.c        \
        auth1.c        \
        auth.c        \
        base64.c        \
        buffer.c        \
        callbacks.c        \
        channels1.c        \
        channels.c        \
        client.c        \
        config.c        \
        connect.c        \
        crc32.c        \
        crypt.c        \
        dh.c        \
        error.c        \
        gcrypt_missing.c        \
        gzip.c        \
        init.c        \
        kex.c        \
        keyfiles.c        \
        keys.c        \
        log.c        \
        match.c        \
        messages.c        \
        misc.c        \
        options.c        \
        packet.c        \
        pcap.c        \
        poll.c        \
        scp.c        \
        server.c        \
        session.c        \
        sftp.c        \
        sftpserver.c        \
        socket.c        \
        string.c        \
        wrapper.c 

HEADERS += 

win32 {
	#SOURCES += libgcrypt.c
	#SOURCES += openssl.c	
} else {
	#SOURCES += openssl.c
}


# controll if show libssh debug message, using this line
# DEFINES += LIBSSH2DEBUG=1 

win32 {
    !win32-g++ {
        DEFINES += LIBSSH2_WIN32 _CRT_SECURE_NO_DEPRECATE
        ## check cl.exe, x64 or x86
        CLARCH=$$system(path)
        VAMD64=$$find(CLARCH,amd64)
        isEmpty(VAMD64) {
            INCLUDEPATH += Z:/librarys/vc-ssl-x86/include Z:/librarys/vc-zlib/include
        } else {
            INCLUDEPATH += Z:/librarys/vc-ssl-x64/include Z:/librarys/vc-zlib/include
        }
    }
} else {
     LIBS += -lssl -lcrypto -lz
}

macx-g++ {
    QMAKE_CFLAGS_DEBUG += -arch i386
    QMAKE_CFLAGS_RELEASE += -arch i386
    QMAKE_CXXFLAGS_DEBUG += -arch i386
    QMAKE_CXXFLAGS_RELEASE += -arch i386
}

QMAKE_CFLAGS_DEBUG += -g
QMAKE_CFLAGS_RELEASE += -g
QMAKE_CXXFLAGS_DEBUG += -g
QMAKE_CXXFLAGS_RELEASE += -g

INCLUDEPATH += ../include/ .

