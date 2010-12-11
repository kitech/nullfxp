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

#include "ui_localview.h"

LocalView::LocalView(QWidget *parent)
    : QWidget(parent)
    , uiw(new Ui::LocalView())
{
    this->uiw->setupUi(this);
    this->setObjectName("LocalFileSystemView");
    ////
    this->status_bar = new QStatusBar();
    this->layout()->addWidget(this->status_bar);
    this->status_bar->showMessage(tr("Ready"));
    this->entriesLabel = new QLabel(tr("Entries label"), this);
    this->status_bar->addPermanentWidget(this->entriesLabel);
    this->entriesLabel->setTextInteractionFlags(this->entriesLabel->textInteractionFlags() 
                                                | Qt::TextSelectableByMouse);

    QLabel *tmpLabel = new QLabel(tr("System"), this);
    this->status_bar->addPermanentWidget(tmpLabel);
#if defined(Q_OS_WIN)
    tmpLabel->setPixmap(QPixmap(":/icons/os/windows.png").scaled(22, 22));
    tmpLabel->setToolTip(tr("Running Windows OS."));
#elif defined(Q_OS_MAC)
    tmpLabel->setPixmap(QPixmap(":/icons/os/osx.png").scaled(22, 22));
    tmpLabel->setToolTip(tr("Running Mac OS X."));
#else
    tmpLabel->setPixmap(QPixmap(":/icons/os/tux.png").scaled(22, 22));
    tmpLabel->setToolTip(tr("Running Linux/Unix Like OS."));
#endif
    
    ////
    model = new QFileSystemModel();
    QObject::connect(model, SIGNAL(directoryLoaded(const QString &)),
                     this, SLOT(onDirectoryLoaded(const QString &)));
    QObject::connect(model, SIGNAL(fileRenamed(const QString &, const QString &, const QString &)),
                     this, SLOT(onFileRenamed(const QString &, const QString &, const QString &)));
    QObject::connect(model, SIGNAL(rootPathChanged(const QString &)),
                     this, SLOT(onRootPathChanged(const QString &)));
    model->setRootPath(""); // on widows this can list all drivers

    // model->setFilter(QDir::AllEntries|QDir::Hidden|QDir::NoDotAndDotDot);
    this->dir_file_model = new LocalDirSortFilterModel();
    this->dir_file_model->setSourceModel(model);

    
    this->uiw->treeView->setModel(this->dir_file_model);
    this->uiw->treeView->setRootIndex(this->dir_file_model->index(""));
    // this->uiw->treeView->setColumnHidden(1, true);
    this->uiw->treeView->setColumnWidth(1, 0);
    this->uiw->treeView->setColumnHidden(2, true);
    this->uiw->treeView->setColumnHidden(3, true);
    this->uiw->treeView->setColumnWidth(0, this->uiw->treeView->columnWidth(0) * 2);    
    this->expand_to_home_directory(this->uiw->treeView->rootIndex(), 1);
    // this->uiw->treeView->expand(this->dir_file_model->index("/home/gzleo"));
  
    this->init_local_dir_tree_context_menu();
    this->uiw->treeView->setAnimated(true);
  
    this->uiw->tableView->setModel(this->model);
    this->uiw->tableView->setRootIndex(this->model->index(QDir::homePath()));
    this->uiw->tableView->verticalHeader()->setVisible(false);

    //change row height of table 
    if (this->model->rowCount(this->model->index(QDir::homePath())) > 0) {
        this->table_row_height = this->uiw->tableView->rowHeight(0) * 2 / 3;
    } else {
        this->table_row_height = 20;
    }
    for (int i = 0; i < this->model->rowCount(this->model->index(QDir::homePath())); i ++) {
        this->uiw->tableView->setRowHeight(i, this->table_row_height);
    }
  
    this->uiw->tableView->resizeColumnToContents(0);
    /////
    QObject::connect(this->uiw->treeView, SIGNAL(clicked(const QModelIndex &)),
                     this, SLOT(slot_dir_tree_item_clicked(const QModelIndex &)));
    QObject::connect(this->uiw->tableView, SIGNAL(doubleClicked(const QModelIndex &)),
                     this, SLOT(slot_dir_file_view_double_clicked(const QModelIndex &)));
    QObject::connect(this->uiw->listView, SIGNAL(doubleClicked(const QModelIndex &)),
                     this, SLOT(slot_dir_file_view_double_clicked(const QModelIndex &)));

    // list view of icon mode
    this->uiw->listView->setModel(this->model);
    this->uiw->listView->setRootIndex(this->model->index(QDir::homePath()));
    this->uiw->listView->setViewMode(QListView::IconMode);
    this->uiw->listView->setGridSize(QSize(80, 80));
    
    ////////ui area custom
    this->uiw->splitter->setStretchFactor(0, 1);
    this->uiw->splitter->setStretchFactor(1, 2);
    //this->uiw->listView->setVisible(false);    //暂时没有功能在里面先隐藏掉

    // dir navbar
    this->is_dir_complete_request = false;
    // this->dir_complete_request_prefix = "";
    QObject::connect(this->uiw->widget, SIGNAL(goHome()),
                     this, SLOT(slot_dir_nav_go_home()));
    QObject::connect(this->uiw->widget, SIGNAL(dirPrefixChanged(const QString &)),
                     this, SLOT(slot_dir_nav_prefix_changed(const QString &)));
    QObject::connect(this->uiw->widget, SIGNAL(dirInputConfirmed(const QString &)),
                     this, SLOT(slot_dir_nav_input_comfirmed(const QString &)));
    QObject::connect(this->uiw->widget, SIGNAL(iconSizeChanged(int)),
                     this, SLOT(slot_icon_size_changed(int)));
    this->uiw->widget->onSetHome(QDir::homePath());

    this->uiw->listView->setSelectionModel(this->uiw->tableView->selectionModel());
    this->setFileListViewMode(GlobalOption::FLV_DETAIL);

    //TODO localview 标题格式: Local(主机名) - 当前所在目录名
    //TODO remoteview 标题格式: user@hostname - 当前所在目录名
    //TODO 状态栏: 信息格式:  n entries (m hidden entries) . --- 与remoteview相同
}


