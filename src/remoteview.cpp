/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   gzl@localhost   *
 *                                                                         *
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


#include "remoteview.h"


#include "sftp-operation.h"


RemoteView::RemoteView(QWidget *parent)
 : QWidget(parent)
{
    this->remoteview.setupUi(this);
    ///////
    status_bar = new QStatusBar(  );    
    this->layout()->addWidget(status_bar);
    status_bar->showMessage("Ready");
    ////////////
    memset(this->m_curr_path,0,sizeof(this->m_curr_path));
    memset(this->m_next_path,0,sizeof(this->m_next_path) );
    this->m_curr_path[0] = '/';
    this->m_next_path[0] = '/';
    
    this->remoteview.treeView->setAcceptDrops(true);
    this->remoteview.treeView->setDragEnabled(true);
    this->remoteview.treeView->setDropIndicatorShown(true);
    this->remoteview.treeView->setDragDropMode(QAbstractItemView::DragDrop);
    //this->remoteview.treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    
    QObject::connect(this->remoteview.treeView,SIGNAL(customContextMenuRequested(const QPoint &)),
                     this,SLOT(slot_dir_tree_customContextMenuRequested (const QPoint & )) );
    QObject::connect( this->remoteview.treeView,SIGNAL(clicked(const QModelIndex & )),
                      this,SLOT(slot_dir_item_clicked(const QModelIndex & ))) ;
    
    this->init_popup_context_menu();
    
    this->keep_alive = false ;
    this->keep_alive_timer = new QTimer();
    this->in_remote_dir_retrive_loop = false;
    this->keep_alive_interval = 30 ;
}

void RemoteView::init_popup_context_menu()
{

    this->dir_tree_context_menu = new QMenu();
    QAction *action ;
    action  = new QAction("transfer",0);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action,SIGNAL(triggered()),this,SLOT(slot_new_transfer()));
    
    action = new QAction(tr("Refresh"),0);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action,SIGNAL(triggered()),this,SLOT(slot_refresh_directory_tree()));
    
}

RemoteView::~RemoteView()
{
    if(this->keep_alive_timer->isActive() )
    {
        this->keep_alive_timer->stop();
    }
    delete this->keep_alive_timer ;
    
}
void RemoteView::slot_show_fxp_command_log(bool show)
{
    this->remoteview.listView->setVisible(show);    
}

void RemoteView::i_init_dir_view(struct sftp_conn * conn)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    this->remote_dir_model = new RemoteDirModel(conn);
    this->remote_dir_model->set_user_home_path(this->user_home_path);
    
    this->remoteview.treeView->setModel(this->remote_dir_model);
    this->remoteview.treeView->setAcceptDrops(true);
    this->remoteview.treeView->setDragEnabled(true);
    this->remoteview.treeView->setDropIndicatorShown(true);
    this->remoteview.treeView->setDragDropMode(QAbstractItemView::DragDrop);            
    //do_globbed_ls( conn , this->m_next_path , this->m_curr_path, 0 );
    QObject::connect(this->remote_dir_model,SIGNAL(new_transfer_requested(QStringList,QStringList)),
                     this,SLOT(slot_new_transfer_requested(QStringList,QStringList )) ) ;
    
    QObject::connect( this->remote_dir_model,SIGNAL(enter_remote_dir_retrive_loop()),
                      this,SLOT(slot_enter_remote_dir_retrive_loop()));
    QObject::connect( this->remote_dir_model,SIGNAL(leave_remote_dir_retrive_loop()),
                      this,SLOT(slot_leave_remote_dir_retrive_loop()));
    
    this->remoteview.treeView->expandAll();
    this->remoteview.treeView->setColumnWidth(0,this->remoteview.treeView->columnWidth(0)*2);
}

void RemoteView::slot_disconnect_from_remote_host()
{
    this->remoteview.treeView->setModel(0);
    delete this->remote_dir_model ;
    this->remote_dir_model = 0 ;
}

void RemoteView::slot_dir_tree_customContextMenuRequested ( const QPoint & pos )
{
    
    this->dir_tree_context_menu->popup(this->mapToGlobal(pos));
}

void RemoteView::slot_new_transfer()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
     
    QString file_path   ;
    QStringList remote_file_names;
    
    if( this->in_remote_dir_retrive_loop )
    {
        QMessageBox::warning(this,tr("attentions:"),tr("retriving remote directory tree,wait a minute please.") );
        return ;
    }
    
    QItemSelectionModel *ism = this->remoteview.treeView->selectionModel();
    
    if( ism == 0 )
    {
        QMessageBox::critical(this,tr("waring..."),tr("maybe you haven't connected"));
        return ;
    }
    
    QModelIndexList mil = ism->selectedIndexes()   ;
    
    for(int i = 0 ; i < mil.size(); i +=4 )
    {
        QModelIndex midx = mil.at(i);
        qDebug()<<midx ;
        directory_tree_item * dti = (directory_tree_item*) midx.internalPointer();
        qDebug()<<dti->file_name.c_str()<<" "<<dti->file_type.c_str()
                <<" "<< dti->strip_path.c_str() ;
        file_path = dti->strip_path.c_str();
        //file_type = dti->file_type.c_str() ;
        remote_file_names << file_path ;
    }
    
    //emit new_transfer_requested("/vmlinuz-2.6.18.2-34-xen");
    //emit new_transfer_requested(file_path,file_type );
    emit new_transfer_requested(remote_file_names);
}

