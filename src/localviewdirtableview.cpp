// localviewdirtableview.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-06-16 17:43:57 +0800
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>

// #include "remotedirmodel.h"
#include "localviewdirtableview.h"

LocalViewDirTableView::LocalViewDirTableView(QWidget*parent) : QTableView(parent)
{

}

LocalViewDirTableView::~LocalViewDirTableView()
{
}

/**
 * 当源和目标都是同一主机的时候忽略此事件
 * and compat with native file manager
 */
void LocalViewDirTableView::dragEnterEvent(QDragEnterEvent *event) 
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    // qDebug()<<event<<event->source()<<this<<this->parentWidget()->parentWidget()->parentWidget()<<event->source()->parentWidget()->parentWidget()->parentWidget();
    // if( event->source() == this->parentWidget()->parentWidget()->parentWidget() 
    //     || event->source()->parentWidget()->parentWidget()->parentWidget() == this->parentWidget()->parentWidget()->parentWidget()){
    //     event->ignore();
    //     qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    // } else {
    //     qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //     QTableView::dragEnterEvent(event);
    // }

    qDebug()<<event<<event->source();
    if (event->source()) {
        qDebug()<<event->source()->parentWidget();
        qDebug()<<event->source()->parentWidget()->parentWidget();
        qDebug()<<event->source()->parentWidget()->parentWidget()->parentWidget();
    }
    qDebug()<<this<<this->parentWidget()->parentWidget()->parentWidget();

    if (event->source() == NULL) {
        // must be from native file manager, such as dolpin and so on.
        QTableView::dragEnterEvent(event);
    } else {
        if( event->source() == this->parentWidget()->parentWidget()->parentWidget() 
            || event->source()->parentWidget()->parentWidget()->parentWidget()
            == this->parentWidget()->parentWidget()->parentWidget()) {
            event->ignore();
        } else {
            QTableView::dragEnterEvent(event);
        }
    }

    // QTableView::dragEnterEvent(event);
}

// void LocalViewDirTableView::dragLeaveEvent(QDragLeaveEvent *event) 
// {
//     qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
//     QTableView::dragLeaveEvent(event ) ;
// }

void LocalViewDirTableView::dragMoveEvent(QDragMoveEvent *event)
{
    //qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QTableView::dragMoveEvent(event);
}

void LocalViewDirTableView::dropEvent(QDropEvent *event) 
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QTableView::dropEvent(event ) ;
}

// void LocalViewDirTableView::startDrag(Qt::DropActions supportedActions)
// {
//     qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
//     QTableView::startDrag(supportedActions);
// }

// void LocalViewDirTableView::mousePressEvent(QMouseEvent *event)
// {
//     if (event->button() == Qt::LeftButton) {
//         this->dragStartPosition = event->pos();
//     }
//     QTableView::mousePressEvent(event);
// }
// void LocalViewDirTableView::mouseMoveEvent(QMouseEvent *event)
// {
//     qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
//     if (!(event->buttons() & Qt::LeftButton))
//         return;
//     if ((event->pos() - dragStartPosition).manhattanLength()
//          < QApplication::startDragDistance())
//         return;
    
//     qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
//     //有效性检查
//     QItemSelectionModel *ism = this->selectionModel();
//     if (!this->indexAt(this->dragStartPosition).isValid() 
//         || ism == 0) {
//         QTableView::mouseMoveEvent(event);
//         return;
//     }
//     qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
//     emit drag_ready();
    
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
    //QTableView::mouseMoveEvent(event );
// }
