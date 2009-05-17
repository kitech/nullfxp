// syncdiffermodel.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-08-10 21:13:15 +0800
// Last-Updated: 2009-05-17 16:34:18 +0800
// Version: $Id$
// 

#include "utils.h"

#include "synchronizewindow.h"
#include "syncdiffermodel.h"

SyncDifferModel::SyncDifferModel(QObject *parent)
    :QAbstractItemModel(parent)
{
    this->sync_win = static_cast<SynchronizeWindow*>(parent);
}

SyncDifferModel::~SyncDifferModel()
{
    q_debug()<<"destructured";
}

QModelIndex SyncDifferModel::index(int row, int column, const QModelIndex &parent) const
{
    //q_debug()<<row<<","<<column<<" "<<parent;
    assert(row >= 0);

    QModelIndex idx = QModelIndex();
    if (!parent.isValid()) {
        idx = this->createIndex(row, column, -1);
    } else {
        //idx = this->createIndex(row, column, parent.row());
        //qDebug()<<"create sub model"<<parent.row();
        q_debug()<<"not possible now";
    }
    return idx;
}
QModelIndex SyncDifferModel::parent(const QModelIndex &index) const
{
    //q_debug()<<""<<index;    
    QModelIndex idx = QModelIndex();
    return idx;

    if (!index.isValid()) {

    } else {
        //idx = index((int)(index.internalPointer()), 0, idx);
        //idx = createIndex(0,0,0);
        int row = index.internalId();
        if (row == -1) {
        } else {
            assert(row != -1);
            idx = this->index(row, 0, idx);        
        }
    }
    return idx;
}

QVariant   SyncDifferModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        assert(1 == 2);
    }

    //q_debug()<<""<<index;
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    LIBSSH2_SFTP_ATTRIBUTES * attr = NULL;
    
    switch (index.column()) {
    case 0:
        return this->mMergedFiles.at(index.row()).first;
        break;
    case 1:
        attr = this->mMergedFiles.at(index.row()).second;
        return this->sync_win->diffDesciption(attr->flags);
        break;
    case 2:
        return this->mTransferStatus.at(index.row());
        break;
    default:
        q_debug()<<"this is impossible:"<<index.row();
        break;
    };
    
    return QVariant();
}
int SyncDifferModel::rowCount(const QModelIndex &index) const
{
    //q_debug()<<""<<index;

    if (!index.isValid()) {
        //return this->sync_win->synckeys.count();
        return this->mMergedFiles.count();
    } else {
        return 0;
    }
    return 0;
}
int SyncDifferModel::columnCount(const QModelIndex &index) const
{
    //q_debug()<<""<<index;
    if(!index.isValid()) {
        return 3;
    }else{
        return 3;
    }
    return 0;
}
bool SyncDifferModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }
    if (role == Qt::EditRole) {        
    }
    
    if (index.column() == 2) {
        this->mTransferStatus[index.row()] = value.toString();
        emit dataChanged(index, index);
    } else {
        q_debug()<<"Not impled";
        return true;
    }

    return true;
}

QVariant SyncDifferModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);

    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    switch (section) {
    case 0:
        return QString(tr("File Name"));
        break;
    case 1:
        return QString(tr("Diff status"));
        break;
    case 2:
        return QString(tr("Sync status"));
        break;
    default:
        break;
    };
    return QVariant();
}

void SyncDifferModel::maybe_has_data()
{
    qDebug()<<"aaaaaaaaaa";
    emit layoutChanged ();
}

bool SyncDifferModel::setDiffFiles(QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > files)
{
    this->mMergedFiles = files;
    this->mTransferStatus.resize(files.count());
    
    return true;
}

QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> SyncDifferModel::getFile(const QModelIndex & index) const
{
    QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> file;
    if (index.row() >= this->rowCount(QModelIndex())) {
        q_debug()<<"Invalid index";
    } else {
        file = this->mMergedFiles.at(index.row());
    }
    return file;
}

int SyncDifferModel::getRowNumberByFileName(QString fileName)
{
    int rowno = 0;
    int rowCount = this->rowCount(QModelIndex());

    for (int i = 0 ; i < rowCount; i++) {
        if (this->mMergedFiles.at(i).first == fileName) {
            rowno = i;
            break;
        }
    }

    return rowno;
}

void SyncDifferModel::startSyncFile(QString fileName, quint64 fileSize)
{
    int rowno = this->getRowNumberByFileName(fileName);
    QModelIndex idx = this->index(rowno, 2, QModelIndex());
    QString statusLine = QString(tr("Starting sync %1 Bytes ...")).arg(fileSize);
    this->setData(idx, statusLine);
}
void SyncDifferModel::stopSyncFile(QString fileName, int status)
{
    Q_UNUSED(status);

    int rowno = this->getRowNumberByFileName(fileName);
    QModelIndex idx = this->index(rowno, 2, QModelIndex());
    QString statusLine = QString(tr("Sync success"));
    this->setData(idx, statusLine);
}
void SyncDifferModel::changeTransferedPercent(QString fileName, int percent, 
                                              quint64 transferdLength, quint64 lastBlockLength)
{
    Q_UNUSED(transferdLength);
    Q_UNUSED(lastBlockLength);

    int rowno = this->getRowNumberByFileName(fileName);
    QModelIndex idx = this->index(rowno, 2, QModelIndex());
    QString statusLine = QString(tr("Synced %1% ...")).arg(percent);
    this->setData(idx, statusLine);    
}

