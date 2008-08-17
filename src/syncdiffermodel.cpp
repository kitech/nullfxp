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
// Last-Updated: 日  8月 17 13:29:14 2008 (CST)
//           By: 刘光照<liuguangzhao@users.sf.net>
//     Update #: 1
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
    //q_debug()<<row<<","<<column<<" "<<parent;
    assert(row >= 0);

    QModelIndex idx = QModelIndex();
    if(!parent.isValid()) {
        idx = this->createIndex(row, column, -1);
    }else{
        idx = this->createIndex(row, column, parent.row());
        //qDebug()<<"create sub model"<<parent.row();
    }
    return idx;
}
QModelIndex SyncDifferModel::parent(const QModelIndex &index) const
{
    //q_debug()<<""<<index;
    QModelIndex idx = QModelIndex();
    if(!index.isValid()) {

    }else{
        //idx = index((int)(index.internalPointer()), 0, idx);
        //idx = createIndex(0,0,0);
        int row = index.internalId();
        if(row == -1) {
        }else{
            assert( row != -1);
            idx = this->index(row, 0, idx);        
        }
    }
    return idx;
}

QVariant   SyncDifferModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) {
        assert(1 == 2);
    }

    //q_debug()<<""<<index;
    if(role != Qt::DisplayRole) {
        return QVariant();
    }

    //    return QString("vsdf");
    int row = index.internalId();
    if(row == -1) {
        row = index.row();
    }
    
    QString key = this->sync_win->synckeys.at(row).first;
    QHash<QString, int> elem = this->sync_win->syncer.value(key);

    //qDebug()<<key;
    //qDebug()<<elem;

    switch(index.column()) {
    case 0:
        if (!index.parent().isValid()) {
            //return "kkkkkkkkkkkkkk";
            return key;
        } else {
            QString vv = elem.keys().at(index.row());
            return vv;
            //            qDebug()<<elem;
            return "vasdfsdf";
        }
        break;
    case 1:
        return "isdvsA";
    case 2:
        return "vsdf";
        break;
    default:
        q_debug()<<"that is impossible";
        break;
    };

    return QVariant();
}
int SyncDifferModel::rowCount(const QModelIndex &index) const
{
    //q_debug()<<""<<index;

    if(!index.isValid()) {
        return this->sync_win->synckeys.count();
    }else{
        int row = index.internalId();
        if(row == -1) {
            row = index.row();
        }else{
            return 0;
        }
        //qDebug()<<"data of row:"<<row;
        QString key = this->sync_win->synckeys.at(row).first;
        int has_child = this->sync_win->synckeys.at(row).second;
        if(has_child == -1) {
            //有子结点
            QHash<QString, int> elem = this->sync_win->syncer.value(key);
            //qDebug()<<elem;
            //Q_ASSERT(row >= 0 && row < elem.count());        
            int subrows = elem.count() ;
            //q_debug()<<key<<" should have "<<subrows<<" child row";
            //return 1;
            return subrows - 1;
        } else {
            return 0;
        }
    }
}
int SyncDifferModel::columnCount(const QModelIndex &index) const
{
    //q_debug()<<""<<index;
    if(!index.isValid()) {
        return 3;
    }else{
        return 3;
    }
}

void SyncDifferModel::maybe_has_data()
{
    qDebug()<<"aaaaaaaaaa";
    emit layoutChanged ();

}

