###################
INCLUDE (CheckIncludeFiles)
INCLUDE (CheckFunctionExists)
INCLUDE (CheckLibraryExists)
INCLUDE (CheckVariableExists)
INCLUDE (CheckTypeSize)


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


SET ( QT_USE_QT3SUPPORT true)
SET (QT_USE_QTASSISTANT true)
SET( QT_USE_QTDESIGNER true)
SET ( QT_USE_QTMOTIF true )
SET ( QT_USE_QTMAIN true)
SET ( QT_USE_QTNETWORK on)
SET ( QT_USE_QTNSPLUGIN true )
SET ( QT_USE_QTOPENGL true)
SET ( QT_USE_QTSQL true)
SET ( QT_USE_QTXML true)

INCLUDE_DIRECTORIES( . 

    ${QT_INCLUDES}
    
    ${CMAKE_CURRENT_BINARY_DIR}
)

SET(qtproject_UIS
    aboutnullfxp.ui  nullfxp.ui         remotehostconnectingstatusdialog.ui   remoteview.ui
localview.ui     progressdialog.ui  remotehostquickconnectfinfodailog.ui

)


QT4_WRAP_UI(qtproject_UIS_H ${qtproject_UIS})

SET( qtproject_SRCS
aboutnullfxp.cpp            remotehostconnectingstatusdialog.cpp
localview.cpp               remotehostconnectthread.cpp
nullfxp.cpp                 remotehostquickconnectinfodialog.cpp
progressdialog.cpp          remoteview.cpp
remotedirmodel.cpp          transferthread.cpp
remotedirretrivethread.cpp
)

SET ( qtproject_moc_HDRS 
aboutnullfxp.h            remotehostconnectingstatusdialog.h
localview.h               remotehostconnectthread.h
nullfxp.h                 remotehostquickconnectinfodialog.h
progressdialog.h          remoteview.h
remotedirmodel.h          transferthread.h
remotedirretrivethread.h
)
QT4_WRAP_CPP(qtproject_moc_SRCS ${qtproject_moc_HDRS})


###############
ADD_EXECUTABLE(../bin/nullfxp 

aboutnullfxp.cpp                          
atomicio.cpp                              
bufaux.cpp                                nullfxp.cpp
bufbn.cpp                                 progressdialog.cpp
buffer.cpp                                progressmeter.cpp
glob.cpp                                  remotedirmodel.cpp
localview.cpp                             remotedirretrivethread.cpp
main.cpp                                  remotehostconnectingstatusdialog.cpp
misc.cpp                                  remotehostconnectthread.cpp
remotehostquickconnectinfodialog.cpp
remoteview.cpp
sftp-client.cpp
sftp-common.cpp
sftp.cpp
sftp-glob.cpp
sftp-wrapper.cpp
transferthread.cpp
xmalloc.cpp

${qtproject_UIS_H}  ${qtproject_moc_SRCS}

)

TARGET_LINK_LIBRARIES(../bin/nullfxp ${QT_LIBRARIES} ssl )

##http://qtnode.net/wiki?title=Qt_with_cmake&printable=yes
