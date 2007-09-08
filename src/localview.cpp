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


#include "localview.h"

#include "sftp-client.h"

#define REMOTE_CODEC "UTF-8"

LocalView::LocalView ( QWidget *parent )
		: QWidget ( parent )
{
	localView.setupUi ( this );
    ////
    this->status_bar = new QStatusBar();
    this->layout()->addWidget(this->status_bar);
    this->status_bar->showMessage("Ready");
    
    ////
	model = new QDirModel();
	this->localView.treeView->setModel ( model );
	this->localView.treeView->setRootIndex ( model->index ( QDir::homePath() ) );

	this->init_local_dir_tree_context_menu();
    
    ////////ui area custom
    this->localView.splitter->setStretchFactor(0,3);
    this->localView.splitter->setStretchFactor(1,1);
    this->localView.listView->setVisible(false);    //暂时没有功能在里面先隐藏掉
}


LocalView::~LocalView()
{}

void LocalView::init_local_dir_tree_context_menu()
{
	this->local_dir_tree_context_menu = new QMenu();

	QAction *action = new QAction ( "transer it",0 );

	this->local_dir_tree_context_menu->addAction ( action );

	QObject::connect ( this->localView.treeView,SIGNAL ( customContextMenuRequested ( const QPoint & ) ),
	                   this , SLOT ( slot_local_dir_tree_context_menu_request ( const QPoint & ) ) );
	QObject::connect ( action,SIGNAL ( triggered() ), this,SLOT ( slot_local_new_upload_requested() ) );

	////reresh action
	action = new QAction ( "refresh",0 );
	this->local_dir_tree_context_menu->addAction ( action );

	QObject::connect ( action,SIGNAL ( triggered() ),
	                   this,SLOT ( slot_refresh_directory_tree() ) );
}

void LocalView::set_sftp_connection ( struct sftp_conn* conn )
{
	this->sftp_connection = conn ;
}

// void LocalView::slot_remote_new_transfer_requested(QString filename)
// {
//     qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
//
//     qDebug()<<" remote will transer:"<<filename ;
//
//     QItemSelectionModel * ism = this->localView.treeView->selectionModel();
//
//     QModelIndexList mil = ism->selectedIndexes();
//
//     qDebug()<< mil ;
//
//     qDebug()<< model->fileName(mil.at(0));
//     qDebug()<< model->filePath(mil.at(0));
//     //qDebug()<< model->fileName(mil.at(2));
//     //do_download(this->sftp_connection,filename.toAscii().data(),
//     //            "/home/gzl/vmlinuz-2.6.18.2-34-xen",0);
//     //std::string remote_file = filename.toAscii().data();
//     char local_file[256];
//     char remote_file[256];
//     strcpy(remote_file,filename.toAscii().data() );
//     strcpy(local_file,model->filePath(mil.at(0)).toAscii().data() );
//     strcat(local_file,"/");
//     strcat(local_file,filename.split('/').at(filename.split('/').size()-1).toAscii().data() ) ;
//
//     qDebug()<<" remote file: "<<filename
//             << " save to local :" << local_file ;
//
//     do_download(this->sftp_connection,remote_file, local_file , 0);
// }

void LocalView::slot_local_dir_tree_context_menu_request ( const QPoint & pos )
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

	this->local_dir_tree_context_menu->popup ( this->mapToGlobal ( pos ) );

}

void LocalView::slot_local_new_upload_requested()
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	QTextCodec * codec = QTextCodec::codecForName(REMOTE_CODEC);
    
    QStringList local_file_names ;
    QString local_file_name;
    QByteArray ba;
    
    QItemSelectionModel * ism = this->localView.treeView->selectionModel();
    
	QModelIndexList mil = ism->selectedIndexes();

	qDebug() << mil ;

    for( int i = 0 ; i < mil.count() ; i += this->model->columnCount(QModelIndex()) )
    {
        qDebug() << model->fileName ( mil.at ( i ) );
        qDebug() << model->filePath ( mil.at ( i ) );
    
        local_file_name = model->filePath ( mil.at ( i ) );
        
        ba = codec->fromUnicode(local_file_name);
        //qDebug()<<" orginal name:"<< local_file_name <<" unicode name:"<< QString(ba.data() );
        local_file_name = ba ;
        //emit  new_upload_requested("/home/gzl/hehe.txt");
        //emit  new_upload_requested ( local_file_name , local_file_type );
        
        local_file_names << local_file_name;
    }
    emit   new_upload_requested( local_file_names );
}

QString LocalView::get_selected_directory()
{
    QTextCodec * codec = QTextCodec::codecForName(REMOTE_CODEC);
    
	QString local_path ;
	QItemSelectionModel * ism = this->localView.treeView->selectionModel();

	QModelIndexList mil = ism->selectedIndexes();

	qDebug() << mil ;

	qDebug() << model->fileName ( mil.at ( 0 ) );
	qDebug() << model->filePath ( mil.at ( 0 ) );

	QString local_file = model->filePath ( mil.at ( 0 ) );

	local_path = model->filePath ( mil.at ( 0 ) );

    QByteArray ba = codec->fromUnicode(local_path);
    qDebug()<<" orginal name:"<< local_path
            <<" unicode name:"<< QString(ba.data() );
    local_path = ba ;
	return local_path ;

}

void LocalView::slot_refresh_directory_tree()
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

	QItemSelectionModel * ism = this->localView.treeView->selectionModel();

	if ( ism !=0 )
	{
		QModelIndexList mil = ism->selectedIndexes();

		model->refresh ( mil.at ( 0 ) );
	}

}

void LocalView::closeEvent ( QCloseEvent * event )
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    event->ignore ();
    //this->setVisible(false); 
    QMessageBox::information(this,tr("attemp to close this window?"),tr("you cat's close this window."));
}





