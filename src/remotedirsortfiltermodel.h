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
#ifndef REMOTEDIRSORTFILTERMODEL_H
#define REMOTEDIRSORTFILTERMODEL_H

#include <QtCore>
#include <QtGui>

class RemoteDirModel ;
/**
	@author liuguangzhao <liuguangzhao@users.sourceforge.net>
*/
class RemoteDirSortFilterModel : public QSortFilterProxyModel
{
Q_OBJECT
public:
    RemoteDirSortFilterModel(QObject *parent = 0);

    virtual ~RemoteDirSortFilterModel();
    
    virtual QModelIndex index ( const QString & path, int column = 0 ) const;
    
    virtual void setSourceModel ( QAbstractItemModel * sourceModel );
            
    virtual QString filePath(const QModelIndex &index) const;
    virtual bool isDir(const QModelIndex &index) const;
    
    virtual void setFilter ( QDir::Filters filters );
    
    protected:
        virtual bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const;
    protected:
        RemoteDirModel * source_model; 
        QDir::Filters filters;
};


/**
	@author liuguangzhao <liuguangzhao@users.sourceforge.net>
 */
class RemoteDirSortFilterModelEX : public RemoteDirSortFilterModel
{
    Q_OBJECT
    public:
        RemoteDirSortFilterModelEX(QObject *parent = 0);

        virtual ~RemoteDirSortFilterModelEX();
    
//         QModelIndex index ( const QString & path, int column = 0 ) const;
//     
//         virtual void setSourceModel ( QAbstractItemModel * sourceModel );
//             
//         QString filePath(const QModelIndex &index) const;
//         bool isDir(const QModelIndex &index) const;
//     
//         void setFilter ( QDir::Filters filters );
    
    protected:
        virtual bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const;
//     private:
//         RemoteDirModel * source_model; 
//         QDir::Filters filters;
};

#endif
