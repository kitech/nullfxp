# nullfxp.pro --- 
# 
# Author: liuguangzhao
# Copyright (C) 2007-2010 liuguangzhao@users.sf.net
# URL: http://www.qtchina.net http://nullget.sourceforge.net
# Created: 2009-05-18 22:02:53 +0800
# Version: $Id$
# 

# simple check qt version
QMAKEVERSION = $$[QMAKE_VERSION]
ISQT4 = $$find(QMAKEVERSION, ^[2-9])

isEmpty( ISQT4 ) {
    error("Use the qmake include with Qt4.4 or greater, on Debian that is qmake-qt4");
}

SUBDIRS +=  \
# src/libssh2 \       # compile uncomment, run comment on android
 src #\
 #src/unitest

TEMPLATE = subdirs 
CONFIG = ordered
DESTDIR = .

OTHER_FILES += \
    android/res/drawable-hdpi/icon.png \
    android/res/drawable-ldpi/icon.png \
    android/res/drawable-mdpi/icon.png \
    android/res/values/libs.xml \
    android/res/values/strings.xml \
    android/src/eu/licentia/necessitas/ministro/IMinistro.aidl \
    android/src/eu/licentia/necessitas/ministro/IMinistroCallback.aidl \
    android/src/eu/licentia/necessitas/mobile/QtAndroidContacts.java \
    android/src/eu/licentia/necessitas/mobile/QtCamera.java \
    android/src/eu/licentia/necessitas/mobile/QtFeedback.java \
    android/src/eu/licentia/necessitas/mobile/QtLocation.java \
    android/src/eu/licentia/necessitas/mobile/QtMediaPlayer.java \
    android/src/eu/licentia/necessitas/mobile/QtSensors.java \
    android/src/eu/licentia/necessitas/mobile/QtSystemInfo.java \
    android/src/eu/licentia/necessitas/industrius/QtSurface.java \
    android/src/eu/licentia/necessitas/industrius/QtActivity.java \
    android/src/eu/licentia/necessitas/industrius/QtApplication.java \
    android/src/eu/licentia/necessitas/industrius/QtLayout.java \
    android/AndroidManifest.xml
