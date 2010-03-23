// taskqueueview.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-03-23 19:03:21 +0800
// Version: $Id$
// 

#include "taskqueueview.h"


TaskQueueView::TaskQueueView(QWidget *parent)
    : QWidget(parent)
{
    this->ui_win.setupUi(this);

    this->taskQueueModel = new  QSqlTableModel();
    this->ui_win.tableView->setModel(this->taskQueueModel);
}

TaskQueueView::~TaskQueueView()
{
    delete this->taskQueueModel;
}
