SUBDIRS +=  \
 src/libssh2 \
 src \
 src/unitest

TEMPLATE = subdirs 
CONFIG += warn_on \
          qt \
          thread  \
 ordered \
 console
DESTDIR = .

