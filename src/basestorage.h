// basestorage.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-04-04 14:46:49 +0000
// Last-Updated: 2009-06-21 11:43:50 +0000
// Version: $Id$
// 

#include <QtCore>
#include <QDataStream>

class BaseStorage
{
public:
    static BaseStorage * instance();
    ~BaseStorage();
    bool open();
    bool close();
    bool save();

    bool addHost(QMap<QString, QString> host, QString catPath = QString::null);
    bool removeHost(QString show_name, QString catPath = QString::null);
    bool updateHost(QMap<QString,QString> host, QString newName = QString::null, QString catPath = QString::null);
    bool clearHost(QString catPath = QString::null);

    bool containsHost(QString show_name, QString catPath = QString::null);

    QMap<QString, QMap<QString,QString> > & getAllHost(QString catPath = QString::null);
    QMap<QString,QString> & getHost(QString show_name, QString catPath = QString::null);
    QStringList getNameList(QString catPath = QString::null);

    int hostCount(QString catPath = QString::null);
  
signals:
    void hostListChanged(QString catPath = QString::null);
    void hostLIstChanged(QString show_name, QString catPath = QString::null);

private:
    BaseStorage();

    static BaseStorage * mInstance;
    QMap<QString,QMap<QString,QString> >  hosts;
    QVector<QMap<QString, QString> > vec_hosts;
    bool opened;
    bool changed;
    QDataStream  ioStream;

    enum {PROTO_NONE, PROTO_SFTP, PROTO_FTP};
    enum {LTYPE_ALL = 0x01|0x02|0x04|0x08, LTYPE_PASSWD=0x01, LTYPE_PKEY = 0x02, 
          LTYPE_KBI = 0x04, LTYPE_GASSAPI = 0x08};

    //host 结点结构
    class hnode{
    public:
        int hid;
        int ptype; //协议类型
        int ltype; //登陆类型
        short port;
        short passive;
        char *nname; //node name, or show name
        char *hname; //host name
        char *uname; // user name , login name
        char *passwd; // 
        char *pkeypath; //登陆密钥路径
    };
    QVector< hnode*>  hv;

    QMap<QString, QString> node_to_map( hnode *node);
    hnode * map_to_node(QMap<QString, QString> map);
    int generate_hid();
    bool init();

    QString storagePath;
};


/* basestorage.h ends here */