LocalView::~LocalView()
{
}

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
    
    QObject::connect(this->uiw->treeView, SIGNAL(customContextMenuRequested(const QPoint &)),
                     this, SLOT(slot_local_dir_tree_context_menu_request(const QPoint &)));
    QObject::connect(this->uiw->tableView, SIGNAL(customContextMenuRequested(const QPoint &)),
                     this, SLOT(slot_local_dir_tree_context_menu_request(const QPoint &)));
    QObject::connect(this->uiw->listView, SIGNAL(customContextMenuRequested(const QPoint &)),
                     this, SLOT(slot_local_dir_tree_context_menu_request(const QPoint &)));
}

// 可以写成一个通用的expand_to_directory(QString fullPath, int level);
// 仅会被调用一次，在该实例的构造函数中
void LocalView::expand_to_home_directory(QModelIndex parent_model, int level)
{
    Q_UNUSED(parent_model);
    QString homePath = QDir::homePath();
    QStringList homePathParts = QDir::homePath().split('/');
    // qDebug()<<home_path_grade<<level<<row_cnt;
    QStringList stepPathParts;
    QString tmpPath;
    QModelIndex currIndex;

    // windows fix case: C:/abcd/efg/hi
    bool unixRootFix = true;
    if (homePath.length() > 1 && homePath.at(1) == ':') {
        unixRootFix = false;
    }
    
    for (int i = 0; i < homePathParts.count(); i++) {
        stepPathParts << homePathParts.at(i);
        tmpPath = (unixRootFix ? QString("/") : QString()) + stepPathParts.join("/");
        /// qDebug()<<tmpPath<<stepPathParts;
        currIndex = this->dir_file_model->index(tmpPath);
        this->uiw->treeView->expand(currIndex);
    }
    if (level == 1) {
        this->uiw->treeView->scrollTo(currIndex);
    }
    //qDebug()<<" root row count:"<< row_cnt ;
}

