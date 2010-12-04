// ftpview.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-05-05 21:49:36 +0800
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>

#include "globaloption.h"
#include "utils.h"

#include "progressdialog.h"
#include "localview.h"
#include "ftpview.h"
#include "netdirsortfiltermodel.h"

#include "fileproperties.h"
#include "ftphostinfodialog.h"

#ifndef _MSC_VER
#warning "wrapper lower class, drop this include"
#endif

#include "rfsdirnode.h"

#include "ftpconnection.h"
#include "libftp/libftp.h"

FTPView::FTPView(QMdiArea *main_mdi_area, LocalView *local_view, QWidget *parent)
    : RemoteView(main_mdi_area, local_view, parent)
{
    this->init_popup_context_menu();
    this->rconn = NULL;
}

void FTPView::init_popup_context_menu()
{
    RemoteView::init_popup_context_menu();

    // encoding select menu
    QMenu *emenu = this->encodingMenu();
    this->dir_tree_context_menu->addMenu(emenu);
}

FTPView::~FTPView()
{
    // because this line will call in base view class RemoteView
    // this->uiw.treeView->setModel(0);
    // delete this->remote_dir_model;
}

// TODO 根据服务器端的SYST值，设置初始化编码。WIN->GBK, OTHER->UTF-8
QMenu *FTPView::encodingMenu()
{
    QString codecNames = "UTF-8,ISO-8895-1,,GBK,BIG5,EUC-JP,EUC-KR,KIO8-R,KIO8-U";
    QMenu *emenu = new QMenu(tr("Charactor encoding"), 0);
    QAction *action = NULL;
    QActionGroup *ag = new QActionGroup(this);
    QString codecName;

    QStringList codecList = codecNames.split(',');
    for (int i = 0 ; i < codecList.count(); ++i) {
        codecName = codecList.at(i).trimmed();
        action = new QAction(codecName, 0);
        emenu->addAction(action);
        if (codecName.isEmpty()) {
            action->setSeparator(true);
        } else {
            action->setCheckable(true);
            action->setActionGroup(ag);
            QObject::connect(action, SIGNAL(triggered()), this, SLOT(encodingChanged()));
        }
        if (i == 0) {
            action->setChecked(true);
        }
    }

    // action = new QAction("UTF-8", 0);
    // action->setCheckable(true);
    // action->setChecked(true);
    // action->setActionGroup(ag);
    // emenu->addAction(action);
    // QObject::connect(action, SIGNAL(triggered()), this, SLOT(encodingChanged()));
    
    // action = new QAction("ISO-8859-1", 0);
    // action->setCheckable(true);
    // action->setActionGroup(ag);
    // emenu->addAction(action);
    // QObject::connect(action, SIGNAL(triggered()), this, SLOT(encodingChanged()));

    // action = new QAction("", 0);
    // action->setSeparator(true);
    // emenu->addAction(action);
    
    // action = new QAction("GBK", 0);
    // action->setCheckable(true);
    // action->setActionGroup(ag);
    // emenu->addAction(action);
    // QObject::connect(action, SIGNAL(triggered()), this, SLOT(encodingChanged()));

    // action = new QAction("BIG5", 0);
    // action->setCheckable(true);
    // action->setActionGroup(ag);
    // emenu->addAction(action);
    // QObject::connect(action, SIGNAL(triggered()), this, SLOT(encodingChanged()));

    return emenu;
}

void FTPView::slot_show_fxp_command_log(bool show)
{
    this->uiw.listView->setVisible(show);    
}

void FTPView::i_init_dir_view()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    this->remote_dir_model = new RemoteDirModel();
    this->remote_dir_model->setConnection(this->conn);

    // operation log
    this->m_operationLogModel = new QStringListModel();
    this->uiw.listView->setModel(this->m_operationLogModel);
    // TODO the log msg maybe come from another object, but not remote_dir_model
    QObject::connect(this->remote_dir_model, SIGNAL(operationTriggered(QString)), 
                     this, SLOT(slot_operation_triggered(QString)));
    QObject::connect(this->remote_dir_model, SIGNAL(directoryLoaded(const QString &)),
                     this, SLOT(onDirectoryLoaded(const QString &)));
    
    this->remote_dir_model->set_user_home_path(this->user_home_path);
    this->m_tableProxyModel = new DirTableSortFilterModel();
    this->m_tableProxyModel->setSourceModel(this->remote_dir_model);
    this->m_treeProxyModel = new DirTreeSortFilterModel();
    this->m_treeProxyModel->setSourceModel(this->remote_dir_model);
    
    this->uiw.treeView->setModel(m_treeProxyModel);
    this->uiw.treeView->setAcceptDrops(true);
    this->uiw.treeView->setDragEnabled(false);
    this->uiw.treeView->setDropIndicatorShown(true);
    this->uiw.treeView->setDragDropMode(QAbstractItemView::DropOnly);

    QObject::connect(this->remote_dir_model,
                     SIGNAL(sig_drop_mime_data(const QMimeData *, Qt::DropAction, int, int, const QModelIndex &)),
                     this, SLOT(slot_drop_mime_data(const QMimeData *, Qt::DropAction, int, int, const QModelIndex &)));
    
    QObject::connect(this->remote_dir_model, SIGNAL(enter_remote_dir_retrive_loop()),
                     this, SLOT(slot_enter_remote_dir_retrive_loop()));
    QObject::connect(this->remote_dir_model, SIGNAL(leave_remote_dir_retrive_loop()),
                     this, SLOT(slot_leave_remote_dir_retrive_loop()));
    
    this->uiw.treeView->expandAll();
    this->uiw.treeView->setColumnWidth(0, this->uiw.treeView->columnWidth(0)*2);
    
    // 这里设置为true时，导致这个treeView不能正确显示滚动条了，为什么呢?
    //this->uiw.treeView->setColumnHidden( 1, false);
    this->uiw.treeView->setColumnWidth(1, 0); // 使用这种方法隐藏看上去就正常了。
    this->uiw.treeView->setColumnHidden(2, true);
    this->uiw.treeView->setColumnHidden(3, true);
    
    /////tableView
    QModelIndex homeIndex = this->remote_dir_model->index(this->user_home_path);
    this->uiw.tableView->setModel(this->m_tableProxyModel);
    this->uiw.tableView->setRootIndex(this->m_tableProxyModel->mapFromSource(homeIndex));

    //change row height of table 
    if (this->m_tableProxyModel->rowCount(homeIndex) > 0) {
        this->table_row_height = this->uiw.tableView->rowHeight(0)*2/3;
    } else {
        this->table_row_height = 20;
    }
    for (int i = 0; i < this->m_tableProxyModel->rowCount(homeIndex); i ++) {
        this->uiw.tableView->setRowHeight(i, this->table_row_height);
    }
    this->uiw.tableView->resizeColumnToContents(0);

    // list view
    this->uiw.listView_2->setModel(this->m_tableProxyModel);
    this->uiw.listView_2->setRootIndex(this->m_tableProxyModel->mapFromSource(homeIndex));
    this->uiw.listView_2->setViewMode(QListView::IconMode);
    this->uiw.listView_2->setGridSize(QSize(80, 80));
    this->uiw.listView_2->setSelectionModel(this->uiw.tableView->selectionModel());
    
    /////
    QObject::connect(this->uiw.treeView, SIGNAL(clicked(const QModelIndex &)),
                     this, SLOT(slot_dir_tree_item_clicked(const QModelIndex &)));
    QObject::connect(this->uiw.tableView, SIGNAL(doubleClicked(const QModelIndex &)),
                     this, SLOT(slot_dir_file_view_double_clicked(const QModelIndex &)));
    QObject::connect(this->uiw.tableView, SIGNAL(drag_ready()),
                     this, SLOT(slot_drag_ready()));
    QObject::connect(this->uiw.listView_2, SIGNAL(doubleClicked(const QModelIndex &)),
                     this, SLOT(slot_dir_file_view_double_clicked(const QModelIndex &)));

    this->uiw.widget->onSetHome(this->user_home_path);

    //TODO 连接remoteview.treeView 的drag信号
    
    //显示SSH服务器信息
    // QString ssh_server_version = libssh2_session_get_remote_version(this->ssh2_sess);
    // int ssh_sftp_version = libssh2_sftp_get_version(this->ssh2_sftp);
    // QString status_msg = QString("Ready. (%1  FTP: V%2)").arg(ssh_server_version).arg(ssh_sftp_version); 
    // this->status_bar->showMessage(status_msg);
}

