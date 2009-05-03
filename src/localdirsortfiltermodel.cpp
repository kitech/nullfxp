/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   liuguangzhao@users.sf.net   *
 *
 *   http://www.qtchina.net                                                *
 *   http://nullget.sourceforge.net                                        *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

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

QModelIndex LocalDirSortFilterModel::index ( const QString & path, int column  ) const
{
    return this->mapFromSource(this->source_model->index( path , column ));
}
QModelIndex LocalDirSortFilterModel::index(int& row, int column, QModelIndex& parent) const
{
    return this->mapFromSource(this->source_model->index(row, column, this->mapToSource(parent)));
}

void LocalDirSortFilterModel::setSourceModel ( QAbstractItemModel * sourceModel )
{
    this->source_model = static_cast<QDirModel*>(sourceModel) ;
    QSortFilterProxyModel::setSourceModel(sourceModel);
}

QString LocalDirSortFilterModel::filePath ( const QModelIndex &index ) const
{
    return this->source_model->filePath( this->mapToSource(index) );
}
QString LocalDirSortFilterModel::fileName ( const QModelIndex &index ) const
{
    return this->source_model->filePath( this->mapToSource(index) );
}
bool LocalDirSortFilterModel::isDir ( const QModelIndex &index ) const
{
    return this->source_model->isDir( this->mapToSource(index) );
}

void LocalDirSortFilterModel::refresh ( const QModelIndex & parent  )
{
    this->source_model->refresh(parent);
}

bool LocalDirSortFilterModel::filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    if( this->source_model->isDir( this->source_model->index(source_row, 0, source_parent)))
        return true;
    else 
        return false;
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