void LocalView::expand_to_directory(QString path, int level)
{
    QString homePath = path;
    QStringList homePathParts = homePath.split('/');
    // qDebug()<<home_path_grade<<level<<row_cnt;
    QStringList stepPathParts;
    QString tmpPath;
    QModelIndex curr_model;

    // windows fix case: C:/abcd/efg/hi
    bool unixRootFix = true;
    if (homePath.length() > 1 && homePath.at(1) == ':') {
        unixRootFix = false;
    }
    
    for (int i = 0; i < homePathParts.count(); i++) {
        stepPathParts << homePathParts.at(i);
        tmpPath = (unixRootFix ? QString("/") : QString()) + stepPathParts.join("/");
        /// qDebug()<<tmpPath<<stepPathParts;
        curr_model = this->dir_file_model->index(tmpPath);
        this->uiw->treeView->expand(curr_model);
    }
    if (level == 1) {
        this->uiw->treeView->scrollTo(curr_model);
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

// can not support recursive selected now.
void LocalView::slot_local_new_upload_requested()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    TaskPackage pkg(PROTO_FILE);
    QString local_file_name;
    QByteArray ba;

    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    QModelIndex cidx, idx, pidx;

    cidx = ism->currentIndex();
    pidx = cidx.parent();

    for (int i = ism->model()->rowCount(pidx) - 1 ; i >= 0 ; --i) {
        if (ism->isRowSelected(i, pidx)) {
            QModelIndex midx = idx = ism->model()->index(i, 0, pidx);
            if (this->curr_item_view == this->uiw->treeView) {
                midx = this->dir_file_model->mapToSource(midx);
            }
            qDebug()<<this->model->fileName(midx);
            qDebug()<<this->model->filePath(midx);
            local_file_name = this->model->filePath(midx);
            pkg.files<<local_file_name;
        }
    }
    
    // QModelIndexList mil = ism->selectedIndexes(); // TODO should fix win x64

    // for (int i = 0 ; i < mil.count() ; i += this->curr_item_view->model()->columnCount(QModelIndex())) {
    //     QModelIndex midx = mil.at(i);
    //     if (this->curr_item_view==this->uiw->treeView) {
    //         midx = this->dir_file_model->mapToSource(midx);
    //     }
    //     qDebug()<<this->model->fileName(midx);
    //     qDebug()<<this->model->filePath(midx);
    //     local_file_name = this->model->filePath(midx);
    //     pkg.files<<local_file_name;
    // }
    emit new_upload_requested(pkg);
}

QString LocalView::get_selected_directory()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QString local_path;
    QItemSelectionModel *ism = this->uiw->treeView->selectionModel();
    QModelIndex cidx, idx;

    if (ism == 0) {        
        return QString();
    }

    if (!ism->hasSelection()) {
          return QString();
    }

    // QModelIndexList mil = ism->selectedIndexes();
    // if (mil.count() == 0) {
    //     return QString();
    // }
    
    //qDebug() << mil ;
    //qDebug() << model->fileName ( mil.at ( 0 ) );
    //qDebug() << model->filePath ( mil.at ( 0 ) );

    cidx = ism->currentIndex();
    idx = ism->model()->index(cidx.row(), 0, cidx.parent());

    // QString local_file = this->dir_file_model->filePath(mil.at(0));
    // local_path = this->dir_file_model->filePath(mil.at(0));

    QString local_file = this->dir_file_model->filePath(idx);
    local_path = this->dir_file_model->filePath(idx);

    return local_path;
}

void LocalView::slot_refresh_directory_tree()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    QItemSelectionModel *ism = this->uiw->treeView->selectionModel();
    QModelIndex cidx, idx;

    if (ism != 0) {
        if (ism->hasSelection()) {
            cidx = ism->currentIndex();
            idx = ism->model()->index(cidx.row(), 0, cidx.parent());
            // QModelIndex origIndex = this->dir_file_model->mapToSource(mil.at(0));
            QModelIndex origIndex = this->dir_file_model->mapToSource(idx);
        }
        // QModelIndexList mil = ism->selectedIndexes();
        // if (mil.count() > 0) {
        //     // model->refresh(mil.at(0));
        //     QModelIndex origIndex = this->dir_file_model->mapToSource(mil.at(0));
        //     q_debug()<<mil.at(0)<<origIndex;
        //     // model->refresh(origIndex);
        // }
    }
    this->dir_file_model->refresh(this->uiw->tableView->rootIndex());
}
void LocalView::update_layout()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //     this->dir_file_model->refresh( this->uiw->tableView->rootIndex());
    //     this->model->refresh(QModelIndex());
    this->slot_refresh_directory_tree();
}