void FTPView::slot_disconnect_from_remote_host()
{
    this->uiw.treeView->setModel(0);
    delete this->remote_dir_model;
    this->remote_dir_model = 0;
}

void FTPView::slot_dir_tree_customContextMenuRequested(const QPoint &pos)
{
    this->curr_item_view = static_cast<QAbstractItemView*>(sender());
    QPoint real_pos = this->curr_item_view->mapToGlobal(pos);
    real_pos = QPoint(real_pos.x() + 10, real_pos.y() + 26);
    attr_action->setEnabled(!this->in_remote_dir_retrive_loop);
    this->dir_tree_context_menu->popup(real_pos);
}

void FTPView::slot_new_transfer()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
     
    QString file_path;
    TaskPackage remote_pkg(PROTO_FTP);
    
    if (this->in_remote_dir_retrive_loop) {
        QMessageBox::warning(this, tr("Notes:"), tr("Retriving remote directory tree,wait a minute please."));
        return;
    }
    
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    QModelIndex cidx, idx, pidx;
    if (ism == 0) {
        QMessageBox::critical(this, tr("Waring..."), tr("Maybe you haven't connected"));
        return;
    }
    if (!ism->hasSelection()) {
        return;
    }
    
    // QModelIndexList mil = ism->selectedIndexes();
    // for(int i = 0; i < mil.size(); i +=4 ) {
    //     QModelIndex midx = mil.at(i);
    //     QModelIndex proxyIndex = midx;
    //     QModelIndex sourceIndex = (this->curr_item_view == this->uiw.treeView)
    //         ? this->m_treeProxyModel->mapToSource(midx)
    //         : this->m_tableProxyModel->mapToSource(midx);
    //     QModelIndex useIndex = sourceIndex;

    //     qDebug()<<this->remote_dir_model->fileName(useIndex)<<" "<<" "<<this->remote_dir_model->filePath(useIndex);
    //     file_path = this->remote_dir_model->filePath(useIndex);
    //     remote_pkg.files<<file_path;
    // }

    cidx = ism->currentIndex();
    pidx = cidx.parent();

    for (int i = ism->model()->rowCount(pidx) - 1; i >= 0; --i) {
        if (ism->isRowSelected(i, pidx)) {
            idx = ism->model()->index(i, 0, cidx.parent());
            QModelIndex midx = idx;
            QModelIndex proxyIndex = midx;
            QModelIndex sourceIndex = (this->curr_item_view == this->uiw.treeView)
                ? this->m_treeProxyModel->mapToSource(midx)
                : this->m_tableProxyModel->mapToSource(midx);
            QModelIndex useIndex = sourceIndex;

            qDebug()<<this->remote_dir_model->fileName(useIndex)<<" "<<" "<<this->remote_dir_model->filePath(useIndex);
            file_path = this->remote_dir_model->filePath(useIndex);
            remote_pkg.files<<file_path;
        }
    }

    remote_pkg.host = this->conn->hostName;
    remote_pkg.port = QString("%1").arg(this->conn->port);
    remote_pkg.username = this->conn->userName;
    remote_pkg.password = this->conn->password;
    remote_pkg.pubkey = this->conn->pubkey;

    this->slot_new_download_requested(remote_pkg);
}

QString FTPView::get_selected_directory()
{
    QString file_path;
    
    QItemSelectionModel *ism = this->uiw.treeView->selectionModel();
    QModelIndex cidx, idx, pidx;
    
    if (ism == 0) {
        QMessageBox::critical(this, tr("Waring..."),
                              tr("Maybe you haven't connected"));                
        return file_path;
    }
    if (!ism->hasSelection()) {
        return QString();
    }
    
    // QModelIndexList mil = ism->selectedIndexes();
    // for (int i = 0; i < mil.size(); i += 4) {
    //     QModelIndex midx = mil.at(i);
    //     QModelIndex proxyIndex = midx;
    //     QModelIndex sourceIndex = this->m_treeProxyModel->mapToSource(midx);
    //     QModelIndex useIndex = sourceIndex;

    //     if (this->remote_dir_model->isDir(useIndex)) {
    //         file_path = this->remote_dir_model->filePath(useIndex);
    //     } else {
    //         file_path = "";
    //     }
    //     break; // only use the first line
    // }

    cidx = ism->currentIndex();
    pidx = cidx.parent();

    for (int i = ism->model()->rowCount(pidx) - 1; i >= 0 ; --i) {
        if (ism->isRowSelected(i, pidx)) {
            idx = ism->model()->index(i, 0, pidx);
            QModelIndex midx = idx;
            QModelIndex proxyIndex = midx;
            QModelIndex sourceIndex = this->m_treeProxyModel->mapToSource(midx);
            QModelIndex useIndex = sourceIndex;

            if (this->remote_dir_model->isDir(useIndex)) {
                file_path = this->remote_dir_model->filePath(useIndex);
            } else {
                file_path = "";
            }
            break; // only use the first line
        }
    }
    
    return file_path;
}

