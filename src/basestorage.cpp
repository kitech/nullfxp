// basestorage.cpp --- 
// 
// Filename: basestorage.cpp
// Description: 
// Author: liuguangzhao
// Maintainer: 
// Created: 五  4月  4 14:39:04 2008 (CST)
// Version: 
// Last-Updated: 五  8月  8 12:02:01 2008 (CST)
//           By: 刘光照<liuguangzhao@users.sf.net>
//     Update #: 2
// URL: 
// Keywords: 
// Compatibility: 
// 
// 

// Commentary: 
// 
// 
// 
// 

// Change log:
// 
// 
// 

// Code:

#include "basestorage.h"


BaseStorage * BaseStorage::mInstance = NULL;
BaseStorage * BaseStorage::instance()
{
    if(BaseStorage::mInstance == NULL) {
        BaseStorage::mInstance = new BaseStorage();
    }
    return BaseStorage::mInstance;
}

BaseStorage::BaseStorage()
{
}

BaseStorage::~BaseStorage()
{
    if(this->opened)
        this->close();
}

bool BaseStorage::open()
{
#ifdef Q_OS_WIN
    QString config_path = QCoreApplication::applicationDirPath() + QString("/.nullfxp");
#elif Q_OS_MAC
    //TODO should where?
    QString config_path = QDir::homePath()+QString("/.nullfxp");
#else
    QString config_path = QDir::homePath()+QString("/.nullfxp");
#endif

    if(!QDir().exists(config_path))
        if(!QDir().mkdir(config_path)) return false;

    QFile *fp = new QFile(config_path + QString("/hosts.db"));
    fp->open(QIODevice::ReadWrite|QIODevice::Unbuffered);
    this->opened = true;
    this->changed = false;
    this->ioStream.setDevice(fp);
    this->ioStream>>this->hosts;
    this->ioStream>>this->vec_hosts;

    //qDebug()<<__FILE__<<__LINE__<<this->hosts;

    return true;
}

bool BaseStorage::close()
{
    QFile * fp = (QFile*)this->ioStream.device();
    if(this->changed)
        this->save();
    if(!fp->isOpen())
        fp->close();
    this->opened = false;
    return true;
    delete fp;
}

bool BaseStorage::save()
{
    if(!this->changed) return true;
    QFile * fp = (QFile*)this->ioStream.device();
    fp->resize(0);
    this->ioStream<<this->hosts;
    this->ioStream<<this->vec_hosts;

    //qDebug()<<__FILE__<<__LINE__<<this->hosts;

    return true;
}
bool BaseStorage::addHost(QMap<QString,QString> host)
{
    QMap<QString, QMap<QString,QString> >::iterator it;
    int hid =1;

    for(it = this->hosts.begin(); it != this->hosts.end(); it ++) {
        if((*it)["show_name"] == host["show_name"]) return false;
    }

    hid = this->generate_hid();
    host["hid"] = QString("%1").arg(hid);

    this->hosts[host["show_name"]] = host;
    this->changed = true;

    return true;
}
bool BaseStorage::removeHost(QString show_name)
{
    if(this->containsHost(show_name)) {
        this->hosts.remove(show_name);
        this->changed = true;
        return true;
    }
    return false;
}

bool BaseStorage::updateHost(QMap<QString,QString> host, QString new_name)
{
    QMap<QString, QString> old_host;

    if(this->containsHost(host["show_name"])){
        old_host = this->hosts[host["show_name"]];
        if(!old_host.contains("hid")) {
            host["hid"] = QString("%1").arg(this->generate_hid());
        }else{
            host["hid"] = old_host["hid"];
        }
        this->hosts[host["show_name"]] = host;
        if(new_name == QString::null) {
            
        }else{
            if(this->containsHost(new_name)) {
                qDebug()<<"name conflict";
            }else{
                QString old_name = host["show_name"];
                host["show_name"] = new_name;
                this->hosts[new_name] = host;
                this->hosts.remove(old_name);
                
            }
        }
    }else{
        if(!host.contains("hid") || host["hid"].length() == 0) {
            host["hid"] = QString("%1").arg(this->generate_hid());
            //qDebug()<<host<<__LINE__;
        }
        this->hosts[host["show_name"]] = host;
    }
    this->changed = true;
    return true;
}
bool BaseStorage::clearHost()
{
    this->hosts.clear();
    this->save();
    return true;
}

