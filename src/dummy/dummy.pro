DESTDIR = .

TEMPLATE = app

CONFIG -= qt

SOURCES += ../plinker/readconf.c \
../plinker/ssh.c \
../plinker/sshconnect1.c \
../plinker/sshconnect2.c \
../plinker/sshconnect.c \
../plinker/sshtty.c \
../plinker/clientloop.c
CONFIG += console

TARGET = ../../bin/plinker


DEFINES += HAVE_CONFIG_H

QMAKE_CXXFLAGS_DEBUG += -Wsign-compare \
-Wno-pointer-sign \
-std=gnu99
QMAKE_CXXFLAGS_RELEASE += -Wsign-compare \
-Wno-pointer-sign \
-std=gnu99
INCLUDEPATH += ../plinker

LIBS += ../plinker/libssh.a \
../plinker/openbsd-compat/libopenbsd_compat.a \
-lresolv \
-lcrypt \
-lcrypto \
-lssl \
-lz
TARGETDEPS += ../plinker/openbsd-compat/libopenbsd_compat.a \
../plinker/libssh.a
