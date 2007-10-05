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

#include "globaloption.h"
#include "utils.h"

#include "progressdialog.h"
#include "localview.h"
#include "remoteview.h"

#include "fileproperties.h"

RemoteView::RemoteView(LocalView * local_view ,QWidget *parent)
 : QWidget(parent)
{
    this->remoteview.setupUi(this);
    this->local_view = local_view ;
    ///////
    status_bar = new QStatusBar(  );    
    this->layout()->addWidget(status_bar);
    status_bar->showMessage(tr("Ready"));
    ////////////
    
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
    
    action = new QAction(tr("Properties..."),0);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action,SIGNAL(triggered()),this,SLOT(slot_show_properties()));
    attr_action = action ;
    
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
    this->remoteview.treeView->setModel(0);
    delete this->remote_dir_model ;
}

void RemoteView::slot_show_fxp_command_log(bool show)
{
    this->remoteview.listView->setVisible(show);    
}

void RemoteView::i_init_dir_view( )
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    this->remote_dir_model = new RemoteDirModel( );
    this->remote_dir_model->set_ssh2_handler(this->ssh2_sess,this->ssh2_sftp,this->ssh2_sock );
    
    this->remote_dir_model->set_user_home_path(this->user_home_path);
    
    this->remoteview.treeView->setModel(this->remote_dir_model);
    this->remoteview.treeView->setAcceptDrops(true);
    this->remoteview.treeView->setDragEnabled(true);
    this->remoteview.treeView->setDropIndicatorShown(true);
    this->remoteview.treeView->setDragDropMode(QAbstractItemView::DragDrop);            
    QObject::connect(this->remote_dir_model,SIGNAL(new_transfer_requested(QStringList,QStringList)),
                     this,SLOT(slot_new_upload_requested(QStringList,QStringList )) ) ;
    
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
    attr_action->setEnabled( ! this->in_remote_dir_retrive_loop );
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
        qDebug()<<dti->file_name<<" "<<dti->file_type
                <<" "<< dti->strip_path  ;
        file_path = dti->strip_path;
        //file_type = dti->file_type  ;
        remote_file_names << file_path ;
    }
    
    //emit new_transfer_requested("/vmlinuz-2.6.18.2-34-xen");
    //emit new_transfer_requested(file_path,file_type );
    //emit new_transfer_requested(remote_file_names);
    //这里是指的下载文件
    this->slot_new_download_requested( remote_file_names ) ;
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
        qDebug()<<dti->file_name <<" "<<dti->file_type 
                <<" "<< dti->strip_path  ;
        file_path = dti->strip_path ;
        if( dti->file_type.at(0) == 'd' || dti->file_type.at(0) == 'D'
            || dti->file_type.at(0) == 'l' )
        {
            //TODO dti->file_type.at(0) == 'l' ，这时不一定是目录，可能会出错。
        }
        else
        {
            file_path = "" ;
        }
    }
    
    return file_path ;

}

void RemoteView::set_ssh2_handler( void * ssh2_sess , void * ssh2_sftp, int ssh2_sock )
{
    this->ssh2_sess = (LIBSSH2_SESSION*) ssh2_sess ;
    this->ssh2_sftp = (LIBSSH2_SFTP * ) ssh2_sftp ;
    this->ssh2_sock = ssh2_sock ;
}
LIBSSH2_SESSION * RemoteView::get_ssh2_sess()
{
    return this->ssh2_sess ;
}
LIBSSH2_SFTP * RemoteView::get_ssh2_sftp ()
{
    return this->ssh2_sftp ;
}
int RemoteView::get_ssh2_sock ( )
{
    return this->ssh2_sock ;
}

void RemoteView::set_host_info( QString host_name , QString   user_name , QString password )
{

    this->host_name = host_name ;
    this->user_name = user_name ;
    this->password = password ;

    this->setWindowTitle(this->windowTitle() + ": " + this->user_name + "@" + this->host_name );
}

void RemoteView::set_user_home_path(std::string user_home_path)
{
    this->user_home_path = user_home_path ;
}

