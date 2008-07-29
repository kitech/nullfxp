TEMPLATE = app
CONFIG += qt thread console warn_on ordered  
TARGET = nullfxp
DESTDIR = ../bin

system(gcc -o gv.exe get_ver.c)
!win32 {
    VERSION = $$system(./gv.exe nullfxp-version.h)
}
win32 {
	CONFIG += release
    VERSION = $$system(gv nullfxp-version.h)
} else:solaris-g++ {
        QT -= webkit
} else {
	QT += webkit
	CONFIG += debug release
}
QT += network 

UI_DIR = obj
MOC_DIR = obj
OBJECTS_DIR = obj

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
 forwardconnectinfodialog.ui \
 ui/hostlistdialog.ui \
 ui/encryptiondetaildialog.ui \
 ui/fileexistaskdialog.ui

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
 forwardconnectinfodialog.cpp \
 basestorage.cpp \
 sessiondialog.cpp \
 fileexistaskdialog.cpp \
 encryptiondetailfocuslabel.cpp \
 encryptiondetaildialog.cpp \
 rfsdirnode.cpp \
 completelineeditdelegate.cpp \
 synchronizeoptiondialog.cpp \
 synchronizewindow.cpp

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
 forwardconnectinfodialog.h \
 basestorage.h \
 sessiondialog.h \
 fileexistaskdialog.h \
 encryptiondetailfocuslabel.h \
 encryptiondetaildialog.h \
 rfsdirnode.h \
 completelineeditdelegate.h \
 synchronizeoptiondialog.h \
 synchronizewindow.h

DISTFILES += ../CMakeLists.txt \
CMakeLists.txt \
libssh2/CMakeLists.txt


win32 {
    debug {
        LIBPATH += ./libssh2/src/debug
        LIBPATH += Z:/librarys/mw-ssl/lib
    }
    release {
        LIBPATH += ./libssh2/src/release 
        LIBPATH += Z:/librarys/mw-ssl/lib
    }
    LIBS += -lssh2  -lssl -lcrypto -lws2_32  -lgdi32 
    #-lgcrypt -lgpg-error 
}else {
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

HOST_MACHINE = $$system(gcc -dumpmachine)
HOST_GCC_VERSION = $$system(gcc -dumpversion)
DEFINES += GCC_MV=\"\\\"$$HOST_MACHINE-g++-$$HOST_GCC_VERSION\\\"\"


INCLUDEPATH += . ./libssh2/include

RESOURCES = nullfxp.qrc
