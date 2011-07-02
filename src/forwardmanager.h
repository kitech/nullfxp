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

class Connector;
class Connection;

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

private:
    Ui::ForwardManager *uiw;

    Connector *mconnector;
    Connection *mconn;
};


#endif /* _FORWARDMANAGER_H_ */
