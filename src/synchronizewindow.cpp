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

#include "basestorage.h"
#include "synchronizewindow.h"

SyncWalker::SyncWalker(QObject *parent)
    :QThread(parent)
{

}
SyncWalker::~SyncWalker()
{

}
void SyncWalker::run()
{

}


SynchronizeWindow::SynchronizeWindow(QWidget *parent, Qt::WindowFlags flags)
    :QWidget(parent, flags)
{
    this->ui_win.setupUi(this);
}

SynchronizeWindow::~SynchronizeWindow()
{

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
    
}
void SynchronizeWindow::stop()
{
    
}





