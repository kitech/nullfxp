SOURCES += main.cpp \
 nullfxp.cpp \
 xmalloc.cpp \
 buffer.cpp \
 misc.cpp \
 bufaux.cpp \
 bufbn.cpp \
 sftp-common.cpp \
 sftp-client.cpp \
 atomicio.cpp \
 sftp-glob.cpp \
 sftp.cpp \
 glob.cpp \
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
 aboutnullfxp.cpp
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt
TARGET = ../bin/nullfxp

QT += network

DESTDIR = .

FORMS += nullfxp.ui \
 localview.ui \
 remoteview.ui \
 progressdialog.ui \
 remotehostconnectingstatusdialog.ui \
 remotehostquickconnectfinfodailog.ui \
 aboutnullfxp.ui

HEADERS += nullfxp.h \
 sftp.h \
 config.h \
 defines.h \
 includes.h \
 xmalloc.h \
 log.h \
 buffer.h \
 glob.h \
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
 atomicio.h \
 misc.h \
 aboutnullfxp.h

LIBS += -lssl

DISTFILES += ../CMakeLists.txt \
CMakeLists.txt
