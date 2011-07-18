// yasshconnection.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-07-18 15:21:49 +0000
// Version: $Id$
// 

#include "yasshconnection.h"

#include "libssh/libssh.h"
#include "libssh/libsshpp.hpp"

void mytest_libssh()
{
    ssh_session ssh_sess = ssh_new();
    if (ssh_sess == NULL) {
        
    }
    qDebug()<<ssh_sess;
    ssh_free(ssh_sess);
}


YaSSHConnection::YaSSHConnection(QObject *parent)
    : Connection(parent)
{
}

YaSSHConnection::~YaSSHConnection()
{
    
}

