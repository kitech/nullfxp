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
// Last-Updated: 日  8月 17 13:29:08 2008 (CST)
//           By: 刘光照<liuguangzhao@users.sf.net>
//     Update #: 2
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

#include "syncdiffermodel.h"
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
    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> >rfiles;
    QHash<QString, int> local_entity;

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
        qDebug()<<"new xxxxxxxxxxxxx begin";
        dname = this->parent->remote_dir + "/" + this->parent->dirs.at(0);
        q_debug()<<dname;
        hsftp = libssh2_sftp_opendir(ssh2_sftp, dname.toAscii().data());
        if(hsftp == NULL) {
            Q_ASSERT(hsftp != NULL);
        }
        remote_list.clear();
        local_list.clear();
        nodes.clear();
        rfiles.clear();
        local_entity.clear();

        //local first
        dname = this->parent->local_dir + "/" + this->parent->dirs.at(0);
        Q_ASSERT(QDir().exists(dname));
        local_list = QDir(dname).entryList();
        //qDebug()<<local_list;
        for(int i = local_list.count() - 1; i >= 0 ; i--) {
            if(local_list.at(i) == "." || local_list.at(i) == "..") {
            }else{
                //local_entity.insert(local_list.at(i), this->parent->ST_LZERO);
                nodes.insert(local_list.at(i), this->parent->ST_LZERO);
            }
        }
        //qDebug()<<local_entity;
        qDebug()<<nodes;

        //remote
        while(libssh2_sftp_readdir(hsftp, fname, sizeof(fname), &ssh2_attr) > 0) {
            if(QString(fname) == "." || QString(fname) == "..") continue;
            //qDebug()<<"rfname: "<< fname;
            LIBSSH2_SFTP_ATTRIBUTES *rf_attrib = (LIBSSH2_SFTP_ATTRIBUTES*)calloc(1, sizeof(LIBSSH2_SFTP_ATTRIBUTES));
            memcpy(rf_attrib, &ssh2_attr, sizeof(LIBSSH2_SFTP_ATTRIBUTES));
            rfiles.append(QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*>(QString(fname), rf_attrib));
        }
        qDebug()<<rfiles;

        for(int i = 0;i <rfiles.count(); i ++) {
            dname = rfiles.at(i).first;
            LIBSSH2_SFTP_ATTRIBUTES *rf_attrib = rfiles.at(i).second;
            if(S_ISDIR(rf_attrib->permissions)) {
                QString ldname = this->parent->local_dir + "/" + this->parent->dirs.at(0);
                QString rname = this->parent->dirs.at(0) + "/" + dname;
                if(QFileInfo(ldname).exists()) {
                    if(QFileInfo(ldname).isDir()) {
                        this->parent->dirs<<rname;
                        nodes.remove(dname);
                    }else{
                        q_debug()<<"remote is dir, but local is not dir???";
                    }
                }else{
                    this->parent->synckeys.append(QPair<QString, int>(rname, this->parent->ST_RZERO));                                          emit found_row();                        
                }
            }else{
                if(nodes.contains(dname)) {                    
                    QFileInfo fi(this->parent->local_dir + "/" + this->parent->dirs.at(0) + "/" + dname);
                    time_t rtime = fi.lastModified().toTime_t();
                    qDebug()<<rtime<<"=?"<<rf_attrib->mtime<<" "<<dname;
                    if(rtime == rf_attrib->mtime) {
                        nodes[dname] = this->parent->ST_LRSAME;
                    }else if(rtime < rf_attrib->mtime) {
                        nodes[dname] = this->parent->ST_RNEW;
                    }else/* if(rtime > rf_attrib->mtime) */{
                        nodes[dname] = this->parent->ST_LNEW;
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
                    //QHash<QString, int> lodir;
                    //lodir.insert("...", it.value());
                    //this->parent->syncer.insert(this->parent->dirs.at(0)+"/"+it.key(), lodir);
                    //lodir.insert(it.key(), it.value());
                    QString path = this->parent->dirs.at(0)+"/"+it.key();
                    this->parent->synckeys.append(QPair<QString, int>(path, it.value()));                   
                    delist<<it.key();
                }
            }
        }
        for(int i = 0; i < delist.count() ; i++) {
            nodes.remove(delist.at(i));
        }
        emit found_row();

        this->parent->syncer.insert(this->parent->dirs.at(0), nodes);
        this->parent->synckeys.append(QPair<QString, int>(this->parent->dirs.at(0), -1));

        libssh2_sftp_closedir(hsftp);
        this->parent->dirs.removeFirst();

        emit found_row();
        qDebug()<<"new xxxxxxxxxxxxx enddddddddddd";
    }
    libssh2_sftp_shutdown(ssh2_sftp);
    libssh2_session_free(ssh2_sess);

    //qDebug()<<this->parent->syncer;
    //qDebug()<<this->parent->synckeys;
    //Q_ASSERT(this->parent->syncer.count() == this->parent->synckeys.count());

    /*
    for(int i = 0; i < this->parent->synckeys.count(); i ++) {
        QString first = this->parent->synckeys.at(i).first;
        int second = this->parent->synckeys.at(i).second;
        if(second == -1) {
            qDebug()<<"=> "<<first;
            QHash<QString, int> elem = this->parent->syncer.value(first);
            QHash<QString, int>::iterator it;
            for(it = elem.begin(); it != elem.end(); it++) {
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
        }else{
            if(second == SynchronizeWindow::ST_LZERO) {
                qDebug()<<"=> "<<first<<"    upload only";
            }else if(second == SynchronizeWindow::ST_RZERO) {
                qDebug()<<"=> "<<first<<"    download only";
            }else{
                Q_ASSERT(1 == 2);
            }            
        }
    }
    */
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

    model = new SyncDifferModel(this);
    this->ui_win.treeView->setModel(model);

    QObject::connect(walker, SIGNAL(found_row()), model, SLOT(maybe_has_data()));
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
    this->ui_win.treeView->expandAll();
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