void LocalView::closeEvent(QCloseEvent *event)
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    event->ignore();
    //this->setVisible(false); 
    // QMessageBox::information(this, tr("Attemp to close this window?"), tr("Close this window is not needed."));
    // 把这个窗口最小化是不是好些。
    this->showMinimized();
}

void LocalView::slot_dir_tree_item_clicked(const QModelIndex &index)
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QString file_path;
    
    file_path = this->dir_file_model->filePath(index);
    this->uiw->tableView->setRootIndex(this->model->index(file_path));
    for (int i = 0 ; i < this->model->rowCount(this->model->index(file_path)); i ++)
        this->uiw->tableView->setRowHeight(i, this->table_row_height);
    this->uiw->tableView->resizeColumnToContents(0);

    this->uiw->listView->setRootIndex(this->model->index(file_path));

    this->uiw->widget->onNavToPath(file_path);
    this->onUpdateEntriesStatus();
}

void LocalView::slot_dir_file_view_double_clicked(const QModelIndex &index)
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //TODO if the clicked item is direcotry ,
    //expand left tree dir and update right table view
    // got the file path , tell tree ' model , then expand it
    //文件列表中的双击事件
    //1。 本地主机，如果是目录，则打开这个目录，如果是文件，则使用本机的程序打开这个文件
    //2。 对于远程主机，　如果是目录，则打开这个目录，如果是文件，则提示是否要下载它。
    QString file_path;
    
    if (this->model->isDir(index)) {
        this->uiw->treeView->expand( this->dir_file_model->index(this->model->filePath(index)).parent());        
        this->uiw->treeView->expand( this->dir_file_model->index(this->model->filePath(index)));
        this->slot_dir_tree_item_clicked(this->dir_file_model->index(this->model->filePath(index)));
        this->uiw->treeView->selectionModel()->clearSelection();
        this->uiw->treeView->selectionModel()->select(this->dir_file_model->index(this->model->filePath(index)), QItemSelectionModel::Select ) ;
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
    QString dir_name;
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    QModelIndex cidx, idx;

    // QModelIndexList mil;
    if (ism == 0 || !ism->hasSelection()) {
        // qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        qDebug()<<" selectedIndexes count :"<< ism->hasSelection() << " why no item selected????";
        QMessageBox::critical(this, tr("Waring..."), tr("No item selected"));
        return;
    }

    // mil = ism->selectedIndexes() ;
    cidx = ism->currentIndex();
    idx = ism->model()->index(cidx.row(), 0, cidx.parent());

    // QModelIndex midx = mil.at(0);
    QModelIndex midx = idx;
    QModelIndex aim_midx = (this->curr_item_view == this->uiw->treeView)
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
        // qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        qDebug()<<" selectedIndexes count :"<< ism->hasSelection() << " why no item selected????";
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
    QString dir_name;
    
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    // QModelIndexList mil;
    QModelIndex cidx, idx;

    if (ism == 0 || !ism->hasSelection()) {
        qDebug()<<" selectedIndexes count :"<< ism->hasSelection() << " why no item selected????";
        QMessageBox::critical(this, tr("Waring..."), tr("No item selected"));
        return;
    }    
    
    // mil = ism->selectedIndexes();

    cidx = ism->currentIndex();
    idx = ism->model()->index(cidx.row(), 0, cidx.parent());
    
    // QModelIndex midx = mil.at(0);
    QModelIndex midx = idx;
    QModelIndex aim_midx = (this->curr_item_view == this->uiw->treeView) 
        ? this->dir_file_model->mapToSource(midx): midx;

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
        if (this->curr_item_view == this->uiw->treeView) {
            QModelIndex tree_midx = this->dir_file_model->mapFromSource(aim_midx);
            this->slot_dir_tree_item_clicked(tree_midx.parent());
        }
        this->slot_refresh_directory_tree();
    }
}

