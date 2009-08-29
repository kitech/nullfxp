// baserfsmodel.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-07-17 20:44:54 +0800
// Version: $Id$
// 

#ifndef BASERFSMODEL_H
#define BASERFSMODEL_H

#include <QtGlobal>
#include <QtCore>
#include <QtGui>


class BaseRFSModel: public QAbstractItemModel
{
    Q_OBJECT;

public:
    BaseRFSModel(QObject * parent=0);
    ~BaseRFSModel();
    
    //model相关函数的实现
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) const;



    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orienation, int role = Qt::DisplayRole) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    QMimeData * mimeData(const QModelIndexList &indexes) const;
    QStringList mimeTypes() const;
    Qt::DropActions supportedDropActions() const;

    //让model可编辑修改的函数实现
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    public slots:
    bool submit();
    void revert();

private:
    bool init();
    enum {POP_NO_NEED_NO_DATA = 0,
          POP_NO_NEED_WITH_DATA = 1,
          POP_WITH_NEED_WANT_UPDATE = 2,
          POP_UPDATING = 8,
          POP_NEWEST = 9  
    };
    struct RFSDirNode{
    RFSDirNode(): parent(0){}
        ~RFSDirNode(){}
        QString name; //文件名
        QString path; //文件所在的目录
        QString mode;
        quint32 imode;
        quint64 size;
        QString mtime;
        QString atime;
        QString ctime;
        RFSDirNode *parent;
        QVector<RFSDirNode*> children;
        QString md5;
        char    pstatus;
        char    status;
        int     rowNum;
    };
    const static int column = 4;
    RFSDirNode * root;
};

#endif
