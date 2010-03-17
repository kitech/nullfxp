# src.pro --- 
# 
# Author: liuguangzhao
# Copyright (C) 2007-2010 liuguangzhao@users.sf.net
# URL: http://www.qtchina.net http://nullget.sourceforge.net
# Created: 2008-05-18 22:03:19 +0800
# Version: $Id$
# 

TEMPLATE = app
CONFIG += qt thread console warn_on ordered  
TARGET = nullfxp
DESTDIR = ../bin

VERSION=2.0.1.79  # using in nullfxp-version.h

include(../install.pri)

win32 {
	CONFIG += release
    !win32-g++ {
         CONFIG -= embed_manifest_exe
         CONFIG -= embed_manifest_dll
    }
} else:solaris-g++ {
    QT -= webkit
} else {
#	QT += webkit
	CONFIG += debug
}
QT += network

UI_DIR = obj
MOC_DIR = obj
OBJECTS_DIR = obj

FORMS += nullfxp.ui \
 localview.ui \
 remoteview.ui \
 progressdialog.ui \
 connectingstatusdialog.ui \
 quickconnectinfodialog.ui \
 aboutnullfxp.ui \
 globaloptionsdialog.ui \
 fileproperties.ui \
 synchronizeoptiondialog.ui \
 synchronizewindow.ui \
 forwardconnectdaemon.ui \
 forwarddebugwindow.ui \
 forwardconnectinfodialog.ui \
 ui/hostlistdialog.ui \
 ui/encryptiondetaildialog.ui \
 ui/fileexistaskdialog.ui \
 ui/updatedialog.ui \
 ui/systeminfodialog.ui

SOURCES += main.cpp \
 nullfxp.cpp \
 localview.cpp \
 remoteview.cpp \
 remotedirmodel.cpp \
 sftpdirmodel.cpp \
 progressdialog.cpp \
 transportor.cpp \
# remotehostconnectthread.cpp \
 connectingstatusdialog.cpp \
 quickconnectinfodialog.cpp \
 aboutnullfxp.cpp \
 globaloption.cpp \
 globaloptionsdialog.cpp \
 utils.cpp \
 fileproperties.cpp \
 remotedirsortfiltermodel.cpp \
 remoteviewdirtableview.cpp \
 remotedirtreeview.cpp \
 localdirsortfiltermodel.cpp \
 localdirorginalmodel.cpp \
 nullfxpext.cpp \
 forwardconnectdaemon.cpp \
 forwarddebugwindow.cpp \
 forwardconnectinfodialog.cpp \
 basestorage.cpp \
 sessiondialog.cpp \
 fileexistaskdialog.cpp \
 encryptiondetailfocuslabel.cpp \
 encryptiondetaildialog.cpp \
 rfsdirnode.cpp \
 completelineeditdelegate.cpp \
 synchronizeoptiondialog.cpp \
 synchronizewindow.cpp \
 syncdiffermodel.cpp \
 sshfileinfo.cpp \
 synctransferthread.cpp \
 taskpackage.cpp \
 sftpfile.cpp \
 connection.cpp \
 mimetypeshash.cpp \
 sshconnection.cpp \
 ftpconnection.cpp \
 connector.cpp \
 ftpview.cpp \
 sftpview.cpp \
 dirretriver.cpp \
 ftpdirretriver.cpp \
 sshdirretriver.cpp \
 libftp/libftp.cpp \
 updatedialog.cpp \
 systeminfodialog.cpp

HEADERS += nullfxp.h \
 localview.h \
 remoteview.h \
 remotedirmodel.h \
 sftpdirmodel.h \
 progressdialog.h \
 transportor.h \
# remotehostconnectthread.h \
 connectingstatusdialog.h \
 quickconnectinfodialog.h \
 aboutnullfxp.h \
 globaloption.h \
 globaloptionsdialog.h \
 fileproperties.h \
 remotedirsortfiltermodel.h \
 remoteviewdirtableview.h \
 remotedirtreeview.h \
 localdirsortfiltermodel.h \
 localdirorginalmodel.h \
 forwardconnectdaemon.h \
 forwarddebugwindow.h \
 forwardconnectinfodialog.h \
 basestorage.h \
 sessiondialog.h \
 fileexistaskdialog.h \
 encryptiondetailfocuslabel.h \
 encryptiondetaildialog.h \
 rfsdirnode.h \
 completelineeditdelegate.h \
 synchronizeoptiondialog.h \
 synchronizewindow.h \
 syncdiffermodel.h \
 synctransferthread.h \
 sftpfile.h \
 sshconnection.h \
 connection.h \
 ftpconnection.h \
 connector.h \
 ftpview.h \
 sftpview.h \
 dirretriver.h \
 ftpdirretriver.h \
 sshdirretriver.h \
 libftp/libftp.h \
 updatedialog.h \
 systeminfodialog.h

DISTFILES += ../CMakeLists.txt \
          CMakeLists.txt \
          libssh2/CMakeLists.txt

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
		LIBS += -lqtmain -lzlib -llibeay32 -lssleay32 -ladvapi32 -luser32 -lwsock32
    }
    LIBS += -lssh2 -lws2_32  -lgdi32 
    #-lgcrypt -lgpg-error 
} else {
    LIBS += libssh2/src/libssh2.a -lssl -lcrypto
    TARGETDEPS += libssh2/src/libssh2.a
}

CONFIG(release, debug|release) {
    DEFINES += NDEBUG QT_NO_DEBUG_OUTPUT
}

CONFIG(debug, debug|release) {
    DEFINES += DEBUG 
}
DEFINES -= NDEBUG QT_NO_DEBUG_OUTPUT

win32-g++ {     
} else:win32 {
     DEFINES += LIBSSH2_WIN32 _CRT_SECURE_NO_DEPRECATE GCC_MV=\"\\\"MSVC-2005-Express-Edition\\\"\"
     RC_FILE = nullfxp.rc 
} else {
#  QMAKE_CXXFLAGS += -std=c++0x
}

!win32 {
    HOST_MACHINE = $$system(gcc -dumpmachine)
    HOST_GCC_VERSION = $$system(gcc -dumpversion)
    DEFINES += GCC_MV=\"\\\"$$HOST_MACHINE-g++-$$HOST_GCC_VERSION\\\"\"
}
DEFINES += NULLFXP_VERSION_STR=\"\\\"$$VERSION\\\"\" 
DEFINES += NXDATADIR=\"\\\"$$DATADIR\\\"\"
DEFINES += NXPKGDATADIR=\"\\\"$$PKGDATADIR\\\"\"

INCLUDEPATH += . ./libssh2/include

RESOURCES = nullfxp.qrc

# install settings
# DISTFILES += ./bin/nullfxp ./bin/unitest
document.path = $$DATADIR/docs
document.files = ../INSTALL ../README ../AUTHORS ../ChangeLog

icons.path = $$DATADIR/icons
icons.files = ./icons/* 

mimes.path = $$DATADIR/icons/mimetypes
mimes.files = ./icons/mimetypes/*

target.path += $$BINDIR
INSTALLS += target document icons mimes
