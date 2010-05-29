// rfsdirnode.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-07-19 13:55:48 +0000
// Version: $Id$
// 


#ifndef RFSDIRNODE_H
#define RFSDIRNODE_H

#include <QtCore>

#include "libssh2.h"
#include "libssh2_sftp.h"

enum {
    POP_NO_NEED_NO_DATA = 0x01,
    POP_NO_NEED_WITH_DATA = 0x02,
    POP_WITH_NEED_WANT_UPDATE = 0x04,
    POP_UPDATING = 0x08,
    POP_NEWEST = 0x10,
    FLAG_DELETED = 0x20
};

//这个类中存储的字符串改为Qt内部使用的Unicode编码。
class NetDirNode
{
public:
    NetDirNode()
    {
        this->prevFlag = 0xFF;
        this->retrFlag = 0;
        this->deleted = false;
        this->pNode = 0;
        this->onRow = -1;
        this->linkToDir = false;
        memset(&this->attrib, 0, sizeof(this->attrib));
    }
    ~NetDirNode();

    unsigned char prevFlag;    // 前一步的retrived 状态值。在改变retrived的状态的时候使用。
    unsigned char retrFlag;  // 1 , 0
    bool deleted;   // 1 , 0 ;

    /*
      0 表示UI没有要求过的，所以我们也没有取过数据的            
      1 表示UI没有请求过，我们自己填入了部分数据（如用户指定他的初始化目录的时候），即这表示需要更新的结点
      2 表示UI已经请求过，我们只取回来了一部分数据，即这表示需要立即更新的结点
      8 表示UI已经请求过，我们已经放到处理队列的，但还没有获取来数据的
      9 表示UI已经请求过，并且更新到了最新目录结构状态。
      
      状态转换：
      0------> 8 ------ > 9 
      1------> 8 -------> 9
      2------> 8 -------> 9
    */

    // std::map<int, directory_tree_item *> child_items; // <rowseq, p*> // why not use order stable vector?
    QHash<int, NetDirNode*> childNodes;
    // QVector<directory_tree_item*> childItems;

    NetDirNode *pNode;
    short onRow;
    //N , S , T , D
    //T = D , F , L

    // TODO 去掉一些变量，减小内存用量
    QString fullPath;   // no last / is is dir, but the root / ...
    QString _fileName;
    bool linkToDir; // 是否是链接到目录的链接
     
    LIBSSH2_SFTP_ATTRIBUTES attrib; // 改用QUrlInfo ??? 这个好象不错啊,不过这个占用内存小

public:
    bool isDir();
    bool isSymLink();
    bool isSymLinkToDir();
    int childCount();
    bool hasChild(QString name);
    NetDirNode *findChindByName(QString name);
    bool matchChecksum(QDateTime mdate, quint64 fsize);
    bool matchChecksum(LIBSSH2_SFTP_ATTRIBUTES *attr);
    // 设置本结点中的子结点名字为name的结点的删除标记
    bool setDeleteFlag(QString name, bool del);
    // 设置本结点的删除标记
    bool setDeleteFlag(bool deleted);

    NetDirNode *parent();
    NetDirNode *childAt(int index);
    QString filePath();
    QString fileName();
    QString fileMode();
    QString fileMDate();
    QString fileADate();
    quint64 fileSize();
    QString strFileSize();
    QString fileType();

    bool copyFrom(NetDirNode *node); // the node must has no child, becuase we omit it
    void dumpTreeRecursive();
};

#endif