void RemoteView::closeEvent ( QCloseEvent * event ) 
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    event->ignore();
    if( this->in_remote_dir_retrive_loop )
    {
        //TODO 怎么能友好的结束
        //QMessageBox::warning(this,tr("attentions:"),tr("retriving remote directory tree,wait a minute please.") );
        //return ;
    }
    //this->setVisible(false);
    if( QMessageBox::question(this,tr("Attemp to close this window?"),tr("Are you sure  disconnect from %1?").arg(this->windowTitle()) ,QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Cancel ) == QMessageBox::Ok )
    {
        this->setVisible(false);
        qDebug()<<"delete remote view";
        delete this ;
    }
}
void RemoteView::slot_custom_ui_area()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //这个设置必须在show之前设置才有效果
    this->remoteview.splitter_2->setStretchFactor(0,5);
    this->remoteview.splitter_2->setStretchFactor(1,1);

    this->remoteview.splitter->setStretchFactor(0,3);
    this->remoteview.splitter->setStretchFactor(1,1);
    //this->remoteview.listView_2->setVisible(false);//暂时没有功能在里面先隐藏掉
    this->remoteview.tableView->setVisible(false);
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
        qDebug()<<dti->file_name<<" "<<dti->file_type
                <<" "<< dti->strip_path ;
        file_path = dti->strip_path ;
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

void RemoteView::slot_show_properties()
{
    QItemSelectionModel *ism = this->remoteview.treeView->selectionModel();
    
    if(ism == 0)
    {
        qDebug()<<" why???? no QItemSelectionModel??";        
        return ;
    }
    
    QModelIndexList mil = ism->selectedIndexes()   ;
    //  文件类型，大小，几个时间，文件权限
    //TODO 从模型中取到这些数据并显示在属性对话框中。
    FileProperties * fp = new FileProperties(this);
    fp->set_ssh2_sftp( this->ssh2_sftp );
    fp->set_file_info_model_list(mil);
    fp->exec();
    delete fp ;
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
    
    this->remote_dir_model->slot_execute_command(dti,midx.internalPointer(),SSH2_FXP_MKDIR,dir_name );
    
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
    
    this->remote_dir_model->slot_execute_command(parent_item,parent_model.internalPointer() ,SSH2_FXP_RMDIR, dti->file_name   );
    
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
    //TODO 处理多选的情况
    QModelIndex midx = mil.at(0);
    directory_tree_item * dti = (directory_tree_item*) midx.internalPointer();
    QModelIndex parent_model =  midx.parent() ;
    directory_tree_item * parent_item = (directory_tree_item*)parent_model.internalPointer();
    
    this->remote_dir_model->slot_execute_command(parent_item,parent_model.internalPointer() ,SSH2_FXP_REMOVE ,  dti->file_name   );
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
    rename_to = QInputDialog::getText(this,tr("Rename to:"),  tr("Input new name:"),
                                         QLineEdit::Normal, tr("Rename to") );
     
	if(  rename_to  == QString::null )
    {
        //qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        //QMessageBox::critical(this,tr("Waring..."),tr("No new name supplyed "));
        return;
    }
	if( rename_to.length() == 0 )
	{
		QMessageBox::critical(this,tr("Waring..."),tr("No new name supplyed "));
		return ;
	}

    this->remote_dir_model->slot_execute_command(parent_item,parent_model.internalPointer() ,SSH2_FXP_RENAME ,  dti->file_name + "!" + rename_to );
}

void RemoteView::slot_new_upload_requested ( QStringList local_file_names,  QStringList remote_file_names )
{
    if( this->in_remote_dir_retrive_loop )
    {
        QMessageBox::warning(this,tr("Attentions:"),tr("retriving remote directory tree,wait a minute please.") );
        return ;
    }
    else
    {
        //emit this->new_transfer_requested(local_file_names,remote_file_names);
        ProgressDialog * pdlg = new ProgressDialog ( this );
        pdlg->set_remote_connection ( this->get_ssh2_sess() ,
                                      this->get_ssh2_sftp(),
                                              this->get_ssh2_sock()  );
    
        pdlg->set_transfer_info ( TransferThread::TRANSFER_PUT,local_file_names , remote_file_names ) ;
        QObject::connect ( pdlg,SIGNAL ( transfer_finished ( int ) ),
                           this,SLOT ( slot_transfer_finished ( int ) ) );
        this->slot_enter_remote_dir_retrive_loop();
        pdlg->exec();
    }
}
void RemoteView::slot_new_upload_requested ( QStringList local_file_names ) 
{
    QString remote_file_name ;
    QStringList remote_file_names ;

    RemoteView * remote_view = this/*->get_top_most_remote_view()*/ ;

    qDebug()<<" window title :" << remote_view->windowTitle() ;
    if ( remote_view->is_in_remote_dir_retrive_loop() )
    {
        QMessageBox::warning ( this,tr ( "attentions:" ),tr ( "retriving remote directory tree,wait a minute please." ) );
        return ;
    }

    remote_file_name = remote_view->get_selected_directory();
    remote_file_names << remote_file_name ;

    if ( remote_file_name.length() == 0 )
    {
        qDebug() <<" selected a remote file directory  please";
        QMessageBox::critical ( this,tr ( "Waring..." ),tr ( "you should selecte a remote file directory." ) );        
    }
    else
    {
        ProgressDialog * pdlg = new ProgressDialog ( this );
        pdlg->set_remote_connection ( remote_view->get_ssh2_sess() ,
                                      remote_view->get_ssh2_sftp(),
                                              remote_view->get_ssh2_sock()  );

        pdlg->set_transfer_info ( TransferThread::TRANSFER_PUT,local_file_names , remote_file_names ) ;
// 		QObject::connect ( pdlg,SIGNAL ( transfer_finished ( int ) ),
// 		                   this,SLOT ( slot_transfer_finished ( int ) ) );
        QObject::connect ( pdlg,SIGNAL ( transfer_finished ( int ) ),
                           remote_view , SLOT ( slot_transfer_finished ( int ) ) );        
        remote_view->slot_enter_remote_dir_retrive_loop();
        pdlg->exec();
    }
}