QPair<QString, QString> FTPView::get_selected_directory(bool pair)
{
    QPair<QString, QString> file_path;
    // QString file_path;
    
    QItemSelectionModel *ism = this->uiw.treeView->selectionModel();
    QModelIndex cidx, idx, pidx;
    
    if (ism == 0) {
        QMessageBox::critical(this, tr("Waring..."),
                              tr("Maybe you haven't connected"));                
        return file_path;
    }
    if (!ism->hasSelection()) {
        return file_path;
    }
    
    // QModelIndexList mil = ism->selectedIndexes();
    // for (int i = 0; i < mil.size(); i +=4) {
    //     QModelIndex midx = mil.at(i);

    //     QModelIndex proxyIndex = midx;
    //     QModelIndex sourceIndex = this->m_treeProxyModel->mapToSource(midx);
    //     QModelIndex useIndex = sourceIndex;

    //     if (this->remote_dir_model->isDir(useIndex)) {
    //         file_path.first = this->remote_dir_model->filePath(useIndex);
    //         file_path.second = this->remote_dir_model->fileName(useIndex);
    //     } else {
    //         file_path.first = "";
    //         file_path.second = "";
    //     }
    // }

    cidx = ism->currentIndex();
    pidx = cidx.parent();

    for (int i = ism->model()->rowCount(pidx) - 1; i >= 0 ; --i) {
        if (ism->isRowSelected(i, pidx)) {
            idx = ism->model()->index(i, 0, pidx);
            QModelIndex midx = idx;

            QModelIndex proxyIndex = midx;
            QModelIndex sourceIndex = this->m_treeProxyModel->mapToSource(midx);
            QModelIndex useIndex = sourceIndex;

            if (this->remote_dir_model->isDir(useIndex)) {
                file_path.first = this->remote_dir_model->filePath(useIndex);
                file_path.second = this->remote_dir_model->fileName(useIndex);
            } else {
                file_path.first = "";
                file_path.second = "";
            }
        }
    }
    
    return file_path;
}

void FTPView::setConnection(Connection *conn)
{
    this->conn = conn;
    this->rconn = static_cast<FTPConnection*>(conn);
    this->setWindowTitle(this->conn->protocol + ": " + this->conn->userName + "@" + this->conn->hostName);
}

void FTPView::closeEvent(QCloseEvent *event)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    event->ignore();
    if (this->in_remote_dir_retrive_loop) {
        //TODO 怎么能友好的结束
        //QMessageBox::warning(this,tr("Attentions:"),tr("Retriving remote directory tree, wait a minute please.") );
        //return;
        //如果说是在上传或者下载,则强烈建议用户先关闭传输窗口，再关闭连接
        if (this->own_progress_dialog != 0) {
            QMessageBox::warning(this, tr("Attentions:"), tr("You can't close connection when transfering file."));
            return;
        }
    }
    //this->setVisible(false);
    if (QMessageBox::question(this, tr("Attemp to close this window?"),tr("Are you sure disconnect from %1?").arg(this->windowTitle()), QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Ok) {
        this->setVisible(false);
        qDebug()<<"delete remote view";
        delete this;
    }
}
void FTPView::slot_custom_ui_area()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QSizePolicy sp;
    sp.setVerticalPolicy(QSizePolicy::Ignored);
    this->uiw.listView->setSizePolicy(sp);
    //这个设置必须在show之前设置才有效果
    this->uiw.splitter->setStretchFactor(0,1);
    this->uiw.splitter->setStretchFactor(1,2);

    this->uiw.splitter_2->setStretchFactor(0,6);
    this->uiw.splitter_2->setStretchFactor(1,1);
    // this->uiw.listView->setVisible(false);//暂时没有功能在里面先隐藏掉
    //this->uiw.tableView->setVisible(false);
    qDebug()<<this->geometry();
    this->setGeometry(this->x(), this->y(), this->width(), this->height()*2);
    qDebug()<<this->geometry();
}

void FTPView::slot_enter_remote_dir_retrive_loop()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    this->in_remote_dir_retrive_loop = true;
    this->remote_dir_model->set_keep_alive(false);
    this->orginal_cursor = this->uiw.splitter->cursor();
    this->uiw.splitter->setCursor(Qt::BusyCursor);
}

void FTPView::slot_leave_remote_dir_retrive_loop()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    this->uiw.splitter->setCursor(this->orginal_cursor);
    this->remote_dir_model->set_keep_alive(true);
    this->in_remote_dir_retrive_loop = false;
    for (int i = 0; i < this->m_tableProxyModel->rowCount(this->uiw.tableView->rootIndex()); i ++) {
        this->uiw.tableView->setRowHeight(i, this->table_row_height);
    }
    this->uiw.tableView->resizeColumnToContents(0);
    this->onUpdateEntriesStatus();
}

void FTPView::update_layout()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QString file_path;
    
    QItemSelectionModel *ism = this->uiw.treeView->selectionModel();
    QModelIndex cidx, idx, pidx;

    if (ism == 0) {
        //QMessageBox::critical(this,tr("waring..."),tr("maybe you haven't connected")); //return file_path;
        qDebug()<<"why???? no QItemSelectionModel??";        
        return;
    }
    
    // QModelIndexList mil = ism->selectedIndexes();
    // if (mil.count() == 0) {
    //     qDebug()<<"selectedIndexes count :"<< mil.count()<<"why no item selected????";
    // }
    if (!ism->hasSelection()) {
        qDebug()<<"selectedIndexes count :"<< ism->hasSelection()<<"why no item selected????";
        return;
    }

    cidx = ism->currentIndex();
    pidx = cidx.parent();

    for (int i = ism->model()->rowCount(pidx) - 1; i >= 0 ; --i) {
        if (ism->isRowSelected(i, pidx)) {
            idx = ism->model()->index(i, 0, pidx);
            QModelIndex midx = idx;
            qDebug()<<midx;
            // 这个地方为什么不使用mapToSource会崩溃呢？
            NetDirNode *dti = static_cast<NetDirNode*>
                (this->m_treeProxyModel->mapToSource(midx).internalPointer());
            qDebug()<<dti->_fileName<<" "<< dti->fullPath;
            file_path = dti->fullPath;
            dti->retrFlag = POP_NO_NEED_WITH_DATA; // 1;
            dti->prevFlag = POP_NEWEST; // 9;
            this->remote_dir_model->slot_remote_dir_node_clicked(this->m_treeProxyModel->mapToSource(midx));
        }
    }
    // for (int i = 0; i < mil.size(); i += 4) {
    //     QModelIndex midx = mil.at(i);
    //     qDebug()<<midx;
    //     // 这个地方为什么不使用mapToSource会崩溃呢？
    //     NetDirNode *dti = static_cast<NetDirNode*>
    //         (this->m_treeProxyModel->mapToSource(midx).internalPointer());
    //     qDebug()<<dti->_fileName<<" "<< dti->fullPath;
    //     file_path = dti->fullPath;
    //     dti->retrFlag = POP_NO_NEED_WITH_DATA; // 1;
    //     dti->prevFlag = POP_NEWEST; // 9;
    //     this->remote_dir_model->slot_remote_dir_node_clicked(this->m_treeProxyModel->mapToSource(midx));
    // }
}

// inhirented from base class
// void FTPView::slot_refresh_directory_tree()
// {
//     this->update_layout();
// }

