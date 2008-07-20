/* remotedirsortfiltermodel.h --- 
 * 
 * Filename: remotedirsortfiltermodel.h
 * Description: 
 * Author: liuguangzhao
 * Maintainer: 
 * Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 五  5月 23 23:11:10 2008 (CST)
 * Version: 
 * Last-Updated: 
 *           By: 
 *     Update #: 0
 * URL: 
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change log:
 * 
 * 
 */

#ifndef REMOTEDIRSORTFILTERMODEL_H
#define REMOTEDIRSORTFILTERMODEL_H

#include <QtCore>
#include <QtGui>

class RemoteDirModel ;

class RemoteDirSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT;
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
    Q_OBJECT;
public:
    RemoteDirSortFilterModelEX(QObject *parent = 0);
    virtual ~RemoteDirSortFilterModelEX();
    
protected:
    virtual bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const;
};

#endif
