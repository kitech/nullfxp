/*
// libssh2_config.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-05-26 21:45:42 +0800
// Last-Updated: 2009-05-26 21:45:49 +0800
// Version: $Id$
// 
*/

#ifndef _LIBSSH2_CONFIG_H_
#define _LIBSSH2_CONFIG_H_

#ifdef WIN32
#ifdef __MINGW32__
#include "mw_libssh2_config.h"
#endif
#ifdef _MSC_VER
#include "../win32/libssh2_config.h"
#endif
#else
#include "un_libssh2_config.h"
#endif

#endif