void LocalView::slot_remove()
{
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    // QModelIndexList mil;
    QModelIndex cidx, idx;

    if (ism == 0 || !ism->hasSelection()) {
        QMessageBox::critical(this, tr("Waring..."), tr("No item selected").leftJustified(50, ' '));
        return;
    }
    // mil = ism->selectedIndexes();
    cidx = ism->currentIndex();
    idx = ism->model()->index(cidx.row(), 0, cidx.parent());

    QString local_file = this->curr_item_view==this->uiw->treeView
        ? this->dir_file_model->filePath(idx) : this->model->filePath(idx);

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
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    // QModelIndexList mil;
    QModelIndex cidx, idx;

    if (ism == 0 || !ism->hasSelection()) {
        QMessageBox::critical(this, tr("Waring..."),
                              tr("No item selected").leftJustified(60, ' '));
        return;
    }
    // mil = ism->selectedIndexes();
    cidx = ism->currentIndex();
    idx = ism->model()->index(cidx.row(), 0, cidx.parent());

    QString local_file = this->curr_item_view==this->uiw->treeView
        ? this->dir_file_model->filePath(idx) : this->model->filePath(idx);
    QString file_name = this->curr_item_view==this->uiw->treeView
        ? this->dir_file_model->fileName(idx) : this->model->fileName(idx);

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
    QModelIndex cidx, idx;

    if (ism == 0) {
        qDebug()<<"Why???? no QItemSelectionModel??";        
        return;
    }
    
    // QModelIndexList mil = ism->selectedIndexes();
    if (!ism->hasSelection()) {
        qDebug()<<" why???? no QItemSelectionModel??";
        return;
    }

    cidx = ism->currentIndex();
    idx = ism->model()->index(cidx.row(), 0, cidx.parent());

    QString local_file = this->curr_item_view==this->uiw->treeView
        ? this->dir_file_model->filePath(idx) : this->model->filePath(idx);
    
    QApplication::clipboard()->setText(local_file);
}

void LocalView::slot_show_properties()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    QModelIndex cidx, idx;
    
    if (ism == 0) {
        qDebug()<<"Why???? no QItemSelectionModel??";
        return;
    }
    
    // QModelIndexList mil = ism->selectedIndexes();
    if (!ism->hasSelection()) {
        qDebug()<<"Why???? no QItemSelectionModel??";
        return;
    }

    cidx = ism->currentIndex();
    idx = ism->model()->index(cidx.row(), 0, cidx.parent());

    QString local_file = this->curr_item_view==this->uiw->treeView
        ? this->dir_file_model->filePath(idx) : this->model->filePath(idx);
    //  文件类型，大小，几个时间，文件权限
    //TODO 从模型中取到这些数据并显示在属性对话框中。
    LocalFileProperties *fp = new LocalFileProperties(this);
    fp->set_file_info_model_list(local_file);
    fp->exec();
    delete fp;
}

void LocalView::rm_file_or_directory_recursively()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
}

void LocalView::onDirectoryLoaded(const QString &path)
{
    q_debug()<<path;
    if (this->model->filePath(this->uiw->tableView->rootIndex()) == path) {
        this->uiw->tableView->resizeColumnToContents(0);
        this->onUpdateEntriesStatus();
    }

    if (this->is_dir_complete_request) {
        this->is_dir_complete_request = false;

        QString prefix = this->dir_complete_request_prefix;
        
        QStringList matches;
        QModelIndex currIndex;
        QModelIndex sourceIndex = this->model->index(path);
        int rc = this->model->rowCount(sourceIndex);
        q_debug()<<"lazy load dir rows:"<<rc;
        for (int i = rc - 1; i >= 0; --i) {
            currIndex = this->model->index(i, 0, sourceIndex);
            if (this->model->isDir(currIndex)) {
                matches << this->model->filePath(currIndex);
            }
        }
        this->uiw->widget->onSetCompleteList(prefix, matches);
    }
}

void LocalView::onFileRenamed(const QString &path, const QString &oldName, const QString &newName)
{
    q_debug()<<path<<oldName<<newName;
}

void LocalView::onRootPathChanged(const QString &newPath)
{
    q_debug()<<newPath;
}

void LocalView::onUpdateEntriesStatus()
{
    int entries = this->model->rowCount(this->uiw->tableView->rootIndex());
    QString msg = QString("%1 entries").arg(entries);
    // this->status_bar->showMessage(msg);
    this->entriesLabel->setText(msg);
}

void LocalView::slot_dir_nav_go_home()
{
    q_debug()<<"";
    // check if current index is home index
    // if no, callepse all and expand to home
    // tell dir nav instance the home path

    QModelIndex sourceIndex = this->uiw->tableView->rootIndex();
    QString rootPath = this->model->filePath(sourceIndex);
    if (rootPath != QDir::homePath()) {
        this->uiw->treeView->collapseAll();
        this->expand_to_home_directory(QModelIndex(), 1);
        this->uiw->tableView->setRootIndex(this->model->index(QDir::homePath()));
    }
    this->uiw->widget->onSetHome(QDir::homePath());
}

