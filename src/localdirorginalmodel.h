// localdirorginalmodel.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-08-29 16:22:54 +0800
// Version: $Id$
// 

#ifndef LOCALDIRFILEMODEL_H
#define LOCALDIRFILEMODEL_H

#include <QtCore>
#include <QtGui>

// #include <QDirModel>

/**
	@author liuguangzhao <liuguangzhao@users.sf.net>
*/
// class LocalDirOrginalModel : public QDirModel
class LocalDirOrginalModel : public QFileSystemModel
{
    Q_OBJECT;
public:
    LocalDirOrginalModel(QObject *parent = 0);

    virtual ~LocalDirOrginalModel();

};

#endif
