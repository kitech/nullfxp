// remotedirsortfiltermodel.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-08-23 22:53:31 +0000
// Version: $Id$
// 

#include "remotedirmodel.h"
#include "remotedirsortfiltermodel.h"

#ifndef _MSC_VER
#warning "wrapper lower class, drop this include"
#endif

#include "rfsdirnode.h"

void dump_tree_node_item(directory_tree_item *node_item)
{
    directory_tree_item *item = (directory_tree_item *)node_item ;
    assert(node_item != 0);
    qDebug()<<"====================>>>>";
    qDebug()<<"Retrived="<<node_item->retrived;
    qDebug()<<"prev_retr_flag="<<node_item->prev_retr_flag;
    qDebug()<<"name="<<QString(node_item->file_name );
    qDebug()<<"Type="<<QString(node_item->fileType() );
    qDebug()<<"Size="<<QString(node_item->strFileSize());
    qDebug()<<"Date="<<QString(node_item->fileMDate());
    qDebug()<<"ChildCount="<<node_item->childItems.count();
    qDebug()<< "DeleteFlag="<<node_item->delete_flag;
    qDebug()<<"<<<<====================";
}

//////////////////////////
///
////////////////////////////
RemoteDirSortFilterModel::RemoteDirSortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    this->source_model = 0;
    this->setDynamicSortFilter(true);
}

RemoteDirSortFilterModel::~RemoteDirSortFilterModel()
{
}
bool RemoteDirSortFilterModel::canFetchMore(const QModelIndex &parent) const
{
    qDebug()<<__FUNCTION__<<__LINE__<<parent<<this->source_model;
    if (this->source_model == 0) {
        return false;
    }
    return QSortFilterProxyModel::canFetchMore(parent);
}

QModelIndex RemoteDirSortFilterModel::index(const QString &path, int column) const
{
    return this->mapFromSource(this->source_model->index(path, column));
}
void RemoteDirSortFilterModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    this->source_model = static_cast<RemoteDirModel*>(sourceModel);
    QSortFilterProxyModel::setSourceModel(sourceModel);
}

QString RemoteDirSortFilterModel::filePath(const QModelIndex &index) const
{
    return this->source_model->filePath(this->mapToSource(index));
}
bool RemoteDirSortFilterModel::isDir(const QModelIndex &index) const
{
    return this->source_model->isDir(this->mapToSource(index)) 
        || this->source_model->isSymLinkToDir(this->mapToSource(index));
}

bool RemoteDirSortFilterModel::isSymLink(const QModelIndex &index) const
{
    return this->source_model->isSymLink(this->mapToSource(index));
}

bool RemoteDirSortFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    //qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<this->filters;
    if (this->filters & QDir::Hidden) {
        //qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
        return true;
    } else {
        QString file_name = this->source_model->data(this->source_model->index(source_row, 0, source_parent), Qt::DisplayRole).toString();
        //qDebug()<<this->source_model->data(this->source_model->index(source_row, 0, source_parent), Qt::DisplayRole).toString();
        if (file_name.at(0) == '.') {
            return false;
        } else {
            return true;
        }
        return true;
    }

    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

// TODO cleanup this method
void RemoteDirSortFilterModel::setFilter(QDir::Filters filters)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<this;
    directory_tree_item *dti = 0;

    // 这个函数写的挺奇怪了，这个persistentIndexList到底是什么东西。在什么时候有用呢。
    //
    this->filters = filters;
    // if (strcmp(this->metaObject()->className(), "RemoteDirSortFilterModelEX") == 0) {
    //     //qDebug()<<this->persistentIndexList();
    //     for (int i=0; i<this->persistentIndexList().count(); i++) {
    //         //qDebug()<<i;
    //         dti = static_cast<directory_tree_item*>(this->mapToSource(this->persistentIndexList().at(i)).internalPointer());
    //         //qDebug()<<dti;
    //         qDebug()<<dti->strip_path<<this;
    //         if (dti->strip_path.length() > 0) {
    //             emit layoutAboutToBeChanged();
    //             dti->retrived = 1;
    //             dti->prev_retr_flag = 9;
    //             this->source_model->slot_remote_dir_node_clicked(this->mapToSource(this->persistentIndexList().at(i)));
    
    //             emit layoutChanged();
    //             break;
    //         }
    //     }
    // }
}

bool RemoteDirSortFilterModel::hasChildren(const QModelIndex &parent) const
{
    // qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<""<<parent;
    // return this->source_model->hasChildren(this->mapToSource(parent));
    return QSortFilterProxyModel::hasChildren(parent);
}

QModelIndex	RemoteDirSortFilterModel::mapFromSource ( const QModelIndex & sourceIndex ) const
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<""<<sourceIndex;
    QModelIndex proxyIndex = QSortFilterProxyModel::mapFromSource(sourceIndex);

    return proxyIndex;
}

QModelIndex RemoteDirSortFilterModel::mapToSource(const QModelIndex & proxyIndex ) const
{
    directory_tree_item *dti = 0;
    directory_tree_item *sdti = 0;

    dti = static_cast<directory_tree_item *>(proxyIndex.internalPointer());
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<""<<proxyIndex<<(void*)dti; // proxyIndex's internalPointer is not the same as sourceIndex's internalPointer, so can not read it as sourceIndex's internalPointer do.
    // if (dti != NULL) {
    //     qDebug()<<__FUNCTION__<<": "<<__LINE__<<":";
    //     dump_tree_node_item(dti);
    // }

    QModelIndex smidx = QSortFilterProxyModel::mapToSource(proxyIndex);
    sdti = static_cast<directory_tree_item*>(smidx.internalPointer());
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<""<<smidx<<(void*)sdti;
    if (sdti != NULL) {
        qDebug()<<__FUNCTION__<<": "<<__LINE__<<":";
        dump_tree_node_item(sdti);
    }

    return smidx;
}

int RemoteDirSortFilterModel::rowCount(const QModelIndex & parent) const
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<""<<parent;
    return QSortFilterProxyModel::rowCount(parent);
}

QVariant RemoteDirSortFilterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

//////////////////////
///////  EX
//////////////////////////

RemoteDirSortFilterModelEX::RemoteDirSortFilterModelEX(QObject *parent)
    : RemoteDirSortFilterModel(parent)
{
}

RemoteDirSortFilterModelEX::~RemoteDirSortFilterModelEX()
{
}

bool RemoteDirSortFilterModelEX::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<this->filters<<this->source_model;
    if (this->source_model->isDir(this->source_model->index(source_row, 0, source_parent))
        || this->source_model->isSymLinkToDir(this->source_model->index(source_row, 0, source_parent))) {
        return RemoteDirSortFilterModel::filterAcceptsRow(source_row, source_parent);
    } else {
        return false;
    }
    return RemoteDirSortFilterModel::filterAcceptsRow(source_row, source_parent);
}

// bool RemoteDirSortFilterModelEX::hasChildren(const QModelIndex &parent) const
// {
//     // qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<""<<parent;
//     return RemoteDirSortFilterModel::hasChildren(parent);
// }

