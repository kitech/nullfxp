/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   liuguangzhao@users.sourceforge.net   *
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
#include "localview.h"
#include "globaloption.h"

//#define REMOTE_CODEC "UTF-8"

LocalView::LocalView ( QWidget *parent )
		: QWidget ( parent )
{
	localView.setupUi ( this );
    ////
    this->status_bar = new QStatusBar();
    this->layout()->addWidget(this->status_bar);
    this->status_bar->showMessage(tr("Ready"));
    
    ////
	model = new QDirModel();
	this->localView.treeView->setModel ( model );
	this->localView.treeView->setRootIndex ( model->index ( "/" ) );
    //this->localView.treeView->setRootIndex ( model->index ( QDir::homePath() ) );
    this->localView.treeView->setColumnWidth(0,this->localView.treeView->columnWidth(0)*2);    
    this->expand_to_home_directory(this->localView.treeView->rootIndex (),1 );
    
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

	QAction *action = new QAction ( tr("Upload"),0 );

	this->local_dir_tree_context_menu->addAction ( action );

	QObject::connect ( this->localView.treeView,SIGNAL ( customContextMenuRequested ( const QPoint & ) ),
	                   this , SLOT ( slot_local_dir_tree_context_menu_request ( const QPoint & ) ) );
	QObject::connect ( action,SIGNAL ( triggered() ), this,SLOT ( slot_local_new_upload_requested() ) );

    action = new QAction("",0);
    action->setSeparator(true);
    this->local_dir_tree_context_menu->addAction ( action );
    
	////reresh action
	action = new QAction ( tr("Refresh"),0 );
	this->local_dir_tree_context_menu->addAction ( action );

	QObject::connect ( action,SIGNAL ( triggered() ),
	                   this,SLOT ( slot_refresh_directory_tree() ) );
}
//仅会被调用一次，在该实例的构造函数中
void LocalView::expand_to_home_directory( QModelIndex parent_model,int level )
{
    int row_cnt = this->model->rowCount(parent_model) ;
    QString home_path = QDir::homePath();
    QStringList home_path_grade = home_path.split('/');
    //qDebug()<<home_path_grade << level ;
    QModelIndex curr_model ;
    for( int i = 0 ; i < row_cnt ; i ++)
    {
        curr_model = this->model->index(i,0,parent_model) ;
        QString file_name = this->model->fileName(curr_model);
        if( file_name == home_path_grade.at(level) )
        {
            this->localView.treeView->expand(curr_model);
            if( level == home_path_grade.count()-1)
            {
                break;
            }
            else
            {
                this->expand_to_home_directory(curr_model,level+1);
                break;
            }
        }
    }
    if( level == 1 )
    {
        this->localView.treeView->scrollTo( curr_model );
    }
    //qDebug()<<" root row count:"<< row_cnt ;
}

void LocalView::slot_local_dir_tree_context_menu_request ( const QPoint & pos )
{
	//qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QPoint real_pos = this->mapToGlobal(pos);
    real_pos = QPoint(real_pos.x()+12,real_pos.y()+36);
	this->local_dir_tree_context_menu->popup ( real_pos );

}

void LocalView::slot_local_new_upload_requested()
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	//QTextCodec * codec = QTextCodec::codecForName(REMOTE_CODEC);
    QTextCodec * codec = GlobalOption::instance()->locale_codec ;
    
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
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    //QTextCodec * codec = QTextCodec::codecForName(REMOTE_CODEC);
    QTextCodec * codec = GlobalOption::instance()->locale_codec ;
    
	QString local_path ;
	QItemSelectionModel * ism = this->localView.treeView->selectionModel();

    if( ism == 0 )
    {        
        return QString();
    }
    
    
	QModelIndexList mil = ism->selectedIndexes();

    if(mil.count() == 0 )
    {
        return QString();
    }
    
	//qDebug() << mil ;

	//qDebug() << model->fileName ( mil.at ( 0 ) );
	//qDebug() << model->filePath ( mil.at ( 0 ) );

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
void LocalView::update_layout()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    this->model->refresh(QModelIndex());
}

void LocalView::closeEvent ( QCloseEvent * event )
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    event->ignore ();
    //this->setVisible(false); 
    QMessageBox::information(this,tr("attemp to close this window?"),tr("you cat's close this window."));
}





