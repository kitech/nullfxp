// localdirfilemodel.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-06-16 17:39:34 +0800
// Version: $Id$
// 

#ifndef LOCALDIRFILEMODEL_H
#define LOCALDIRFILEMODEL_H

#include <QtCore>
#include <QtGui>

// #include <QDirModel>

// class LocalDirFileModel : public QDirModel
class LocalDirFileModel : public QFileSystemModel
{
    Q_OBJECT;
public:
    LocalDirFileModel(QObject *parent = 0);
    virtual ~LocalDirFileModel();

};

#endif
