// synchronizewindow.cpp --- 
// 
// Filename: synchronizewindow.cpp
// Description: 
// Author: 刘光照<liuguangzhao@users.sf.net>
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 五  8月  8 13:44:42 2008 (CST)
// Version: 
// Last-Updated: 日  8月 10 11:57:41 2008 (CST)
//           By: 刘光照<liuguangzhao@users.sf.net>
//     Update #: 1
// URL: 
// Keywords: 
// Compatibility: 
// 
// 

// Commentary: 
// 
// 
// 
// 

// Change log:
// 
// 
// 

#include "utils.h"
#include "remotehostconnectthread.h"
#include "basestorage.h"
#include "synchronizewindow.h"

SyncWalker::SyncWalker(QObject *parent)
    :QThread(parent)
{
    this->parent = static_cast<SynchronizeWindow*>(parent);
}
SyncWalker::~SyncWalker()
{
    q_debug()<<"destructured";
}
/*
 */
void SyncWalker::run()
{
    LIBSSH2_SESSION *ssh2_sess = NULL;
    LIBSSH2_SFTP *ssh2_sftp = NULL;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_attr;
    LIBSSH2_SFTP_HANDLE *hsftp = NULL;

    char fname[512];
    QString dname;
    QStringList local_list, remote_list;    
    QHash<QString, int> nodes;

    QMap<QString, QString> host;
    BaseStorage *storage = BaseStorage::instance();
    storage->open();
    host = storage->getHost(this->parent->sess_name);
    q_debug()<<host;
    q_debug()<<this->parent->local_dir;
    
    RemoteHostConnectThread *conn 
        = new RemoteHostConnectThread(host["user_name"], host["password"], host["host_name"],
                                      host["port"].toShort(), host["pubkey"]);
    conn->run();
    ssh2_sess = (LIBSSH2_SESSION*)conn->get_ssh2_sess();
    delete conn; conn = NULL;

    q_debug()<<"here?";
    
    ssh2_sftp = libssh2_sftp_init(ssh2_sess);
    this->parent->dirs<<".";
    q_debug()<<"here?";
    while(this->parent->dirs.count() > 0) {
        q_debug()<<"here?";
        dname = this->parent->remote_dir + "/" + this->parent->dirs.at(0);
        q_debug()<<dname;
        hsftp = libssh2_sftp_opendir(ssh2_sftp, dname.toAscii().data());
        if(hsftp == NULL) {
            assert(hsftp != NULL);
        }
        remote_list.clear();
        local_list.clear();
        nodes.clear();

        //local first
        dname = this->parent->local_dir + "/" + this->parent->dirs.at(0);
        assert(QDir().exists(dname));
        local_list = QDir(dname).entryList();
        qDebug()<<local_list;
        for(int i = 0; i < local_list.count(); i++) {
            if(local_list.at(i) == "." || local_list.at(i) == "..") continue;
            nodes.insert(local_list.at(i), 0);
        }

        //remote
        while(libssh2_sftp_readdir(hsftp, fname, sizeof(fname), &ssh2_attr) > 0) {
            if(QString(fname) == "." || QString(fname) == "..") continue;
            qDebug()<<"rfname: "<< fname;
            //remote_list<<QString(fname);
            if(S_ISDIR(ssh2_attr.permissions)) {
                dname = (this->parent->dirs.at(0) + "/" + QString(fname));
                if(QDir().exists(this->parent->local_dir + "/" + QString(fname))) {
                    this->parent->dirs<<dname;
                }
            }
            if(nodes.contains(QString(fname))) {
                nodes[QString(fname)] = 1;
            }else{
                nodes.insert(QString(fname), 0);
            }
        }

        qDebug()<<nodes;

        libssh2_sftp_closedir(hsftp);
        q_debug()<<"here?";
        this->parent->dirs.removeFirst();
    }
    libssh2_sftp_shutdown(ssh2_sftp);
    libssh2_session_free(ssh2_sess);

    q_debug()<<"here?";
}

//////////////////////
/////////////////////
SynchronizeWindow::SynchronizeWindow(QWidget *parent, Qt::WindowFlags flags)
    :QWidget(parent, flags)
{
    this->ui_win.setupUi(this);
    walker = new SyncWalker(this);
    QObject::connect(walker, SIGNAL(status_msg(QString)), this, SLOT(slot_status_msg(QString)));
    QObject::connect(this->ui_win.toolButton_4, SIGNAL(clicked()), this, SLOT(start()));
    this->running = false;
}

SynchronizeWindow::~SynchronizeWindow()
{
    delete this->walker;
}

void SynchronizeWindow::set_sync_param(QString local_dir, QString sess_name, QString remote_dir, bool recursive, int way)
{
    this->local_dir = local_dir;
    this->sess_name = sess_name;
    this->remote_dir = remote_dir;
    this->recursive = recursive;
    this->way = way;
}

void SynchronizeWindow::start()
{
    if(!this->running) {
        this->running = true;
        this->walker->start();
    }
}
void SynchronizeWindow::stop()
{
}

void SynchronizeWindow::slot_status_msg(QString msg)
{
    this->ui_win.label->setText(msg);
}




