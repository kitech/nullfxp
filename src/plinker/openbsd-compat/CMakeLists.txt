
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)

ADD_DEFINITIONS(-DHAVE_CONFIG_H  -Wsign-compare -Wno-pointer-sign -std=gnu99 )

ADD_LIBRARY( openbsd_compat STATIC  
#ADD_CUSTOM_TARGET(openpsd_compat ALL echo
base64.c basename.c bindresvport.c daemon.c dirname.c getcwd.c getgrouplist.c getopt.c getrrsetbyname.c glob.c inet_aton.c inet_ntoa.c inet_ntop.c mktemp.c readpassphrase.c realpath.c rresvport.c setenv.c setproctitle.c sha2.c sigact.c strlcat.c strlcpy.c strmode.c strsep.c strtonum.c strtoll.c strtoul.c vis.c 

bsd-arc4random.c bsd-asprintf.c bsd-closefrom.c bsd-cray.c bsd-cygwin_util.c bsd-getpeereid.c bsd-misc.c bsd-nextstep.c bsd-openpty.c bsd-poll.c bsd-snprintf.c bsd-waitpid.c fake-rfc2553.c openssl-compat.c xmmap.c xcrypt.c 

port-aix.c port-irix.c port-linux.c port-solaris.c port-tun.c port-uw.c

)

