// taskqueue.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-03-22 11:45:59 +0800
// Version: $Id$
// 

#include "taskqueue.h"

TaskQueue *TaskQueue::inst = new TaskQueue();
TaskQueue::TaskQueue(QObject *parent)
    : QObject(parent)
{
    this->init();
}

TaskQueue::~TaskQueue()
{
    this->finalize();
}

const TaskQueue *TaskQueue::instance()
{
    return TaskQueue::inst;
}

void TaskQueue::init()
{
    int rv = sqlite3_open_v2(":memory", &this->pDB, 
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX,
                             NULL);
    Q_ASSERT(rv == SQLITE_OK);
    if (rv != SQLITE_OK) {
        qDebug()<<"Sqlite error:"<<rv<<":"<<sqlite3_errmsg(this->pDB);
        sqlite3_close(this->pDB);
        this->pDB = NULL;
    }

    // 创建表，
}

void TaskQueue::finalize()
{
    int rv = sqlite3_close(this->pDB);
}

void TaskQueue::slot_set_transfer_percent(int percent, int total_transfered, int transfer_delta)
{
}
void TaskQueue::slot_transfer_thread_finished()
{
}
void TaskQueue::slot_new_file_transfer_started(QString new_file_name)
{
}
        
void TaskQueue::slot_transfer_got_file_size(int size)
{
}




