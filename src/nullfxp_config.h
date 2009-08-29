// nullfxp_config.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-05-17 16:44:47 +0800
// Version: $Id$
// 

#ifndef _NULLFXP_CONFIG_H_
#define _NULLFXP_CONFIG_H_
        
#ifdef WIN32
  #ifdef _MSC_VER
  #include "config_win32_msvc.h"
  #else
  #include "config_win32_g++.h"
  #endif
#else        
#include "config_linux.h"
#endif

#endif /* _NULLFXP_CONFIG_H_ */
