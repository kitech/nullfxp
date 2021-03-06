// remotedirmodel.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-05-25 09:48:00 +0000
// Version: $Id$
// 


#ifndef REMOTEDIRMODEL_H
#define REMOTEDIRMODEL_H

#include <QtCore>
#include <QtGui>
#include <QAbstractItemModel>

#include "libssh2.h"
#include "libssh2_sftp.h"

#include "sshdirretriver.h"

class RFSDirNode;
class NetDirNode;
class Connection;
class DirRetriver;

/**
 * @author liuguangzhao <liuguangzhao@users.sf.net >
 *
 * 这个model必须做成lazy模式的，因为它是远程目录，只有需要显示的时候才去取来远程目录结构
 */

class RemoteDirModel : public QAbstractItemModel
{
    Q_OBJECT;
public:
    RemoteDirModel(QObject *parent = 0);
    virtual ~RemoteDirModel();

    //仅需要调用一次的函数,并且是在紧接着该类的初始化之后调用。
    void set_user_home_path(QString user_home_path);
    //这个调用应该在set_user_home_path之前
    void setConnection(Connection *conn);
                
    ////model 函数
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(const QString & path, int column = 0) const; // 效率可能有问题
        
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
        
    bool rowMoveTo(const QModelIndex &from, const QModelIndex &to);
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
        
    bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
		
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent);
    Qt::DropActions supportedDropActions() const;

    QString filePath(const QModelIndex &index) const;
    QString fileName(const QModelIndex &index) const;
    bool isDir(const QModelIndex &index) const;
    bool isSymLink(const QModelIndex &index) const;
    bool isSymLinkToDir(const QModelIndex &index) const;

    void dump_tree_node_item(NetDirNode *node_item) const;
public slots:
    void slot_remote_dir_node_retrived(NetDirNode *parent_item, void *parent_persistent_index, NetDirNode *newNodes);
    void slot_remote_dir_node_clicked(const QModelIndex &index);
        
    void slot_execute_command(NetDirNode *parent_item, void *parent_persistent_index, int cmd, QString params );

    void execute_command_finished(NetDirNode *parent_item, void *parent_persistent_index,
                                  int cmd, int status);
    //keep_alive
    void set_keep_alive(bool keep_alive, int time_out = DEFAULT_KEEP_ALIVE_TIMEOUT);

private slots:        
    /// time_out 秒                
    void slot_keep_alive_time_out();

signals:
    void sig_drop_mime_data(const QMimeData *data, Qt::DropAction action,
                            int row, int column, const QModelIndex &parent);
        
    //for wait option
    void enter_remote_dir_retrive_loop();
    void leave_remote_dir_retrive_loop();
    void directoryLoaded(const QString &path);
    void operationTriggered(QString text) const; // for rowCount method
    // see http://lists.trolltech.com/qt-interest/2002-09/thread00073-0.html
    
private:
    enum { DEFAULT_KEEP_ALIVE_TIMEOUT = 30*1000 };
    NetDirNode *rootNode;
    LIBSSH2_SESSION *ssh2_sess;
    Connection *conn;

    DirRetriver *dir_retriver;

    // 递归查找树
    QModelIndex find_node_item_by_path_elements(NetDirNode *parent_node_item,
                                                QStringList &path_elements, int level) const;


    QString user_home_path;
        
    bool    keep_alive;
    QTimer  *keep_alive_timer;
    int     keep_alive_interval;
};

#endif
