// forwardmanager.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-06-29 17:38:52 +0000
// Version: $Id$
// 

#include "simplelog.h"
#include "basestorage.h"

#include "connector.h"
#include "sshconnection.h"

#include "ui_forwardmanager.h"
#include "forwardmanager.h"
#include "forwardportworker.h"

ForwardManager::ForwardManager(QWidget *parent)
    : QDialog(parent)
    , uiw(new Ui::ForwardManager())
{
    this->uiw->setupUi(this);

    QObject::connect(this->uiw->toolButton_2, SIGNAL(clicked()), this, SLOT(slot_show_session_menu()));

    QObject::connect(this->uiw->pushButton_6, SIGNAL(clicked()), this, SLOT(slot_forward_connect_start()));
}

ForwardManager::~ForwardManager()
{
}

void ForwardManager::slot_show_session_menu()
{
    qLogx()<<"";
    QMenu *popmenu = new QMenu(this);
    QObject::connect(popmenu, SIGNAL(aboutToHide()), this, SLOT(slot_session_list_menu_hide()));

    QAction *action;

    BaseStorage * storage = BaseStorage::instance();
    
    QStringList nlist = storage->getNameList();

    for(int i = 0; i< nlist.count(); i++) {
        action = new QAction(nlist.at(i), this);
        QObject::connect(action, SIGNAL(triggered()), 
                         this, SLOT(slot_session_item_selected()));
        popmenu->addAction(action);        
    }

    QPoint pos = this->uiw->toolButton_2->pos();
    pos = this->mapToGlobal(pos);
    pos.setX(pos.x() + 35);
    popmenu->popup(pos);
    
}

void ForwardManager::slot_session_item_selected()
{
    QAction *a = static_cast<QAction *>(sender());
    this->uiw->lineEdit_6->setText(a->text());
    this->uiw->lineEdit_4->setText(a->text());
}

// OK
void ForwardManager::slot_session_list_menu_hide()
{
    // q_debug()<<sender();

    sender()->deleteLater();
}

void ForwardManager::slot_forward_connect_start()
{
    
    QString sess_name;
    QString fsess_name;
    QString local_hostname;
    QString remote_hostname;
    unsigned short local_port;
    unsigned short remote_port;

    sess_name = this->uiw->lineEdit_6->text();

    QMap<QString, QString> host;
    host = BaseStorage::instance()->getHost(sess_name);

    // this->connect_status_dailog = new ConnectingStatusDialog(username, hostname, port, this, Qt::Dialog);
    // QObject::connect(this->connect_status_dailog, SIGNAL(cancel_connect()),
    //                  this, SLOT(slot_cancel_connect()));

    this->mconnector = new Connector();
    this->mconnector->setHostInfo(host);
    QObject::connect(this->mconnector, SIGNAL(connect_finished(int, Connection *)),
                     this, SLOT(slot_connect_remote_host_finished (int, Connection *)));

    // QObject::connect(this->connector, SIGNAL(connect_state_changed(QString)),
    //                  this->connect_status_dailog, SLOT(slot_connect_state_changed(QString)));

    this->mconnector->start();
    // this->connect_status_dailog->exec();
}

void ForwardManager::slot_forward_connect_stop()
{
    
}

void ForwardManager::slot_connect_remote_host_finished (int eno, Connection *conn)
{
    qLogx()<<eno<<conn;
    
    int iret = -1;
    LIBSSH2_LISTENER *lsner = NULL;
    int bound_port = 0;

    this->mconn = conn;
    lsner = libssh2_channel_forward_listen_ex(conn->sess, "192.168.1.103", 1234, &bound_port, 10);
    qLogx()<<lsner<<libssh2_session_last_errno(conn->sess);
    Q_ASSERT(1234 == bound_port);

    ForwardPortWorker *fwp = new ForwardPortWorker(lsner);
    fwp->start();

    sender()->deleteLater();
}
