// connection.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-05-28 23:15:47 +0000
// Version: $Id$
// 

#include "connection.h"

Connection::Connection(QObject *parent)
    : QObject(parent)
{
}
Connection::~Connection()
{
}

int Connection::alivePing()
{
    return 0;
}

int Connection::connect()
{
    return 0;
}
int Connection::disconnect()
{
    return 0;
}
int Connection::reconnect()
{
    return 0;
}
