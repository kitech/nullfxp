// localfilesystemmodel.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-06-21 17:41:41 +0000
// Version: $Id$
// 

#include "localfilesystemmodel.h"


LocalFileSystemModel::LocalFileSystemModel(QObject *parent)
    : QFileSystemModel(parent)
{
    this->setReadOnly(false);
}

LocalFileSystemModel::~LocalFileSystemModel()
{
}

Qt::DropActions LocalFileSystemModel::supportedDropActions() const
{
    Qt::DropActions acts = Qt::CopyAction | Qt::MoveAction | QFileSystemModel::supportedDropActions();
    // qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<acts;
    
    return acts;
}

QStringList LocalFileSystemModel::mimeTypes() const
{

    QStringList mtypes;
    
    mtypes<<"text/uri-list";
    mtypes<<"application/task-package";
    // mtypes<<"text/plain";
    // mtypes<<QAbstractItemModel::mimeTypes();
    mtypes << QFileSystemModel::mimeTypes();
    //qDebug()<<mtypes ;
    //mtypes = QAbstractItemModel::mimeTypes();
    //qDebug()<<mtypes;
    //return QAbstractItemModel::mimeTypes();

    // qDebug()<<__FILE__<<__LINE__<<__FUNCTION__<<mtypes;

    return mtypes;

}

// TODO 是否把QModelIndex转换成QPersistentModelIndex?
bool LocalFileSystemModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, 
                                        int column, const QModelIndex & parent)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    qDebug()<<data->formats()<<data->urls()<<parent<<sender();
    emit this->sig_drop_mime_data(data, action, row, column, parent);
    bool ret = true;
    // 	qDebug() <<"signals emited";
    return true;
    return ret;

    return QFileSystemModel::dropMimeData(data, action, row, column, parent);
}