void FTPView::slot_show_properties()
{
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    QModelIndex cidx, idx, pidx;
    
    if (ism == 0) {
        qDebug()<<" why???? no QItemSelectionModel??";        
        return;
    }
    if (!ism->hasSelection()) {
        return;
    }

    cidx = ism->currentIndex();
    pidx = cidx.parent();

    QModelIndexList aim_mil;

    for (int i = 0; i < ism->model()->columnCount(pidx) ; ++i) {
        idx = ism->model()->index(cidx.row(), i, pidx);
        aim_mil << idx;
    }
    
    // QModelIndexList mil = ism->selectedIndexes();
    // QModelIndexList aim_mil;
    // if (this->curr_item_view == this->uiw.treeView) {
    //     for (int i = 0; i < mil.count(); i ++) {
    //         aim_mil<<this->m_treeProxyModel->mapToSource(mil.at(i));
    //         // aim_mil << mil.at(i);
    //     }
    // } else if (this->curr_item_view == this->uiw.listView_2) {
    //     QModelIndex currIndex;
    //     QModelIndex tmpIndex;
    //     for (int i = 0; i < mil.count(); i ++) {
    //         currIndex = this->m_tableProxyModel->mapToSource(mil.at(i));
    //         aim_mil<<currIndex;
    //         // aim_mil<<mil.at(i);
    //         for (int col = 1; col < 4; col ++) {
    //             tmpIndex = this->remote_dir_model->index(currIndex.row(), col, currIndex.parent());
    //             aim_mil << tmpIndex;
    //         }
    //     }
    // } else if (this->curr_item_view == this->uiw.tableView) {
    //     if (mil.count() % 4 == 0) {
    //         for (int i = 0; i < mil.count(); i ++) {
    //             aim_mil<<this->m_tableProxyModel->mapToSource(mil.at(i));
    //             // aim_mil<<mil.at(i);
    //         }
    //     } else {
    //         QModelIndex currIndex;
    //         QModelIndex tmpIndex;
    //         for (int i = 0; i < mil.count(); i ++) {
    //             currIndex = this->m_tableProxyModel->mapToSource(mil.at(i));
    //             aim_mil<<currIndex;
    //             // aim_mil<<mil.at(i);
    //             for (int col = 1; col < 4; col ++) {
    //                 tmpIndex = this->remote_dir_model->index(currIndex.row(), col, currIndex.parent());
    //                 aim_mil << tmpIndex;
    //             }
    //         }
    //     }
    // } else {
    //     Q_ASSERT(1 == 2);
    // }

    if (aim_mil.count() == 0) {
        qDebug()<<"why???? no QItemSelectionModel??";
        return;
    }
    // 文件类型，大小，几个时间，文件权限
    // TODO 从模型中取到这些数据并显示在属性对话框中。
    FileProperties *fp = new FileProperties(this);
    // fp->set_ssh2_sftp(this->ssh2_sftp);
    fp->setConnection(this->conn);
    fp->set_file_info_model_list(aim_mil);
    fp->exec();
    delete fp;
}

void FTPView::slot_mkdir()
{
    QString dir_name;
    
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    QModelIndex cidx, idx, pidx;
    
    if (ism == 0) {
        qDebug()<<"why???? no QItemSelectionModel??";
        QMessageBox::critical(this, tr("Waring..."), tr("Maybe you haven't connected"));                
        return;
    }
    
    // QModelIndexList mil = ism->selectedIndexes();
    // if (mil.count() == 0) {
    //     qDebug()<<"selectedIndexes count :"<<mil.count()<<" why no item selected????";
    //     QMessageBox::critical(this, tr("Waring..."), tr("No item selected"));
    //     return;
    // }
    if (!ism->hasSelection()) {
        qDebug()<<"selectedIndexes count :"<<ism->hasSelection()<<" why no item selected????";
        QMessageBox::critical(this, tr("Waring..."), tr("No item selected"));
        return;
    }
    
    cidx = ism->currentIndex();
    idx = ism->model()->index(cidx.row(), 0, cidx.parent());

    QModelIndex midx = idx;
    QModelIndex aim_midx = (this->curr_item_view == this->uiw.treeView)
        ? this->m_treeProxyModel->mapToSource(midx): this->m_tableProxyModel->mapToSource(midx);
    NetDirNode *dti = (NetDirNode*)(aim_midx.internalPointer());
    
    //检查所选择的项是不是目录
    if (!this->remote_dir_model->isDir(aim_midx)) {
        QMessageBox::critical(this, tr("waring..."),
                              tr("The selected item is not a directory."));
        return;
    }
    
    dir_name = QInputDialog::getText(this, tr("Create directory:"),
                                     tr("Input directory name:").leftJustified(60, ' '),
                                     QLineEdit::Normal,
                                     tr("new_direcotry"));
    if (dir_name == QString::null) {
        return;
    } 
    if (dir_name.length() == 0) {
        qDebug()<<"selectedIndexes count :"<<ism->hasSelection()<<"why no item selected????";
        QMessageBox::critical(this, tr("waring..."), tr("no directory name supplyed "));
        return;
    }

    // TODO 将 file_path 转换编码再执行下面的操作
    QPersistentModelIndex *persisIndex = new QPersistentModelIndex(aim_midx);
    this->remote_dir_model->slot_execute_command(dti, persisIndex, SSH2_FXP_MKDIR, dir_name);
}

