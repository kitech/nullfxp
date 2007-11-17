/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   liuguangzhao@users.sourceforge.net   *
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

#include "remotedirmodel.h"
#include "remotedirsortfiltermodel.h"

RemoteDirSortFilterModel::RemoteDirSortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    this->setFilterKeyColumn( 2 );
    this->setFilterRegExp(QRegExp("^(d|l).*",Qt::CaseInsensitive));
}


RemoteDirSortFilterModel::~RemoteDirSortFilterModel()
{
}
QModelIndex RemoteDirSortFilterModel::index ( const QString & path, int column  ) const
{
    return this->source_model->index( path , column );
}
void RemoteDirSortFilterModel::setSourceModel ( QAbstractItemModel * sourceModel )
{
    this->source_model = static_cast<RemoteDirModel*>(sourceModel) ;
    QSortFilterProxyModel::setSourceModel(sourceModel);
}

QString RemoteDirSortFilterModel::filePath ( const QModelIndex &index ) const
{
    return this->source_model->filePath( this->mapToSource(index) );
}
bool RemoteDirSortFilterModel::isDir ( const QModelIndex &index ) const
{
    return this->source_model->isDir( this->mapToSource(index) );
}

