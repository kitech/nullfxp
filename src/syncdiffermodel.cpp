// syncdiffermodel.cpp --- 
// 
// Filename: syncdiffermodel.cpp
// Description: 
// Author: 刘光照<liuguangzhao@users.sf.net>
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 日  8月 10 21:13:15 2008 (CST)
// Version: 
// Last-Updated: 
//           By: 
//     Update #: 0
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

#include "utils.h"

#include "synchronizewindow.h"
#include "syncdiffermodel.h"

SyncDifferModel::SyncDifferModel(QObject *parent)
    :QAbstractItemModel(parent)
{
    this->sync_win = static_cast<SynchronizeWindow*>(parent);
}

SyncDifferModel::~SyncDifferModel()
{
    q_debug()<<"destructured";
}

QModelIndex SyncDifferModel::index(int row, int column, const QModelIndex &parent) const
{
    q_debug()<<row<<","<<column<<" "<<parent;
    QModelIndex idx = QModelIndex();
    if(!parent.isValid()) {
        idx = this->createIndex(row, column, -1);
    }else{
        idx = this->createIndex(row, column, parent.row());
    }
    return idx;
}
QModelIndex SyncDifferModel::parent(const QModelIndex &index) const
{
    q_debug()<<""<<index;
    QModelIndex idx = QModelIndex();
    if(!index.isValid()) {

    }else{
        //idx = index((int)(index.internalPointer()), 0, idx);
        //idx = createIndex(0,0,0);
        int row = index.internalId();
        idx = this->index(row, 0, QModelIndex());
    }
    return idx;
}

QVariant   SyncDifferModel::data(const QModelIndex &index, int role) const
{
    q_debug()<<""<<index;
    if(role != Qt::DisplayRole) {
        return QVariant();
    }

    return QString("vsdf");
    
    QString key = this->sync_win->synckeys.at(index.row());
    QHash<QString, int> elem = this->sync_win->syncer.value(key);

    qDebug()<<key;
    qDebug()<<elem;

    return QVariant();
}
int SyncDifferModel::rowCount(const QModelIndex &index) const
{
    q_debug()<<""<<index;

    if(!index.isValid()) {
        return 1;
    }else{
        return 0;
    }
}
int SyncDifferModel::columnCount(const QModelIndex &index) const
{
    q_debug()<<""<<index;
    if(!index.isValid()) {
        return 1;
    }else{
        return 0;
    }
}
