// dirretriver.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-05-25 09:50:13 +0000
// Version: $Id$
// 

#ifndef DIRRETRIVER_H
#define DIRRETRIVER_H

#include <cassert>

#include <QtCore>
#include <QThread>

#include "sftp-const.h"
#include "libssh2.h"
#include "libssh2_sftp.h"

#define SSH2_FXP_KEEP_ALIVE 8888

// class RFSDirNode;
// class directory_tree_item;
class NetDirNode;
class Connection;

/**
 *
 */
class DirRetriver : public QThread
{
    Q_OBJECT;
public:
    DirRetriver(QObject *parent = 0);
    virtual ~DirRetriver();

    //在实例初始化后马上调用，否则会导致程序崩溃
    virtual void setConnection(Connection *conn);
    
    virtual void run();
        
    virtual void add_node(NetDirNode *parent_item, void *parent_persistent_index);
    
    virtual void slot_execute_command(NetDirNode *parent_item, void *parent_persistent_index,
                              int cmd, QString params);

protected:
    class command_queue_elem
    {
    public:
        command_queue_elem()
        {  
            this->parent_item = 0;
            this->parent_persistent_index = 0;
            this->cmd = -1;
            this->retry_times = 0;
        }
               
        NetDirNode *parent_item;
        void *parent_persistent_index;
        int  cmd;
        QString  params;
        int  retry_times;
    };

protected:
    virtual int  retrive_dir();
    virtual int  mkdir();
    virtual int  rmdir();
    virtual int  rm_file_or_directory_recursively();  // <==> rm -rf
    virtual int  rm_file_or_directory_recursively_ex(QString parent_path);  // <==> rm -rf
    virtual int  rename();
        
    virtual int keep_alive();
    virtual int fxp_do_ls_dir(QString parent_path, QVector<QMap<char, QString> > & fileinfos);
    virtual int fxp_do_ls_dir(QString parent_path, QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > & fileinfos);

    virtual int fxp_realpath();

signals:
    void enter_remote_dir_retrive_loop();
    void leave_remote_dir_retrive_loop();
        
    void remote_dir_node_retrived(NetDirNode *parent_item, void *parent_persistent_index, NetDirNode *newNodes);
        
    void execute_command_finished(NetDirNode *parent_item, void *parent_persistent_index,
                                  int cmd, int status);

protected:

    std::map<NetDirNode *, void *> dir_node_process_queue;
    std::vector<command_queue_elem*>  command_queue;
       
    LIBSSH2_SESSION *ssh2_sess;
    LIBSSH2_SFTP *ssh2_sftp;
    Connection *conn;
};

#endif
