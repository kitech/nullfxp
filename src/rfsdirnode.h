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
    POP_NO_NEED_NO_DATA = 0,
    POP_NO_NEED_WITH_DATA = 1,
    POP_WITH_NEED_WANT_UPDATE = 2,
    POP_UPDATING = 8,
    POP_NEWEST = 9  
};

//这个类中存储的字符串改为Qt内部使用的Unicode编码。
class directory_tree_item : public QObject
{
    Q_OBJECT;
public:
    directory_tree_item(QObject *parent = 0)
    {
        this->prev_retr_flag = 0;
        this->retrived = 0;
        this->parent_item = 0;
        this->row_number = -1;
        this->delete_flag = 0 ;
        this->linkToDir = false;
        memset(&this->attrib, 0, sizeof(this->attrib));
    }
    ~directory_tree_item();

    unsigned char prev_retr_flag;    // 前一步的retrived 状态值。在改变retrived的状态的时候使用。
    unsigned char retrived;  // 1 , 0
    unsigned char delete_flag;   // 1 , 0 ;
    /*
      0 表示UI没有要求过的，所以我们也没有取过数据的            
      1 表示UI没有请求过，我们自己填入了部分数据（如用户指定他的初始化目录的时候），即这表示需要更新的结点
      2 表示UI已经请求过，我们只取回来了一部分数据，即这表示需要更新的结点
      8 表示UI已经请求过，我们已经放到处理队列的，但还没有获取来数据的
      9 表示UI已经请求过，并且更新到了最新目录结构状态。
        
      状态转换：
      0------> 8 ------ > 9 
      1------> 8 -------> 9
      2------> 8 -------> 9
    */

    QVector<directory_tree_item *> children;
    
    std::map<int, directory_tree_item *> child_items;
    directory_tree_item *parent_item;
    int row_number;    //指的是所包含的子结点个数 //这个也没有用了吧
    //N , S , T , D
    //T = D , F , L

    // TODO 去掉一些变量，减小内存用量
    QString strip_path;
    QString file_name;
    bool   linkToDir; // 是否是链接到目录的链接
     
    LIBSSH2_SFTP_ATTRIBUTES attrib;

public:
    bool isDir();
    bool isSymLink();
    bool isSymLinkToDir();
    int childCount();
    bool hasChild(QString name);
    bool setDeleteFlag(QString name, bool del);
    directory_tree_item *parent();
    directory_tree_item *childAt(int index);
    QString filePath();
    QString fileName();
    QString fileMode();
    QString fileMDate();
    QString fileADate();
    quint64 fileSize();
    QString strFileSize();
    QString fileType();
};

#endif
