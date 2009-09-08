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
    enum {TYPE_MIN = 0, TYPE_ASCII, TYPE_EBCID, TYPE_BIN,
          TYPE_IMAGE, TYPE_LOCAL_BYTE,
          TYPE_MAX};
    int connect(const QString host, short port = 21);
    int login(const QString &user = QString(), const QString &password = QString());
    int logout();
    int connectDataChannel();
    int list(QString path);
    int nlist(QString path);
    int pwd(QString &path); // returned path
    int mkdir(QString path);
    int rmdir(QString path);
    int remove(const QString path);
    int rename(const QString src, const QString dest);
    int passive();
    int rein(const QString &user = QString(), const QString &password = QString()); // 重新登陆
    int type(int type);
    int noop();
    int system(QString &type);

    QVector<QUrlInfo> getDirList();
    QString getServerBanner();
    
private:
    int parsePasvPort(QString &host, short &port);
    QByteArray readAll(QTcpSocket *sock);
    QByteArray readAllByEndSymbol(QTcpSocket *sock);
    bool parseDir(const QByteArray &buffer, const QString &userName, QUrlInfo *info);

private:
    QTcpSocket *qsock;
    QTcpSocket *qdsock;
    QString pasvHost;
    short pasvPort;
    QVector<QUrlInfo> dirList;
    QString servBanner;
};

        
#endif /* _LIBFTP_H_ */
