// security.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-10-07 16:11:23 +0800
// Version: $Id$
// 

//---------------------------------------------------------------------------
#ifndef _SECURITY_H_
#define _SECURITY_H_

#include <iostream>
#include <string>

//---------------------------------------------------------------------------
#define PWALG_SIMPLE 1
#define PWALG_SIMPLE_MAGIC 0xA3
#define PWALG_SIMPLE_STRING ("0123456789ABCDEF")
#define PWALG_SIMPLE_MAXLEN 50
#define PWALG_SIMPLE_FLAG 0xFF
std::string EncryptPassword(std::string Password, std::string Key, int Algorithm = PWALG_SIMPLE);
std::string DecryptPassword(std::string Password, std::string Key, int Algorithm = PWALG_SIMPLE);
std::string SetExternalEncryptedPassword(std::string Password);
bool GetExternalEncryptedPassword(std::string Encrypted, std::string & Password);
//---------------------------------------------------------------------------

#endif /* _SECURITY_H_ */
