// localdirtreeview.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-08-29 16:30:05 +0800
// Version: $Id: localdirtreeview.cpp 756 2011-06-21 15:01:08Z liuguangzhao $
// 

#include <QtCore>
#include <QtGui>

// #include "localdirmodel.h"
#include "localdirtreeview.h"

LocalDirTreeView::LocalDirTreeView(QWidget* parent): QTreeView(parent)
{
}


LocalDirTreeView::~LocalDirTreeView()
{
}

/**
 * 当源和目标都是同一主机的时候忽略此事件
 */
void LocalDirTreeView::dragEnterEvent ( QDragEnterEvent * event ) 
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //qDebug()<<event<<event->source()<<this<<this->parentWidget()->parentWidget()->parentWidget()<<event->source()->parentWidget()->parentWidget()->parentWidget();
    if( event->source() == this->parentWidget()->parentWidget()->parentWidget() 
        || event->source()->parentWidget()->parentWidget()->parentWidget() == this->parentWidget()->parentWidget()->parentWidget()){
        event->ignore();
    } else {
        QTreeView::dragEnterEvent ( event );
    }
}
void LocalDirTreeView::dragLeaveEvent ( QDragLeaveEvent * event ) 
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QTreeView::dragLeaveEvent ( event ) ;
}
void LocalDirTreeView::dragMoveEvent ( QDragMoveEvent * event )
{
    //qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QTreeView::dragMoveEvent (  event ) ;
}

void LocalDirTreeView::dropEvent ( QDropEvent * event ) 
{
    //qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QTreeView::dropEvent ( event ) ;
}
void LocalDirTreeView::startDrag ( Qt::DropActions supportedActions )
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QTreeView::startDrag ( supportedActions );
}

void LocalDirTreeView::mousePressEvent ( QMouseEvent * event )
{
    if (event->button() == Qt::LeftButton ) {
        this->dragStartPosition = event->pos();
    }
    QTreeView::mousePressEvent ( event );
}
void LocalDirTreeView::mouseMoveEvent ( QMouseEvent * event )
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength()
        < QApplication::startDragDistance()) {
        return;
    }
    
    //有效性检查
    QItemSelectionModel * ism = this->selectionModel();
    if( ! this->indexAt( this->dragStartPosition ) .isValid() 
        || ism == 0 ) {
        QTreeView::mouseMoveEvent ( event );
        return;
    }
    emit drag_ready();
    
//     QDrag *drag = new QDrag(this);
//     QMimeData *mimeData = new QMimeData;
    // 
//     QModelIndexList mil = ism->selectedIndexes();
//     QList<QUrl>  drag_urls;
//     //drag_urls<< QUrl("nrsftp://heheh")<<QUrl("nrsftp://hehhefff");
//     for(int i = 0 ; i< mil.count() ;i += this->model()->columnCount() )
//     {
//         QModelIndex midx = mil.at(i);
//         drag_urls<< QUrl( QString("nrsftp://gzl:passwd@sf.net:22") + qobject_cast<RemoteDirModel*>(this->model())->filePath(midx) + "#1234556");
//     }
    //     
//     //mimeData->setData("text/uri-list" , "data");
//     mimeData->setUrls(drag_urls);
//     drag->setMimeData(mimeData);
    // 
//     Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction);
    
    //parent_event:
    //QTableView::mouseMoveEvent ( event );
}


