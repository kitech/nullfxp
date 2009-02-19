// synctransferthread.cpp --- 
// 
// Filename: synctransferthread.cpp
// Description: 
// Author: 刘光照<liuguangzhao@users.sf.net>
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 日  1月 11 10:19:22 2009 (CST)
// Version: 
// Last-Updated: 
//           By: 
//     Update #: 0
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

#include "synctransferthread.h"

SyncTransferThread::SyncTransferThread(QObject *parent)
    : QThread(parent)
{
    ssh2_sess = NULL;
    ssh2_sftp = NULL;
    hsftp = NULL;    
}
SyncTransferThread::~SyncTransferThread()
{
}

void SyncTransferThread::run()
{
    QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> task;
    int rc ;
    while (true) {
        this->wcMutex.lock();
        q_debug()<<"before waiting...";
        this->wc.wait(&this->wcMutex);
        q_debug()<<"after waiting...";
        
        while (!this->taskQueue.empty()) {
            this->tMutex.lock();
            task = this->taskQueue.dequeue();
            this->tMutex.unlock();
            
            rc = this->transfer_impl(task); 
            if (rc == TASK_ERROR_CONN_FAILD) {
                q_debug()<<"conn error";
                this->wcMutex.unlock();
                goto out_thread;
            }
            if (rc == TASK_ERROR_OTHER) {
                q_debug()<<"other error";
            }
            if (rc == TASK_OK) {
            }
        }
        this->wcMutex.unlock();
    }
 out_thread:
    return ;
}

int SyncTransferThread::transfer_impl(QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> task)
{
    if (!this->ssh2_sess) {
        if (!this->connectToRemoteHost()) {
            return TASK_ERROR_CONN_FAILD;
        }
    }
    
    
    
    return TASK_OK;
}

bool SyncTransferThread::setRemoteSession(QString sess)
{
    this->sess_name = sess;
    this->start();
    return true;
}
bool SyncTransferThread::addTask(QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> task)
{
    if (task.second != NULL) {
        this->tMutex.lock();
        this->taskQueue.enqueue(task);
        this->tMutex.unlock();
        this->wc.wakeOne();
    } else {
        q_debug()<<"Invalid task:"<<task;
    }
    return true;
}

bool SyncTransferThread::connectToRemoteHost()
{
    //this->remoteBasePath = this->parent->remote_dir;

    QMap<QString, QString> host;
    BaseStorage *storage = BaseStorage::instance();
    storage->open();
    this->remoteHost = host = storage->getHost(sess_name);
    q_debug()<<host;
    
    RemoteHostConnectThread *conn 
        = new RemoteHostConnectThread(host["user_name"], host["password"], host["host_name"],
                                      host["port"].toShort(), host["pubkey"]);
    conn->run();
    ssh2_sess = (LIBSSH2_SESSION*)conn->get_ssh2_sess();
    delete conn; conn = NULL;
    Q_ASSERT(ssh2_sess != NULL);

    ssh2_sftp = libssh2_sftp_init(ssh2_sess);
    
    if (ssh2_sftp != NULL) {
        Q_ASSERT(ssh2_sftp != NULL);
        q_debug()<<"connect ok";
        return true;
    }
    
    return false;
}

