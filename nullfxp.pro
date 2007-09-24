SUBDIRS +=  \
 src/libssh2 \
  src

TEMPLATE = subdirs 
CONFIG += warn_on \
          qt \
          thread  \
 ordered \
 console
DESTDIR = .

