execute_process(
   COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/configure
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
   )
#execute_process(
#    COMMAND make
#    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#)

#SUBDIRS(openbsd-compat)
add_subdirectory(openbsd-compat)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/openbsd-compat ${CMAKE_CURRENT_SOURCE_DIR} )

ADD_DEFINITIONS(-DHAVE_CONFIG_H  -Wsign-compare -Wno-pointer-sign -std=gnu99 )


#AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/openbsd-compat openbsd_compat_SRCS)
#MESSAGE( "${CMAKE_CURRENT_SOURCE_DIR} gggggg")


ADD_LIBRARY(ssh STATIC 
   acss.c authfd.c authfile.c bufaux.c bufbn.c buffer.c 
	canohost.c channels.c cipher.c cipher-acss.c cipher-aes.c 
	cipher-bf1.c cipher-ctr.c cipher-3des1.c cleanup.c 
	compat.c compress.c crc32.c deattack.c fatal.c hostfile.c 
	log.c match.c md-sha256.c moduli.c nchan.c packet.c 
	readpass.c rsa.c ttymodes.c xmalloc.c 
	atomicio.c key.c dispatch.c kex.c mac.c uidswap.c uuencode.c misc.c 
	monitor_fdpass.c rijndael.c ssh-dss.c ssh-rsa.c
	
	dh.c kexdh.c 
	kexgex.c kexdhc.c kexgexc.c scard.c msg.c progressmeter.c dns.c 
	entropy.c scard-opensc.c gss-genr.c umac.c
	)

ADD_EXECUTABLE(../../bin/plinker ssh.c readconf.c clientloop.c sshtty.c  
	sshconnect.c sshconnect1.c sshconnect2.c )

ADD_DEPENDENCIES(../../bin/plinker openpsd_compat)
#SET(CMAKE_LIBRARY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/openbsd-compat/:${CMAKE_LIBRARY_PATH})
# LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/openbsd-compat/) 没有用？
TARGET_LINK_LIBRARIES(../../bin/plinker ssh openbsd_compat ssl z crypto crypt resolv)
