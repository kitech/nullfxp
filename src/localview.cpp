// localview.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-05-31 15:26:15 +0800
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>

#include "utils.h"
#include "remoteview.h"
#include "localview.h"
#include "globaloption.h"
#include "fileproperties.h"

LocalView::LocalView(QWidget *parent )
    : QWidget(parent)
{
    localView.setupUi(this);
    this->setObjectName("lv");
    ////
    this->status_bar = new QStatusBar();
    this->layout()->addWidget(this->status_bar);
    this->status_bar->showMessage(tr("Ready"));
    
    ////
    model = new QDirModel();
    //     model->setFilter( QDir::AllEntries|QDir::Hidden|QDir::NoDotAndDotDot );
    this->dir_file_model = new LocalDirSortFilterModel();
    this->dir_file_model->setSourceModel(model);
    
    this->localView.treeView->setModel(this->dir_file_model);
    this->localView.treeView->setRootIndex(this->dir_file_model->index("/"));
    // this->localView.treeView->setColumnHidden(1, true);
    this->localView.treeView->setColumnWidth(1, 0);
    this->localView.treeView->setColumnHidden(2, true);
    this->localView.treeView->setColumnHidden(3, true);
    this->localView.treeView->setColumnWidth(0, this->localView.treeView->columnWidth(0) * 2);    
    this->expand_to_home_directory(this->localView.treeView->rootIndex(), 1);
  
    this->init_local_dir_tree_context_menu();
    this->localView.treeView->setAnimated(true);
  
    this->localView.tableView->setModel(this->model);
    this->localView.tableView->setRootIndex( this->model->index(QDir::homePath()));
    this->localView.tableView->verticalHeader()->setVisible(false);

    //change row height of table 
    if (this->model->rowCount(this->model->index(QDir::homePath())) > 0) {
        this->table_row_height = this->localView.tableView->rowHeight(0) * 2 / 3;
    } else {
        this->table_row_height = 20 ;
    }
    for (int i = 0; i < this->model->rowCount(this->model->index(QDir::homePath())); i ++) {
        this->localView.tableView->setRowHeight(i, this->table_row_height);
    }
  
    this->localView.tableView->resizeColumnToContents(0);
    /////
    QObject::connect(this->localView.treeView, SIGNAL(clicked(const QModelIndex &)),
                     this, SLOT(slot_dir_tree_item_clicked(const QModelIndex &)));
    QObject::connect(this->localView.tableView, SIGNAL(doubleClicked(const QModelIndex &)),
                     this, SLOT(slot_dir_file_view_double_clicked(const QModelIndex &)));
    
    ////////ui area custom
    this->localView.splitter->setStretchFactor(0, 1);
    this->localView.splitter->setStretchFactor(1, 2);
    //this->localView.listView->setVisible(false);    //暂时没有功能在里面先隐藏掉

    //TODO localview 标题格式: Local(主机名) - 当前所在目录名
    //TODO remoteview 标题格式: user@hostname - 当前所在目录名
    //TODO 状态栏: 信息格式:  n entries (m hidden entries) . --- 与remoteview相同
}


LocalView::~LocalView()
{}

