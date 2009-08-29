// remotedirtreeview.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-08-29 16:29:51 +0800
// Version: $Id$
// 

#ifndef REMOTEDIRTREEVIEW_H
#define REMOTEDIRTREEVIEW_H

#include <QTreeView>

/**
	@author liuguangzhao <liuguangzhao@users.sf.net >
*/
class RemoteDirTreeView : public QTreeView
{
Q_OBJECT
public:
    RemoteDirTreeView(QWidget* parent);

    ~RemoteDirTreeView();

    signals:
        void drag_ready( );
        
    protected:
        virtual void dragEnterEvent ( QDragEnterEvent * event ) ;
        virtual void dragLeaveEvent ( QDragLeaveEvent * event ) ;
        virtual void dragMoveEvent ( QDragMoveEvent * event );
        virtual void dropEvent ( QDropEvent * event ) ;
        virtual void startDrag ( Qt::DropActions supportedActions );
        virtual void mouseMoveEvent ( QMouseEvent * event );
        virtual void mousePressEvent ( QMouseEvent * event );
        //virtual void mouseReleaseEvent(QMouseEvent* event);
    private:
        QPoint dragStartPosition;
        
    public :
        int test_use_qt_designer_prompt ;
};

#endif
