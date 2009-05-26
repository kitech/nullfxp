#ifndef _LIBSSH2_CONFIG_H_
#define _LIBSSH2_CONFIG_H_

#define HAVE_GETTIMEOFDAY
#ifdef WIN32
#ifdef __MINGW32__
#include "mw_libssh2_config.h"
#endif
#ifdef _MSC_VER
#include "../win32/libssh2_config.h"
#undef HAVE_GETTIMEOFDAY
#endif
#else
#include "un_libssh2_config.h"
#endif

#endif
