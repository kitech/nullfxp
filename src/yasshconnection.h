// yasshconnection.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-07-18 15:21:26 +0000
// Version: $Id$
// 

#ifndef _YASSHCONNECTION_H_
#define _YASSHCONNECTION_H_

#include "connection.h"

class YaSSHConnection : public Connection
{
    Q_OBJECT;
public:
    explicit YaSSHConnection(QObject *parent = 0);
    virtual ~YaSSHConnection();


};


#endif /* _YASSHCONNECTION_H_ */
