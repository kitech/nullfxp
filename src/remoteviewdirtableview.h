// remoteviewdirtableview.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-06-16 17:43:45 +0800
// Version: $Id$
// 

#ifndef REMOTEVIEWDIRTABLEVIEW_H
#define REMOTEVIEWDIRTABLEVIEW_H

#include <QTableView>

/**
 *
 */
class RemoteViewDirTableView : public QTableView
{
    Q_OBJECT;
public:
    RemoteViewDirTableView(QWidget*parent);
    virtual ~RemoteViewDirTableView();

signals:
    void drag_ready();
        
protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void startDrag(Qt::DropActions supportedActions);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
private:
    QPoint dragStartPosition;
        
public:
    int test_use_qt_designer_prompt;
};

#endif
