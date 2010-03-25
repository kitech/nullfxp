// taskqueueview.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-03-23 19:02:54 +0800
// Version: $Id$
// 

#ifndef _TASKQUEUEVIEW_H_
#define _TASKQUEUEVIEW_H_

#include <QtCore>
#include <QtGui>
#include <QtSql>

#include "ui_taskqueueview.h"

class TaskQueueModel;

class TaskQueueView : public QWidget
{
    Q_OBJECT;
public:
    TaskQueueView(QWidget *parent = 0);
    ~TaskQueueView();

private:
    Ui::TaskQueueView ui_win;
    TaskQueueModel *taskQueueModel;
};

#endif /* _TASKQUEUEVIEW_H_ */
