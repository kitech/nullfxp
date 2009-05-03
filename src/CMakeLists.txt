### CMakeLists.txt --- 
## 
## Filename: CMakeLists.txt
## Description: 
## Author: liuguangzhao
## Maintainer: 
## Copyright (C) 2007-2010 liuguangzhao <liuguangzhao@users.sf.net>
## http://www.qtchina.net
## http://nullget.sourceforge.net
## Created: 一  5月  5 21:50:26 2008 (CST)
## Version: 
## Last-Updated: 
##           By: 
##     Update #: 0
## URL: 
## Keywords: 
## Compatibility: 
## 
######################################################################
## 
### Commentary: 
## 
## 
## 
######################################################################
## 
### Change log:
## 
## 
######################################################################

ADD_DEFINITIONS(-g -DHAVE_CONFIG_H -O2 -Wall -Wpointer-arith -Wuninitialized -Wsign-compare     )

#SUBDIRS(plinker)
SUBDIRS(libssh2 unitest)

###################
INCLUDE (CheckIncludeFile)
INCLUDE (CheckIncludeFiles)
INCLUDE (CheckFunctionExists)
INCLUDE (CheckLibraryExists)
INCLUDE (CheckVariableExists)
INCLUDE (CheckTypeSize)
INCLUDE (CheckSymbolExists)

INCLUDE (${CMAKE_ROOT}/Modules/FindTCL.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/FindQt4.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/UseQt4.cmake)

#############

#其他没有处理到的部分
CHECK_TYPE_SIZE( "char" SIZEOF_CHAR )
CHECK_TYPE_SIZE( "int" SIZEOF_INT )
CHECK_TYPE_SIZE( "long int" SIZEOF_LONG_INT )
CHECK_TYPE_SIZE( "long long int" SIZEOF_LONG_LONG_INT )
CHECK_TYPE_SIZE( "short int" SIZEOF_SHORT_INT )

include(${QT_USE_FILE})


SET (QT_USE_QT3SUPPORT true)
SET (QT_USE_QTASSISTANT true)
SET( QT_USE_QTDESIGNER true)
SET ( QT_USE_QTMOTIF true )
SET ( QT_USE_QTMAIN true)
SET ( QT_USE_QTNETWORK true)
SET ( QT_USE_QTNSPLUGIN true )
SET ( QT_USE_QTOPENGL true)
SET ( QT_USE_QTSQL true)
SET ( QT_USE_QTXML true)

INCLUDE_DIRECTORIES( . 

    ${QT_INCLUDES}
    
    ${CMAKE_CURRENT_BINARY_DIR}
    
    ${CMAKE_CURRENT_SOURCE_DIR}/libssh2/include
)

SET(qtproject_QRCS nullfxp.qrc )
QT4_ADD_RESOURCES(qtproject_QRC_SRCS ${qtproject_QRCS})

SET(qtproject_UIS
  aboutnullfxp.ui  
  fileproperties.ui
  nullfxp.ui         
  remotehostconnectingstatusdialog.ui   
  remoteview.ui
  localview.ui     
  progressdialog.ui  
  remotehostquickconnectfinfodailog.ui
  globaloptionsdialog.ui
  forwardconnectdaemon.ui
  forwarddebugwindow.ui
  forwardconnectinfodialog.ui
  ui/hostlistdialog.ui
  ui/fileexistaskdialog.ui
  ui/encryptiondetaildialog.ui
# 
)
QT4_WRAP_UI(qtproject_UIS_H ${qtproject_UIS})

# 
SET( qtproject_SRCS
aboutnullfxp.cpp     
fileproperties.cpp       
remotehostconnectingstatusdialog.cpp
localview.cpp
localdirsortfiltermodel.cpp
remotehostconnectthread.cpp
nullfxp.cpp                 
remotehostquickconnectinfodialog.cpp
progressdialog.cpp          
remoteview.cpp
remoteviewdirtableview.cpp
remotedirmodel.cpp
remotedirsortfiltermodel.cpp
localdirorginalmodel.cpp
transferthread.cpp
remotedirretrivethread.cpp
globaloptionsdialog.cpp
forwardconnectdaemon.cpp
fileexistaskdialog.cpp
encryptiondetaildialog.cpp
encryptiondetailfocuslabel.cpp
 )

 SET ( qtproject_moc_HDRS 
aboutnullfxp.h    
fileproperties.h        
remotehostconnectingstatusdialog.h
localview.h
localdirsortfiltermodel.h
remotehostconnectthread.h
nullfxp.h                 
remotehostquickconnectinfodialog.h
progressdialog.h          
remoteview.h
remoteviewdirtableview.h
remotedirmodel.h
remotedirsortfiltermodel.h
localdirorginalmodel.h
transferthread.h
remotedirretrivethread.h
globaloptionsdialog.h
forwardconnectdaemon.h
forwardconnectinfodialog.h
forwarddebugwindow.h
sessiondialog.h
fileexistaskdialog.h
encryptiondetaildialog.h
encryptiondetailfocuslabel.h
 )
QT4_WRAP_CPP(qtproject_moc_SRCS ${qtproject_moc_HDRS})


###############

 IF(WIN32)
		INCLUDE_DIRECTORIES(Z:/librarys/mw-ssl/include)
		LINK_DIRECTORIES(Z:/librarys/mw-ssl/lib Z:/cross/mingw/lib)
		LINK_LIBRARIES( crypto gdi32 ws2_32 ssl ssh2 )
		#MESSAGE("\twin32...")
 ELSE(WIN32)
    #MESSAGE("\tunix...")
    LINK_LIBRARIES( pthread )
 ENDIF(WIN32)
 
ADD_EXECUTABLE(../bin/nullfxp 
#     tmpmain.cpp 
${qtproject_QRC_SRCS}
    main.cpp
####################
    utils.cpp
########

    remotehostconnectthread.cpp
    remotedirretrivethread.cpp
    remotedirmodel.cpp
    remotedirsortfiltermodel.cpp
    localdirorginalmodel.cpp
    localdirsortfiltermodel.cpp
    transferthread.cpp
    remoteview.cpp
    remoteviewdirtableview.cpp
    localview.cpp
    remotehostquickconnectinfodialog.cpp
    remotehostconnectingstatusdialog.cpp
    progressdialog.cpp
    aboutnullfxp.cpp
    nullfxp.cpp
    nullfxpext.cpp
##########################
    globaloption.cpp
    globaloptionsdialog.cpp
    fileproperties.cpp
    forwardconnectdaemon.cpp
    forwardconnectinfodialog.cpp
    forwarddebugwindow.cpp
    sessiondialog.cpp
    basestorage.cpp
    fileexistaskdialog.cpp
    encryptiondetailfocuslabel.cpp
    encryptiondetaildialog.cpp
############################
    ${qtproject_UIS_H}  ${qtproject_moc_SRCS}
)



TARGET_LINK_LIBRARIES(../bin/nullfxp ${QT_LIBRARIES}  ssl  ssh2  )

ADD_DEPENDENCIES(../bin/nullfxp libssh2)

##http://qtnode.net/wiki?title=Qt_with_cmake&printable=yes
