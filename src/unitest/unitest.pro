# unitest.pro --- 
# 
# Author: liuguangzhao
# Copyright (C) 2007-2010 liuguangzhao@users.sf.net
# URL: http://www.qtchina.net http://nullget.sourceforge.net
# Created: 2009-05-18 22:04:17 +0800
# Version: $Id$
# 

TEMPLATE = app
TARGET = 
DESTDIR = ../../bin
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += .. ../libssh2/include/

include(../../install.pri)

QT += testlib network

# Input
HEADERS += testnullfxp.h
SOURCES += testmain.cpp testnullfxp.cpp

!win32 {
    SOURCES += gccfeatures.cpp
}

SOURCES += ../basestorage.cpp ../utils.cpp ../globaloption.cpp \
          ../sshfileinfo.cpp \
          ../libftp/curlftp.cpp ../libftp/curlftp_callback.cpp \
          ../security.cpp

HEADERS += ../libftp/curlftp.h  ../globaloption.h

win32 {
    win32-g++ {
    } else {
        DEFINES += LIBSSH2_WIN32 _CRT_SECURE_NO_DEPRECATE
         CONFIG -= embed_manifest_exe
         CONFIG -= embed_manifest_dll

        ## check cl.exe, x64 or x86
        CLARCH=$$system(path)
        VAMD64=$$find(CLARCH,amd64)
        isEmpty(VAMD64) {
             # from qt 4.7, use QMAKE_LIBDIR instead of LIBPATH
             LIBPATH += Z:/librarys/vc-ssl-x86/lib Z:/librarys/vc-zlib/static32   # depcreated
             QMAKE_LIBDIR += Z:/librarys/vc-ssl-x86/lib Z:/librarys/vc-zlib/static32
             LIBS += -lcurllib
        } else {
             LIBPATH += Z:/librarys/vc-ssl-x64/lib Z:/librarys/vc-zlib/staticx64 # depcreated
             QMAKE_LIBDIR += Z:/librarys/vc-ssl-x64/lib Z:/librarys/vc-zlib/staticx64
             QMAKE_LIBDIR += Z:/librarys/libcurl/dllx64
             LIBS += -llibcurl_imp
        }

         LIBPATH += Z:/librarys/libcurl/lib/Release
         CMAKE_LIBDIR += Z:/librarys/libcurl/lib/Release
         INCLUDEPATH += Z:/librarys/libcurl/include
         
#         LIBS += -lcurllib

    }
    RC_FILE = unitest.rc
} else: macx-g++ {
} else {
    GCC_VERSION = $$system(gcc -dumpversion)
    ## TODO gcc > 4.4.0 has -std=c++0x arguments
    QMAKE_CXXFLAGS += -std=c++0x -std=c++11 -std=gnu++11 -fno-exceptions  # -fno-rtti
    QMAKE_CXXFLAGS += -g
}

win32:LIBS += -lQtTest -lws2_32  -lgdi32 
unix:LIBS += -lcurl

target.path = $$BINDIR
INSTALLS += target
