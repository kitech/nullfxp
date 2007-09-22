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

#include "sftp.h"
#include "sftp-operation.h"

#include "remoteview.h"





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
    
    this->in_remote_dir_retrive_loop = false;
}

void RemoteView::init_popup_context_menu()
{

    this->dir_tree_context_menu = new QMenu();
    QAction *action ;
    action  = new QAction(tr("Download"),0);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action,SIGNAL(triggered()),this,SLOT(slot_new_transfer()));
    
    action = new QAction("",0);
    action->setSeparator(true);
    this->dir_tree_context_menu->addAction(action);
    
    action = new QAction(tr("Refresh"),0);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action,SIGNAL(triggered()),this,SLOT(slot_refresh_directory_tree()));
    
    action = new QAction("",0);
    action->setSeparator(true);
    this->dir_tree_context_menu->addAction(action);
        
    action = new QAction(tr("Create directory..."),0);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action,SIGNAL(triggered()),this,SLOT(slot_mkdir()));
    
    action = new QAction(tr("Delete directory"),0);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action,SIGNAL(triggered()),this,SLOT(slot_rmdir()));

    action = new QAction(tr("Rename..."),0);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action,SIGNAL(triggered()),this,SLOT(slot_rename()));
    
    action = new QAction("",0);
    action->setSeparator(true);
    this->dir_tree_context_menu->addAction(action);
        
    //递归删除目录功能，删除文件的用户按钮
    action = new QAction(tr("Remove recursively !!!"),0);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action,SIGNAL(triggered()),this,SLOT(rm_file_or_directory_recursively()));
    
}

RemoteView::~RemoteView()
{
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
    QPoint real_pos = this->mapToGlobal(pos);
    real_pos = QPoint(real_pos.x()+12,real_pos.y()+36);
    this->dir_tree_context_menu->popup(real_pos);
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
    this->remote_dir_model->set_keep_alive(false);
    this->orginal_cursor = this->remoteview.treeView->cursor();
    this->remoteview.treeView->setCursor(Qt::BusyCursor);
}

void RemoteView::slot_leave_remote_dir_retrive_loop()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    this->remoteview.treeView->setCursor(this->orginal_cursor);    
    this->remote_dir_model->set_keep_alive(true);
    this->in_remote_dir_retrive_loop = false ;
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

void RemoteView::slot_mkdir()
{
    QString dir_name ;
    
    QItemSelectionModel *ism = this->remoteview.treeView->selectionModel();
    
    if(ism == 0)
    {
        qDebug()<<" why???? no QItemSelectionModel??";
        QMessageBox::critical(this,tr("waring..."),tr("maybe you haven't connected"));                
        return  ;
        return ;
    }
    
    QModelIndexList mil = ism->selectedIndexes()   ;
    
    if( mil.count() == 0 )
    {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        QMessageBox::critical(this,tr("waring..."),tr("no item selected"));
        return ;
    }
    
    QModelIndex midx = mil.at(0);
    directory_tree_item * dti = (directory_tree_item*) midx.internalPointer();
    
    //TODO 检查所选择的项是不是目录
    
    
    dir_name = QInputDialog::getText(this,tr("Create directory:"),
                                      tr("Input directory name:"),
                                         QLineEdit::Normal,
                                         tr("new_direcotry") );
     
    if(  dir_name.length () == 0 )
    {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        QMessageBox::critical(this,tr("waring..."),tr("no directory name supplyed "));
        return;
    }
    //TODO 将 file_path 转换编码再执行下面的操作
    
    this->remote_dir_model->slot_execute_command(dti,midx.internalPointer(),SSH2_FXP_MKDIR,std::string(dir_name.toAscii().data()) );
    
}

void RemoteView::slot_rmdir()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QItemSelectionModel *ism = this->remoteview.treeView->selectionModel();
    
    if(ism == 0)
    {
        qDebug()<<" why???? no QItemSelectionModel??";
        QMessageBox::critical(this,tr("waring..."),tr("maybe you haven't connected"));                
        return  ;
        return ;
    }
    
    QModelIndexList mil = ism->selectedIndexes()   ;
    
    if( mil.count() == 0 )
    {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        QMessageBox::critical(this,tr("waring..."),tr("no item selected"));
        return ;
    }
    
    QModelIndex midx = mil.at(0);
    directory_tree_item * dti = (directory_tree_item*) midx.internalPointer();
    QModelIndex parent_model =  midx.parent() ;
    directory_tree_item * parent_item = (directory_tree_item*)parent_model.internalPointer();
    
    //TODO 检查所选择的项是不是目录
    
    this->remote_dir_model->slot_execute_command(parent_item,parent_model.internalPointer() ,SSH2_FXP_RMDIR,std::string( dti->file_name ) );
    
}

void RemoteView::rm_file_or_directory_recursively()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QItemSelectionModel *ism = this->remoteview.treeView->selectionModel();
    
    if(ism == 0)
    {
        qDebug()<<" why???? no QItemSelectionModel??";
        QMessageBox::critical(this,tr("waring..."),tr("maybe you haven't connected"));                
        return  ;
        return ;
    }
    
    QModelIndexList mil = ism->selectedIndexes()   ;
    
    if( mil.count() == 0 )
    {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        QMessageBox::critical(this,tr("waring..."),tr("no item selected"));
        return ;
    }
    
    QModelIndex midx = mil.at(0);
    directory_tree_item * dti = (directory_tree_item*) midx.internalPointer();
    QModelIndex parent_model =  midx.parent() ;
    directory_tree_item * parent_item = (directory_tree_item*)parent_model.internalPointer();
    
    this->remote_dir_model->slot_execute_command(parent_item,parent_model.internalPointer() ,SSH2_FXP_REMOVE , std::string( dti->file_name ) );
}

void RemoteView::slot_rename()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QItemSelectionModel *ism = this->remoteview.treeView->selectionModel();
    
    if(ism == 0)
    {
        qDebug()<<" why???? no QItemSelectionModel??";
        QMessageBox::critical(this,tr("waring..."),tr("maybe you haven't connected"));                
        return  ;
        return ;
    }
    
    QModelIndexList mil = ism->selectedIndexes()   ;
    
    if( mil.count() == 0 )
    {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        QMessageBox::critical(this,tr("waring..."),tr("no item selected"));
        return ;
    }
    
    QModelIndex midx = mil.at(0);
    directory_tree_item * dti = (directory_tree_item*) midx.internalPointer();
    QModelIndex parent_model =  midx.parent() ;
    directory_tree_item * parent_item = (directory_tree_item*)parent_model.internalPointer();
    
    QString rename_to ;
    rename_to = QInputDialog::getText(this,tr("Rename to:"),
                                      tr("Input new name:"),
                                         QLineEdit::Normal,
                                         tr("rename") );
     
    if(  rename_to.length () == 0 )
    {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        QMessageBox::critical(this,tr("waring..."),tr("no new name supplyed "));
        return;
    }
    
    //TODO 将 rename_to 转换编码再执行下面的操作
    
    this->remote_dir_model->slot_execute_command(parent_item,parent_model.internalPointer() ,SSH2_FXP_RENAME , std::string( dti->file_name + "!" + std::string( rename_to.toAscii().data() ) ) );
    
}

