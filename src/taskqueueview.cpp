// taskqueueview.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-03-23 19:03:21 +0800
// Version: $Id$
// 


#include "taskqueue.h"
#include "taskqueuemodel.h"

#include "taskqueueview.h"

TaskQueueView::TaskQueueView(QWidget *parent)
    : QWidget(parent)
{
    this->ui_win.setupUi(this);

    this->taskQueueModel = TaskQueueModel::instance();
    this->taskQueueModel->setTable("task_queue");
    this->ui_win.tableView->setModel(this->taskQueueModel);

    this->ctxMenu = NULL;
    QObject::connect(this->ui_win.tableView, SIGNAL(customContextMenuRequested(const QPoint &)),
                     this, SLOT(slotCustomContextMenuRequested(const QPoint &)));
}

TaskQueueView::~TaskQueueView()
{
    delete this->taskQueueModel;
}

void TaskQueueView::initContextMenu()
{
    if (this->ctxMenu != NULL) {
        return ;
    }

    QAction *action = NULL;

    this->ctxMenu = new QMenu(this);
    action = new QAction(tr("&Select All"), this);
    this->ctxMenu->addAction(action);
    this->ctxMenu->addSeparator();

    action = new QAction(tr("&Transfer All"), this);
    this->ctxMenu->addAction(action);
    action = new QAction(tr("T&ransfer Selected"), this);
    this->ctxMenu->addSeparator();
    
    action = new QAction(tr("R&emove All"), this);
    this->ctxMenu->addAction(action);
    action = new QAction(tr("Re&move Selected"), this);
    this->ctxMenu->addAction(action);
    action = new QAction(tr("Rem&ove Finished"), this);
    this->ctxMenu->addAction(action);
    this->ctxMenu->addSeparator();

    action = new QAction(tr("&Cancel Selected"), this);
    this->ctxMenu->addAction(action);
    action = new QAction(tr("C&ancel All"), this);

}

void TaskQueueView::slotCustomContextMenuRequested(const QPoint & pos)
{
    if (this->ctxMenu == NULL) {
        this->initContextMenu();
    }
    Q_ASSERT(this->ctxMenu != NULL);
    this->ctxMenu->popup(this->ui_win.tableView->mapToGlobal(pos));
}
