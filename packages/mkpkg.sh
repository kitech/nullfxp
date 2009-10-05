#!/bin/bash


appname=`basename $0 | sed s,\.sh$,,`
dirname=`dirname $0`
firstchar=`dirname $0|cut -c 1`
if [ "${firstchar}" != "/" ]; then
   dirname=$PWD/$dirname
fi

#LD_LIBRARY_PATH=$dirname:$dirname/lib
#export LD_LIBRARY_PATH
#$dirname/lib/$appname $*

mkdir -p $dirname/../nullfxp
mkdir -p $dirname/../nullfxp/bin
mkdir -p $dirname/../nullfxp/lib
mkdir -p $dirname/../nullfxp/share
mkdir -p $dirname/../nullfxp/doc
mkdir -p $dirname/../nullfxp/man
mkdir -p $dirname/../nullfxp/plugins

rm -v $dirname/../nullfxp/lib/*
cp -v $dirname/nullfxp $dirname/../nullfxp/bin/
chmod +x $dirname/../nullfxp/bin/nullfxp

cp -v $dirname/../bin/nullfxp $dirname/../nullfxp/lib/
cp -v $dirname/../bin/unitest $dirname/../nullfxp/bin/
cp -pv $dirname/../bin/plink   $dirname/../nullfxp/lib/
cp -pv $dirname/../bin/plink   $dirname/../nullfxp/bin/
cp -pv $dirname/../bin/touch.exe $dirname/../nullfxp/bin/

strip -s -v $dirname/../nullfxp/lib/nullfxp
strip -s -v $dirname/../nullfxp/bin/unitest
strip -s -v $dirname/../nullfxp/lib/plink

cp -Rv $dirname/../src/icons $dirname/../nullfxp/share/
rm -vfr $dirname/../nullfxp/share/icons/.svn
rm -vfr $dirname/../nullfxp/share/icons/mimetypes/.svn
rm -vfr $dirname/../nullfxp/share/icons/.directory
rm -vfr $dirname/../nullfxp/share/icons/mimetypes/.directory
ln -sv ../share/icons $dirname/../nullfxp/lib/

# VER_FILE=$dirname/../src/nullfxp-version.h
VER_FILE=$dirname/../src/src.pro
echo $VER_FILE
# VERSION=`cat $VER_FILE|grep NullFXP|awk '{print $4}'`
VERSION=`cat src/src.pro |grep VERSION=|awk '{print $1}'|awk -F= '{print $2}'`
echo $VERSION

USED_OPENSSL_SSL=`ldd $dirname/../nullfxp/lib/nullfxp|grep libssl|awk '{print $3}'` 
USED_OPENSSL_CRYPTO=`ldd $dirname/../nullfxp/lib/nullfxp|grep libcrypto|awk '{print $3}'` 
USED_GSSAPI=`ldd $dirname/../nullfxp/lib/nullfxp|grep libgssapi|awk '{print $3}'` 
USED_KRB5SO=`ldd $dirname/../nullfxp/lib/nullfxp|grep libkrb5.so|awk '{print $3}'` 
USED_K5CRYPTO=`ldd $dirname/../nullfxp/lib/nullfxp|grep libk5crypto|awk '{print $3}'` 
USED_KRB5SUPPORT=`ldd $dirname/../nullfxp/lib/nullfxp|grep libkrb5support|awk '{print $3}'` 
USED_EXPAT=`ldd $dirname/../nullfxp/lib/nullfxp|grep libexpat|awk '{print $3}'` 
USED_KEYUTILS=`ldd $dirname/../nullfxp/lib/nullfxp|grep libkeyutils|awk '{print $3}'`
USED_SELINUX=`ldd $dirname/../nullfxp/lib/nullfxp|grep libselinux|awk '{print $3}'`

echo $USED_OPENSSL_SSL $USED_OPENSSL_CRYPTO
cp -v $USED_OPENSSL_SSL $dirname/../nullfxp/lib/
cp -v $USED_OPENSSL_CRYPTO $dirname/../nullfxp/lib/

cp -v $USED_GSSAPI $dirname/../nullfxp/lib/
cp -v $USED_KRB5SO $dirname/../nullfxp/lib/
cp -v $USED_K5CRYPTO $dirname/../nullfxp/lib/
cp -v $USED_KRB5SUPPORT $dirname/../nullfxp/lib/

cp -v $USED_EXPAT $dirname/../nullfxp/lib/
cp -v $USED_KEYUTILS $dirname/../nullfxp/lib/
cp -v $USED_SELINUX $dirname/../nullfxp/lib/


USED_QT=`ldd $dirname/../nullfxp/lib/nullfxp|grep libQt|awk '{print $3}'`
echo $USED_QT
if [ x"$USED_QT" = x"" ]; then
    LINK_TYPE=static
else
    LINK_TYPE=shared
    for lib_name in $USED_QT 
    do
	#cp -v $lib_name $dirname/../nullfxp/lib/
	echo $lib_name
    done
fi

#echo $USE_QTCORE $LINK_TYPE
#LINK_TYPE=static
LINK_PLATFORM=`uname`

TAR_CMD=tar
if [ x"${LINK_PLATFORM}" = x"SunOS" ]; then
    TAR_CMD=gtar
fi
if [ x"${LINK_PLATFORM}" = x"FreeBSD" ] ; then
	FBSDVER=`uname -r`
	FBSDVER=`echo $FBSDVER|cut -c 1`
	LINK_PLATFORM="$LINK_PLATFORM""$FBSDVER"    
	echo $LINK_PLATFORM
fi

MARCH=`uname -m`
echo "package info: $LINK_PLATFORM $LINK_TYPE $VERSION"
$TAR_CMD  jcvf $dirname/../nullfxp-$VERSION-$LINK_TYPE-qt4.$MARCH.$LINK_PLATFORM.tar.bz2 $dirname/../nullfxp
