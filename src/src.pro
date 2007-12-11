SOURCES += main.cpp \
 nullfxp.cpp \
 localview.cpp \
 remoteview.cpp \
 remotedirmodel.cpp \
 progressdialog.cpp \
 transferthread.cpp \
 remotedirretrivethread.cpp \
 remotehostconnectthread.cpp \
 remotehostconnectingstatusdialog.cpp \
 remotehostquickconnectinfodialog.cpp \
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
 forwardconnectinfodialog.cpp
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt \
 debug \
 console
TARGET = ../bin/nullfxp

QT += network

DESTDIR = .

FORMS += nullfxp.ui \
 localview.ui \
 remoteview.ui \
 progressdialog.ui \
 remotehostconnectingstatusdialog.ui \
 remotehostquickconnectfinfodailog.ui \
 aboutnullfxp.ui \
 globaloptionsdialog.ui \
 fileproperties.ui \
 synchronizeoptiondialog.ui \
 synchronizewindow.ui \
 forwardconnectdaemon.ui \
 forwarddebugwindow.ui \
 forwardconnectinfodialog.ui

HEADERS += nullfxp.h \
 localview.h \
 remoteview.h \
 remotedirmodel.h \
 progressdialog.h \
 transferthread.h \
 remotedirretrivethread.h \
 remotehostconnectthread.h \
 remotehostconnectingstatusdialog.h \
 remotehostquickconnectinfodialog.h \
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
 forwardconnectinfodialog.h


DISTFILES += ../CMakeLists.txt \
CMakeLists.txt \
 libssh2/CMakeLists.txt






DEFINES += LIBSSH2DEBUG

CONFIG -= release




win32 {
    debug {
        LIBPATH += ./libssh2/src/debug
        LIBPATH += E:/librarys/mw-ssl/lib
    }
    release {
        LIBPATH += ./libssh2/src/release 
        LIBPATH += E:/librarys/mw-ssl/lib
    }
    LIBS += -lssh2  -lssl -lcrypto -lws2_32  -lgdi32 
    #-lgcrypt -lgpg-error 
}else {
    LIBS += libssh2/src/libssh2.a \
-lssl

    TARGETDEPS += libssh2/src/libssh2.a
}

INCLUDEPATH += ./libssh2/include

RESOURCES = nullfxp.qrc
