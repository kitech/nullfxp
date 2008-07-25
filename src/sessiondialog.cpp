// sessiondialog.cpp --- 
// 
// Filename: sessiondialog.cpp
// Description: 
// Author: 刘光照<liuguangzhao@users.sf.net>
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 三  7月 16 21:35:27 2008 (CST)
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


#include "remotehostquickconnectinfodialog.h"

#include "sessiondialog.h"

SessionDialog::SessionDialog(QWidget * parent)
    :QDialog(parent)
{
    this->sess_dlg.setupUi(this);
    this->sess_dlg.treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
#if QT_VERTION >= 0x0404000
    this->sess_dlg.treeView->setHeaderHidden(true);
#else
    this->sess_dlg.treeView->header()->setVisible(false);
#endif

    this->host_list_model = new QStringListModel();
    this->storage = 0;
    {
        this->loadHost();
    }
    QObject::connect(this->sess_dlg.treeView,SIGNAL(customContextMenuRequested(const QPoint&)),
                     this, SLOT(slot_ctx_menu_requested(const QPoint &)));
    QObject::connect(this->sess_dlg.treeView,SIGNAL(doubleClicked(const QModelIndex&)),
                     this,SLOT(slot_conntect_selected_host(const QModelIndex&)));
    QObject::connect(this->sess_dlg.toolButton,SIGNAL(clicked()),
                     this,SLOT(slot_conntect_selected_host()));
    QObject::connect(this->sess_dlg.toolButton_2,SIGNAL(clicked()),
                     this,SLOT(slot_quick_connect()));
    QObject::connect(this->sess_dlg.toolButton_3,SIGNAL(clicked()),
                     this,SLOT(slot_remove_selected_host()));
    this->host_list_ctx_menu = 0;
    this->info_dlg = 0;
}

SessionDialog::~SessionDialog()
{
    if(this->storage != 0)
        this->storage->close();
    delete this->host_list_model;
}

bool SessionDialog::loadHost()
{
    if(this->storage == 0) {
        this->storage = BaseStorage::instance();
        this->storage->open();
    }

    QMap<QString,QMap<QString,QString> > hosts;
    hosts = this->storage->getAllHost();

    QStringList host_show_names;
    if(hosts.count() > 0) {
        QMap<QString,QMap<QString,QString> >::iterator it;
        for(it=hosts.begin();it!=hosts.end();it++)
            host_show_names<<it.key();
    }

    this->host_list_model->setStringList(host_show_names);
    this->sess_dlg.treeView->setModel(this->host_list_model);
    return true;
}

void SessionDialog::slot_ctx_menu_requested(const QPoint & pos)
{
    if(this->host_list_ctx_menu == 0) {
        this->host_list_ctx_menu = new QMenu(this);
        this->action_connect = new QAction(tr("&Connect to host"),this);
        this->host_list_ctx_menu->addAction(this->action_connect);
        this->action_edit = new QAction(tr("&View host info ..."),this);
        this->host_list_ctx_menu->addAction(this->action_edit);

        this->action_rename = new QAction(tr("&Rename host ..."),this);
        this->host_list_ctx_menu->addAction(this->action_rename);

        this->action_sep = new QAction("",0);
        this->action_sep->setSeparator(true);
        this->host_list_ctx_menu->addAction(this->action_sep);
        
        this->action_remove = new QAction(tr("&Remove host ..."), this);
        this->host_list_ctx_menu->addAction(this->action_remove);




        QObject::connect(this->action_connect,SIGNAL(triggered()),this,SLOT(slot_conntect_selected_host()));
        QObject::connect(this->action_edit,SIGNAL(triggered()),this,SLOT(slot_edit_selected_host()));
        QObject::connect(this->action_rename,SIGNAL(triggered()),this,SLOT(slot_rename_selected_host()));

        QObject::connect(this->action_remove,SIGNAL(triggered()),this,SLOT(slot_remove_selected_host()));
    }
    this->host_list_ctx_menu->popup(this->sess_dlg.treeView->mapToGlobal(pos));
}

