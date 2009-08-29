// sftpdirmodel.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-07-27 09:20:38 +0800
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>
#include <QFileIconProvider>

class SftpDirModel : public QAbstractItemModel
{
    Q_OBJECT;
public:
    SftpDirModel(QObject *parent = 0);
    ~SftpDirModel();

    // QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    // QModelIndex parent(const QModelIndex &child) const;

    // int rowCount(const QModelIndex &parent = QModelIndex()) const;
    // int columnCount(const QModelIndex &parent = QModelIndex()) const;

    // QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    // bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    // QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    // bool hasChildren(const QModelIndex &index = QModelIndex()) const;
    // Qt::ItemFlags flags(const QModelIndex &index) const;

    // void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    // QStringList mimeTypes() const;
    // QMimeData *mimeData(const QModelIndexList &indexes) const;
    // bool dropMimeData(const QMimeData *data, Qt::DropAction action,
    //                   int row, int column, const QModelIndex &parent);
    // Qt::DropActions supportedDropActions() const;

    // SftpDirModel specific API

};
