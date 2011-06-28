// synchronizeoptiondialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-08-08 13:45:05 +0000
// Version: $Id$
// 

#include "utils.h"
#include "basestorage.h"
#include "synchronizewindow.h"
#include "ui_synchronizeoptiondialog.h"
#include "synchronizeoptiondialog.h"

/*
  sync type:
    Upload only - List those files that may need to be transferred from the local machine to the remote machine.

    Download only - List those files that may need to be transferred from the remote machine to the local machine.

    Mirror-both - List both the Upload and Download files. If you choose this option, Quick Synchronize will transfer files in both directions. This takes place when a file has been removed from one side or if a file has a different timestamp.

    Mirror-local - List both the Upload files and files that will be deleted with this action. If you choose this option, Quick Synchronize will modify the remote directory to look exactly like the local directory. If the same file exists in both directories, the remote file will be overwritten even if it is newer. Remote files that do not exist in the local directory will be deleted. The local directory will not be changed.

    Mirror-remote - List both the Download files and files that will be deleted with this action. If you choose this option, Quick Synchronize will modify the local directory to look exactly like the remote directory. If the same file exists in both directories, the local file will be overwritten even if it is newer. Local files that do not exist in the remote directory will be deleted. The remote directory will not be changed.
 */

SynchronizeOptionDialog::SynchronizeOptionDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , uiw(new Ui::SynchronizeOptionDialog())
{
    this->uiw->setupUi(this);

    QObject::connect(this->uiw->toolButton, SIGNAL(clicked()),
                     this, SLOT(slot_select_local_base_directory()));

    QObject::connect(this->uiw->toolButton_2, SIGNAL(clicked()),
                     this, SLOT(slot_show_session_list()));

    QObject::connect(this->uiw->buttonBox, SIGNAL(accepted()),
                     this, SLOT(slot_option_accepted()));
}

SynchronizeOptionDialog::~SynchronizeOptionDialog()
{
    q_debug()<<"destructured";
}

void SynchronizeOptionDialog::slot_select_local_base_directory()
{
    QString dir;

    dir = QFileDialog::getExistingDirectory(this, tr("Select directory"), ".");
    if(!dir.isEmpty()) {
        this->uiw->lineEdit->setText(dir);
    }
}

// TODO session edit auto complete when input
void SynchronizeOptionDialog::slot_show_session_list()
{
    q_debug()<<"";
    QMenu *popmenu = new QMenu(this);
    QObject::connect(popmenu, SIGNAL(aboutToHide()), this, SLOT(slot_session_list_menu_hide()));

    QAction *action;

    BaseStorage * storage = BaseStorage::instance();
    
    QStringList nlist = storage->getNameList();

    for(int i = 0; i< nlist.count(); i++) {
        action = new QAction(nlist.at(i), this);
        QObject::connect(action, SIGNAL(triggered()), 
                         this, SLOT(slot_session_item_selected()));
        popmenu->addAction(action);        
    }

    QPoint pos = this->uiw->toolButton_2->pos();
    pos = this->mapToGlobal(pos);
    pos.setX(pos.x() + 35);
    popmenu->popup(pos);
}

void SynchronizeOptionDialog::slot_session_item_selected()
{
    QAction *a = static_cast<QAction *>(sender());
    this->uiw->lineEdit_2->setText(a->text());
}

void SynchronizeOptionDialog::slot_option_accepted()
{
    QString str;
    str = this->uiw->lineEdit->text().trimmed();
    if(str.isEmpty()) {
        //
        QMessageBox::warning(this, tr("Invalide parameter"), tr("You must input local directory"));
        this->show();
        return;
    }else{
        if(!QFile::exists(str)) {
            QMessageBox::warning(this, tr("Invalide parameter"), tr("You selected local directory not exists"));
            this->show();
            return;
        }
    }
    str = this->uiw->lineEdit_2->text().trimmed();
    if(str.isEmpty()) {
        QMessageBox::warning(this, tr("Invalide parameter"), tr("You must select a session"));        
        this->show();
        return;
    }
    str = this->uiw->lineEdit_3->text().trimmed();
    if(str.isEmpty()) {
        QMessageBox::warning(this, tr("Invalide parameter"), tr("You must input remote direcotry"));
        this->show();
        return;
    }else{
        if(str.at(0) != '/') {
            QMessageBox::warning(this, tr("Invalide parameter"), tr("You selected remote directory not exists"));
            this->show();
            return;
        }
    }

    SynchronizeWindow *syncwin = new SynchronizeWindow((QWidget*)this->parent(), Qt::Dialog);
    syncwin->set_sync_param(this->uiw->lineEdit->text().trimmed(),
                            this->uiw->lineEdit_2->text().trimmed(),
                            this->uiw->lineEdit_3->text().trimmed(),
                            this->uiw->checkBox->isChecked(),
                            this->uiw->comboBox->currentIndex());
    syncwin->show();
    this->deleteLater();
}

// OK
void SynchronizeOptionDialog::slot_session_list_menu_hide()
{
    // q_debug()<<sender();

    sender()->deleteLater();
}