void  SessionDialog::slot_conntect_selected_host(const QModelIndex & index)
{
    if(index.isValid())
    {
        //qDebug()<<index;
        QString show_name = index.data().toString();
        QMap<QString,QString> host = this->storage->getHost(show_name);
        QMap<QString,QString> host_new = QMap<QString, QString>(host);
        this->selected_host = host;
        emit this->connect_remote_host_requested(host); 
        this->setVisible(false);
        this->accept();
    }else{
        this->slot_show_no_item_tip();
    }
}
QMap<QString,QString>  SessionDialog::get_host_map()
{
    QMap<QString,QString> host;
    if(this->info_dlg != 0)
        host = ((RemoteHostQuickConnectInfoDialog*)this->info_dlg)->get_host_map();
    else
        host = selected_host;
    return host;
}

void SessionDialog::slot_conntect_selected_host()
{
    QItemSelectionModel * ism = 0;
    QModelIndexList mil ;

    ism = this->sess_dlg.treeView->selectionModel();
    mil = ism->selectedIndexes();
    //qDebug()<<mil;

    if(mil.count() > 0) {
        //qDebug()<<mil.at(0).data();
        this->slot_conntect_selected_host(mil.at(0));
    }else{
        this->slot_show_no_item_tip();
    }
}

void SessionDialog::slot_edit_selected_host()
{
    QItemSelectionModel * ism = 0;
    QModelIndexList mil;

    ism = this->sess_dlg.treeView->selectionModel();
    mil = ism->selectedIndexes();
    //qDebug()<<mil;
    if(mil.count() > 0) {
        //qDebug()<<mil.at(0).data();
        QString show_name = mil.at(0).data().toString();
        //qDebug()<<show_name;
        QMap<QString,QString> host = this->storage->getHost(show_name);
        QMap<QString,QString> host_new;
        //qDebug()<<host;
        RemoteHostQuickConnectInfoDialog * info_dlg = new RemoteHostQuickConnectInfoDialog(this);
        info_dlg->set_active_host(host);
        if(info_dlg->exec() == QDialog::Accepted)
        {
            host_new = info_dlg->get_host_map();
            if(host_new != host)
            {
                this->storage->updateHost(host_new);
                this->storage->save();
            }
        }
        delete info_dlg;
    }else{
        this->slot_show_no_item_tip();
    }
}

void SessionDialog::slot_rename_selected_host()
{
    QItemSelectionModel * ism = 0;
    QModelIndexList mil;
    bool ok;

    ism = this->sess_dlg.treeView->selectionModel();
    mil = ism->selectedIndexes();
    //qDebug()<<mil;
    if(mil.count() > 0) {
        QString new_name = 
            QInputDialog::getText(this, tr("Rename host:"), tr("Type the new name:"), 
                                  QLineEdit::Normal, mil.at(0).data().toString(), &ok);
        if(ok && !new_name.isEmpty() && new_name != mil.at(0).data().toString()) {
            QMap<QString, QString> host = this->storage->getHost(mil.at(0).data().toString());
            this->storage->updateHost(host, new_name);
            this->host_list_model->setData(mil.at(0), new_name);
        }
    }else{
        this->slot_show_no_item_tip();
    }
}

void SessionDialog::slot_remove_selected_host()
{
    QItemSelectionModel * ism = 0;
    QModelIndexList mil;

    ism = this->sess_dlg.treeView->selectionModel();
    mil = ism->selectedIndexes();
    //qDebug()<<mil;
    if(mil.count() > 0) {
        //waning user
        int ret = QMessageBox::question(this,tr("Remote host:") ,tr("Are you sure remote it ?"), QMessageBox::Yes,QMessageBox::No );
        if(ret == QMessageBox::Yes) {
            this->storage->removeHost(mil.at(0).data().toString());
            this->host_list_model->removeRows(mil.at(0).row(),1,QModelIndex());
        }
    }else{
        this->slot_show_no_item_tip();
    }
}

void SessionDialog::slot_quick_connect()
{
    this->setVisible(false);
    emit quick_connect();
    this->reject();
}

void SessionDialog::slot_show_no_item_tip()
{
    QString msg = QString("<br>&nbsp;&nbsp;&nbsp;<b>") + tr("No selected host.") 
        + QString("&nbsp;&nbsp;</b><br>");
    QPoint pos = this->sess_dlg.toolButton_3->pos();
    pos.setY(pos.y() + 80);
    QToolTip::showText(this->mapToGlobal(pos), msg, this);
}

//
// sessiondialog.cpp ends here