void FTPView::rm_file_or_directory_recursively()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    QModelIndex cidx, idx, pidx;
    
    if (ism == 0) {
        qDebug()<<"why???? no QItemSelectionModel??";
        QMessageBox::critical(this, tr("Waring..."), tr("Maybe you haven't connected"));                
        return;
    }
    
    // QModelIndexList mil = ism->selectedIndexes();
    // if (mil.count() == 0) {
    //     qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
    //     QMessageBox::critical(this, tr("Waring..."), tr("No item selected"));
    //     return;
    // }
    if (!ism->hasSelection()) {
        qDebug()<<" selectedIndexes count :"<< ism->hasSelection() << " why no item selected????";
        QMessageBox::critical(this, tr("Waring..."), tr("No item selected"));
        return;
    }

    cidx = ism->currentIndex();
    pidx = cidx.parent();

    // 支持多选情况
    bool firstWarning = true;
    for (int i = 0 ; i < ism->model()->rowCount(pidx); i++) {
        if (!ism->isRowSelected(i, pidx)) {
            continue;
        }
        idx = ism->model()->index(i, 0, pidx);
        QModelIndex midx = idx;
        QModelIndex aim_midx = (this->curr_item_view == this->uiw.treeView) 
            ? this->m_treeProxyModel->mapToSource(midx)
            : this->m_tableProxyModel->mapToSource(midx);
        NetDirNode *dti = (NetDirNode*) aim_midx.internalPointer();
        if (firstWarning) { // 只有第一次提示用户操作，其他的不管
            QString hintMsg;
            if (ism->model()->rowCount(pidx) > 0) {
                hintMsg = QString(tr("Are you sure remove all of these files/directories?"));
            } else {
                hintMsg = dti->isDir() 
                    ? QString(tr("Are you sure remove this directory and it's subnodes?\n    %1/"))
                    .arg(dti->filePath())
                    : QString(tr("Are you sure remove this file?\n    %1"))
                    .arg(dti->filePath());
            }
            if (QMessageBox::warning(this, tr("Warning:"), hintMsg,
                                     QMessageBox::Yes, QMessageBox::Cancel) == QMessageBox::Yes) {
            } else {
                break;
            }
            firstWarning = false;
        }
        QModelIndex parent_model =  aim_midx.parent();
        NetDirNode *parent_item = (NetDirNode*)parent_model.internalPointer();

        QPersistentModelIndex *persisIndex = new QPersistentModelIndex(parent_model);
        this->remote_dir_model->slot_execute_command(parent_item, persisIndex,
                                                     SSH2_FXP_REMOVE, dti->_fileName);
    }

    // 支持多选情况
    // TODO magic number
    // bool firstWarning = true;
    // for (int i = mil.count() - 1 - 3; i >= 0; i -= 4) {
    //     QModelIndex midx = mil.at(i);
    //     QModelIndex aim_midx = (this->curr_item_view == this->uiw.treeView) 
    //         ? this->m_treeProxyModel->mapToSource(midx)
    //         : this->m_tableProxyModel->mapToSource(midx);
    //     NetDirNode *dti = (NetDirNode*) aim_midx.internalPointer();
    //     if (firstWarning) { // 只有第一次提示用户操作，其他的不管
    //         QString hintMsg;
    //         if (mil.count()/4 > 1) {
    //             hintMsg = QString(tr("Are you sure remove all of these files/directories?"));
    //         } else {
    //             hintMsg = dti->isDir() 
    //                 ? QString(tr("Are you sure remove this directory and it's subnodes?\n    %1/"))
    //                 .arg(dti->filePath())
    //                 : QString(tr("Are you sure remove this file?\n    %1"))
    //                 .arg(dti->filePath());
    //         }
    //         if (QMessageBox::warning(this, tr("Warning:"), hintMsg,
    //                                  QMessageBox::Yes, QMessageBox::Cancel) == QMessageBox::Yes) {
    //         } else {
    //             break;
    //         }
    //         firstWarning = false;
    //     }
    //     QModelIndex parent_model =  aim_midx.parent();
    //     NetDirNode *parent_item = (NetDirNode*)parent_model.internalPointer();

    //     QPersistentModelIndex *persisIndex = new QPersistentModelIndex(parent_model);
    //     this->remote_dir_model->slot_execute_command(parent_item, persisIndex,
    //                                                  SSH2_FXP_REMOVE, dti->_fileName);
    // }
}

void FTPView::slot_rename()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    QModelIndex cidx, idx;
    
    if (ism == 0) {
        qDebug()<<"why???? no QItemSelectionModel??";
        QMessageBox::critical(this, tr("waring..."), tr("maybe you haven't connected"));                
        return;
    }
    
    // QModelIndexList mil = ism->selectedIndexes();
    // if (mil.count() == 0 ) {
    //     qDebug()<<"selectedIndexes count :"<<mil.count()<<"why no item selected????";
    //     QMessageBox::critical(this, tr("waring..."), tr("no item selected"));
    //     return;
    // }
    if (!ism->hasSelection()) {
        qDebug()<<"selectedIndexes count :"<<ism->hasSelection()<<"why no item selected????";
        QMessageBox::critical(this, tr("waring..."), tr("no item selected"));
        return;
    }
    
    cidx = ism->currentIndex();
    idx = ism->model()->index(cidx.row(), 0, cidx.parent());
    this->curr_item_view->edit(idx);
}
void FTPView::slot_copy_path()
{
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    QModelIndex cidx, idx;    
    
    if (ism == 0) {
        qDebug()<<"why???? no QItemSelectionModel??";
        QMessageBox::critical(this, tr("Waring..."), tr("Maybe you haven't connected"));
        return;
    }
    
    // QModelIndexList mil = ism->selectedIndexes();
    // if (mil.count() == 0) {
    //     qDebug()<<"selectedIndexes count :"<<mil.count()<<"why no item selected????";
    //     QMessageBox::critical(this, tr("Waring..."), tr("No item selected").leftJustified(50, ' '));
    //     return;
    // }
    if (!ism->hasSelection()) {
        qDebug()<<"selectedIndexes count :"<<ism->hasSelection()<<"why no item selected????";
        QMessageBox::critical(this, tr("Waring..."), tr("No item selected").leftJustified(50, ' '));
        return;
    }
    
    cidx = ism->currentIndex();
    idx = ism->model()->index(cidx.row(), 0, cidx.parent());

    QModelIndex midx = idx;
    QModelIndex aim_midx = (this->curr_item_view == this->uiw.treeView) 
        ? this->m_treeProxyModel->mapToSource(midx)
        : this->m_tableProxyModel->mapToSource(midx);    
    NetDirNode *dti = (NetDirNode*)aim_midx.internalPointer();

    QApplication::clipboard()->setText(dti->fullPath);
}

void FTPView::slot_copy_url()
{
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    QModelIndex cidx, idx;

    if (ism == 0) {
        qDebug()<<" why???? no QItemSelectionModel??";
        QMessageBox::critical(this, tr("Waring..."), tr("Maybe you haven't connected"));
        return ;
    }
    
    // QModelIndexList mil = ism->selectedIndexes();
    // if (mil.count() == 0) {
    //     qDebug()<<"selectedIndexes count :"<<mil.count()<<" why no item selected????";
    //     QMessageBox::critical(this, tr("Waring..."), tr("No item selected").leftJustified(50, ' '));
    //     return;
    // }
    if (!ism->hasSelection()) {
        qDebug()<<"selectedIndexes count :"<<ism->hasSelection()<<" why no item selected????";
        QMessageBox::critical(this, tr("Waring..."), tr("No item selected").leftJustified(50, ' '));
        return;
    }

    cidx = ism->currentIndex();
    idx = ism->model()->index(cidx.row(), 0, cidx.parent());

    QModelIndex midx = idx;
    QModelIndex aim_midx = (this->curr_item_view == this->uiw.treeView) 
        ? this->m_treeProxyModel->mapToSource(midx)
        : this->m_tableProxyModel->mapToSource(midx);    
    NetDirNode *dti = (NetDirNode*) aim_midx.internalPointer();

    QString url = QString("ftp://%1@%2:%3%4").arg(this->conn->userName)
        .arg(this->conn->hostName).arg(this->conn->port).arg(dti->fullPath);
    QApplication::clipboard()->setText(url);
}


void FTPView::slot_new_upload_requested(TaskPackage local_pkg, TaskPackage remote_pkg)
{
    FTPView *remote_view = this;
    ProgressDialog *pdlg = new ProgressDialog();

    pdlg->set_transfer_info(local_pkg, remote_pkg);

    QObject::connect(pdlg, SIGNAL(transfer_finished(int, QString)),
                     remote_view, SLOT(slot_transfer_finished (int, QString)));

    this->main_mdi_area->addSubWindow(pdlg);
    pdlg->show();
    this->own_progress_dialog = pdlg;

}

