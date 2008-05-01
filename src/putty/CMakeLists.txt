PROJECT(putty)                                                              

SET(CMAKE_VERBOSE_MAKEFILE on)
SET(CMAKE_BUILD_TYPE "debug")
SET(CMAKE_INSTALL_PREFIX "/opt")
IF(WIN32)
  MESSAGE("build platform: windows")
ELSEIF(CYGWIN)
  MESSAGE("build platform: cygwin")
ELSEIF(MINGW)
  MESSAGE("build platform: mingw")
ELSEIF(UNIX)
  MESSAGE("build platform: unix/linux")
ELSEIF(MAC)
  MESSAGE("build platform: mac")
ELSEIF(MACOSX)
  MESSAGE("build platform: macosx")
ELSE(WIN32)
  MESSAGE("build platform: others")
ENDIF(WIN32)
###################
INCLUDE (CheckIncludeFile)
INCLUDE (CheckIncludeFiles)
INCLUDE (CheckFunctionExists)
INCLUDE (CheckLibraryExists)
INCLUDE (CheckVariableExists)
INCLUDE (CheckTypeSize)

#INCLUDE (${CMAKE_ROOT}/Modules/FindTCL.cmake)
#INCLUDE (${CMAKE_ROOT}/Modules/FindQt4.cmake)
#INCLUDE (${CMAKE_ROOT}/Modules/UseQt4.cmake)
#############
CHECK_FUNCTION_EXISTS(getaddrinfo HAVE_GETADDRINFO)
CHECK_FUNCTION_EXISTS(ptsname HAVE_PTSNAME)
CHECK_FUNCTION_EXISTS(setresuid HAVE_SETRESUID)
CHECK_FUNCTION_EXISTS(strsignal HAVE_STRSIGNAL)
CHECK_INCLUDE_FILE("sys/select.h" HAVE_SYS_SELECT_H)
CHECK_FUNCTION_EXISTS(updwtmpx HAVE_UPDWTMPX)
CHECK_INCLUDE_FILE(utmpx.h HAVE_UTMPX_H)
##########################
SET(PACKAGE_BUGREPORT "liuguangzhao@users.sourceforge.net")
SET(PACKAGE_NAME "putty")
SET(PACKAGE_STRING "putty")
SET(PACKAGE_TARNAME "gz")
SET(PACKAGE_VERSION "l")

#########################
FILE(READ "version.def" VERSION_DEF)
ADD_DEFINITIONS(-g -O2 -Wall -Werror  -DHAVE_CONFIG_H  -DEMBED_LIB  ${VERSION_DEF} )
INCLUDE_DIRECTORIES(. ./charset/ ./windows/ ./unix/ ./mac/ ./macosx/)
EXECUTE_PROCESS(COMMAND /bin/sed -e s/undef/cmakedefine/g unix/uxconfig.in  OUTPUT_FILE unix/uxconfig.in.cmake)
CONFIGURE_FILE(${CMAKE_CURRENT_DIRECTORY}./unix/uxconfig.in.cmake ${CMAKE_CURRENT_DIRECTORY}./unix/uxconfig.h)


#######################################
#ADD_EXECUTABLE(plink
#be_all_s.c cmdline.c cproxy.c ldisc.c logging.c misc.c pinger.c 
#portfwd.c proxy.c raw.c rlogin.c settings.c ssh.c sshaes.c
#ssharcf.c sshblowf.c sshbn.c sshcrc.c sshcrcda.c sshdes.c 
#sshdh.c sshdss.c sshmd5.c sshpubk.c sshrand.c sshrsa.c
#sshsh256.c sshsh512.c sshsha.c sshzlib.c telnet.c time.c
#timing.c tree234.c
#unix/ux_x11.c unix/uxagentc.c unix/uxcons.c unix/uxmisc.c 
#unix/uxnet.c unix/uxnoise.c unix/uxplink.c unix/uxproxy.c
#unix/uxsel.c unix/uxser.c unix/uxsignal.c unix/uxstore.c
#version.c wildcard.c x11fwd.c
#)

#ADD_EXECUTABLE(pscp
#be_none.c cmdline.c cproxy.c int64.c logging.c misc.c pinger.c 
#portfwd.c proxy.c pscp.c settings.c sftp.c ssh.c sshaes.c 
#ssharcf.c sshblowf.c sshbn.c sshcrc.c sshcrcda.c sshdes.c 
#sshdh.c sshdss.c sshmd5.c sshpubk.c sshrand.c sshrsa.c 
#sshsh256.c sshsh512.c sshsha.c sshzlib.c time.c timing.c 
#tree234.c unix/uxagentc.c unix/uxcons.c unix/uxmisc.c unix/uxnet.c unix/uxnoise.c 
#unix/uxproxy.c unix/uxsel.c unix/uxsftp.c unix/uxstore.c version.c wildcard.c 
#x11fwd.c
#)

# ADD_EXECUTABLE(psftp
# be_none.c cmdline.c cproxy.c int64.c logging.c misc.c pinger.c 
# portfwd.c proxy.c psftp.c settings.c sftp.c ssh.c sshaes.c 
# ssharcf.c sshblowf.c sshbn.c sshcrc.c sshcrcda.c sshdes.c 
# sshdh.c sshdss.c sshmd5.c sshpubk.c sshrand.c sshrsa.c 
# sshsh256.c sshsh512.c sshsha.c sshzlib.c time.c timing.c 
# tree234.c unix/uxagentc.c unix/uxcons.c unix/uxmisc.c unix/uxnet.c unix/uxnoise.c 
# unix/uxproxy.c unix/uxsel.c unix/uxsftp.c unix/uxstore.c version.c wildcard.c 
# x11fwd.c
# )

ADD_LIBRARY(sftp STATIC
be_none.c cmdline.c cproxy.c int64.c logging.c misc.c pinger.c 
portfwd.c proxy.c psftp.c settings.c sftp.c ssh.c sshaes.c 
ssharcf.c sshblowf.c sshbn.c sshcrc.c sshcrcda.c sshdes.c 
sshdh.c sshdss.c sshmd5.c sshpubk.c sshrand.c sshrsa.c 
sshsh256.c sshsh512.c sshsha.c sshzlib.c time.c timing.c 
tree234.c unix/uxagentc.c unix/uxcons.c unix/uxmisc.c unix/uxnet.c unix/uxnoise.c 
unix/uxproxy.c unix/uxsel.c unix/uxsftp.c unix/uxstore.c version.c wildcard.c 
x11fwd.c
)

ADD_EXECUTABLE(mysftp
mysftp.c
psftp.c
cmdline.c
)
ADD_DEPENDENCIES(mysftp sftp)
TARGET_LINK_LIBRARIES(mysftp sftp)