void LocalView::init_local_dir_tree_context_menu()
{
    this->local_dir_tree_context_menu = new QMenu();

    QAction *action = new QAction(tr("Upload"), 0);

    this->local_dir_tree_context_menu->addAction(action);

    QObject::connect(action, SIGNAL(triggered()), this, SLOT(slot_local_new_upload_requested()));

    action = new QAction("", 0);
    action->setSeparator(true);
    this->local_dir_tree_context_menu->addAction(action);
    
    ////reresh action
    action = new QAction(tr("Refresh"), 0);
    this->local_dir_tree_context_menu->addAction(action);
    
    QObject::connect(action, SIGNAL(triggered()),
                     this, SLOT(slot_refresh_directory_tree()));
                       
    action = new QAction(tr("Properties..."), 0);
    this->local_dir_tree_context_menu->addAction(action);
    QObject::connect(action,SIGNAL(triggered()), this, SLOT(slot_show_properties()));
    //////
    action = new QAction(tr("Show Hidden"), 0);
    action->setCheckable(true);
    this->local_dir_tree_context_menu->addAction(action);
    QObject::connect(action, SIGNAL(toggled(bool)), this, SLOT(slot_show_hidden(bool)));
    action = new QAction("", 0);
    action->setSeparator(true);
    this->local_dir_tree_context_menu->addAction(action);

    action = new QAction(tr("Copy &Path"), 0);
    this->local_dir_tree_context_menu->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(slot_copy_path_url()));
    
    action = new QAction(tr("Create directory..."), 0);
    this->local_dir_tree_context_menu->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(slot_mkdir()));

    action = new QAction(tr("Delete directory"), 0);
    this->local_dir_tree_context_menu->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(slot_rmdir()));
    action = new QAction(tr("Remove file"), 0);
    this->local_dir_tree_context_menu->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(slot_remove()));

    action = new QAction("", 0);
    action->setSeparator(true);
    this->local_dir_tree_context_menu->addAction(action);
  
    //递归删除目录，删除文件的用户功能按钮
    // action = new QAction(tr("Remove recursively !!!"), 0);
    // this->local_dir_tree_context_menu->addAction(action);
    // QObject::connect(action, SIGNAL(triggered()), this, SLOT(rm_file_or_directory_recursively()));

    // action = new QAction("", 0);
    // action->setSeparator(true);
    // this->local_dir_tree_context_menu->addAction(action);

    action = new QAction(tr("Rename ..."), 0);
    this->local_dir_tree_context_menu->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(slot_rename()));
    
    QObject::connect(this->localView.treeView, SIGNAL(customContextMenuRequested(const QPoint &)),
                     this, SLOT(slot_local_dir_tree_context_menu_request(const QPoint &)));
    QObject::connect(this->localView.tableView,SIGNAL ( customContextMenuRequested(const QPoint &)),
                     this, SLOT(slot_local_dir_tree_context_menu_request (const QPoint &)));
}
//仅会被调用一次，在该实例的构造函数中
void LocalView::expand_to_home_directory(QModelIndex parent_model, int level)
{
    int row_cnt = this->dir_file_model->rowCount(parent_model) ;
    QString home_path = QDir::homePath();
    QStringList home_path_grade = home_path.split('/');
    //qDebug()<<home_path_grade << level << row_cnt;
    QModelIndex curr_model ;
    for (int i = 0 ; i < row_cnt ; i ++) {
        curr_model = this->dir_file_model->index(i,0,parent_model) ;
        QString file_name = this->dir_file_model->data(curr_model).toString();
        //qDebug()<<file_name;
        if (file_name == home_path_grade.at(level)) {
            this->localView.treeView->expand(curr_model);
            if (level == home_path_grade.count() - 1) {
                break;
            } else {
                this->expand_to_home_directory(curr_model, level+1);
                break;
            }
        }
    }
    if (level == 1) {
        this->localView.treeView->scrollTo(curr_model);
    }
    //qDebug()<<" root row count:"<< row_cnt ;
}

void LocalView::slot_local_dir_tree_context_menu_request(const QPoint & pos)
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    this->curr_item_view = static_cast<QAbstractItemView*>(sender());
    QPoint real_pos = this->curr_item_view->mapToGlobal(pos);
    real_pos = QPoint(real_pos.x()+2, real_pos.y() + 32);
    this->local_dir_tree_context_menu->popup(real_pos);
    
}

void LocalView::slot_local_new_upload_requested()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    TaskPackage pkg(PROTO_FILE);
    QString local_file_name;
    QByteArray ba;

    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    
    QModelIndexList mil = ism->selectedIndexes();

    for (int i = 0 ; i < mil.count() ; i += this->curr_item_view->model()->columnCount(QModelIndex())) {
        QModelIndex midx = mil.at(i);
        if (this->curr_item_view==this->localView.treeView) {
            midx = this->dir_file_model->mapToSource(midx);
        }
        qDebug()<<this->model->fileName(midx);
        qDebug()<<this->model->filePath(midx);
        local_file_name = this->model->filePath(midx);
        pkg.files<<local_file_name;
    }
    emit new_upload_requested(pkg);
}

QString LocalView::get_selected_directory()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QString local_path ;
    QItemSelectionModel * ism = this->localView.treeView->selectionModel();

    if (ism == 0) {        
        return QString();
    }
    
    QModelIndexList mil = ism->selectedIndexes();

    if (mil.count() == 0) {
        return QString();
    }
    
    //qDebug() << mil ;
    //qDebug() << model->fileName ( mil.at ( 0 ) );
    //qDebug() << model->filePath ( mil.at ( 0 ) );

    QString local_file = this->dir_file_model->filePath(mil.at(0));

    local_path = this->dir_file_model->filePath(mil.at(0));

    return local_path ;
}

void LocalView::slot_refresh_directory_tree()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    QItemSelectionModel * ism = this->localView.treeView->selectionModel();

    if (ism !=0) {
	QModelIndexList mil = ism->selectedIndexes();
        if (mil.count() > 0)
            model->refresh(mil.at(0));
    }
    this->dir_file_model->refresh(this->localView.tableView->rootIndex());
}
void LocalView::update_layout()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
//     this->dir_file_model->refresh( this->localView.tableView->rootIndex());
//     this->model->refresh(QModelIndex());
    this->slot_refresh_directory_tree();
}