void LocalView::slot_dir_nav_prefix_changed(const QString &prefix)
{
    // q_debug()<<""<<prefix;
    QStringList matches;
    QModelIndex sourceIndex = this->model->index(prefix);
    QModelIndex currIndex;

    if (prefix == "") {
        sourceIndex = this->model->index(0, 0, QModelIndex());
    } else if (prefix.length() > 1
               && prefix.toUpper().at(0) >= 'A' && prefix.toUpper().at(0) <= 'Z'
               && prefix.at(1) == ':') {
        // leave not changed
    }else if (!sourceIndex.isValid()) {
        int pos = prefix.lastIndexOf('/');
        if (pos == -1) {
            pos = prefix.lastIndexOf('\\');
        }
        if (pos == -1) {
            // how deal this case
            Q_ASSERT(pos >= 0);
        }
        sourceIndex = this->model->index(prefix.left(prefix.length() - pos));
    }
    if (sourceIndex.isValid()) {
        if (this->model->canFetchMore(sourceIndex)) {
            while (this->model->canFetchMore(sourceIndex)) {
                this->is_dir_complete_request = true;
                this->dir_complete_request_prefix = prefix;
                this->model->fetchMore(sourceIndex);
            }
            // sience qt < 4.7 has no directoryLoaded signal, so we should execute now if > 0
            if (strcmp(qVersion(), "4.7.0") < 0) {
                // QTimer::singleShot(this, SLOT(
            }
        } else {
            int rc = this->model->rowCount(sourceIndex);
            q_debug()<<"lazy load dir rows:"<<rc;
            for (int i = rc - 1; i >= 0; --i) {
                currIndex = this->model->index(i, 0, sourceIndex);
                if (this->model->isDir(currIndex)) {
                    matches << this->model->filePath(currIndex);
                }
            }
            this->uiw->widget->onSetCompleteList(prefix, matches);
        }
    } else {
        // any operation for this case???
        q_debug()<<"any operation for this case???"<<prefix;
    }
}

void LocalView::slot_dir_nav_input_comfirmed(const QString &prefix)
{
    q_debug()<<"";

    QModelIndex sourceIndex = this->model->index(prefix);
    QModelIndex currIndex = this->uiw->tableView->rootIndex();
    QString currPath = this->model->filePath(currIndex);

    if (!sourceIndex.isValid()) {
        q_debug()<<"directory not found!!!"<<prefix;
        // TODO, show status???
        this->status_bar->showMessage(tr("Directory not found: %1").arg(prefix));
        return;
    }

    if (currPath != prefix) {
        this->uiw->treeView->collapseAll();
        this->expand_to_directory(prefix, 1);
        this->uiw->tableView->setRootIndex(this->model->index(prefix));
        this->uiw->listView->setRootIndex(this->uiw->tableView->rootIndex());
    }
}

void LocalView::slot_icon_size_changed(int value)
{
    q_debug()<<value;
    this->uiw->listView->setGridSize(QSize(value, value));
}

void LocalView::setFileListViewMode(int mode)
{
    if (mode == GlobalOption::FLV_LARGE_ICON) {
        this->uiw->tableView->setVisible(false);
        this->uiw->listView->setVisible(true);
        this->slot_icon_size_changed(96);
        this->uiw->listView->setViewMode(QListView::IconMode);
    } else if (mode == GlobalOption::FLV_SMALL_ICON) {
        this->uiw->tableView->setVisible(false);
        this->uiw->listView->setVisible(true);
        this->slot_icon_size_changed(48);
        this->uiw->listView->setViewMode(QListView::IconMode);
    } else if (mode == GlobalOption::FLV_LIST) {
        this->uiw->tableView->setVisible(false);
        this->uiw->listView->setVisible(true);
        this->slot_icon_size_changed(32);
        this->uiw->listView->setViewMode(QListView::ListMode);
    } else if (mode == GlobalOption::FLV_DETAIL) {
        this->uiw->tableView->setVisible(true);
        this->uiw->listView->setVisible(false);
    } else {
        Q_ASSERT(1 == 2);
    }
}

