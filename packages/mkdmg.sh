#!/bin/sh

VER=
VER=`pwd|awk -F"/" '{print $4}'`
echo "Get version: $VER "
# run in nullfxp-xxx root director
macdeployqt bin/nullfxp.app -verbose=3 -dmg
cp -v bin/nullfxp.dmg ${VER}_MacOSX_Intel.dmg