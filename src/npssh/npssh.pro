TEMPLATE = app
CONFIG += thread console warn_on ordered
QT -= core gui
TARGET = npssh
DESTDIR = ./ # ../../bin

VERSION=2.1.94  # using in nullfxp-version.h

UI_DIR = obj
MOC_DIR = obj
OBJECTS_DIR = obj

INCLUDEPATH += . ../libssh2/include ../libssh/include
QMAKE_LIBDIR += ../libssh2/src/release ../libssh/libssh/release
LIBS += ../libssh2/src/libssh2.a ../libssh/libssh/libkssh.a -lssl -lcrypto -lz -lrt
POST_TARGETDEPS += ../libssh2/src/libssh2.a ../libssh/libssh/libkssh.a

QMAKE_CXXFLAGS += -g -std=c++11
SOURCES = npssh.cpp hostlist.cpp


        