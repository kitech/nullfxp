SUBDIRS +=  \
 src/plinker/openbsd-compat \
 src/plinker \
 src/dummy  \
 src 
 
TEMPLATE = subdirs 
CONFIG += warn_on \
          qt \
          thread  \
 ordered \
 console
DESTDIR = .

