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
 utils.cpp
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt \
 console \
 debug
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
 globaloptionsdialog.ui

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
 globaloptionsdialog.h


DISTFILES += ../CMakeLists.txt \
CMakeLists.txt \
 plinker/CMakeLists.txt \
 plinker/openbsd-compat/CMakeLists.txt \
 libssh2/CMakeLists.txt






DEFINES += LIBSSH2DEBUG

CONFIG -= release

INCLUDEPATH += ./libssh2/include

LIBS += libssh2/src/libssh2.a \
-lssl
TARGETDEPS += libssh2/src/libssh2.a

