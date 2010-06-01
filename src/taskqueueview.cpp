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
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onSelectAll()));
    this->ctxMenu->addAction(action);
    this->ctxMenu->addSeparator();

    action = new QAction(tr("&Transfer All"), this);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onTransferAll()));
    this->ctxMenu->addAction(action);
    action = new QAction(tr("T&ransfer Selected"), this);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onTransferSelected()));
    this->ctxMenu->addAction(action);
    this->ctxMenu->addSeparator();
    
    action = new QAction(tr("R&emove All"), this);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onRemoveAll()));
    this->ctxMenu->addAction(action);
    action = new QAction(tr("Re&move Selected"), this);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onRemoveSelected()));
    this->ctxMenu->addAction(action);
    action = new QAction(tr("Rem&ove Finished"), this);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onRemoveFinished()));
    this->ctxMenu->addAction(action);
    this->ctxMenu->addSeparator();

    action = new QAction(tr("&Cancel Selected"), this);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onCancelSelected()));
    this->ctxMenu->addAction(action);
    action = new QAction(tr("C&ancel All"), this);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onCancelAll()));
    this->ctxMenu->addAction(action);
}

void TaskQueueView::slotCustomContextMenuRequested(const QPoint & pos)
{
    if (this->ctxMenu == NULL) {
        this->initContextMenu();
    }
    Q_ASSERT(this->ctxMenu != NULL);
    this->ctxMenu->popup(this->ui_win.tableView->mapToGlobal(pos));
}

void TaskQueueView::onSelectAll()
{

}

void TaskQueueView::onTransferAll()
{
    
}
void TaskQueueView::onTransferSelected()
{
    
}

void TaskQueueView::onRemoveAll()
{
    
}
void TaskQueueView::onRemoveSelected()
{
    
}
void TaskQueueView::onRemoveFinished()
{
    
}

void TaskQueueView::onCancelAll()
{
    
}
void TaskQueueView::onCancelSelected()
{
    
}
