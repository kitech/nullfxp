// curlftp_callback.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-07-19 20:58:08 +0800
// Version: $Id$
// 
#ifndef _CURLFTP_CALLBACK_H_
#define _CURLFTP_CALLBACK_H_

#ifdef __cplusplus
extern "C" {
#endif

    size_t callback_read_dir(void *ptr, size_t size, size_t nmemb, void *userp);

    size_t callback_read_file(void *ptr, size_t size, size_t nmemb, void *userp);

    size_t callback_write_file(void *ptr, size_t size, size_t nmemb, void *userp);

#ifdef __cplusplus
    };
#endif

#endif /* _CURLFTP_CALLBACK_H_ */
