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

#include "ui_taskqueueview.h"
#include "taskqueueview.h"

TaskQueueView::TaskQueueView(QWidget *parent)
    : QWidget(parent)
    , uiw(new Ui::TaskQueueView())
{
    this->uiw->setupUi(this);

    this->taskQueueModel = NULL;
    // this->taskQueueModel = TaskQueueModel::instance();
    // this->taskQueueModel->setTable("task_queue");
    // this->uiw->tableView->setModel(this->taskQueueModel);

    this->ctxMenu = NULL;
    QObject::connect(this->uiw->tableView, SIGNAL(customContextMenuRequested(const QPoint &)),
                     this, SLOT(slotCustomContextMenuRequested(const QPoint &)));

    // resize column tableView
    this->uiw->tableView->setColumnWidth(0, 35);
    this->uiw->tableView->setColumnWidth(1, 170);
    this->uiw->tableView->setColumnWidth(2, 170);
    this->uiw->tableView->setColumnWidth(5, 65);
    this->uiw->tableView->setColumnWidth(10, 150);
    this->uiw->tableView->setColumnWidth(11, 150);
}

TaskQueueView::~TaskQueueView()
{
    if (this->taskQueueModel != NULL) {
        delete this->taskQueueModel;
    }
    delete this->uiw;
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
    this->ctxMenu->popup(this->uiw->tableView->mapToGlobal(pos));
}

void TaskQueueView::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // hope delay the model's init process;
    if (this->taskQueueModel == NULL) {
        this->taskQueueModel = TaskQueueModel::instance();
        this->taskQueueModel->setTable("task_queue");
        this->uiw->tableView->setModel(this->taskQueueModel);
    }
}

void TaskQueueView::onSelectAll()
{
    QItemSelectionModel *ism = this->uiw->tableView->selectionModel();
    int rc = this->taskQueueModel->rowCount(QModelIndex());
    int cc = this->taskQueueModel->columnCount(QModelIndex());
    
    QModelIndex topLeftIndex = this->taskQueueModel->index(0, 0, QModelIndex());
    QModelIndex bottomRightIndex = this->taskQueueModel->index(rc - 1, cc - 1, QModelIndex());
    QItemSelection is(topLeftIndex, bottomRightIndex);

    ism->select(is, QItemSelectionModel::ClearAndSelect);

    for (int r = 0; r < rc; ++ r) {
        
    }
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
