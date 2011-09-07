hacks:
    为了防止config.h文件冲突，编译fixcfg.sh可把源代码中的#include "config.h"替换为#include "libssh_config.h"
    misc.c 中gettimeofday函数声明为static，否则与libssh2中的gettimeofday实现冲突。
    channels1.c 中应该包含头文件 #include "libssh_config.h" 否则，WITH_SSH1将无法从配置定义文件中取得。
        这个问题是还是在命令行加入WITH_SSH1参数，这样不需要改动源代码了。
    编译过程，需要使用LIBSSH_STATIC宏，编译为静态库，在调用该的工程编译的时候，也需要定义这个宏。

    链接这个库，需要的系统库-lws2_32 -lshell32
    
    kssh.lib(pcap.obj) : error LNK2019: unresolved external symbol _gettimeofday referenced in function _ssh_pcap
