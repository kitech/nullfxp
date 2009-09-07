// libftp.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-09-08 00:50:41 +0800
// Version: $Id$
// 

#ifndef _LIBFTP_H_
#define _LIBFTP_H_

#include <QtCore>
#include <QtNetwork>

class LibFtp : public QObject
{
    Q_OBJECT;
public:
    LibFtp(QObject *parent = 0);
    virtual ~LibFtp();

    int connect();
    int login();
    int logout();
    int list(QString path);
    int nlist(QString path);
    int pwd();
    int rmdir(QString path);

private:
    QTcpSocket *qsock;
    QTcpSocket *qdsock;
    
};

        
#endif /* _LIBFTP_H_ */
