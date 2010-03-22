// taskqueue.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-03-22 11:45:36 +0800
// Version: $Id$
// 

#ifndef _TASKQUEUE_H_
#define _TASKQUEUE_H_

#include <QtCore>

#include "sqlite/sqlite3.h"

// memery sqlite table for job queue, single instance model
class TaskQueue : public QObject
{
    Q_OBJECT;
public:
    bool inited;
    ~TaskQueue();
    static TaskQueue *instance();

public slots:
    void slot_set_transfer_percent(int percent, int total_transfered, int transfer_delta);
    void slot_transfer_thread_finished();
    void slot_new_file_transfer_started(QString new_file_name);
        
    void slot_transfer_got_file_size(int size);
 
protected:
    TaskQueue(QObject *parent = 0);
    void init();
    void finalize();
    
private:
    static TaskQueue *inst;

    sqlite3 *pDB;
};

/*
  CREATE TABLE task_queue (
  id UNSIGNED BIG INT PRIMARY KEY,
  file_name VARCHAR(128),
  dest_path VARCHAR(256),
  file_size UNSIGNED BIG INT,
  got_size  UNSIGNED BIG INT,
  got_percent INT2,
  eclapsed_time VARCHAR(10),
  left_time VARCHAR(10),
  speed VARCHAR(10),
  status VARCHAR(10),
  start_time VARCHAR(16),
  finish_time VARCHAR(16)
);

 */
#endif /* _TASKQUEUE_H_ */
