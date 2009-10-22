// localdirsortfiltermodel.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2007-11-14 22:54:04
// Version: $Id$
// 

#ifndef LOCALDIRSORTFILTERMODEL_H
#define LOCALDIRSORTFILTERMODEL_H

#include <QDirModel>
#include <QSortFilterProxyModel>

/**
 *
 */
class LocalDirSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT;
public:
    LocalDirSortFilterModel(QObject *parent = 0);
    ~LocalDirSortFilterModel();
    
    QModelIndex index(const QString &path, int column = 0) const;
    QModelIndex index(int& row, int column, QModelIndex& parent) const;
    virtual void setSourceModel(QAbstractItemModel *sourceModel);
            
    QString filePath(const QModelIndex &index) const;
    QString fileName(const QModelIndex &index) const;
    
    bool isDir(const QModelIndex &index) const;
    
public slots:
    void refresh(const QModelIndex &parent = QModelIndex());

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
    QDirModel *source_model; 
};

#endif
