// sessiondialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-07-16 21:35:27 +0000
// Last-Updated: 
// Version: $Id$
// 

#include "remotehostquickconnectinfodialog.h"

#include "sessiondialog.h"

SessionDirModel::SessionDirModel(const QStringList &nameFilters, QDir::Filters filters, QDir::SortFlags sort, QObject *parent)
    : QDirModel(nameFilters, filters, sort, parent)
{
    
}
SessionDirModel::SessionDirModel(QObject *parent)
    : QDirModel(parent)
{
}
SessionDirModel::~SessionDirModel()
{    
}

QVariant SessionDirModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DecorationRole && !this->isDir(index)) {
        return QIcon(":icons/computer.png");
    }
    return QDirModel::data(index, role);
}

SessionDialog::SessionDialog(QWidget * parent)
    :QDialog(parent)
{
    this->ui_win.setupUi(this);

    this->storage = BaseStorage::instance();

    this->ui_win.treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->ui_win.treeView->setExpandsOnDoubleClick(false);
    this->ui_win.treeView->setItemsExpandable(true);
    this->ui_win.treeView->setAnimated(true);
#if QT_VERTION >= 0x0404000
    this->ui_win.treeView->setHeaderHidden(true);
#else
    this->ui_win.treeView->header()->setVisible(false);
#endif

    this->sessPath = BaseStorage::instance()->getSessionPath();

    this->sessTree = new SessionDirModel();
    this->sessTree->setReadOnly(false);
    this->ui_win.treeView->setModel(this->sessTree);
    this->ui_win.treeView->setRootIndex(this->sessTree->index(this->sessPath));
    this->ui_win.treeView->setColumnHidden(1, true);
    this->ui_win.treeView->setColumnHidden(2, true);
    this->ui_win.treeView->setColumnHidden(3, true);

    QObject::connect(this->ui_win.treeView,SIGNAL(customContextMenuRequested(const QPoint&)),
                     this, SLOT(slot_ctx_menu_requested(const QPoint &)));
    QObject::connect(this->ui_win.treeView,SIGNAL(doubleClicked(const QModelIndex&)),
                     this,SLOT(slot_conntect_selected_host(const QModelIndex&)));
    QObject::connect(this->ui_win.toolButton,SIGNAL(clicked()),
                     this,SLOT(slot_conntect_selected_host()));
    QObject::connect(this->ui_win.toolButton_2,SIGNAL(clicked()),
                     this,SLOT(slot_quick_connect()));
    QObject::connect(this->ui_win.toolButton_3,SIGNAL(clicked()),
                     this,SLOT(slot_remove_selected_host()));
    this->host_list_ctx_menu = 0;
    this->info_dlg = 0;
}

SessionDialog::~SessionDialog()
{
}

void SessionDialog::slot_ctx_menu_requested(const QPoint & pos)
{
    if(this->host_list_ctx_menu == 0) {
        this->host_list_ctx_menu = new QMenu(this);
        this->action_connect = new QAction(tr("&Connect to host"),this);
        this->host_list_ctx_menu->addAction(this->action_connect);
        this->action_edit = new QAction(tr("&View host info ..."),this);
        this->host_list_ctx_menu->addAction(this->action_edit);

        this->action_rename = new QAction(tr("&Rename host ..."),this);
        this->host_list_ctx_menu->addAction(this->action_rename);

        this->action_sep = new QAction("",0);
        this->action_sep->setSeparator(true);
        this->host_list_ctx_menu->addAction(this->action_sep);
        
        this->action_remove = new QAction(tr("&Remove host ..."), this);
        this->host_list_ctx_menu->addAction(this->action_remove);

        QObject::connect(this->action_connect,SIGNAL(triggered()),this,SLOT(slot_conntect_selected_host()));
        QObject::connect(this->action_edit,SIGNAL(triggered()),this,SLOT(slot_edit_selected_host()));
        QObject::connect(this->action_rename,SIGNAL(triggered()),this,SLOT(slot_rename_selected_host()));

        QObject::connect(this->action_remove,SIGNAL(triggered()),this,SLOT(slot_remove_selected_host()));
    }
    this->host_list_ctx_menu->popup(this->ui_win.treeView->mapToGlobal(pos));
}

void  SessionDialog::slot_conntect_selected_host(const QModelIndex & index)
{
    if (index.isValid()){
        if (this->sessTree->isDir(index)) {
            if (this->ui_win.treeView->isExpanded(index)) {
                this->ui_win.treeView->collapse(index);
            } else {
                this->ui_win.treeView->expand(index);
            }
        } else {
            QString show_name = index.data().toString();
            QMap<QString,QString> host = this->storage->getHost(show_name);
            QMap<QString,QString> host_new = QMap<QString, QString>(host);
            this->selected_host = host;
            emit this->connect_remote_host_requested(host); 
            this->setVisible(false);
            this->accept();
        }
    } else {
        this->slot_show_no_item_tip();
    }
}
QMap<QString,QString>  SessionDialog::get_host_map()
{
    QMap<QString,QString> host;
    if (this->info_dlg != 0)
        host = ((RemoteHostQuickConnectInfoDialog*)this->info_dlg)->get_host_map();
    else
        host = selected_host;
    return host;
}

