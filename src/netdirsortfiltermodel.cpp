// netdirsortfiltermodel.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-08-23 22:53:31 +0000
// Version: $Id$
// 

#include "remotedirmodel.h"
#include "netdirsortfiltermodel.h"

#ifndef _MSC_VER
#warning "wrapper lower class, drop this include"
#endif

#include "rfsdirnode.h"
#include "utils.h"

void dump_tree_node_item(NetDirNode *node_item)
{
    NetDirNode *item = (NetDirNode *)node_item ;
    assert(node_item != 0);
    qDebug()<<"====================>>>>";
    qDebug()<<"Retrived="<<node_item->retrFlag;
    qDebug()<<"prev_retr_flag="<<node_item->prevFlag;
    qDebug()<<"path="<<QString(node_item->filePath() );
    qDebug()<<"name="<<QString(node_item->_fileName );
    qDebug()<<"Type="<<QString(node_item->fileType() );
    qDebug()<<"Size="<<QString(node_item->strFileSize());
    qDebug()<<"Date="<<QString(node_item->fileMDate());
    qDebug()<<"ChildCount="<<node_item->childNodes.count();
    qDebug()<< "DeleteFlag="<<node_item->deleted;
    qDebug()<<"<<<<====================";
}

DirTreeSortFilterModel::DirTreeSortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    this->setDynamicSortFilter(true);
}

DirTreeSortFilterModel::~DirTreeSortFilterModel()
{
}

// QModelIndex DirTreeSortFilterModel::parent ( const QModelIndex & child ) const
// {
//     // qDebug()<<__FUNCTION__<<__LINE__<<child<<child.isValid();
//     NetDirNode *dti = 0;
//     NetDirNode *sdti = 0;

//     QModelIndex sourceIndex = this->mapToSource(child);
//     // qDebug()<<sourceIndex.isValid();
    
//     dti = static_cast<NetDirNode *>(sourceIndex.internalPointer());
//     if (dti != NULL) {
//         // qDebug()<<__FUNCTION__<<": "<<__LINE__<<":";
//         // dump_tree_node_item(dti);
//     }

//     QModelIndex parent = QSortFilterProxyModel::parent(child);
//     return parent;
// }

bool DirTreeSortFilterModel::hasChildren(const QModelIndex &parent) const
{
    // qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<""<<parent<<parent.isValid();

    NetDirNode *dti = 0;
    NetDirNode *sdti = 0;

    QModelIndex sourceIndex = this->mapToSource(parent);
    // qDebug()<<sourceIndex.isValid()<<parent.isValid();

    if (!sourceIndex.isValid()) {
        // the root / node???
        return true;
    } else {
        dti = static_cast<NetDirNode *>(sourceIndex.internalPointer());
        if (dti != NULL) {
            // qDebug()<<__FUNCTION__<<": "<<__LINE__<<":";
            // dump_tree_node_item(dti);
            bool hc = dti->isDir() || dti->isSymLinkToDir();
            return hc;
        }
    }
    return false;
}

void DirTreeSortFilterModel::setFilter(QDir::Filters filters)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<this;

    this->filters = filters;
    // must use this to let filter applyed at once.
    this->invalidateFilter();
}