void LocalView::closeEvent(QCloseEvent * event)
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    event->ignore();
    //this->setVisible(false); 
    // QMessageBox::information(this, tr("Attemp to close this window?"), tr("Close this window is not needed."));
    // 把这个窗口最小化是不是好些。
    this->showMinimized();
}

void LocalView::slot_dir_tree_item_clicked(const QModelIndex & index)
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QString file_path ;
    
    file_path = this->dir_file_model->filePath(index);
    this->localView.tableView->setRootIndex(this->model->index(file_path));
    for (int i = 0 ; i < this->model->rowCount(this->model->index(file_path)); i ++)
        this->localView.tableView->setRowHeight(i, this->table_row_height);
    this->localView.tableView->resizeColumnToContents(0);
}

void LocalView::slot_dir_file_view_double_clicked(const QModelIndex & index)
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //TODO if the clicked item is direcotry ,
    //expand left tree dir and update right table view
    // got the file path , tell tree ' model , then expand it
    //文件列表中的双击事件
    //1。　本地主机，如果是目录，则打开这个目录，如果是文件，则使用本机的程序打开这个文件
    //2。对于远程主机，　如果是目录，则打开这个目录，如果是文件，则提示是否要下载它。
    QString file_path ;
    
    if (this->model->isDir(index)) {
        this->localView.treeView->expand( this->dir_file_model->index(this->model->filePath(index)).parent());        
        this->localView.treeView->expand( this->dir_file_model->index(this->model->filePath(index)));
        this->slot_dir_tree_item_clicked(this->dir_file_model->index(this->model->filePath(index)));
        this->localView.treeView->selectionModel()->clearSelection();
        this->localView.treeView->selectionModel()->select(this->dir_file_model->index(this->model->filePath(index)), QItemSelectionModel::Select ) ;
    } else {
        qDebug()<<" double clicked a regular file , no op now,only now";
    }
}
//TODO accept drop 

void LocalView::slot_show_hidden(bool show)
{
    if (show) {
        this->model->setFilter(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);
    } else {
        this->model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot );
    }
}

void LocalView::slot_mkdir()
{
    QString dir_name ;
    
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    QModelIndexList mil;
    if (ism == 0 || ism->selectedIndexes().count() == 0) {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        QMessageBox::critical(this, tr("Waring..."), tr("No item selected"));
        return ;
    }    
    mil = ism->selectedIndexes() ;

    QModelIndex midx = mil.at(0);
    QModelIndex aim_midx = (this->curr_item_view == this->localView.treeView)
        ? this->dir_file_model->mapToSource(midx): midx ;

    //检查所选择的项是不是目录
    if (!this->model->isDir(aim_midx)) {
        QMessageBox::critical(this, tr("Waring..."), tr("The selected item is not a directory."));
        return ;
    }
    
    dir_name = QInputDialog::getText(this, tr("Create directory:"),
                                     tr("Input directory name:").leftJustified(100, ' '),
                                     QLineEdit::Normal, tr("new_direcotry"));
    if (dir_name == QString::null) {
        return ;
    } 
    if (dir_name.length () == 0) {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        QMessageBox::critical(this, tr("Waring..."), tr("No directory name supplyed."));
        return;
    }

    if (!QDir().mkdir(this->model->filePath(aim_midx) + "/" + dir_name)) {
        QMessageBox::critical(this, tr("Waring..."), tr("Create directory faild."));
    } else {
        this->slot_refresh_directory_tree();
    }
}

void LocalView::slot_rmdir()
{
    QString dir_name ;
    
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    QModelIndexList mil;
    if (ism == 0 || ism->selectedIndexes().count() == 0) {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        QMessageBox::critical(this, tr("Waring..."), tr("No item selected"));
        return ;
    }    
    mil = ism->selectedIndexes() ;

    QModelIndex midx = mil.at(0);
    QModelIndex aim_midx = (this->curr_item_view == this->localView.treeView) 
        ? this->dir_file_model->mapToSource(midx): midx ;

    //检查所选择的项是不是目录
    if (!this->model->isDir(aim_midx)) {
        QMessageBox::critical(this, tr("Waring..."), tr("The selected item is not a directory."));
        return ;
    }
    qDebug()<<QDir(this->model->filePath(aim_midx)).count();
    if (QDir(this->model->filePath(aim_midx)).count() > 2) {
        QMessageBox::critical(this, tr("Waring..."), tr("Selected director not empty."));
        return;
    }
    if (!QDir().rmdir(this->model->filePath(aim_midx))) {
        QMessageBox::critical(this, tr("Waring..."), tr("Delete directory faild. Mayby the directory is not empty."));
    } else {
        if (this->curr_item_view == this->localView.treeView) {
            QModelIndex tree_midx = this->dir_file_model->mapFromSource(aim_midx);
            this->slot_dir_tree_item_clicked(tree_midx.parent());
        }
        this->slot_refresh_directory_tree();
    }
}

