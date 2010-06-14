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

# exit;

PKGDIR=/tmp/nullfxp-$NXVER-0
rm -fvr /tmp/nullfxp-$NXVER-0
mkdir $PKGDIR

cp -va $NXROOT/packages/debian/DEBIAN $PKGDIR/
rm -fv $PKGDIR/DEBIAN/.svn

mkdir -pv $PKGDIR/usr/share/applications $PKGDIR/usr/bin $PKGDIR/usr/share/nullfxp
cp -v $NXROOT/bin/nullfxp $PKGDIR/usr/bin/
cp -v $NXROOT/src/data/nullfxp.desktop $PKGDIR/usr/share/applications/

cp -va $NXROOT/src/icons $PKGDIR/usr/share/nullfxp/
find $PKGDIR/ -name .svn | xargs rm -vfr

PNXSIZE=`du -s /tmp/nullfxp-$NXVER-0 | awk '{print $1}'`
# echo $PNXSIZE

sed -i "s@NXVER@$NXVER@g" $PKGDIR/DEBIAN/control
sed -i "s@NXSIZE@$PNXSIZE@g" $PKGDIR/DEBIAN/control

cd $PKGDIR/../
dpkg -b nullfxp-$NXVER-0
# tar zcvf nullfxp-$NXVER.tar.gz nullfxp-$NXVER-0

# cleanup
# rm -fvr $PKGDIR

# come back
cd -
