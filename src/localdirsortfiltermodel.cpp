// localdirsortfiltermodel.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2007-11-14 22:54:04
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>

#include "localdirsortfiltermodel.h"


LocalDirSortFilterModel::LocalDirSortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
//     this->setFilterKeyColumn( 2 );
//     QString filter_exp = QString("^(%1|%2).*").arg(
//                                  QApplication::translate("QFileDialog",
// #ifdef Q_WS_WIN
//             "File Folder", "Match Windows Explorer"
// #else
//                     "Folder", "All other platforms"
// #endif
//                                                         )
//                                                         ).arg(
//                                                           QApplication::translate("QFileDialog",
// #ifdef Q_OS_MAC
//                                                           "Alias", "Mac OS X Finder"
// #else
//                                                                   "Shortcut", "All other platforms"
// #endif
//                                                                                 )
//                                                       );
    //qDebug()<<filter_exp; 
    //this->setFilterRole(Qt::DisplayRole);
    //this->setFilterRegExp(QRegExp("^.*(o).*", Qt::CaseInsensitive));
    //this->setFilterFixedString("Folder");
    
    //qDebug()<<filter_exp<<this->filterRegExp(); 
    this->source_model = 0;
}


LocalDirSortFilterModel::~LocalDirSortFilterModel()
{
}

QModelIndex LocalDirSortFilterModel::index(const QString &path, int column) const
{
    return this->mapFromSource(this->source_model->index( path , column ));
}
QModelIndex LocalDirSortFilterModel::index(int& row, int column, QModelIndex& parent) const
{
    return this->mapFromSource(this->source_model->index(row, column, this->mapToSource(parent)));
}

void LocalDirSortFilterModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    // this->source_model = static_cast<QDirModel*>(sourceModel);
    this->source_model = static_cast<QFileSystemModel*>(sourceModel);
    QSortFilterProxyModel::setSourceModel(sourceModel);
}

QString LocalDirSortFilterModel::filePath(const QModelIndex &index) const
{
    return this->source_model->filePath(this->mapToSource(index));
}
QString LocalDirSortFilterModel::fileName(const QModelIndex &index) const
{
    return this->source_model->fileName(this->mapToSource(index));
}
bool LocalDirSortFilterModel::isDir(const QModelIndex &index) const
{
    return this->source_model->isDir(this->mapToSource(index));
}

void LocalDirSortFilterModel::refresh(const QModelIndex & parent)
{
    // this->source_model->refresh(parent);
}

bool LocalDirSortFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    if (this->source_model->isDir(this->source_model->index(source_row, 0, source_parent))) {
        return true;
    } else {
        return false;
    }
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

