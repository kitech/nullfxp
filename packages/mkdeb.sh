#!/bin/sh
#
#Author: liuguangzhao@users.sf.net

PKGDIR=/tmp/nullfxp-2.0.1-0
rm -fvr /tmp/nullfxp-2.0.1-0
mkdir $PKGDIR

cp -va debian/DEBIAN $PKGDIR/
rm -fv $PKGDIR/DEBIAN/.svn

mkdir -pv $PKGDIR/usr/share/applications $PKGDIR/usr/bin $PKGDIR/usr/share/nullfxp
cp -v ../bin/nullfxp $PKGDIR/usr/bin/

cp -va ../src/icons $PKGDIR/usr/share/nullfxp/
find $PKGDIR/usr/share/nullfxp/ -name .svn | xargs rm -vfr

cd $PKGDIR/../
dpkg -b nullfxp-2.0.1-0

# cleanup
rm -fvr $PKGDIR

# come back
cd -
