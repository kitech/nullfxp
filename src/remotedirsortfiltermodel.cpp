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
//     this->setFilterKeyColumn( 2 );
//     this->setFilterRegExp(QRegExp("^(d|l).*",Qt::CaseInsensitive));
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
    return this->source_model->filePath( this->mapToSource(index) );
}
bool RemoteDirSortFilterModel::isDir ( const QModelIndex &index ) const
{
    return this->source_model->isDir( this->mapToSource(index) );
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
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    this->filters = filters;
    emit layoutAboutToBeChanged();
    //this->source_model->slot_remote_dir_node_clicked(this->source_model->index(0,0,QModelIndex()) );
    directory_tree_item * dti = static_cast<directory_tree_item*>(this->source_model->index(0,0,QModelIndex()) .internalPointer());
    //qDebug()<<dti->file_name<<" "<<dti->file_type<<" "<< dti->strip_path ;
    //file_path = dti->strip_path ;
    dti->retrived = 1;
    dti->prev_retr_flag=9;
    this->source_model->slot_remote_dir_node_clicked(this->source_model->index(0,0,QModelIndex()) );
    
    emit layoutChanged();
}

//////////////////////
///////  EX
//////////////////////////

RemoteDirSortFilterModelEX::RemoteDirSortFilterModelEX(QObject *parent)
    : RemoteDirSortFilterModel(parent)
{
//     this->setFilterKeyColumn( 2 );
//     this->setFilterRegExp(QRegExp("^(d|l).*",Qt::CaseInsensitive));
}


RemoteDirSortFilterModelEX::~RemoteDirSortFilterModelEX()
{
}
// QModelIndex RemoteDirSortFilterModelEX::index ( const QString & path, int column  ) const
// {
//     return this->source_model->index( path , column );
// }
// void RemoteDirSortFilterModelEX::setSourceModel ( QAbstractItemModel * sourceModel )
// {
//     this->source_model = static_cast<RemoteDirModel*>(sourceModel) ;
//     RemoteDirSortFilterModel::setSourceModel(sourceModel);
// }
// 
// QString RemoteDirSortFilterModelEX::filePath ( const QModelIndex &index ) const
// {
//     return this->source_model->filePath( this->mapToSource(index) );
// }
// bool RemoteDirSortFilterModelEX::isDir ( const QModelIndex &index ) const
// {
//     return this->source_model->isDir( this->mapToSource(index) );
// }


bool RemoteDirSortFilterModelEX::filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<this->filters;
        if( this->source_model->isDir( this->source_model->index(source_row, 0, source_parent)))
        {
            //QString file_name = this->source_model->data(this->source_model->index(source_row, 0, source_parent), Qt::DisplayRole).toString();
        //qDebug()<<this->source_model->data(this->source_model->index(source_row, 0, source_parent), Qt::DisplayRole).toString();
//             if(file_name.at(0) == '.') return true;
//             else return false;
//             return true;
            return RemoteDirSortFilterModel::filterAcceptsRow(source_row, source_parent);
        }else{
            return false;
        }
//     if(this->filters &  QDir::Hidden )
//     {
//         //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
//         if( this->source_model->isDir( this->source_model->index(source_row, 0, source_parent)))
//         {
//             //QString file_name = this->source_model->data(this->source_model->index(source_row, 0, source_parent), Qt::DisplayRole).toString();
//         //qDebug()<<this->source_model->data(this->source_model->index(source_row, 0, source_parent), Qt::DisplayRole).toString();
// //             if(file_name.at(0) == '.') return true;
// //             else return false;
//             return true;
//         }else{
//             return false;
//         }
// 
//     }else{
//         //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
//         if( this->source_model->isDir( this->source_model->index(source_row, 0, source_parent)))
//         {
//             QString file_name = this->source_model->data(this->source_model->index(source_row, 0, source_parent), Qt::DisplayRole).toString();
//             //qDebug()<<this->source_model->data(this->source_model->index(source_row, 0, source_parent), Qt::DisplayRole).toString();
//             if(file_name.at(0) == '.') return false;
//             else return true;
//         }
//         else{ 
//             return false;
//         }
//     }

    return RemoteDirSortFilterModel::filterAcceptsRow(source_row, source_parent);
}

// void RemoteDirSortFilterModelEX::setFilter ( QDir::Filters filters )
// {
//     qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
//     
//     this->filters = filters;
//     emit layoutAboutToBeChanged();
//     //this->source_model->slot_remote_dir_node_clicked(this->source_model->index(0,0,QModelIndex()) );
//     directory_tree_item * dti = static_cast<directory_tree_item*>(this->source_model->index(0,0,QModelIndex()) .internalPointer());
//     //qDebug()<<dti->file_name<<" "<<dti->file_type<<" "<< dti->strip_path ;
//     //file_path = dti->strip_path ;
//     dti->retrived = 1;
//     dti->prev_retr_flag=9;
//     this->source_model->slot_remote_dir_node_clicked(this->source_model->index(0,0,QModelIndex()) );
//     
//     emit layoutChanged();
// }


