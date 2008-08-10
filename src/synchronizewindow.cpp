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
    :QThread(0)
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

    ssh2_sftp = libssh2_sftp_init(ssh2_sess);
    this->parent->dirs<<".";
    this->parent->syncer.clear();
    while(this->parent->dirs.count() > 0) {
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
            nodes.insert(local_list.at(i), this->parent->ST_LZERO);
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
                }else{
                    QHash<QString, int> dodir;
                    dodir.insert("...", this->parent->ST_RZERO);
                    this->parent->syncer.insert(dname, dodir);
                }
            }else{
                if(nodes.contains(QString(fname))) {
                    //checking time stamp
                    QFileInfo fi(this->parent->local_dir + "/" + this->parent->dirs.at(0) + "/" + QString(fname));
                    time_t rtime = fi.lastModified().toTime_t();
                    qDebug()<<rtime<<"=?"<<ssh2_attr.mtime;
                    if(rtime == ssh2_attr.mtime) {
                        nodes[QString(fname)] = this->parent->ST_LRSAME;
                    }else if(rtime < ssh2_attr.mtime) {
                        nodes[QString(fname)] = this->parent->ST_RNEW;
                    }else/* if(rtime > ssh2_attr.mtime) */{
                        nodes[QString(fname)] = this->parent->ST_LNEW;
                    }
                }else{
                    nodes.insert(QString(fname), this->parent->ST_RZERO);
                }
            }
        }

        qDebug()<<nodes;
        QHash<QString, int>::iterator it;
        QStringList delist;
        for(it = nodes.begin(); it != nodes.end(); it++) {
            if(it.value() == SynchronizeWindow::ST_LZERO) {
                if(QFileInfo(this->parent->local_dir+"/"+this->parent->dirs.at(0)+"/"+it.key()).isDir()) {
                    QHash<QString, int> lodir;
                    lodir.insert("...", it.value());
                    this->parent->syncer.insert(this->parent->dirs.at(0)+"/"+it.key(), lodir);
                    //nodes.remove(it.key());
                    delist<<it.key();
                }
            }
        }
        for(int i = 0; i < delist.count() ; i++) {
            nodes.remove(delist.at(0));
        }
        this->parent->syncer.insert(this->parent->dirs.at(0), nodes);

        libssh2_sftp_closedir(hsftp);
        this->parent->dirs.removeFirst();
    }
    libssh2_sftp_shutdown(ssh2_sftp);
    libssh2_session_free(ssh2_sess);

    qDebug()<<this->parent->syncer;
    QHash<QString, QHash<QString, int> >::iterator hit;
    for(hit = this->parent->syncer.begin(); hit != this->parent->syncer.end(); hit++) {
        if(hit.value().count() == 1 && hit.value().begin().key() == "...") {
            if(hit.value().begin().value() == SynchronizeWindow::ST_LZERO) {
                qDebug()<<"=> "<<hit.key()<<"    upload only";
            }else if(hit.value().begin().value() == SynchronizeWindow::ST_RZERO) {
                qDebug()<<"=> "<<hit.key()<<"    download only";
            }else{
                Q_ASSERT(1 == 2);
            }
            continue;
        }
        qDebug()<<"=> "<<hit.key();
        QHash<QString, int> dh = hit.value();
        QHash<QString, int>::iterator it;
        for(it = dh.begin(); it != dh.end(); it++) {
            switch(it.value()) {
            case SynchronizeWindow::ST_LZERO:
                qDebug()<<"        "<<it.key()<<"    upload only";
                break;
            case SynchronizeWindow::ST_RZERO:
                qDebug()<<"        "<<it.key()<<"    download only";
                break;
            case SynchronizeWindow::ST_LRSAME:
                qDebug()<<"        "<<it.key()<<"    the same";
                break;
            case SynchronizeWindow::ST_LNEW:
                qDebug()<<"        "<<it.key()<<"    local is newer";
                break;
            case SynchronizeWindow::ST_RNEW:
                qDebug()<<"        "<<it.key()<<"    remote is newer";
                break;
            default:
                Q_ASSERT(1 == 2);
                break;
            };
        }
    }
}

//////////////////////
/////////////////////
SynchronizeWindow::SynchronizeWindow(QWidget *parent, Qt::WindowFlags flags)
    :QWidget(parent, flags)
{
    this->ui_win.setupUi(this);
    walker = new SyncWalker(this);
    QObject::connect(walker, SIGNAL(status_msg(QString)), this, SLOT(slot_status_msg(QString)));
    QObject::connect(walker, SIGNAL(finished()), this, SLOT(slot_finished()));
    QObject::connect(this->ui_win.toolButton_4, SIGNAL(clicked()), this, SLOT(start()));
    this->running = false;
}

SynchronizeWindow::~SynchronizeWindow()
{
    q_debug()<<"destructured";    
    if(this->walker->isRunning()) {
        q_debug()<<"walker thread is running";
    }
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
void SynchronizeWindow::slot_finished()
{
    this->running = false;
}

void SynchronizeWindow::slot_status_msg(QString msg)
{
    this->ui_win.label->setText(msg);
}

void SynchronizeWindow::closeEvent(QCloseEvent *evt)
{
    //q_debug()<<"";
    this->deleteLater();
    QWidget::closeEvent(evt);
}


