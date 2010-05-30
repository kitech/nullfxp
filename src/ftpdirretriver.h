// ftpdirretriver.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-05-25 09:50:13 +0000
// Version: $Id$
// 

#ifndef _FTPDIRRETRIVER_H_
#define _FTPDIRRETRIVER_H_

#include <cassert>
#include <QtCore>
#include <QThread>

#include "dirretriver.h"

// class RFSDirNode;
class NetDirNode;
class Connection;

/**
 * ftp dir and file list retrive thread
 */
class FTPDirRetriver : public DirRetriver
{
    Q_OBJECT;
public:
    FTPDirRetriver(QObject *parent = 0);
    virtual ~FTPDirRetriver();

    //在实例初始化后马上调用，否则会导致程序崩溃
    // void set_ssh2_handler(void *ssh2_sess);
    virtual void setConnection(Connection *conn);
    // LIBSSH2_SFTP *get_ssh2_sftp();
    
    virtual void run();
        
    virtual void add_node(NetDirNode *parent_item, void *parent_persistent_index);
    
    virtual void slot_execute_command(NetDirNode *parent_item, void *parent_persistent_index,
                              int cmd, QString params);

protected:
    // class command_queue_elem
    // {
    // public:
    //     command_queue_elem()
    //     {  
    //         this->parent_item = 0;
    //         this->parent_model_internal_pointer = 0;
    //         this->cmd = -1;
    //         this->retry_times = 0;
    //     }
               
    //     NetDirNode *parent_item;
    //     void *parent_model_internal_pointer;
    //     int  cmd;
    //     QString  params;
    //     int  retry_times;
    // };

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
        
    void remote_dir_node_retrived(NetDirNode *parent_item, void *parent_persistent_index);
        
    void execute_command_finished(NetDirNode *parent_item, void *parent_persistent_index,
                                  int cmd, int status);

};

#endif /* _FTPDIRRETRIVER_H_ */
