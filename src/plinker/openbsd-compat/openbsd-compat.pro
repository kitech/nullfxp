TEMPLATE = lib

CONFIG += staticlib \
 console

DESTDIR = .

SOURCES += base64.c \
basename.c \
bindresvport.c \
bsd-arc4random.c \
bsd-asprintf.c \
bsd-closefrom.c \
bsd-cray.c \
bsd-cygwin_util.c \
bsd-getpeereid.c \
bsd-misc.c \
bsd-nextstep.c \
bsd-openpty.c \
bsd-poll.c \
bsd-snprintf.c \
bsd-waitpid.c \
daemon.c \
dirname.c \
fake-rfc2553.c \
getcwd.c \
getgrouplist.c \
getopt.c \
getrrsetbyname.c \
glob.c \
inet_aton.c \
inet_ntoa.c \
inet_ntop.c \
mktemp.c \
openssl-compat.c \
port-aix.c \
port-irix.c \
port-linux.c \
port-solaris.c \
port-tun.c \
port-uw.c \
readpassphrase.c \
realpath.c \
rresvport.c \
setenv.c \
setproctitle.c \
sha2.c \
sigact.c \
strlcat.c \
strlcpy.c \
strmode.c \
strsep.c \
strtoll.c \
strtonum.c \
strtoul.c \
vis.c \
xcrypt.c \
xmmap.c
HEADERS += base64.h \
bsd-cray.h \
bsd-cygwin_util.h \
bsd-misc.h \
bsd-nextstep.h \
bsd-poll.h \
bsd-waitpid.h \
fake-rfc2553.h \
getrrsetbyname.h \
glob.h \
openbsd-compat.h \
openssl-compat.h \
port-aix.h \
port-irix.h \
port-linux.h \
port-solaris.h \
port-tun.h \
port-uw.h \
readpassphrase.h \
sha2.h \
sigact.h \
sys-queue.h \
sys-tree.h \
vis.h
TARGET = openbsd_compat

QMAKE_CXXFLAGS_DEBUG += -std=gnu99 \
 -Wsign-compare \
 -Wno-pointer-sign
CONFIG -= qt

DEFINES += HAVE_CONFIG_H

QMAKE_CXXFLAGS_RELEASE += -std=gnu99 \
 -Wsign-compare \
 -Wno-pointer-sign


QMAKE_CFLAGS = "-Wsign-compare -Wno-pointer-sign -std=gnu99"
INCLUDEPATH += ..