bool DirTreeSortFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    // source_parent is already source model, do not need use mapToSource;
    QModelIndex testIndex = this->sourceModel()->index(source_row, 0, source_parent);
    NetDirNode *node = static_cast<NetDirNode*>(testIndex.internalPointer());
    QString file_name = node->fileName();
    QString file_path = node->filePath();

    if (file_name.length() == 0 && file_path.length() == 0) {
        return false;   // for new added empty node
    } 
    if (node->isDir() || node->isSymLinkToDir()) {
        if (this->filters & QDir::Hidden) {
            return true;
        } else {
            if (file_name.at(0) == '.') {
                return false;
            } else {
                return true;
            }
        }
    } else {
        return false;
    }

    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);

    ///////////////////////////// 
    // //qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<this->filters;
    // if (this->filters & QDir::Hidden) {
    //     //qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //     return true;
    // } else {
    //     // source_parent is already source model, do not need use mapToSource;
    //     QModelIndex testIndex = this->sourceModel()->index(source_row, 0, source_parent);
    //     q_debug()<<testIndex;
    //     NetDirNode *node = static_cast<NetDirNode*>(testIndex.internalPointer());
    //     dump_tree_node_item(node);
    //     // QString file_name = this->source_model->data(this->source_model->index(source_row, 0, source_parent), Qt::DisplayRole).toString();
    //     QString file_name = node->fileName();
    //     //qDebug()<<this->source_model->data(this->source_model->index(source_row, 0, source_parent), Qt::DisplayRole).toString();
    //     if (file_name.length() == 0) {
    //         return true;   // for new added empty node
    //     } else if (file_name.at(0) == '.') {
    //         return false;
    //     } else {
    //         return true;
    //     }
    //     return true;
    // }

    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

DirTableSortFilterModel::DirTableSortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    this->setDynamicSortFilter(true);
}
DirTableSortFilterModel::~DirTableSortFilterModel()
{
}

bool DirTableSortFilterModel::hasChildren(const QModelIndex &parent) const
{
    // qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<""<<parent<<parent.isValid();

    NetDirNode *dti = 0;
    NetDirNode *sdti = 0;

    QModelIndex sourceIndex = this->mapToSource(parent);
    // qDebug()<<sourceIndex.isValid()<<parent.isValid();

    if (!sourceIndex.isValid()) {
        // the root / node???
        return true;
    } else {
        dti = static_cast<NetDirNode *>(sourceIndex.internalPointer());
        if (dti != NULL) {
            // qDebug()<<__FUNCTION__<<": "<<__LINE__<<":";
            // dump_tree_node_item(dti);
            bool hc = dti->isDir() || dti->isSymLinkToDir();
            return hc;
        }
    }
    return false;
}

void DirTableSortFilterModel::setFilter(QDir::Filters filters)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<this;

    this->filters = filters;
    // must use this to let filter applyed at once.
    this->invalidateFilter();
}

bool DirTableSortFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    // source_parent is already source model, do not need use mapToSource;
    QModelIndex testIndex = this->sourceModel()->index(source_row, 0, source_parent);
    NetDirNode *node = static_cast<NetDirNode*>(testIndex.internalPointer());
    QString file_name = node->fileName();
    QString file_path = node->filePath();

    if (file_name.length() == 0 && file_path.length() == 0) {
        return false;   // for new added empty node
    } 
    if (this->filters & QDir::Hidden) {
        return true;
    } else {
        if (file_name.at(0) == '.') {
            return false;
        } else {
            return true;
        }
    }

    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);

    ///////////////////////////// 
    // //qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<this->filters;
    // if (this->filters & QDir::Hidden) {
    //     //qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //     return true;
    // } else {
    //     // source_parent is already source model, do not need use mapToSource;
    //     QModelIndex testIndex = this->sourceModel()->index(source_row, 0, source_parent);
    //     q_debug()<<testIndex;
    //     NetDirNode *node = static_cast<NetDirNode*>(testIndex.internalPointer());
    //     dump_tree_node_item(node);
    //     // QString file_name = this->source_model->data(this->source_model->index(source_row, 0, source_parent), Qt::DisplayRole).toString();
    //     QString file_name = node->fileName();
    //     //qDebug()<<this->source_model->data(this->source_model->index(source_row, 0, source_parent), Qt::DisplayRole).toString();
    //     if (file_name.length() == 0) {
    //         return true;   // for new added empty node
    //     } else if (file_name.at(0) == '.') {
    //         return false;
    //     } else {
    //         return true;
    //     }
    //     return true;
    // }

    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