void FTPView::slot_new_upload_requested(TaskPackage local_pkg)
{
    // QString remote__fileName;
    QPair<QString, QString> remote__fileName;
    FTPView *remote_view = this;
    TaskPackage remote_pkg(PROTO_FTP);

    qDebug()<<"window title :"<<remote_view->windowTitle();

    remote__fileName = remote_view->get_selected_directory(true);    

    // 在向根目录传文件时，这个检测是有问题的。 fullPath="/"  path=""
    if (remote__fileName.first.length() == 0
        && remote__fileName.second.length() == 0) {
        QMessageBox::critical(this, tr("Waring..."),
                              tr("you should selecte a remote file directory."));
    } else {
        remote_pkg.files<<remote__fileName.first;
        remote_pkg.host = this->conn->hostName;
        remote_pkg.username = this->conn->userName;
        remote_pkg.password = this->conn->password;
        remote_pkg.port = QString("%1").arg(this->conn->port);
        remote_pkg.pubkey = this->conn->pubkey;

        this->slot_new_upload_requested(local_pkg, remote_pkg);
    }
}

void FTPView::slot_new_download_requested(TaskPackage local_pkg, TaskPackage remote_pkg)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    FTPView *remote_view = this;
        
    ProgressDialog *pdlg = new ProgressDialog(0);
    // src is remote file , dest if localfile 
    pdlg->set_transfer_info(remote_pkg, local_pkg);
    QObject::connect(pdlg, SIGNAL(transfer_finished(int, QString)),
                     this, SLOT(slot_transfer_finished(int, QString)));
    this->main_mdi_area->addSubWindow(pdlg);
    pdlg->show();
    this->own_progress_dialog = pdlg;
}
void FTPView::slot_new_download_requested(TaskPackage remote_pkg) 
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QString local_file_path;
    FTPView *remote_view = this;
    TaskPackage local_pkg(PROTO_FILE);
    
    local_file_path = this->local_view->get_selected_directory();
    
    qDebug()<<local_file_path;
    if (local_file_path.length() == 0 
        || !QFileInfo(local_file_path).isDir()) {
        qDebug()<<"selected a local file directory  please";
        QMessageBox::critical(this, tr("waring..."), 
                              tr("you should selecte a local file directory."));
    } else {
        local_pkg.files<<local_file_path;
        this->slot_new_download_requested(local_pkg, remote_pkg);
    }
}

void FTPView::slot_transfer_finished(int status, QString errorString)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    FTPView *remote_view = this;
    
    ProgressDialog *pdlg = (ProgressDialog*)sender();

    if (status == 0 || status ==3) {
        //TODO 通知UI更新目录结构,在某些情况下会导致左侧树目录变空。
        //int transfer_type = pdlg->get_transfer_type();
        //if ( transfer_type == TransferThread::TRANSFER_GET )
        {
            this->local_view->update_layout();
        }
        //else if ( transfer_type == TransferThread::TRANSFER_PUT )
        {
            remote_view->update_layout();
        }
        //else
        {
            // xxxxx: shound not occurs
            //assert ( 1== 2 );
        }
    } else if (status != 0 && status != 3) {
        QString errmsg = QString(errorString).arg(status);
        if (errmsg.length() < 50) errmsg = errmsg.leftJustified(50);
        QMessageBox::critical(this, QString(tr("Error: ")), errmsg);
    }
    this->main_mdi_area->removeSubWindow(pdlg->parentWidget());

    delete pdlg;
    this->own_progress_dialog = 0;
    //     remote_view->slot_leave_remote_dir_retrive_loop();
}

/**
 *
 * index is proxyIndex 
 */
void FTPView::slot_dir_tree_item_clicked(const QModelIndex & index)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QString file_path;
    QModelIndex proxyIndex;
    QModelIndex sourceIndex;
    QModelIndex useIndex;

    proxyIndex = index;
    sourceIndex = this->m_treeProxyModel->mapToSource(index);
    // sourceIndex = index;
    useIndex = sourceIndex;

    this->remote_dir_model->slot_remote_dir_node_clicked(useIndex);
    
    // file_path = this->m_treeProxyModel->filePath(index);
    this->uiw.tableView->setRootIndex(this->m_tableProxyModel->mapFromSource(useIndex));
    for (int i = 0; i < this->remote_dir_model->rowCount(useIndex); i ++ ) {
        this->uiw.tableView->setRowHeight(i, this->table_row_height);
    }
    this->uiw.tableView->resizeColumnToContents(0);
    

    this->uiw.listView_2->setRootIndex(this->m_tableProxyModel->mapFromSource(useIndex));

    if (this->remote_dir_model->filePath(useIndex) == ""
        && this->remote_dir_model->fileName(useIndex) == "/") {
        this->uiw.widget->onNavToPath(this->remote_dir_model->fileName(useIndex));
    } else {
        this->uiw.widget->onNavToPath(this->remote_dir_model->filePath(useIndex));
    }

    this->onUpdateEntriesStatus();
}

void FTPView::slot_dir_file_view_double_clicked(const QModelIndex & index)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    // TODO if the clicked item is direcotry ,
    // expand left tree dir and update right table view
    // got the file path , tell tree ' model , then expand it
    // 文件列表中的双击事件
    //1。本地主机，如果是目录，则打开这个目录，如果是文件，则使用本机的程序打开这个文件
    //2。对于远程主机，　如果是目录，则打开这个目录，如果是文件，则提示是否要下载它(或者也可以直接打开这个文件）。

    QModelIndex proxyIndex = index;
    QModelIndex sourceIndex = this->m_tableProxyModel->mapToSource(proxyIndex);
    QModelIndex useIndex = sourceIndex;

    QString file_path;
    if (this->remote_dir_model->isDir(useIndex) || this->remote_dir_model->isSymLinkToDir(useIndex)) {
        this->uiw.treeView->expand(this->m_treeProxyModel->mapFromSource(useIndex).parent());
        this->uiw.treeView->expand(this->m_treeProxyModel->mapFromSource(useIndex));
        this->slot_dir_tree_item_clicked(this->m_treeProxyModel->mapFromSource(useIndex));

        this->uiw.treeView->selectionModel()->clearSelection();
        this->uiw.treeView->selectionModel()->select(this->m_treeProxyModel->mapFromSource(useIndex), 
                                                            QItemSelectionModel::Select);
    } else if (this->remote_dir_model->isSymLink(useIndex)) {
        NetDirNode *node_item = (NetDirNode*)useIndex.internalPointer();
        QPersistentModelIndex *persisIndex = new QPersistentModelIndex(useIndex);
        q_debug()<<node_item->fullPath;
        this->remote_dir_model->dump_tree_node_item(node_item);
        this->remote_dir_model->slot_execute_command(node_item, persisIndex,
                                                     SSH2_FXP_REALPATH, QString(""));
    } else {
        q_debug()<<"double clicked a regular file, no op now, only now";
    }
}