void RemoteView::slot_new_transfer_requested(QStringList local_file_names,                                    QStringList remote_file_names)
{
    if( this->in_remote_dir_retrive_loop )
    {
        QMessageBox::warning(this,tr("attentions:"),tr("retriving remote directory tree,wait a minute please.") );
        return ;
    }
    else
    {
        emit this->new_transfer_requested(local_file_names,remote_file_names);
    }
}

QString RemoteView::get_selected_directory()
{
    QString file_path ;
    
    QItemSelectionModel *ism = this->remoteview.treeView->selectionModel();
    
    if(ism == 0)
    {
        QMessageBox::critical(this,tr("waring..."),tr("maybe you haven't connected"));                
        return file_path ;
    }
    
    QModelIndexList mil = ism->selectedIndexes()   ;
    
    for(int i = 0 ; i < mil.size(); i +=4 )
    {
        QModelIndex midx = mil.at(i);
        qDebug()<<midx ;
        directory_tree_item * dti = (directory_tree_item*) midx.internalPointer();
        qDebug()<<dti->file_name.c_str()<<" "<<dti->file_type.c_str()
                <<" "<< dti->strip_path.c_str() ;
        file_path = dti->strip_path.c_str();
    }
    
    return file_path ;

}


void RemoteView::set_user_home_path(std::string user_home_path)
{
    this->user_home_path = user_home_path ;
}

void RemoteView::closeEvent ( QCloseEvent * event ) 
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    event->ignore();
    //this->setVisible(false);
    QMessageBox::information(this,tr("attemp to close this window?"),tr("you cat's close this window."));
}
void RemoteView::slot_custom_ui_area()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    this->remoteview.splitter_2->setStretchFactor(0,5);
    this->remoteview.splitter_2->setStretchFactor(1,1);

    this->remoteview.splitter->setStretchFactor(0,3);
    this->remoteview.splitter->setStretchFactor(1,1);
    this->remoteview.listView_2->setVisible(false);//暂时没有功能在里面先隐藏掉

}

void RemoteView::slot_dir_item_clicked(const QModelIndex & index)
{
    //qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    assert( remote_dir_model != 0 );

    remote_dir_model->slot_remote_dir_node_clicked(index);

}

void RemoteView::slot_enter_remote_dir_retrive_loop()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    this->in_remote_dir_retrive_loop = true ;
    this->orginal_cursor = this->remoteview.treeView->cursor();
    this->remoteview.treeView->setCursor(Qt::BusyCursor);
}

void RemoteView::slot_leave_remote_dir_retrive_loop()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    this->remoteview.treeView->setCursor(this->orginal_cursor);
    this->in_remote_dir_retrive_loop = false ;
    
}

//TODO
void RemoteView::set_keep_alive(bool keep_alive,int time_out)
{
    //this->keep_alive_interval = time_out ;
    //this->keep_alive = keep_alive ;
    if( keep_alive != this->keep_alive )
    {
        if( this->keep_alive == true )
        {
            this->keep_alive_timer->stop();
        }
        else
        {
            this->keep_alive_timer->start();
        }
        this->keep_alive = keep_alive ;
    }
    if( time_out != this->keep_alive_interval)
    {
        this->keep_alive_interval = time_out ;
        this->keep_alive_timer->setInterval(this->keep_alive_interval);
    }
}

void RemoteView:: slot_keep_alive_time_out()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
}

void RemoteView::update_layout()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QString file_path ;
    
    QItemSelectionModel *ism = this->remoteview.treeView->selectionModel();
    
    if(ism == 0)
    {
        //QMessageBox::critical(this,tr("waring..."),tr("maybe you haven't connected"));                
        //return file_path ;
        qDebug()<<" why???? no QItemSelectionModel??";
        
        return ;
    }
    
    QModelIndexList mil = ism->selectedIndexes()   ;
    
    if( mil.count() == 0 )
    {
            qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
    }
    
    for(int i = 0 ; i < mil.size(); i +=4 )
    {
        QModelIndex midx = mil.at(i);
        qDebug()<<midx ;
        directory_tree_item * dti = (directory_tree_item*) midx.internalPointer();
        qDebug()<<dti->file_name.c_str()<<" "<<dti->file_type.c_str()
                <<" "<< dti->strip_path.c_str() ;
        file_path = dti->strip_path.c_str();
        dti->retrived = 1;
        dti->prev_retr_flag=9;
        this->remote_dir_model->slot_remote_dir_node_clicked(midx);
    }
    
    //return file_path ;
    
    //this->remote_dir_model->update_layout();
}

void RemoteView::slot_refresh_directory_tree()
{
    this->update_layout();
}