void RemoteView::slot_new_download_requested(QStringList local_file_names,   QStringList remote_file_names)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    qDebug() << local_file_names << remote_file_names ;
    
    RemoteView * remote_view = this/*->get_top_most_remote_view()*/ ;
        
	ProgressDialog *pdlg = new ProgressDialog ( this );
    pdlg->set_remote_connection ( remote_view->get_ssh2_sess() ,
                                  remote_view->get_ssh2_sftp(),
                                          remote_view->get_ssh2_sock()  );
	pdlg->set_transfer_info ( TransferThread::TRANSFER_GET,local_file_names,remote_file_names );
	QObject::connect ( pdlg,SIGNAL ( transfer_finished ( int ) ),
	                   this,SLOT ( slot_transfer_finished ( int ) ) );
    remote_view->slot_enter_remote_dir_retrive_loop();
	pdlg->exec();
}
void RemoteView::slot_new_download_requested( QStringList remote_file_names ) 
{
	QStringList local_file_names ;
    
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	QString local_file_path  ;
        
    RemoteView * remote_view = this/*->get_top_most_remote_view() */;
    
	local_file_path = this->local_view->get_selected_directory();
	local_file_names << local_file_path ;
    
	if ( local_file_path.length() == 0 || ! is_dir( GlobalOption::instance()->locale_codec->fromUnicode( local_file_path  ).data() ) )
	{
		qDebug() <<" selected a local file directory  please";
		QMessageBox::critical ( this,tr ( "waring..." ),tr ( "you should selecte a local file directory." ) );
	}
	else
	{
    
		ProgressDialog *pdlg = new ProgressDialog ( this );
        pdlg->set_remote_connection ( remote_view->get_ssh2_sess() ,
                                      remote_view->get_ssh2_sftp(),
                                              remote_view->get_ssh2_sock()  );
		pdlg->set_transfer_info ( TransferThread::TRANSFER_GET,local_file_names,remote_file_names );
		QObject::connect ( pdlg,SIGNAL ( transfer_finished ( int ) ),
		                   this,SLOT ( slot_transfer_finished ( int ) ) );
        remote_view->slot_enter_remote_dir_retrive_loop();
		pdlg->exec();
	}
}

void RemoteView::slot_transfer_finished( int status ) 
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    RemoteView * remote_view = this/*->get_top_most_remote_view()*/ ;
    
    ProgressDialog * pdlg = ( ProgressDialog* ) sender();

    if ( status != 0 )
    {
        QMessageBox::critical ( this,QString ( tr ( "Error: " ) ),
                                QString ( tr ( "Unknown error: %1         " ) ).arg ( status ) );
    }
    else
    {
		//TODO 通知UI更新目录结构
        int transfer_type = pdlg->get_transfer_type();
        if ( transfer_type == TransferThread::TRANSFER_GET )
        {
            this->local_view->update_layout();
        }
        else if ( transfer_type == TransferThread::TRANSFER_PUT )
        {
            remote_view->update_layout();
        }
        else
        {
			// xxxxx: 没有预期到的错误
            assert ( 1== 2 );
        }
    }
    delete pdlg ;
    remote_view->slot_leave_remote_dir_retrive_loop();
}


