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
 localdirfilemodel.cpp \
 remotedirsortfiltermodel.cpp
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
 synchronizewindow.ui

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
 localdirfilemodel.h \
 remotedirsortfiltermodel.h


DISTFILES += ../CMakeLists.txt \
CMakeLists.txt \
 libssh2/CMakeLists.txt






DEFINES += LIBSSH2DEBUG

CONFIG -= release




win32 {
    debug {
        LIBPATH += ./libssh2/src/debug
    }
    release {
        LIBPATH += ./libssh2/src/release
    }
    LIBS += -lssh2 -lgcrypt -lgpg-error -lws2_32
}else {
    LIBS += libssh2/src/libssh2.a \
-lssl

    TARGETDEPS += libssh2/src/libssh2.a
}

INCLUDEPATH += ./libssh2/include

