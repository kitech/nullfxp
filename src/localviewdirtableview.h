// localviewdirtableview.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-06-16 17:43:45 +0800
// Version: $Id: localviewdirtableview.h 673 2010-06-16 09:59:52Z liuguangzhao $
// 

#ifndef LOCALVIEWDIRTABLEVIEW_H
#define LOCALVIEWDIRTABLEVIEW_H

#include <QTableView>

/**
 *
 */
class LocalViewDirTableView : public QTableView
{
    Q_OBJECT;
public:
    LocalViewDirTableView(QWidget*parent);
    virtual ~LocalViewDirTableView();

// signals:
//     void drag_ready();
        
protected:
     virtual void dragEnterEvent(QDragEnterEvent *event);
//     virtual void dragLeaveEvent(QDragLeaveEvent *event);
     virtual void dragMoveEvent(QDragMoveEvent *event);
     virtual void dropEvent(QDropEvent *event);
//     virtual void startDrag(Qt::DropActions supportedActions);
//     virtual void mouseMoveEvent(QMouseEvent *event);
//     virtual void mousePressEvent(QMouseEvent *event);
// private:
//     QPoint dragStartPosition;
        
// public:
//     int test_use_qt_designer_prompt;

};

#endif
