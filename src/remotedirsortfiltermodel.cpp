// remotedirsortfiltermodel.cpp --- 
// 
// Filename: remotedirsortfiltermodel.cpp
// Description: 
// Author: liuguangzhao
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 五  5月 23 22:53:31 2008 (CST)
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


#include "remotedirmodel.h"
#include "remotedirsortfiltermodel.h"

#warning "wrapper lower class, drop this include"
#include "rfsdirnode.h"

RemoteDirSortFilterModel::RemoteDirSortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}


RemoteDirSortFilterModel::~RemoteDirSortFilterModel()
{
}
QModelIndex RemoteDirSortFilterModel::index ( const QString & path, int column  ) const
{
    return this->mapFromSource(this->source_model->index( path , column ));
}
void RemoteDirSortFilterModel::setSourceModel ( QAbstractItemModel * sourceModel )
{
    this->source_model = static_cast<RemoteDirModel*>(sourceModel) ;
    QSortFilterProxyModel::setSourceModel(sourceModel);
}

QString RemoteDirSortFilterModel::filePath ( const QModelIndex &index ) const
{
    return this->source_model->filePath(this->mapToSource(index));
}
bool RemoteDirSortFilterModel::isDir ( const QModelIndex &index ) const
{
    return this->source_model->isDir(this->mapToSource(index));
}


bool RemoteDirSortFilterModel::filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<this->filters;
    if(this->filters &  QDir::Hidden )
    {
        //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
        return true;
    }else{
        QString file_name = this->source_model->data(this->source_model->index(source_row, 0, source_parent), Qt::DisplayRole).toString();
        //qDebug()<<this->source_model->data(this->source_model->index(source_row, 0, source_parent), Qt::DisplayRole).toString();
        if(file_name.at(0) == '.') return false;
        else return true;
        return true;
    }

    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

void RemoteDirSortFilterModel::setFilter ( QDir::Filters filters )
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<this;
    directory_tree_item * dti = 0;

    //这个函数写的挺奇怪了的，这个persistentIndexList到底是什么东西。在什么时候有用呢。
    //
    this->filters = filters;
    if(strcmp(this->metaObject()->className(),"RemoteDirSortFilterModelEX") == 0)
    {
        //qDebug()<<this->persistentIndexList();
        for(int i=0;i<this->persistentIndexList().count();i++)
        {
            //qDebug()<<i;
            dti = static_cast<directory_tree_item*>(this->mapToSource(this->persistentIndexList().at(i)) .internalPointer());
            //qDebug()<<dti;
            qDebug()<<dti->strip_path<<this;
            if(dti->strip_path.length() > 0)
            {
                emit layoutAboutToBeChanged();
                //this->source_model->slot_remote_dir_node_clicked(this->source_model->index(0,0,QModelIndex()) );
                //dti = static_cast<directory_tree_item*>(this->source_model->index(0,0,QModelIndex()) .internalPointer());
                //qDebug()<<dti->file_name<<" "<<dti->file_type<<" "<< dti->strip_path ;
                //file_path = dti->strip_path ;
                dti->retrived = 1;
                dti->prev_retr_flag=9;
                this->source_model->slot_remote_dir_node_clicked(this->mapToSource(this->persistentIndexList().at(i) ));
    
                emit layoutChanged();
                break;
            }
        }
    }
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

bool RemoteDirSortFilterModelEX::filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<this->filters;
    if( this->source_model->isDir( this->source_model->index(source_row, 0, source_parent)))
    {
        return RemoteDirSortFilterModel::filterAcceptsRow(source_row, source_parent);
    }else{
        return false;
    }
    return RemoteDirSortFilterModel::filterAcceptsRow(source_row, source_parent);
}