void LocalView::slot_remove()
{
    QModelIndexList mil;
    QItemSelectionModel * ism = this->curr_item_view->selectionModel();
    if (ism == 0 || ism->selectedIndexes().count() == 0) {
        QMessageBox::critical(this, tr("Waring..."), tr("No item selected").leftJustified(50, ' '));
        return ;
    }
    mil = ism->selectedIndexes();

    QString local_file = this->curr_item_view==this->localView.treeView
        ? this->dir_file_model->filePath(mil.at(0)) : this->model->filePath(mil.at(0));

    if (QMessageBox::question(this, tr("Question..."), 
                             QString("%1\n\t%2").arg(QString(tr("Are you sure remove it?"))).arg(local_file),
                             QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        if (QFile::remove(local_file)) {
            this->slot_refresh_directory_tree();    
        } else {
            q_debug()<<"can not remove file:"<<local_file;
        }
    }
}

void LocalView::slot_rename()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QModelIndexList mil;
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    if (ism == 0 || ism->selectedIndexes().count() == 0) {
        QMessageBox::critical(this, tr("Waring..."),
                              tr("No item selected").leftJustified(60, ' '));
        return;
    }
    mil = ism->selectedIndexes();

    QString local_file = this->curr_item_view==this->localView.treeView
        ? this->dir_file_model->filePath(mil.at(0)) : this->model->filePath(mil.at(0));
    QString file_name = this->curr_item_view==this->localView.treeView
        ? this->dir_file_model->fileName(mil.at(0)) : this->model->fileName(mil.at(0));

    QString rename_to;
    rename_to = QInputDialog::getText(this, tr("Rename to:"), 
                                      tr("Input new name:").leftJustified(100, ' '),
                                      QLineEdit::Normal, file_name );
     
    if (rename_to  == QString::null) {
        //qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        //QMessageBox::critical(this,tr("Waring..."),tr("No new name supplyed "));
        return;
    }
    if (rename_to.length() == 0) {
        QMessageBox::critical(this, tr("Waring..."), tr("No new name supplyed "));
        return;
    }
    q_debug()<<rename_to<<local_file<<this->curr_item_view<<file_name;
    // QTextCodec *codec = GlobalOption::instance()->locale_codec;
    QString file_path = local_file.left(local_file.length()-file_name.length());
    rename_to = file_path + rename_to;

    // 为什么用这个函数,直接用qt的函数不好吗
    if (!QFile::rename(local_file, rename_to)) {
        q_debug()<<"file rename faild";
    }
    // ::rename(codec->fromUnicode(local_file).data(), codec->fromUnicode(rename_to).data());
    
    this->slot_refresh_directory_tree();
}
void LocalView::slot_copy_path_url()
{
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    
    if (ism == 0) {
        qDebug()<<" why???? no QItemSelectionModel??";        
        return ;
    }
    
    QModelIndexList mil = ism->selectedIndexes()   ;
    if (mil.count() == 0) {
        qDebug()<<" why???? no QItemSelectionModel??";
        return;
    }
    QString local_file = this->curr_item_view==this->localView.treeView
        ? this->dir_file_model->filePath(mil.at(0)) : this->model->filePath(mil.at(0));
    
    QApplication::clipboard()->setText(local_file);
}

void LocalView::slot_show_properties()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    
    if (ism == 0) {
        qDebug()<<" why???? no QItemSelectionModel??";        
        return ;
    }
    
    QModelIndexList mil = ism->selectedIndexes()   ;
    if (mil.count() == 0 ) {
        qDebug()<<" why???? no QItemSelectionModel??";
        return;
    }
    QString local_file = this->curr_item_view==this->localView.treeView
        ? this->dir_file_model->filePath(mil.at(0)) : this->model->filePath(mil.at(0));
    //  文件类型，大小，几个时间，文件权限
    //TODO 从模型中取到这些数据并显示在属性对话框中。
    LocalFileProperties * fp = new LocalFileProperties(this);
    fp->set_file_info_model_list(local_file);
    fp->exec();
    delete fp ;
}

void LocalView::rm_file_or_directory_recursively()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
}

