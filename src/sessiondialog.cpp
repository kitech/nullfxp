// sessiondialog.cpp --- 
// 
// Filename: sessiondialog.cpp
// Description: 
// Author: liuguangzhao
// Maintainer: 
// Created: 六  4月  5 18:10:37 2008 (CST)
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
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth
// Floor, Boston, MA 02110-1301, USA.

// 
// 

// Code:

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
                     this,SLOT(slot_conntect_selected_host()));
    QObject::connect(this->sess_dlg.toolButton_3,SIGNAL(clicked()),
                     this,SLOT(slot_remove_selected_host()));
    this->host_list_ctx_menu = 0;
    this->info_dlg = 0;
}

SessionDialog::~SessionDialog()
{
    if(this->storage != 0)
        this->storage->close();
    //delete this->storage;
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
    if(this->host_list_ctx_menu == 0)
    {
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
            emit this->connect_remote_host_requested(host_new);
            this->setVisible(false);
            this->info_dlg = info_dlg;
            this->accept();
        }
        else
        {

        }
    }
}
QMap<QString,QString>  SessionDialog::get_host_map()
{
    QMap<QString,QString> host;
    if(this->info_dlg != 0)
        host = ((RemoteHostQuickConnectInfoDialog*)this->info_dlg)->get_host_map();
    return host;
}

void SessionDialog::slot_conntect_selected_host()
{
    QItemSelectionModel * ism = 0;
    QModelIndexList mil ;

    ism = this->sess_dlg.treeView->selectionModel();
    mil = ism->selectedIndexes();
    //qDebug()<<mil;

    if(mil.count() > 0)
    {
        //qDebug()<<mil.at(0).data();
        this->slot_conntect_selected_host(mil.at(0));
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
    }
}

void SessionDialog::slot_remove_selected_host()
{
    QItemSelectionModel * ism = 0;
    QModelIndexList mil;

    ism = this->sess_dlg.treeView->selectionModel();
    mil = ism->selectedIndexes();
    //qDebug()<<mil;
    if(mil.count() > 0)
    {
        //waning user
        //
        int ret = QMessageBox::question(this,tr("Remote host:") ,tr("Are you sure remote it ?"), QMessageBox::Yes,QMessageBox::No );
        if(ret == QMessageBox::Yes)
        {
            this->storage->removeHost(mil.at(0).data().toString());
            this->host_list_model->removeRows(mil.at(0).row(),1,QModelIndex());
        }
    }
}

//
// sessiondialog.cpp ends here
