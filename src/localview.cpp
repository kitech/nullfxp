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
#include "fileproperties.h"

//#define REMOTE_CODEC "UTF-8"

LocalView::LocalView ( QWidget *parent )
		: QWidget ( parent )
{
	localView.setupUi ( this );
    this->setObjectName("lv");
    ////
    this->status_bar = new QStatusBar();
    this->layout()->addWidget(this->status_bar);
    this->status_bar->showMessage(tr("Ready"));
    
    ////
	model = new QDirModel();
//     model->setFilter( QDir::AllEntries|QDir::Hidden|QDir::NoDotAndDotDot );
    this->dir_file_model = new LocalDirSortFilterModel(  );
    this->dir_file_model->setSourceModel(model);
    
    this->localView.treeView->setModel ( this->dir_file_model );
    this->localView.treeView->setRootIndex (this->dir_file_model->index ( "/" ) );
//     this->localView.treeView->setColumnHidden( 1, true);
//     this->localView.treeView->setColumnHidden( 2, true);
//     this->localView.treeView->setColumnHidden( 3, true);
    //this->localView.treeView->setRootIndex ( model->index ( QDir::homePath() ) );
    this->localView.treeView->setColumnWidth(0,this->localView.treeView->columnWidth(0)*2);    
    this->expand_to_home_directory(this->localView.treeView->rootIndex (),1 );
    
	this->init_local_dir_tree_context_menu();

    this->localView.tableView->setModel( this->model );
    this->localView.tableView->setRootIndex( this->model->index( QDir::homePath() ) );
    this->localView.tableView->verticalHeader()->setVisible(false);

    //change row height of table 
    if( this->model->rowCount( this->model->index( QDir::homePath() ) ) > 0 )
    {
        this->table_row_height = this->localView.tableView->rowHeight(0)*2/3;
    }
    else
    {
        this->table_row_height = 20 ;
    }
    for( int i = 0 ; i < this->model->rowCount( this->model->index( QDir::homePath() ) ); i ++ )
    {
        this->localView.tableView->setRowHeight( i, this->table_row_height );
    }

    this->localView.tableView->resizeColumnToContents( 0 );
    /////
    QObject::connect(this->localView.treeView,SIGNAL(clicked(const QModelIndex &)),
                     this,SLOT( slot_dir_tree_item_clicked(const QModelIndex &))) ;
    QObject::connect( this->localView.tableView,SIGNAL( doubleClicked ( const QModelIndex &  ) ) , this,SLOT( slot_dir_file_view_double_clicked ( const QModelIndex & ) ) );
    
    ////////ui area custom
    this->localView.splitter->setStretchFactor(0,1);
    this->localView.splitter->setStretchFactor(1,2);
    //this->localView.listView->setVisible(false);    //暂时没有功能在里面先隐藏掉
}


LocalView::~LocalView()
{}

