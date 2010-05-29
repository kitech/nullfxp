// sshdirretriver.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-05-25 09:50:13 +0000
// Version: $Id$
// 

#ifndef SSHDIRRETRIVER_H
#define SSHDIRRETRIVER_H

#include <cassert>
#include <vector>
#include <map>

#include <queue>

#include <QtCore>
#include <QThread>

#include "sftp.h"
#include "libssh2.h"
#include "libssh2_sftp.h"

#include "dirretriver.h"

#define SSH2_FXP_KEEP_ALIVE 8888

// class RFSDirNode;
class NetDirNode;
class Connection;

/**
 *
 */
class SSHDirRetriver : public DirRetriver
{
    Q_OBJECT;
public:
    SSHDirRetriver(QObject *parent = 0);
    ~SSHDirRetriver();

    //在实例初始化后马上调用，否则会导致程序崩溃
    void setConnection(Connection *conn);
    // LIBSSH2_SFTP *get_ssh2_sftp();
    
    virtual void run();
        
    void add_node(NetDirNode *parent_item, void *parent_model_internal_pointer);
    
    void slot_execute_command(NetDirNode *parent_item, void *parent_model_internal_pointer,
                              int cmd, QString params);

private:
    class command_queue_elem
    {
    public:
        command_queue_elem()
        {  
            this->parent_item = 0;
            this->parent_model_internal_pointer = 0;
            this->cmd = -1;
            this->retry_times = 0;
        }
               
        NetDirNode *parent_item;
        void *parent_model_internal_pointer;
        int  cmd;
        QString  params;
        int  retry_times;
    };

private:
    int  retrive_dir();
    int  mkdir();
    int  rmdir();
    int  rm_file_or_directory_recursively();  // <==> rm -rf
    int  rm_file_or_directory_recursively_ex(QString parent_path);  // <==> rm -rf
    int  rename();
        
    int keep_alive();
    int fxp_do_ls_dir(QString parent_path, QVector<QMap<char, QString> > & fileinfos);
    int fxp_do_ls_dir(QString parent_path, QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > & fileinfos);

    int fxp_realpath();

signals:
    void enter_remote_dir_retrive_loop();
    void leave_remote_dir_retrive_loop();
        
    void remote_dir_node_retrived(NetDirNode *parent_item, void *parent_model_internal_pointer);
        
    void execute_command_finished(NetDirNode *parent_item, void *parent_model_internal_pointer,
                                  int cmd, int status);

private:

    std::map<NetDirNode *, void *> dir_node_process_queue;
    std::vector<command_queue_elem*>  command_queue;
};

#endif
