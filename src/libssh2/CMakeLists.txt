### CMakeLists.txt --- 
## 
## Author: liuguangzhao
## Copyright (C) 2007-2010 liuguangzhao@users.sf.net
## URL: http://www.qtchina.net http://nullget.sourceforge.net
## Created: 2008-08-29 16:51:05 +0800
## Version: $Id$
######################################################################

#PROJECT(libssh2)

CHECK_FUNCTION_EXISTS(getb67 CRAY_STACKSEG_END)
IF(CRAY_STACKSEG_END)
SET(C_ALLOCA "1")
ENDIF(CRAY_STACKSEG_END)
CHECK_FUNCTION_EXISTS(alloca HAVE_ALLOCA)
CHECK_SYMBOL_EXISTS(alloca alloca.h HAVE_ALLOCA)
CHECK_INCLUDE_FILE(alloca.h HAVE_ALLOCA_H)
CHECK_INCLUDE_FILE("arpa/inet.h" HAVE_ARPA_INET_H)
SET(HAVE_DISABLED_NONBLOCKING "")
CHECK_INCLUDE_FILE(dlfcn.h HAVE_DLFCN_H)
CHECK_INCLUDE_FILE(errno.h HAVE_ERRNO_H)
CHECK_INCLUDE_FILE(fcntl.h HAVE_FCNTL_H)
SET(HAVE_FIONBIO "")
CHECK_FUNCTION_EXISTS(gettimeofday HAVE_GETTIMEOFDAY)
CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)
SET(HAVE_IOCTLSOCKET_CASE "")
SET(HAVE_LIBGCRYPT "")
CHECK_INCLUDE_FILE(memory.h HAVE_MEMORY_H)
CHECK_INCLUDE_FILE(netinet/in.h HAVE_NETINET_IN_H)
CHECK_SYMBOL_EXISTS(O_NONBLOCK fcntl.h HAVE_O_NONBLOCK)
CHECK_FUNCTION_EXISTS(select HAVE_SELECT)
CHECK_SYMBOL_EXISTS(SO_NONBLOCK fcntl.h HAVE_SO_NONBLOCK)
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILE(stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILE(stdio.h HAVE_STDIO_H)
CHECK_INCLUDE_FILE(strings.h HAVE_STRINGS_H)
CHECK_INCLUDE_FILE(sys/ioctl.h HAVE_SYS_IOCTL_H)
CHECK_INCLUDE_FILE(sys/select.h HAVE_SYS_SELECT_H)
CHECK_INCLUDE_FILE(sys/socket.h HAVE_SYS_SOCKET_H)
CHECK_INCLUDE_FILE(sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE(sys/time.h HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE(sys/uio.h HAVE_SYS_UIO_H)
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
SET(LIBSSH2_CRYPT_NONE "")
SET(LIBSSH2_DH_GEX_NEW "1")
SET(LIBSSH2_LIBGCRYPT "")
SET(LIBSSH2_MAC_NONE "")
CHECK_INCLUDE_FILE(stdio.h STDC_HEADERS)
ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64)

FIND_LIBRARY (GCRYPT_LIBRARY gcrypt  DOC "hahaha")

#################
EXECUTE_PROCESS(COMMAND /bin/sed -e s/undef/cmakedefine/g ${PROJECT_SOURCE_DIR}/src/libssh2/src/libssh2_config.h.in  OUTPUT_FILE ${PROJECT_SOURCE_DIR}/src/libssh2/src/libssh2_config.h.in.cmake)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/libssh2/src/libssh2_config.h.in.cmake ${PROJECT_SOURCE_DIR}/src/libssh2/src/libssh2_config.h)
###################
# execute_process(
#    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/configure --prefix=${CMAKE_CURRENT_SOURCE_DIR}/._bin --enable-debug --disable-shared --enable-static
#    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#    )

#TODO 给上面的命令添加参数

#execute_process(
#    COMMAND make
#    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#)

#SUBDIRS(openbsd-compat)
#add_subdirectory(openbsd-compat)
#SUBDIRS(src)
ADD_DEFINITIONS(-g -DHAVE_CONFIG_H -DLIBSSH2DEBUG -Wsign-compare -Wno-pointer-sign  )

ADD_LIBRARY(ssh2 STATIC 
src/channel.c  
src/hostkey.c    
src/mac.c      
src/packet.c     
src/scp.c      
src/transport.c
src/comp.c     
src/kex.c        
src/misc.c     
src/pem.c        
src/session.c  
src/userauth.c
src/crypt.c    
#src/libgcrypt.c  
src/openssl.c  
src/publickey.c  
src/sftp.c

)

#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/openbsd-compat ${CMAKE_CURRENT_SOURCE_DIR} )

#ADD_DEPENDENCIES(../../bin/plinker openpsd_compat)

#TARGET_LINK_LIBRARIES(../../bin/plinker ssh openbsd_compat ssl z crypto crypt resolv)
