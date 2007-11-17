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
#ifndef LOCALDIRSORTFILTERMODEL_H
#define LOCALDIRSORTFILTERMODEL_H

#include <QDirModel>
#include <QSortFilterProxyModel>

/**
	@author liuguangzhao <liuguangzhao@users.sourceforge.net>
*/
class LocalDirSortFilterModel : public QSortFilterProxyModel
{
Q_OBJECT
public:
    LocalDirSortFilterModel(QObject *parent = 0);

    ~LocalDirSortFilterModel();
    
    QModelIndex index ( const QString & path, int column = 0 ) const;
    QModelIndex index(int& row, int column, QModelIndex& parent) const;
    virtual void setSourceModel ( QAbstractItemModel * sourceModel );
            
    QString filePath(const QModelIndex &index) const;
    QString fileName(const QModelIndex &index) const;
    
    bool isDir(const QModelIndex &index) const;
    
    public slots:
        void refresh ( const QModelIndex & parent = QModelIndex() );
    protected:
        virtual bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const;
    private:
        QDirModel * source_model; 

};

#endif
