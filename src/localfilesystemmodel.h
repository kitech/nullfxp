// localfilesystemmodel.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-06-21 17:41:23 +0000
// Version: $Id$
// 

#ifndef _LOCALFILESYSTEMMODEL_H_
#define _LOCALFILESYSTEMMODEL_H_

#include <QtGui>

// for drop mime data override

class LocalFileSystemModel : public QFileSystemModel
{
    Q_OBJECT;
public:
    LocalFileSystemModel(QObject *parent = 0);
    virtual ~LocalFileSystemModel();

    virtual Qt::DropActions supportedDropActions() const;
    virtual QStringList mimeTypes() const;

public:
    virtual bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent);

signals:
    void sig_drop_mime_data(const QMimeData *data, Qt::DropAction action,
                            int row, int column, const QModelIndex &parent);
};

#endif /* _LOCALFILESYSTEMMODEL_H_ */