void FTPView::slot_drag_ready()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //TODO 处理从树目录及文件列表视图中的情况
    //QAbstractItemView * sender_view = qobject_cast<QAbstractItemView*>(sender());
    QString  temp_file_path, remote__fileName;
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    
    //这个视图中所选择的目录优先，如果没有则查找左侧目录树是是否有选择的目录，如果再找不到，则使用右侧表视图的根
    QItemSelectionModel *ism = this->uiw.tableView->selectionModel();
    QModelIndex cidx, idx, pidx;

    // QModelIndexList mil = ism->selectedIndexes();
    // if (mil.count() == 0) {
    //     ism = this->uiw.treeView->selectionModel();
    //     mil = ism->selectedIndexes();
    // }
    if (!ism->hasSelection()) {
        ism = this->uiw.treeView->selectionModel();
    }

    TaskPackage tpkg(PROTO_FTP);
    
    tpkg.host = this->conn->hostName;
    tpkg.username = this->conn->userName;
    tpkg.password = this->conn->password;
    tpkg.port = QString("%1").arg(this->conn->port);
    tpkg.pubkey = this->conn->pubkey;

    cidx = ism->currentIndex();
    pidx = cidx.parent();

    // for (int i = 0; i< mil.count();i += this->remote_dir_model->columnCount()) {
    //     QModelIndex midx = mil.at(i);
    //     temp_file_path = (qobject_cast<RemoteDirModel*>(this->remote_dir_model))
    //         ->filePath(this->m_tableProxyModel->mapToSource(midx) );
    //     tpkg.files<<temp_file_path;
    // }
    for (int i = 0; i< ism->model()->rowCount(pidx); i++) {
        if (!ism->isRowSelected(i, pidx)) {
            continue;
        }
        idx = ism->model()->index(i, 0, pidx);
        QModelIndex midx = idx;
        temp_file_path = (qobject_cast<RemoteDirModel*>(this->remote_dir_model))
            ->filePath(this->m_tableProxyModel->mapToSource(midx) );
        tpkg.files<<temp_file_path;
    }
    
    mimeData->setData("application/task-package", tpkg.toRawData());
    drag->setMimeData(mimeData);
    
    if (ism->hasSelection()) {
        Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction);
        Q_UNUSED(dropAction);
    }
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<"drag->exec returned";
}

bool FTPView::slot_drop_mime_data(const QMimeData *data, Qt::DropAction action,
                                     int row, int column, const QModelIndex &parent)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    Q_UNUSED(row);
    Q_UNUSED(column);
    
    TaskPackage local_pkg(PROTO_FILE);
    TaskPackage remote_pkg(PROTO_FTP);
   
    NetDirNode *aim_item = static_cast<NetDirNode*>(parent.internalPointer());        
    QString remote_file_name = aim_item->fullPath;

    remote_pkg.files<<remote_file_name;

    remote_pkg.host = this->conn->hostName;
    remote_pkg.username = this->conn->userName;
    remote_pkg.password = this->conn->password;
    remote_pkg.port = QString("%1").arg(this->conn->port);
    remote_pkg.pubkey = this->conn->pubkey;
    
    if (data->hasFormat("application/task-package")) {
        local_pkg = TaskPackage::fromRawData(data->data("application/task-package"));
        if (local_pkg.isValid(local_pkg)) {
            // TODO 两个sftp服务器间拖放的情况
            this->slot_new_upload_requested(local_pkg, remote_pkg);
        }
    } else if (data->hasFormat("text/uri-list")) {
        // from localview
        QList<QUrl> files = data->urls();
        if (files.count() == 0) {
            // return false;
            assert(0);
        } else {
            for (int i = 0; i < files.count(); i++) {
                local_pkg.files<<files.at(i).path();
            }
            this->slot_new_upload_requested(local_pkg, remote_pkg);
        }        
    } else {
        qDebug()<<"invalid mime type:"<<data->formats();
    }
    qDebug()<<"drop mime data processed";
    
    return true;
}

void FTPView::slot_show_hidden(bool show)
{
    DirTreeSortFilterModel *tree = (DirTreeSortFilterModel*)this->m_treeProxyModel;
    DirTableSortFilterModel *table = (DirTableSortFilterModel*)this->m_tableProxyModel;
    if (show) {
        if (tree) tree->setFilter(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);
        if (table) table->setFilter(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);
    } else {
        if (tree) tree->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
        if (table) table->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    }
    this->onUpdateEntriesStatus();
}

void FTPView::onUpdateEntriesStatus()
{
    QModelIndex proxyIndex = this->uiw.tableView->rootIndex();
    QModelIndex sourceIndex = this->m_tableProxyModel->mapToSource(proxyIndex);
    QModelIndex useIndex = sourceIndex;
    int entries = this->m_tableProxyModel->rowCount(proxyIndex);
    QString msg = QString("%1 entries").arg(entries);
    this->entriesLabel->setText(msg);
}

void FTPView::onDirectoryLoaded(const QString &path)
{
    q_debug()<<path;
    // if (this->model->filePath(this->uiw.tableView->rootIndex()) == path) {
    //     this->uiw.tableView->resizeColumnToContents(0);
    //     this->onUpdateEntriesStatus();
    // }

    if (this->is_dir_complete_request) {
        this->is_dir_complete_request = false;

        QString prefix = this->dir_complete_request_prefix;
        
        QStringList matches;
        QModelIndex currIndex;
        QModelIndex sourceIndex = this->remote_dir_model->index(path);
        int rc = this->remote_dir_model->rowCount(sourceIndex);
        q_debug()<<"lazy load dir rows:"<<rc;
        for (int i = rc - 1; i >= 0; --i) {
            currIndex = this->remote_dir_model->index(i, 0, sourceIndex);
            if (this->remote_dir_model->isDir(currIndex)) {
                matches << this->remote_dir_model->filePath(currIndex);
            }
        }
        this->uiw.widget->onSetCompleteList(prefix, matches);
    }

}

void FTPView::slot_operation_triggered(QString text)
{
    // wrapper from default SSH2_FXP_ to FTP_ command
    QString myText = text;
    if (myText.indexOf("SSH2_FXP_") != -1) {
        myText = myText.replace("SSH2_FXP", "FTP");
    }

    RemoteView::slot_operation_triggered(myText);
}

void FTPView::slot_dir_nav_go_home()
{
    q_debug()<<"";
    // check if current index is home index
    // if no, callepse all and expand to home
    // tell dir nav instance the home path

    QModelIndex proxyIndex = this->uiw.tableView->rootIndex();
    QModelIndex sourceIndex = this->m_tableProxyModel->mapToSource(proxyIndex);
    QModelIndex useIndex = sourceIndex;
    
    QString rootPath = this->remote_dir_model->filePath(sourceIndex);
    if (rootPath != this->user_home_path) {
        this->uiw.treeView->collapseAll();
        this->expand_to_home_directory(QModelIndex(), 1);
        this->uiw.tableView->setRootIndex(this->remote_dir_model->index(this->user_home_path));
    }
    this->uiw.widget->onSetHome(this->user_home_path);
}