void SessionDialog::slot_conntect_selected_host()
{
    QItemSelectionModel * ism = 0;
    QModelIndexList mil ;

    ism = this->ui_win.treeView->selectionModel();
    mil = ism->selectedIndexes();
    //qDebug()<<mil;

    if (mil.count() > 0) {
        //qDebug()<<mil.at(0).data();
        this->slot_conntect_selected_host(mil.at(0));
    } else {
        this->slot_show_no_item_tip();
    }
}

void SessionDialog::slot_edit_selected_host()
{
    QItemSelectionModel * ism = 0;
    QModelIndexList mil;

    ism = this->ui_win.treeView->selectionModel();
    mil = ism->selectedIndexes();
    //qDebug()<<mil;
    if (mil.count() > 0) {
        //qDebug()<<mil.at(0).data();
        QString show_name = mil.at(0).data().toString();
        //qDebug()<<show_name;
        QMap<QString,QString> host = this->storage->getHost(show_name);
        QMap<QString,QString> host_new;
        //qDebug()<<host;
        RemoteHostQuickConnectInfoDialog * info_dlg = new RemoteHostQuickConnectInfoDialog(this);
        info_dlg->set_active_host(host);
        if (info_dlg->exec() == QDialog::Accepted) {
            host_new = info_dlg->get_host_map();
            if (host_new != host) {
                this->storage->updateHost(host_new);
            }
        }
        delete info_dlg;
    } else {
        this->slot_show_no_item_tip();
    }
}

void SessionDialog::slot_rename_selected_host()
{
    QItemSelectionModel * ism = 0;
    QModelIndexList mil;
    bool ok;
    QString focusPath;
    QString newPath;
    QModelIndex pidx;
    QModelIndex cidx;
    QModelIndex nidx;

    ism = this->ui_win.treeView->selectionModel();
    mil = ism->selectedIndexes();
    //qDebug()<<mil;

    if (mil.count() > 0) {
        cidx = mil.at(0);
        pidx = mil.at(0).parent();
        
        QString new_name = 
            QInputDialog::getText(this, tr("Rename host:"), tr("Type the new name:"), 
                                  QLineEdit::Normal, cidx.data().toString(), &ok);
        new_name = new_name.trimmed();

        if(ok && !new_name.isEmpty() && new_name != cidx.data().toString()) {
            if (this->sessTree->isDir(mil.at(0))) {
                ok = this->ui_win.treeView->isExpanded(cidx);
                focusPath = this->sessTree->filePath(cidx);
                newPath = this->sessTree->filePath(pidx) + "/" + new_name;                                
                nidx = this->sessTree->mkdir(pidx, new_name); // 欺骗QDirModel, 产生新目录缓存
                QDir().rmdir(newPath);           // 使用后端方法删除掉这个目录，QDirModel变不知道。
                QDir().rename(focusPath, newPath);     //在把旧目录移动过来,这时QDirModel认为这个目录是它创建的那个
                if (ok) {
                    this->ui_win.treeView->expand(this->sessTree->index(newPath)); // maybe crash?
                }
            } else {
                QMap<QString, QString> host = this->storage->getHost(mil.at(0).data().toString());
                this->storage->updateHost(host, new_name);
            }
            this->sessTree->refresh(pidx);
        }
    } else {
        this->slot_show_no_item_tip();
    }
}

void SessionDialog::slot_remove_selected_host()
{
    QItemSelectionModel *ism = 0;
    QModelIndexList mil;
    QModelIndex cidx;
    QModelIndex pidx;

    ism = this->ui_win.treeView->selectionModel();
    mil = ism->selectedIndexes();
    //qDebug()<<mil;
    if (mil.count() > 0) {
        cidx = mil.at(0);
        pidx = cidx.parent();

        //waning user
        int ret = QMessageBox::question(this, tr("Remote host:"), tr("Are you sure remote it?"), 
                                        QMessageBox::Yes, QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            if (this->sessTree->isDir(cidx)) {
                if (!this->sessTree->rmdir(cidx)) {
                    qDebug()<<"Dir not empty: "<<this->sessTree->filePath(cidx);
                }
            } else {
                this->storage->removeHost(cidx.data().toString());
            }
            this->sessTree->refresh(pidx);
        }
    } else {
        this->slot_show_no_item_tip();
    }
}

void SessionDialog::slot_quick_connect()
{
    this->setVisible(false);
    emit quick_connect();
    this->reject();
}

void SessionDialog::slot_show_no_item_tip()
{
    QString msg = QString("<br>&nbsp;&nbsp;&nbsp;<b>") + tr("No selected host.") 
        + QString("&nbsp;&nbsp;</b><br>");
    // QPoint pos = this->ui_win.toolButton_3->pos();
    // pos.setY(pos.y() + 80);
    // QToolTip::showText(this->mapToGlobal(pos), msg, this);

    QPoint pos = QCursor::pos();
    QWhatsThis::showText(pos, msg);
}

//
// sessiondialog.cpp ends here
