// forwardmanager.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-06-29 17:38:52 +0000
// Version: $Id$
// 

#include "utils.h"
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

    /////
    QObject::connect(this->uiw->pushButton, SIGNAL(clicked()), this, SLOT(slot_new_forward_session()));
    QObject::connect(this->uiw->pushButton_3, SIGNAL(clicked()), this, SLOT(slot_save_forward_session()));

    QTimer::singleShot(5, this, SLOT(slot_load_forwarder_list()));
    QObject::connect(this->uiw->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
                     this, SLOT(slot_forward_session_item_changed(QListWidgetItem*, QListWidgetItem*)));
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
    this->uiw->lineEdit_2->setText(a->text());
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
    
    sess_name = this->uiw->lineEdit_6->text();
    fsess_name = this->uiw->lineEdit->text();

    if (fsess_name.isEmpty() || sess_name.isEmpty()) {
        qLogx()<<"session name empty:";
        return;
    }

    // check current status
    if (this->mfwdstate.contains(fsess_name)) {
        this->slot_forward_connect_stop();
        return;
    }

    QMap<QString, QString> host;
    host = BaseStorage::instance()->getHost(sess_name);

    // this->connect_status_dailog = new ConnectingStatusDialog(username, hostname, port, this, Qt::Dialog);
    // QObject::connect(this->connect_status_dailog, SIGNAL(cancel_connect()),
    //                  this, SLOT(slot_cancel_connect()));

    Connector *connector = new Connector();
    connector->setHostInfo(host);
    QObject::connect(connector, SIGNAL(connect_finished(int, Connection *)),
                     this, SLOT(slot_connect_remote_host_finished (int, Connection *)));

#if  defined(NS_HAS_CXX0X) 
    //    #warning "modern c++0x mode"
    this->mfwdstate.insert(fsess_name, {connector, 0, 0, 0});
#else
    //    #warning "basic c++ mode"
    ForwardState st(connector, 0, 0); 
    this->mfwdstate.insert(fsess_name, st);
#endif
    // QObject::connect(this->connector, SIGNAL(connect_state_changed(QString)),
    //                  this->connect_status_dailog, SLOT(slot_connect_state_changed(QString)));

    // carefully set ui status
    this->slot_set_ui_state(S_STARTING);

    connector->start();
    // this->connect_status_dailog->exec();
}

void ForwardManager::slot_forward_connect_stop()
{
    int rn = 0;
    QString sess_name;
    QString fsess_name;
    
    sess_name = this->uiw->lineEdit_6->text();
    fsess_name = this->uiw->lineEdit->text();

    Connection *conn = NULL;
    ForwardPortWorker *worker = NULL;
    LIBSSH2_LISTENER *lsner = NULL;
    // check current status
    if (this->mfwdstate.contains(fsess_name)) {
        conn = this->mfwdstate.value(fsess_name).conn;
        worker = this->mfwdstate.value(fsess_name).worker;
        lsner = this->mfwdstate.value(fsess_name).lsner;
        
        qLogx()<<"stop worker:"<<worker;
        if (lsner) {
            this->mfwdstate[fsess_name].lsner = NULL;
            rn = libssh2_channel_forward_cancel(lsner);
        }
        if (worker) {
            this->slot_set_ui_state(S_STOPPING);

            worker->quit();
        } else {
        }
    } else {

    }
}

