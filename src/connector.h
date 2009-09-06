// connector.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-09-06 12:53:35 +0800
// Version: $Id$
// 

#ifndef _CONNECTOR_H_
#define _CONNECTOR_H_

#include <QtCore>
#include <QtNetwork>

#include "connection.h"        

class Connector : public QThread
{
    Q_OBJECT;
public:
    Connector(QObject *parent = 0);
    ~Connector();

    void setHostInfo(QMap<QString, QString> host);
    QString get_status_desc(int status);

    virtual void run();

private slots:
    void slot_finished();

signals:
    void connect_state_changed(QString state_desc);
    void connect_finished(int status, Connection *conn);

private:
    Connection *conn;
    int connect_status;
    bool user_canceled;
};

#endif /* _CONNECTOR_H_ */