void FTPView::slot_dir_nav_prefix_changed(const QString &prefix)
{
    // q_debug()<<""<<prefix;
    QStringList matches;
    QModelIndex sourceIndex = this->remote_dir_model->index(prefix);
    QModelIndex currIndex;

    if (prefix == "") {
        sourceIndex = this->remote_dir_model->index(0, 0, QModelIndex());
    } else if (!sourceIndex.isValid()) {
        int pos = prefix.lastIndexOf('/');
        if (pos == -1) {
            pos = prefix.lastIndexOf('\\');
        }
        if (pos == -1) {
            // how deal this case
            Q_ASSERT(pos >= 0);
        }
        sourceIndex = this->remote_dir_model->index(prefix.left(prefix.length() - pos));
    }
    if (sourceIndex.isValid()) {
        if (this->remote_dir_model->canFetchMore(sourceIndex)) {
            while (this->remote_dir_model->canFetchMore(sourceIndex)) {
                this->is_dir_complete_request = true;
                this->dir_complete_request_prefix = prefix;
                this->remote_dir_model->fetchMore(sourceIndex);
            }
            // sience qt < 4.7 has no directoryLoaded signal, so we should execute now if > 0
            if (strcmp(qVersion(), "4.7.0") < 0) {
                // QTimer::singleShot(this, SLOT(
            }
        } else {
            int rc = this->remote_dir_model->rowCount(sourceIndex);
            q_debug()<<"lazy load dir rows:"<<rc;
            for (int i = rc - 1; i >= 0; --i) {
                currIndex = this->remote_dir_model->index(i, 0, sourceIndex);
                if (this->remote_dir_model->isDir(currIndex)) {
                    matches << this->remote_dir_model->filePath(currIndex);
                }
            }
            this->uiw.widget->onSetCompleteList(prefix, matches);
        }
    } else {
        // any operation for this case???
        q_debug()<<"any operation for this case???"<<prefix;
    }
}

void FTPView::slot_dir_nav_input_comfirmed(const QString &prefix)
{
    q_debug()<<prefix;

    QModelIndex sourceIndex = this->remote_dir_model->index(prefix);
    QModelIndex proxyRootIndex = this->uiw.tableView->rootIndex(); 
    QModelIndex currIndex = this->m_tableProxyModel->mapToSource(proxyRootIndex);
    QString currPath, useIndex;

    if (!sourceIndex.isValid()) {
        q_debug()<<"directory not found!!!"<<prefix;
        // TODO, show status???
        this->status_bar->showMessage(tr("Directory not found: %1").arg(prefix));
        return;
    }

    // q_debug()<<currIndex.isValid();
    if (!currIndex.isValid()) {
        // some case will be !isValid(), eg. the tableView has been set to a invalid QModelIndex
        currPath = "";
    } else {
        currPath = this->remote_dir_model->filePath(currIndex);
    }
    QModelIndex proxyIndex = this->m_tableProxyModel->mapFromSource(sourceIndex);
    q_debug()<<currPath.length()<<currPath.isEmpty()<<currPath;
    if (currPath != prefix) {
        this->uiw.treeView->collapseAll();
        this->expand_to_directory(prefix, 1);
        this->uiw.tableView->setRootIndex(proxyIndex);
        this->uiw.listView_2->setRootIndex(proxyIndex);
    }
}

// 这个图标可使用在FTPS上，显示安全性算法等。简单的FTP不需要显示。
void FTPView::encryption_focus_label_double_clicked()
{
    //qDebug()<<__FILE__<<":"<<__LINE__;
    // EncryptionDetailDialog *enc_dlg = 0;
    char **server_info, **pptr;
    int sftp_version;

    // pptr = server_info = libssh2_session_get_remote_info(this->ssh2_sess);
    // sftp_version = libssh2_sftp_get_version(this->ssh2_sftp); 

    // enc_dlg = new EncryptionDetailDialog(server_info, this);
    // enc_dlg->exec();

    // //if(server_info != NULL) free(server_info);
    // delete enc_dlg;

    // while (*pptr != NULL) {
    //     free(*pptr);
    //     pptr ++;
    // }
    // free(server_info);
}

// 可以输出FTP服务器的操作系统类型，使用编码，欢迎信息
void FTPView::host_info_focus_label_double_clicked()
{
    // HostInfoDetailFocusLabel *hi_label = (HostInfoDetailFocusLabel*)sender();
    // qDebug()<<"hehe"<<hi_label;

    LIBSSH2_CHANNEL *ssh2_channel = 0;
    int rv = -1;
    char buff[1024];
    QString evn_output;
    QString uname_output;
    const char *cmd = "uname -a";

    FTPHostInfoDialog *ftp_hid = new FTPHostInfoDialog();
    QString hostType = "Unix";
    QString welcome = this->rconn->getServerWelcome();
    ftp_hid->setHostType(hostType);
    ftp_hid->setWelcome(welcome);
    ftp_hid->exec();
    delete ftp_hid;

    // ssh2_channel = libssh2_channel_open_session((LIBSSH2_SESSION*)this->ssh2_sess);
    // //libssh2_channel_set_blocking(ssh2_channel, 1);
    // rv = libssh2_channel_exec(ssh2_channel, cmd);
    // qDebug()<<"SSH2 exec: "<<rv;
  
    // memset(buff, 0, sizeof(buff));
    // while ((rv = libssh2_channel_read(ssh2_channel, buff, 1000)) > 0) {
    //     qDebug()<<"Channel read: "<<rv<<" -->"<<buff;
    //     uname_output += QString(buff);
    //     memset(buff, 0, sizeof(buff));
    // }

    // libssh2_channel_close(ssh2_channel);
    // libssh2_channel_free(ssh2_channel);
    
    // qDebug()<<"Host Info: "<<uname_output;
    // hi_label->setToolTip(uname_output);
    
    // QDialog *dlg = new QDialog(this);
    // dlg->setFixedWidth(400);
    // dlg->setFixedHeight(100);
    // QLabel * label = new QLabel("", dlg);
    // label->setWordWrap(true);
    // label->setText(uname_output);
    // // dlg->layout()->addWidget(label);
    // dlg->exec();
    // delete dlg;
}

void FTPView::encodingChanged()
{
    QAction *action = static_cast<QAction*>(sender());
    q_debug()<<action->text()<<this->rconn;
    LibFtp *ftp = this->rconn->ftp;
    int r = ftp->setEncoding(action->text());
    Q_ASSERT(r);

    // refresh all, 
    // 但现在可能非当前目录需要在下次刷新的时候编码才能改正过来。
    this->slot_refresh_directory_tree();
}