bool BaseStorage::containsHost(QString show_name)
{
    QMap<QString, QMap<QString,QString> >::iterator it;

    for(it = this->hosts.begin(); it != this->hosts.end(); it ++)
    {
        if((*it)["show_name"] == show_name) return true;
    }
    return false;
}

QMap<QString, QMap<QString,QString> > & BaseStorage::getAllHost()
{
    if(!this->opened) 
        this->open();
    return this->hosts;
}
QStringList BaseStorage::getNameList()
{
    QStringList nlist;

    nlist = this->hosts.keys();

    return nlist;
}
QMap<QString,QString> & BaseStorage::getHost(QString show_name)
{
    QMap<QString,QString> host;

    if(this->containsHost(show_name))
        return this->hosts[show_name];
    else
        return host;
}

int BaseStorage::hostCount()
{
    return this->hosts.count();
}

QMap<QString, QString> BaseStorage::node_to_map(hnode *node)
{
    QMap<QString, QString> nmap;
    /*
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
    */
    if(node == NULL) return nmap;

    nmap["hid"] = node->hid;
    nmap["ptype"] = node->ptype;
    nmap["ltype"] = node->ltype;
    nmap["port"] = node->port;
    nmap["passive"] = node->passive;
    nmap["nname"] = node->nname;
    nmap["hname"] = node->hname;
    nmap["uname"] = node->uname;
    nmap["passwd"] = node->passwd;
    nmap["pkeypath"] = node->pkeypath;
    
    return nmap;
}

BaseStorage::hnode * BaseStorage::map_to_node(QMap<QString, QString> map)
{
    hnode * node = NULL;

    node = new hnode();
    memset(node, 0, sizeof( hnode));
    
    if(map.contains("hid")) node->hid = map["hid"].toInt();
    else node->hid = 0;

    if(map.contains("ptype")) node->ptype = map["ptype"].toInt();
    else node->ptype = PROTO_SFTP;

    if(map.contains("ltype")) node->ltype = map["ltype"].toInt();
    else node->ltype = LTYPE_ALL;

    if(map.contains("port")) node->port = map["port"].toShort();
    else node->port = 22;

    if(map.contains("passive")) node->passive = map["passive"].toShort();
    else node->passive = 1;

    if(map.contains("nname")) node->nname = strdup(map["nnmae"].toAscii().data());
    else node->nname = NULL;

    if(map.contains("hname")) node->hname = strdup(map["hname"].toAscii().data());
    else node->hname = NULL;

    if(map.contains("uname")) node->uname = strdup(map["uname"].toAscii().data());
    else node->uname = NULL;
    
    if(map.contains("passwd")) node->passwd = strdup(map["passwd"].toAscii().data());
    else node->passwd = NULL;
    
    if(map.contains("pkeypath")) node->pkeypath = strdup(map["pkeypath"].toAscii().data());
    else node->pkeypath = NULL;

    return node;
}

int BaseStorage::generate_hid()
{
    int hid = 1;
    QMap<QString, QMap<QString, QString> >::iterator it;
    QMap<QString, QString> nmap;

    for(int i = 1 ; i < INT_MAX ; i ++){
        hid = 0;
        for( it = this->hosts.begin(); it != this->hosts.end(); it ++) {
            if(it->contains("hid")) {
                if( it->value("hid").toInt() == i){
                    hid = 1;
                    break;
                }
            }
        }
        if(hid == 0) return i;
    }
    
    return 1;
}

// 
// basestorage.cpp ends here

