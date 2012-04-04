// hostlist.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2012-04-04 21:23:18 +0800
// Version: $Id$
// 

#ifndef _HOSTLIST_H_
#define _HOSTLIST_H_

#include <assert.h>
#include <vector>

class HostInfo {
public:
    char username[60] = {0};
    char password[60] = {0};
    char hostname[100] = {0};
    int port = 22;
    char privkey_file[200] = {0};
};

class HostList {
    
public:
    HostList();
    virtual ~HostList();

    bool parser(char *host_file);
    bool rewind();
    bool hasNext();
    HostInfo *getNext();
    
    
private:
    char host_file[200] = {0};
    char *comment_chars = "#;";
    int itor_pos = 0;
    std::vector<HostInfo*> hosts;
};

#endif /* _HOSTLIST_H_ */
