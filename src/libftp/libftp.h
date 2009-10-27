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
    int connect(const QString host, unsigned short port = 21);
    int login(const QString &user = QString(), const QString &password = QString());
    int logout();
    int connectDataChannel();
    int closeDataChannel();
    int list(QString path);
    int lista(QString path);
    int nlist(QString path);
    int pwd(QString &path); // returned path
    int mkdir(QString path);
    int rmdir(QString path);
    int chdir(QString path);
    int put(const QString fileName);
    int putNoWaitResponse(const QString fileName);
    int get(const QString fileName);
    int getNoWaitResponse(const QString fileName);
    int remove(const QString path);
    int rename(const QString src, const QString dest);
    int passive();
    int rein(const QString &user = QString(), const QString &password = QString()); // 重新登陆
    int type(int type);
    int noop();
    int system(QString &type);
    int stat(QString path);
    int port(const QString hostip, const unsigned short port); // fxp
    int portNoWaitResponse(const QString hostip, const unsigned short port);
    int size(QString path, quint64 &siz);
    
    unsigned short pasvPeer(QString &hostip); // get passive peer ip
    int swallowResponse();
    int waitForCtrlResponse();
    
    QVector<QUrlInfo> getDirList();
    QString getServerBanner();
    QTcpSocket *getDataSocket();
    QString errorString();
    int setEncoding(QString encoding);
    
private:
    int parsePasvPort(QString &host, unsigned short &port);
    QByteArray readAll(QTcpSocket *sock);
    QByteArray readAllByEndSymbol(QTcpSocket *sock);
    bool parseDir(const QByteArray &buffer, const QString &userName, QUrlInfo *info);
    void setError(int okno, QString msg);

private:
    QTcpSocket *qsock;
    QTcpSocket *qdsock;
    QString pasvHost;
    unsigned short pasvPort;
    QVector<QUrlInfo> dirList;
    QString servBanner;
    int errnum;
    QString errmsg;
    QString encoding;
    QTextCodec *codec;
};

        
#endif /* _LIBFTP_H_ */
