/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   liuguangzhao@users.sf.net   *
 *
 *   http://www.qtchina.net                                                *
 *   http://nullget.sourceforge.net                                        *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <QtCore>
#include <QtGui>

#include "remotedirmodel.h"
#include "remoteviewdirtableview.h"


RemoteViewDirTableView::RemoteViewDirTableView ( QWidget* parent ) : QTableView ( parent )
{}


RemoteViewDirTableView::~RemoteViewDirTableView()
{}
/**
 * 当源和目标都是同一主机的时候忽略此事件
 */
void RemoteViewDirTableView::dragEnterEvent ( QDragEnterEvent * event ) 
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //qDebug()<<event<<event->source()<<this<<this->parentWidget()->parentWidget()->parentWidget()<<event->source()->parentWidget()->parentWidget()->parentWidget();
    if( event->source() == this->parentWidget()->parentWidget()->parentWidget() 
        || event->source()->parentWidget()->parentWidget()->parentWidget() == this->parentWidget()->parentWidget()->parentWidget()){
        event->ignore();
    }else{
        QTableView::dragEnterEvent ( event )  ;
    }
}
void RemoteViewDirTableView::dragLeaveEvent ( QDragLeaveEvent * event ) 
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QTableView::dragLeaveEvent ( event ) ;
}
void RemoteViewDirTableView::dragMoveEvent ( QDragMoveEvent * event )
{
    //qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QTableView::dragMoveEvent (  event ) ;
}

void RemoteViewDirTableView::dropEvent ( QDropEvent * event ) 
{
    //qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QTableView::dropEvent ( event ) ;
}
void RemoteViewDirTableView::startDrag ( Qt::DropActions supportedActions )
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QTableView::startDrag ( supportedActions );
}

void RemoteViewDirTableView::mousePressEvent ( QMouseEvent * event )
{
    if(event->button() == Qt::LeftButton )
    {
        this->dragStartPosition = event->pos();
    }
    QTableView::mousePressEvent ( event );
}
void RemoteViewDirTableView::mouseMoveEvent ( QMouseEvent * event )
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength()
         < QApplication::startDragDistance())
        return;
    
    //有效性检查
    QItemSelectionModel * ism = this->selectionModel();
    if( ! this->indexAt( this->dragStartPosition ) .isValid() 
      || ism == 0 )
    {
        QTableView::mouseMoveEvent ( event );
        return ;
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
