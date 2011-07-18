/*
// libssh_config.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-05-26 21:45:42 +0800
// Version: $Id: libssh_config.h 455 2009-08-29 09:23:35Z liuguangzhao $
// 
*/

#ifndef _LIBSSH_CONFIG_H_
#define _LIBSSH_CONFIG_H_

#ifdef WIN32
#ifdef __MINGW32__
#include "mw_libssh_config.h"
#endif
#ifdef _MSC_VER
// #include "../win32/libssh_config.h"
#include "win32_libssh_config.h"
#endif
#else
#include "un_libssh_config.h"
#endif

#endif
