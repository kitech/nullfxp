DESTDIR = .

TEMPLATE = lib

CONFIG += staticlib \
 x11 \
 windows \
 console

TARGET = ssh

SOURCES += acss.c \
atomicio.c \
authfd.c \
authfile.c \
bufaux.c \
bufbn.c \
buffer.c \
canohost.c \
channels.c \
cipher-3des1.c \
cipher-acss.c \
cipher-aes.c \
cipher-bf1.c \
cipher.c \
cipher-ctr.c \
cleanup.c \
compat.c \
compress.c \
crc32.c \
deattack.c \
dh.c \
dispatch.c \
dns.c \
entropy.c \
fatal.c \
gss-genr.c \
hostfile.c \
kex.c \
kexdh.c \
kexdhc.c \
kexgex.c \
kexgexc.c \
key.c \
log.c \
mac.c \
match.c \
md-sha256.c \
misc.c \
moduli.c \
monitor_fdpass.c \
msg.c \
nchan.c \
packet.c \
progressmeter.c \
readpass.c \
rijndael.c \
rsa.c \
scard.c \
scard-opensc.c \
ssh-dss.c \
ssh-rsa.c \
ttymodes.c \
uidswap.c \
umac.c \
uuencode.c \
xmalloc.c
HEADERS += acss.h \
atomicio.h \
audit.h \
authfd.h \
authfile.h \
auth.h \
auth-pam.h \
buffer.h \
canohost.h \
channels.h \
cipher.h \
compat.h \
compress.h \
config.h \
crc32.h \
deattack.h \
defines.h \
dh.h \
dispatch.h \
dns.h \
entropy.h \
hostfile.h \
includes.h \
kex.h \
key.h \
log.h \
mac.h \
match.h \
misc.h \
monitor_fdpass.h \
monitor.h \
msg.h \
myproposal.h \
packet.h \
pathnames.h \
platform.h \
progressmeter.h \
rijndael.h \
rsa.h \
scard.h \
ssh1.h \
ssh2.h \
ssh.h \
sshpty.h \
ttymodes.h \
uidswap.h \
umac.h \
uuencode.h \
version.h \
xmalloc.h
CONFIG -= qt


DEFINES += HAVE_CONFIG_H

QMAKE_CXXFLAGS_DEBUG += -Wsign-compare \
-Wno-pointer-sign \
-std=gnu99
QMAKE_CXXFLAGS_RELEASE += -Wsign-compare \
-Wno-pointer-sign \
-std=gnu99
INCLUDEPATH += openbsd-compat

