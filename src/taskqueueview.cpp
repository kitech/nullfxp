// taskqueueview.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-03-23 19:03:21 +0800
// Version: $Id$
// 


#include "taskqueue.h"

#include "taskqueueview.h"

TaskQueueView::TaskQueueView(QWidget *parent)
    : QWidget(parent)
{
    this->ui_win.setupUi(this);

    this->taskQueueDb = QSqlDatabase::addDatabase("QSQLITE", "idtq");
    // this->taskQueueDb.setDatabaseName("tobememory");
    this->taskQueueDb.setDatabaseName("tobememory");
    if (!this->taskQueueDb.open()) {
        qDebug()<<"open sqlite error";
    } else {
        qDebug()<<"open sqlite okkkkkkkkkkkkk";
    }

    this->taskQueueModel = new  QSqlTableModel(0, this->taskQueueDb);
    QObject::connect(TaskQueue::instance(), SIGNAL(insertRow(int)),
                     this, SLOT(insertRow(int)));

    this->taskQueueModel->setTable("task_queue");
    this->ui_win.tableView->setModel(this->taskQueueModel);
    
    
}

TaskQueueView::~TaskQueueView()
{
    delete this->taskQueueModel;
}

bool TaskQueueView::insertRow(int row)
{
    qDebug()<<"insert row........";
    this->taskQueueModel->insertRows(row, 1);
    QModelIndex idx = this->taskQueueModel->index(row, 0);
    this->taskQueueModel->setData(idx, row);
    this->taskQueueModel->submit();

    return true;
}