void LocalView::init_local_dir_tree_context_menu()
{
	this->local_dir_tree_context_menu = new QMenu();

	QAction *action = new QAction ( tr("Upload"),0 );

	this->local_dir_tree_context_menu->addAction ( action );

	QObject::connect ( action,SIGNAL ( triggered() ), this,SLOT ( slot_local_new_upload_requested() ) );

    action = new QAction("",0);
    action->setSeparator(true);
    this->local_dir_tree_context_menu->addAction ( action );
    
	////reresh action
	action = new QAction ( tr("Refresh"),0 );
	this->local_dir_tree_context_menu->addAction ( action );

	QObject::connect ( action,SIGNAL ( triggered() ),
	                   this,SLOT ( slot_refresh_directory_tree() ) );
                       
    action = new QAction(tr("Properties..."),0);
    this->local_dir_tree_context_menu->addAction(action);
    QObject::connect(action,SIGNAL(triggered()),this,SLOT(slot_show_properties()));
    //////
    action = new QAction ( tr("Show Hidden"),0);
    action->setCheckable(true);
    this->local_dir_tree_context_menu->addAction ( action );
    QObject::connect(action, SIGNAL(toggled(bool)), this, SLOT(slot_show_hidden(bool)));
    action = new QAction("",0);
    action->setSeparator(true);
    this->local_dir_tree_context_menu->addAction ( action );
    
    action = new QAction(tr("Create directory..."),0);
    this->local_dir_tree_context_menu->addAction(action);
    QObject::connect(action,SIGNAL(triggered()),this,SLOT(slot_mkdir()));
    action = new QAction ( tr("Rename ..."),0);
    this->local_dir_tree_context_menu->addAction ( action );
    QObject::connect(action, SIGNAL(triggered()),this,SLOT(slot_rename()));
    action = new QAction("",0);
    action->setSeparator(true);
    this->local_dir_tree_context_menu->addAction ( action );
    
    
    QObject::connect ( this->localView.treeView,SIGNAL ( customContextMenuRequested ( const QPoint & ) ),
                       this , SLOT ( slot_local_dir_tree_context_menu_request ( const QPoint & ) ) );
    QObject::connect ( this->localView.tableView,SIGNAL ( customContextMenuRequested ( const QPoint & ) ),
                       this , SLOT ( slot_local_dir_tree_context_menu_request ( const QPoint & ) ) );
}
//仅会被调用一次，在该实例的构造函数中
void LocalView::expand_to_home_directory( QModelIndex parent_model,int level )
{
    int row_cnt = this->dir_file_model->rowCount(parent_model) ;
    QString home_path = QDir::homePath();
    QStringList home_path_grade = home_path.split('/');
    //qDebug()<<home_path_grade << level << row_cnt;
    QModelIndex curr_model ;
    for( int i = 0 ; i < row_cnt ; i ++)
    {
        curr_model = this->dir_file_model->index(i,0,parent_model) ;
        QString file_name = this->dir_file_model->data(curr_model).toString();
        //qDebug()<<file_name;
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
    this->curr_item_view = static_cast<QAbstractItemView*>(sender());
    QPoint real_pos = this->curr_item_view->mapToGlobal(pos);
    real_pos = QPoint(real_pos.x()+2,real_pos.y()+32);
	this->local_dir_tree_context_menu->popup ( real_pos );
    
}

void LocalView::slot_local_new_upload_requested()
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	//QTextCodec * codec = QTextCodec::codecForName(REMOTE_CODEC);
    
    QStringList local_file_names ;
    QString local_file_name;
    QByteArray ba;
    
    //QItemSelectionModel * ism = this->localView.treeView->selectionModel();
    QItemSelectionModel * ism = this->curr_item_view->selectionModel();
    
	QModelIndexList mil = ism->selectedIndexes();

	qDebug() << mil ;

    for( int i = 0 ; i < mil.count() ; i += this->curr_item_view->model()->columnCount(QModelIndex()) )
    {
        qDebug() << (static_cast<QDirModel*>(this->curr_item_view->model()))->fileName ( mil.at ( i ) );
        qDebug() << (static_cast<QDirModel*>(this->curr_item_view->model()))->filePath ( mil.at ( i ) );
    
        local_file_name = (static_cast<QDirModel*>(this->curr_item_view->model()))->filePath ( mil.at ( i ) );
        
		//ba = codec->toUnicode(local_file_name.toAscii()).toAscii();
		//下面这句说明了什么呢？在Mingw平台下，控制台输入正常时即为 Unicode编码，也就是DirModel返回的路径是Unicode编码的
		//qDebug()<<codec->canEncode(local_file_name)<<"桌面"<< GlobalOption::instance()->remote_codec->toUnicode("桌面");
        //qDebug()<<" orginal name:"<< local_file_name <<" unicode name:"<< QString(ba.data() );
        //local_file_name = ba ;
        //emit  new_upload_requested("/home/gzl/hehe.txt");
        //emit  new_upload_requested ( local_file_name , local_file_type );
        //加上协议前缀
        local_file_name = QString("file://"
#ifdef WIN32
					   "/"
#endif
			) + local_file_name ;
        local_file_names << local_file_name;
		qDebug()<< local_file_names ;
    }
    emit   new_upload_requested( local_file_names );
}

QString LocalView::get_selected_directory()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
   
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

    QString local_file = this->dir_file_model->filePath ( mil.at ( 0 ) );

    local_path = this->dir_file_model->filePath ( mil.at ( 0 ) );

    //QByteArray ba = codec->fromUnicode(local_path);
    //qDebug()<<" orginal name:"<< local_path
    //        <<" unicode name:"<< QString(ba.data() );
    //local_path = ba ;
	return local_path ;

}

void LocalView::slot_refresh_directory_tree()
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

	QItemSelectionModel * ism = this->localView.treeView->selectionModel();

	if ( ism !=0 )
	{
		QModelIndexList mil = ism->selectedIndexes();
        if( mil.count() > 0 )
		  model->refresh ( mil.at ( 0 ) );
	}
    this->dir_file_model->refresh( this->localView.tableView->rootIndex());
}
void LocalView::update_layout()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
//     this->dir_file_model->refresh( this->localView.tableView->rootIndex());
//     this->model->refresh(QModelIndex());
    this->slot_refresh_directory_tree();
}

void LocalView::closeEvent ( QCloseEvent * event )
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    event->ignore ();
    //this->setVisible(false); 
    QMessageBox::information(this,tr("attemp to close this window?"),tr("you can't close this window."));
}

void LocalView::slot_dir_tree_item_clicked( const QModelIndex & index)
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QString file_path ;
    
    file_path = this->dir_file_model->filePath(index);
    this->localView.tableView->setRootIndex( this->model->index( file_path )) ;
    for( int i = 0 ; i < this->model->rowCount( this->model->index( file_path ) ); i ++ )
        this->localView.tableView->setRowHeight(i,this->table_row_height );
    this->localView.tableView->resizeColumnToContents ( 0 );
}