void ForwardManager::slot_connect_remote_host_finished (int eno, Connection *conn)
{
    qLogx()<<eno<<conn;
    
    int iret = -1;
    LIBSSH2_LISTENER *lsner = NULL;
    int bound_port = 0;
    int src_port;
    QString dest_hostname;
    int dest_port;
    QString fsess_name;

    Connector *aconnector = static_cast<Connector*>(sender());
    Connection *aconn = conn;
    // check connection status

#if defined(NS_HAS_CXX0X)
    auto it = this->mfwdstate.begin();
#else
    QHash<QString, ForwardState>::iterator it;
#endif
    for (it = this->mfwdstate.begin(); it != this->mfwdstate.end(); it++) {
        if (it.value().connector == aconnector) {
            fsess_name = it.key();
            it.value().conn = conn;
            break;
        }
    }

    Q_ASSERT(!fsess_name.isEmpty());
    this->mfwdstate[fsess_name].connector = NULL;

    src_port = this->uiw->lineEdit_3->text().toInt();
    // lsner = libssh2_channel_forward_listen_ex(conn->sess, NULL, 1234, &bound_port, 10);
    lsner = libssh2_channel_forward_listen_ex(conn->sess, NULL, src_port, &bound_port, 10);
    qLogx()<<lsner<<libssh2_session_last_errno(conn->sess);
    if (lsner == NULL) {
        int eno = libssh2_session_last_errno(conn->sess);
        if (eno == LIBSSH2_ERROR_REQUEST_DENIED) {
            qLogx()<<"Remote server denied forward port request.";
        } else {
            qLogx()<<"Unknown ssh channel error:"<<eno;
        }

        conn->disconnect();
        delete conn;
        this->mfwdstate.remove(fsess_name);

        this->slot_set_ui_state(S_START_READY);
    } else {
        // Q_ASSERT(1234 == bound_port);
        Q_ASSERT(src_port == bound_port);
        this->mfwdstate[fsess_name].lsner = lsner;

        dest_hostname = this->uiw->lineEdit_4->text();
        dest_port = this->uiw->lineEdit_5->text().toInt();

        ForwardPortWorker *fwp = new ForwardPortWorker(lsner, dest_hostname, dest_port);
        this->mfwdstate[fsess_name].worker = fwp;
        QObject::connect(fwp, SIGNAL(finished()), this, SLOT(slot_forward_worker_finished()));
        fwp->start();

        this->slot_set_ui_state(S_STOP_READY);
    }

    sender()->deleteLater();
}

void ForwardManager::slot_forward_worker_finished()
{
    qLogx()<<""<<sender();

    QString fsess_name;
    Connection *conn = NULL;
    ForwardPortWorker *worker = NULL;

    worker = static_cast<ForwardPortWorker*>(sender());
    QHash<QString, ForwardState>::const_iterator it;
    for (it = this->mfwdstate.begin(); it != this->mfwdstate.end(); it++) {
        if (it.value().worker == worker) {
            fsess_name = it.key();
            conn = it.value().conn;
        }
    }
    qLogx()<<"Disconnect remote state for:"<<fsess_name;
    Q_ASSERT(!fsess_name.isEmpty());
    this->mfwdstate.remove(fsess_name);

    worker->deleteLater();
    conn->disconnect();
    delete conn;

    this->slot_set_ui_state(S_START_READY);
}

void ForwardManager::slot_new_forward_session()
{
    qLogx()<<"";

    this->uiw->lineEdit->setText("");
    this->uiw->lineEdit_6->setText("");
    this->uiw->lineEdit_2->setText("");
    this->uiw->lineEdit_3->setText("");
    this->uiw->lineEdit_4->setText("");
    this->uiw->lineEdit_5->setText("");

    this->slot_set_ui_state(S_NEW_SESS);
}

void ForwardManager::slot_load_forwarder_list()
{
    QString fwd_sess_name;
    QString ref_sess_name;
    QString remote_host;
    QString remote_port;
    QString dest_host;
    QString dest_port;
    QString ctime;
    QString mtime;

    QMap<QString, QString> fwd;
    QStringList forwarders = BaseStorage::instance()->getForwarderNames();

    for (int i = 0; i < forwarders.count(); i ++) {
        fwd = BaseStorage::instance()->getForwarder(forwarders.at(i));
        
        fwd_sess_name = fwd["fwd_sess_name"];
        ref_sess_name = fwd["ref_sess_name"];
        remote_host = fwd["remote_host"];
        remote_port = fwd["remote_port"];
        dest_host = fwd["dest_host"];
        dest_port = fwd["dest_port"];
        ctime = fwd["ctime"];
        mtime = fwd["mtime"];

        // QListWidgetItem *item = new QListWidgetItem(fwd_sess_name);

        this->uiw->listWidget->addItem(fwd_sess_name);
    }

}

