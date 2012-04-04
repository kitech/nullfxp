// hostlist.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2012-04-04 21:27:44 +0800
// Version: $Id$
// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "hostlist.h"

HostList::HostList()
{
}

HostList::~HostList()
{

}


bool HostList::parser(char *host_file)
{
    strcpy(this->host_file, host_file);

    FILE *fp = nullptr;
    char line[500] = {0};
    size_t rlen = 0;
    char *p1, *p2, *p3, *p4, *p5;
    char tmp[500];
    
    // p1   p3  p2    p4
    // user:pass@host:port
    fp = fopen(this->host_file, "r");
    assert(fp != NULL);

    while (feof(fp)) {
        if ((rlen = fread(line, sizeof(line), 1, fp)) > 0) {
            if (line[0] == '#' || line[0] == ';') {
                continue;
            }
            p1 = line;
            p2 = strchr(p1, '@');
            if (p2 == NULL) {
                // invalid line
                continue;
            }
            *p2 ++ = '\0';
            
            p3 = strchr(p1, ':');
            if (p3 == NULL) {
                // password 为空
            } else {
                *p3 ++ = '\0';
            }
            
            p4 = strchr(p2, ':');
            if (p4 == NULL) {
                // port 为空
            } else {
                *p4 ++ = '\0';
            }

            HostList 
        }
    }

    return true;
}

bool HostList::rewind()
{

    return true;
}



bool HostList::hasNext()
{

    return true;
}


HostInfo *HostList::getNext()
{

    return nullptr;
}

