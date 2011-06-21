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
}

LocalFileSystemModel::~LocalFileSystemModel()
{
}

bool LocalFileSystemModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    qDebug()<<data->urls()<<parent<<sender();
    emit this->sig_drop_mime_data(data, action, row, column, parent);
    bool ret = true;
    // 	qDebug() <<"signals emited";
    return true;
    return ret;

    return QFileSystemModel::dropMimeData(data, action, row, column, parent);
}
