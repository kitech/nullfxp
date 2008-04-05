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


#include "sessiondialog.h"

SessionDialog::SessionDialog(QWidget * parent)
  :QDialog(parent)
{
  this->sess_dlg.setupUi(this);
  this->sess_dlg.treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  this->host_list_model = new QStringListModel();
  this->storage = 0;
  {
    this->loadHost();
  }
  QObject::connect(this->sess_dlg.treeView,SIGNAL(customContextMenuRequested(const QPoint&)),
		   this, SLOT(slot_ctx_menu_requested(const QPoint &)));
  this->host_list_ctx_menu = 0;
}

SessionDialog::~SessionDialog()
{
  if(this->storage != 0)
    this->storage->close();
  delete this->storage;
  delete this->host_list_model;
}

bool SessionDialog::loadHost()
{
  if(this->storage == 0)
    {
      this->storage = new BaseStorage();
      this->storage->open();
    }

  QMap<QString,QMap<QString,QString> > hosts;
  hosts = this->storage->getAllHost();

  QStringList host_show_names;
  if(hosts.count() > 0)
    {
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
      this->action_connect = new QAction(tr("Connect to host"),this);
      this->host_list_ctx_menu->addAction(this->action_connect);
      this->action_edit = new QAction(tr("View host info ..."),this);
      this->host_list_ctx_menu->addAction(this->action_edit);
    }
  this->host_list_ctx_menu->popup(this->sess_dlg.treeView->mapToGlobal(pos));
}

void SessionDialog::slot_conntect_selected_host()
{

}

void SessionDialog::slot_edit_selected_host()
{

}


//
// sessiondialog.cpp ends here
