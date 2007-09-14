SOURCES += main.cpp \
 nullfxp.cpp \
 sftp-common.cpp \
 sftp-client.cpp \
 sftp-glob.cpp \
 sftp.cpp \
 localview.cpp \
 remoteview.cpp \
 remotedirmodel.cpp \
 sftp-wrapper.cpp \
 progressmeter.cpp \
 progressdialog.cpp \
 transferthread.cpp \
 remotedirretrivethread.cpp \
 remotehostconnectthread.cpp \
 remotehostconnectingstatusdialog.cpp \
 remotehostquickconnectinfodialog.cpp \
 aboutnullfxp.cpp \
 globaloption.cpp \
 globaloptionsdialog.cpp
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt \
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
 globaloptionsdialog.ui

HEADERS += nullfxp.h \
 sftp.h \
 xmalloc.h \
 sftp-operation.h \
 localview.h \
 remoteview.h \
 remotedirmodel.h \
 sftp-wrapper.h \
 sys-queue.h \
 progressdialog.h \
 transferthread.h \
 remotedirretrivethread.h \
 remotehostconnectthread.h \
 remotehostconnectingstatusdialog.h \
 remotehostquickconnectinfodialog.h \
 aboutnullfxp.h \
 libssh.h \
 globaloption.h \
 globaloptionsdialog.h


DISTFILES += ../CMakeLists.txt \
CMakeLists.txt \
 plinker/CMakeLists.txt \
 plinker/openbsd-compat/CMakeLists.txt


INCLUDEPATH += plinker \
plinker/openbsd-compat
LIBS += plinker/libssh.a \
plinker/openbsd-compat/libopenbsd_compat.a \
-lssl
TARGETDEPS += plinker/libssh.a \
plinker/openbsd-compat/libopenbsd_compat.a
