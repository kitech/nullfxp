// syncdiffermodel.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-08-10 21:13:22 +0800
// Last-Updated: 2009-05-17 15:14:03 +0800
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>

#include "libssh2.h"
#include "libssh2_sftp.h"

class SynchronizeWindow;

class SyncDifferModel : public QAbstractItemModel
{
    Q_OBJECT;
public:
    SyncDifferModel(QObject *parent = 0);
    ~SyncDifferModel();

    //5个必须实现的虚函数
    QModelIndex index(int, int, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    QVariant   data(const QModelIndex &index, int role = Qt::DisplayRole ) const;
    int rowCount(const QModelIndex &index) const;
    int columnCount(const QModelIndex &index) const;

    // editable model
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    // other optional
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    //
    bool setDiffFiles(QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > files);
    QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> getFile(const QModelIndex & index) const;

public slots:
    void maybe_has_data();
    void startSyncFile(QString fileName, quint64 fileSize);
    void stopSyncFile(QString fileName, int status);
    void changeTransferedPercent(QString fileName, int percent, quint64 transferdLength, quint64 lastBlockLength);

private:
    int getRowNumberByFileName(QString fileName);

private:
    SynchronizeWindow *sync_win;
    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > mMergedFiles;
    QVector<QString>  mTransferStatus;
};
