SOURCES += channel.c \
comp.c \
crypt.c \
hostkey.c \
kex.c \
mac.c \
misc.c \
#openssl.c \
#libgcrypt.c \
packet.c \
pem.c \
publickey.c \
scp.c \
session.c \
sftp.c \
transport.c \
userauth.c
HEADERS += libgcrypt.h \
libssh2_config.h \
libssh2_priv.h \
openssl.h

win32{
	#SOURCES += libgcrypt.c
	SOURCES += openssl.c	
} else {
	SOURCES += openssl.c
}


DESTDIR = .

TEMPLATE = lib

CONFIG += staticlib \
 console \
 debug

CONFIG -= qt \
 release




DEFINES += HAVE_CONFIG_H \
 LIBSSH2DEBUG=1 LIBSSH2_MD5=1

TARGET = ssh2


QMAKE_CXXFLAGS_DEBUG += -g

QMAKE_CXXFLAGS_RELEASE += -g

INCLUDEPATH += ../include/

