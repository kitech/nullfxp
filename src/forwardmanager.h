// forwardmanager.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-06-29 17:38:40 +0000
// Version: $Id$
// 

#ifndef _FORWARDMANAGER_H_
#define _FORWARDMANAGER_H_

#include <QtGui>

#include "libssh2.h"

class Connector;
class Connection;
class ForwardPortWorker;

namespace Ui {
    class ForwardManager;
};

class ForwardManager : public QDialog
{
    Q_OBJECT;
public:
    ForwardManager(QWidget *parent = 0);
    virtual ~ForwardManager();

public slots:
    void slot_show_session_menu();
    void slot_session_item_selected();
    void slot_session_list_menu_hide();
    void slot_forward_connect_start();
    void slot_forward_connect_stop();

    void slot_connect_remote_host_finished (int eno, Connection *conn);

    void slot_forward_worker_finished();
    void slot_listen_channel_error(int eno);

private slots:
    void slot_new_forward_session();
    void slot_load_forwarder_list();
    void slot_save_forward_session();
    void slot_reset_forward_session();
    void slot_delete_forward_session();
    void slot_forward_session_item_changed(QListWidgetItem * current, QListWidgetItem * previous );
    void slot_set_ui_state(int state);
    void slot_setting_change();
    void slot_setting_change(const QString &value);
    // void slot_setting_change(int value);
    void slot_mannual_set_dest_ipaddr(int state);

private:
    Ui::ForwardManager *uiw;

    enum UiState {
        S_MIN = 0,
        S_NEW_SESS,
        S_START_READY,
        S_STARTING,
        S_STOP_READY,
        S_STOPPING,
        S_MAX
    };

    enum StateKey {
        KEY_MIN = 0,
        KEY_CONNECTOR,
        KEY_CONNECTION,
        KEY_WORKER,
        KEY_MAX
    };
    class ForwardState {
    public:
#if !defined(NS_HAS_CXX0X)
        ForwardState(Connector *a, Connection *b, ForwardPortWorker *c)
            : connector(a), conn(b), worker(c), lsner(NULL), want_reconn(false) {}
        ForwardState(){}
#endif
        Connector *connector;
        Connection *conn;
        ForwardPortWorker *worker;
        LIBSSH2_LISTENER *lsner;
        bool want_reconn;
    };
    QHash<QString, ForwardState> mfwdstate;
    int meditstate;
};


#endif /* _FORWARDMANAGER_H_ */
