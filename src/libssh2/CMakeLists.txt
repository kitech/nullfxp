execute_process(
   COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/configure --prefix=${CMAKE_CURRENT_SOURCE_DIR}/._bin --enable-debug --disable-shared --enable-static
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
   )
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