void LocalView::slot_dir_file_view_double_clicked( const QModelIndex & index )
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //TODO if the clicked item is direcotry ,
     //expand left tree dir and update right table view
    // got the file path , tell tree ' model , then expand it
    //文件列表中的双击事件
    //1。　本地主机，如果是目录，则打开这个目录，如果是文件，则使用本机的程序打开这个文件
    //2。对于远程主机，　如果是目录，则打开这个目录，如果是文件，则提示是否要下载它。
    QString file_path ;
    
    if( this->model->isDir( index ) )
    {
        this->localView.treeView->expand( this->dir_file_model->index(this->model->filePath(index)).parent());        
        this->localView.treeView->expand( this->dir_file_model->index(this->model->filePath(index)));
        this->slot_dir_tree_item_clicked(this->dir_file_model->index(this->model->filePath(index)));
        this->localView.treeView->selectionModel()->clearSelection();
        this->localView.treeView->selectionModel()->select(this->dir_file_model->index(this->model->filePath(index)), QItemSelectionModel::Select ) ;
    }
    else
    {
        qDebug()<<" double clicked a regular file , no op now,only now";
    }
}
//TODO accept drop 

void LocalView::slot_show_hidden(bool show)
{
    if(show){
        this->model->setFilter(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);
    }
    else{
        this->model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot );
    }
}

void LocalView::slot_mkdir()
{
    QString dir_name ;
    
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    QModelIndexList mil;
    if(ism == 0 || ism->selectedIndexes().count() == 0)
    {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        QMessageBox::critical(this,tr("Waring..."),tr("No item selected"));
        return ;
    }    
    mil = ism->selectedIndexes() ;

    QModelIndex midx = mil.at(0);
    QModelIndex aim_midx = (this->curr_item_view == this->localView.treeView) ? this->dir_file_model->mapToSource(midx): midx ;

    //检查所选择的项是不是目录
    if(!this->model->isDir(aim_midx))
    {
        QMessageBox::critical(this,tr("Waring..."),tr("The selected item is not a directory."));
        return ;
    }
    
    dir_name = QInputDialog::getText(this,tr("Create directory:"),
                                     tr("Input directory name:")
                                             +"                                                        ",
                                             QLineEdit::Normal,
                                             tr("new_direcotry") );
    if( dir_name == QString::null )
    {
        return ;
    } 
    if(  dir_name.length () == 0 )
    {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        QMessageBox::critical(this,tr("Waring..."),tr("No directory name supplyed."));
        return;
    }
    //TODO 将 file_path 转换编码再执行下面的操作
    if(!QDir().mkdir(this->model->filePath(aim_midx) + "/" + dir_name))
    {
        QMessageBox::critical(this,tr("Waring..."),tr("Create directory faild."));
    }
    else
    {
        this->slot_refresh_directory_tree();
    }
}
void LocalView::slot_rename()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QModelIndexList mil;
    QItemSelectionModel * ism = this->curr_item_view->selectionModel();
    if( ism == 0 || ism->selectedIndexes().count() == 0)
    {
        QMessageBox::critical(this,tr("Waring..."),tr("No item selected")+"                         ");
        return ;
    }
    mil = ism->selectedIndexes();

    QString local_file = this->curr_item_view==this->localView.treeView?this->dir_file_model->filePath ( mil.at ( 0 ) ) : this->model->filePath(mil.at(0));
    QString file_name = this->curr_item_view==this->localView.treeView?this->dir_file_model->fileName ( mil.at ( 0 ) ) : this->model->fileName(mil.at(0));
    //QByteArray ba = codec->fromUnicode(local_path);
    //qDebug()<<" orginal name:"<< local_path
    //        <<" unicode name:"<< QString(ba.data() );
    //local_path = ba ;
    QString rename_to ;
    rename_to = QInputDialog::getText(this,tr("Rename to:"),  tr("Input new name:")
            +"                                                        ",
                                         QLineEdit::Normal, file_name );
     
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
    QTextCodec * codec = GlobalOption::instance()->locale_codec;
    QString file_path = local_file.left(local_file.length()-file_name.length());
    rename_to = file_path + rename_to;
    ::rename( codec->fromUnicode(local_file).data(), codec->fromUnicode(rename_to).data());
    
    this->slot_refresh_directory_tree();
}

void LocalView::slot_show_properties()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    
    if(ism == 0)
    {
        qDebug()<<" why???? no QItemSelectionModel??";        
        return ;
    }
    
    QModelIndexList mil = ism->selectedIndexes()   ;
    if( mil.count() == 0 ){
        qDebug()<<" why???? no QItemSelectionModel??";
        return;
    }
    QString local_file = this->curr_item_view==this->localView.treeView?this->dir_file_model->filePath ( mil.at ( 0 ) ) : this->model->filePath(mil.at(0));
    //  文件类型，大小，几个时间，文件权限
    //TODO 从模型中取到这些数据并显示在属性对话框中。
    LocalFileProperties * fp = new LocalFileProperties(this);
    fp->set_file_info_model_list(local_file);
    fp->exec();
    delete fp ;
}



