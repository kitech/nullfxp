// remotedirsortfiltermodel.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-08-23 22:11:10 +0000
// Version: $Id$
// 

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

    virtual bool canFetchMore(const QModelIndex &parent) const;    

    virtual QModelIndex index(const QString &path, int column = 0) const;    
    virtual void setSourceModel(QAbstractItemModel *sourceModel);
            
    virtual QString filePath(const QModelIndex &index) const;
    virtual bool isDir(const QModelIndex &index) const;
    virtual bool isSymLink(const QModelIndex &index) const;
    virtual void setFilter(QDir::Filters filters);

    virtual bool hasChildren(const QModelIndex &parent) const;

    virtual QModelIndex	mapToSource ( const QModelIndex & proxyIndex ) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    
protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
protected:
    RemoteDirModel *source_model; 
    QDir::Filters filters;
};


/**
 * @author liuguangzhao <liuguangzhao@users.sf.net>
 */

class RemoteDirSortFilterModelEX : public RemoteDirSortFilterModel
{
    Q_OBJECT;
public:
    RemoteDirSortFilterModelEX(QObject *parent = 0);
    virtual ~RemoteDirSortFilterModelEX();
    
    // 已经继承自 RemoteDirSortFilterModel
    // virtual bool hasChildren(const QModelIndex &parent) const;

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
};

#endif
