#!/bin/sh
# mkdeb.sh --- 
# 
# Author: liuguangzhao
# Copyright (C) 2007-2010 liuguangzhao@users.sf.net
# URL: 
# Created: 2010-06-14 18:34:13 +0800
# Version: $Id$
# 

SHPATH=`readlink -f $0`
NXROOT=`dirname $SHPATH`
NXROOT="$NXROOT/.."
NXROOT=`readlink -f $NXROOT`
# echo $NXROOT

NXROOTBASENAME=`basename $NXROOT`

# echo $NXROOTBASENAME

TMPNXVER=`echo $NXROOTBASENAME | awk -F\- '{print $2}'`
if [ x"$TMPNXVER" = x"svn" ] ; then
#    NXVER="r666"
    NXVER=r`LC_ALL=C svn info |grep Revision|grep -v grep| awk '{print $2}'`
else
    NXVER=$TMPNXVER
fi
# echo $NXVER
# NXARCH=`arch` # some linux not arch installed default
NXARCH=`uname -m`
for ha in i686 i586 i486
do
    if [ x"$NXARCH" = x"$ha" ] ; then
	    NXARCH=i386
	    break;
    fi
done
# exit;

PKGDIR=/tmp/nullfxp-$NXVER-$NXARCH-0
rm -fr /tmp/nullfxp-$NXVER-$NXARCH-0
mkdir $PKGDIR

cp -va $NXROOT/packages/debian/DEBIAN $PKGDIR/
rm -fv $PKGDIR/DEBIAN/.svn

mkdir -pv $PKGDIR/usr/share/applications $PKGDIR/usr/bin $PKGDIR/usr/share/nullfxp/docs
mkdir -pv $PKGDIR/usr/share/nullfxp/translations

for ic in 16 22 24 32 48 64 128
do
    mkdir -pv "$PKGDIR/usr/share/icons/hicolor/${ic}x${ic}/apps/"
    cp -v $NXROOT/src/icons/nullget-1.png "$PKGDIR/usr/share/icons/hicolor/${ic}x${ic}/apps/nullfxp.png"
done

USE_QT_TYPE=
SHARED_QT=`ldd $NXROOT/bin/nullfxp | grep libQt`
if [ x"$SHARED_QT" = x"" ] ; then
    USE_QT_TYPE="static-qt4"
else
    USE_QT_TYPE="shared-qt4"
fi

cp -v $NXROOT/bin/nullfxp $PKGDIR/usr/bin/
strip -s -v $PKGDIR/usr/bin/nullfxp
cp -v $NXROOT/bin/touch.exe $PKGDIR/usr/bin/
cp -v $NXROOT/bin/unitest $PKGDIR/usr/bin/
cp -v $NXROOT/src/data/nullfxp.desktop $PKGDIR/usr/share/applications/
cp -v $NXROOT/AUTHORS $PKGDIR/usr/share/nullfxp/docs/
cp -v $NXROOT/ChnageLog $PKGDIR/usr/share/nullfxp/docs/
cp -v $NXROOT/NEWS $PKGDIR/usr/share/nullfxp/docs/
cp -v $NXROOT/INSTALL $PKGDIR/usr/share/nullfxp/docs/
cp -v $NXROOT/TODO $PKGDIR/usr/share/nullfxp/docs/
cp -v $NXROOT/README $PKGDIR/usr/share/nullfxp/docs/

cp -va $NXROOT/src/icons $PKGDIR/usr/share/nullfxp/
find $PKGDIR/ -name .svn | xargs rm -vfr


mkdir -pv $PKGDIR/usr/share/icons/hicolor/

PNXSIZE=`du -s /tmp/nullfxp-$NXVER-0 | awk '{print $1}'`
# echo $PNXSIZE

sed -i "s@NXVER@$NXVER@g" $PKGDIR/DEBIAN/control
sed -i "s@NXSIZE@$PNXSIZE@g" $PKGDIR/DEBIAN/control
sed -i "s@NXARCH@$NXARCH@g" $PKGDIR/DEBIAN/control

cd $PKGDIR/../
dpkg -b nullfxp-$NXVER-$NXARCH-0
# tar zcvf nullfxp-$NXVER.tar.gz nullfxp-$NXVER-0

mv -v nullfxp-$NXVER-$NXARCH-0.deb nullfxp-$NXVER-$USE_QT_TYPE.$NXARCH-0.deb

# cleanup
# rm -fvr $PKGDIR

# come back
cd -
