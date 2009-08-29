### CMakeLists.txt --- 
## 
## Author: liuguangzhao
## Copyright (C) 2007-2010 liuguangzhao@users.sf.net
## URL: http://www.qtchina.net http://nullget.sourceforge.net
## Created: 2008-08-29 16:50:11 +0800
## Version: $Id$
######################################################################

PROJECT(nullfxp)

SET(CMAKE_VERBOSE_MAKEFILE on)

#LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src/plinker/openbsd-compat/) #没有用？
#SUBDIRS(src)
ADD_SUBDIRECTORY(src)

