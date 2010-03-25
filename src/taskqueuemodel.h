// taskqueuemodel.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-03-25 10:35:10 +0800
// Version: $Id$
// 

#ifndef _TASKQUEUEMODEL_H_
#define _TASKQUEUEMODEL_H_


#include <QtCore>
#include <QtSql>

#include "sqlite/sqlite3.h"

// memery sqlite table for job queue, single instance model
class TaskQueueModel : public QSqlTableModel
{
    Q_OBJECT;
public:
    bool inited;
    ~TaskQueueModel();
    static TaskQueueModel *instance();

public slots:
    void slot_set_transfer_percent(int percent, int total_transfered, int transfer_delta);
    void slot_transfer_thread_finished();
    void slot_new_file_transfer_started(QString new_file_name);
        
    void slot_transfer_got_file_size(int size);
 
signals:
    void insertRow(int row);

protected:
    TaskQueueModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());
    void init();
    void finalize();
    
private:
    static TaskQueueModel *inst;

    sqlite3 *pDB;
};

/*
  CREATE TABLE task_queue (
  id INTEGER PRIMARY KEY,
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

#endif /* _TASKQUEUEMODEL_H_ */