void ForwardManager::slot_save_forward_session()
{
    qLogx()<<"";
    QMap<QString, QString> fwd;

    QString fwd_sess_name;
    QString ref_sess_name;
    QString remote_host;
    QString remote_port;
    QString dest_host;
    QString dest_port;
    QString ctime;
    QString mtime;

    fwd_sess_name = this->uiw->lineEdit->text();
    ref_sess_name = this->uiw->lineEdit_6->text();
    remote_host = this->uiw->lineEdit_2->text();
    remote_port = this->uiw->lineEdit_3->text();
    dest_host = this->uiw->lineEdit_4->text();
    dest_port = this->uiw->lineEdit_5->text();

    fwd["fwd_sess_name"] = fwd_sess_name;
    fwd["ref_sess_name"] = ref_sess_name;
    fwd["remote_host"] = remote_host;
    fwd["remote_port"] = remote_port;
    fwd["dest_host"] = dest_host;
    fwd["dest_port"] = dest_port;
    
    if (!BaseStorage::instance()->addForwarder(fwd)) {
        qLogx()<<"add forwarder error.";
    } else {
        
    }
}

void ForwardManager::slot_forward_session_item_changed(QListWidgetItem * current, QListWidgetItem * previous)
{
    qLogx()<<current<<previous;
    QString curr_fwd_sess_name = current->text();
    QString prev_fwd_sess_name = previous ? previous->text() : QString();
    QMap<QString, QString> curr_fwd, prev_fwd;

    QString fwd_sess_name;
    QString ref_sess_name;
    QString remote_host;
    QString remote_port;
    QString dest_host;
    QString dest_port;
    QString ctime;
    QString mtime;

    curr_fwd = BaseStorage::instance()->getForwarder(curr_fwd_sess_name);

    fwd_sess_name = curr_fwd["fwd_sess_name"];
    ref_sess_name = curr_fwd["ref_sess_name"];
    remote_host = curr_fwd["remote_host"];
    remote_port = curr_fwd["remote_port"];
    dest_port = curr_fwd["dest_host"];
    dest_port = curr_fwd["dest_port"];


    this->uiw->lineEdit->setText(fwd_sess_name);
    this->uiw->lineEdit_6->setText(ref_sess_name);
    this->uiw->lineEdit_2->setText(remote_host);
    this->uiw->lineEdit_3->setText(remote_port);
    this->uiw->lineEdit_4->setText(dest_host);
    this->uiw->lineEdit_5->setText(dest_port);

    // restore status
    Connection *conn = NULL;
    ForwardPortWorker *worker = NULL;
    if (this->mfwdstate.contains(fwd_sess_name)) {
        this->slot_set_ui_state(S_STOP_READY);
    } else {
        this->slot_set_ui_state(S_START_READY);
    }
}

void ForwardManager::slot_set_ui_state(int state)
{
    Q_ASSERT (state > S_MIN && state < S_MAX);

    switch (state) {
    case S_NEW_SESS:
        this->uiw->pushButton_6->setEnabled(true);
        this->uiw->pushButton_6->setText(tr("Start"));
        this->uiw->toolButton->setIcon(QIcon(":/icons/network-disconnect.png"));
        this->uiw->label_12->setText(tr("New..."));
        break;
    case S_START_READY:
        this->uiw->pushButton_6->setEnabled(true);
        this->uiw->pushButton_6->setText(tr("Start"));
        this->uiw->toolButton->setIcon(QIcon(":/icons/network-disconnect.png"));
        this->uiw->label_12->setText(tr("Ready Start"));
        break;
    case S_STARTING:
        this->uiw->pushButton_6->setEnabled(false);
        this->uiw->pushButton_6->setText(tr("Starting..."));
        this->uiw->toolButton->setIcon(QIcon(":/icons/network-disconnect.png"));
        this->uiw->label_12->setText(tr("Starting..."));
        break;
    case S_STOP_READY:
        this->uiw->pushButton_6->setEnabled(true);
        this->uiw->pushButton_6->setText(tr("Stop"));
        this->uiw->toolButton->setIcon(QIcon(":/icons/network-connect.png"));
        this->uiw->label_12->setText(tr("Started"));
        break;
    case S_STOPPING:
        this->uiw->pushButton_6->setEnabled(false);
        this->uiw->pushButton_6->setText(tr("Stopping..."));
        this->uiw->toolButton->setIcon(QIcon(":/icons/network-disconnect.png"));
        this->uiw->label_12->setText(tr("Stopping..."));
        break;
    default:
        Q_ASSERT(1==2);
        break;
    };
}
