// synchronizeoptiondialog.cpp --- 
// 
// Filename: synchronizeoptiondialog.cpp
// Description: 
// Author: 刘光照<liuguangzhao@users.sf.net>
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 五  8月  8 13:45:05 2008 (CST)
// Version: 
// Last-Updated: 
//           By: 
//     Update #: 0
// URL: 
// Keywords: 
// Compatibility: 
// 
// 

// Commentary: 
// 
// 
// 
// 

// Change log:
// 
// 
// 

#include "basestorage.h"
#include "synchronizewindow.h"
#include "synchronizeoptiondialog.h"

SynchronizeOptionDialog::SynchronizeOptionDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    this->ui_dlg.setupUi(this);

    QObject::connect(this->ui_dlg.toolButton, SIGNAL(clicked()),
                     this, SLOT(slot_select_local_base_directory()));

    QObject::connect(this->ui_dlg.toolButton_2, SIGNAL(clicked()),
                     this, SLOT(slot_show_session_list()));

    QObject::connect(this->ui_dlg.buttonBox, SIGNAL(accepted()),
                     this, SLOT(slot_option_accepted()));
}

SynchronizeOptionDialog::~SynchronizeOptionDialog()
{

}

void SynchronizeOptionDialog::slot_select_local_base_directory()
{
    QString dir;

    dir = QFileDialog::getExistingDirectory(this, tr("Select directory"), ".");
    if(!dir.isEmpty()) {
        this->ui_dlg.lineEdit->setText(dir);
    }
}

void SynchronizeOptionDialog::slot_show_session_list()
{
    QMenu * popmenu = new QMenu(this);
    QAction *action;

    BaseStorage * storage = BaseStorage::instance();
    storage->open();
    
    QStringList nlist = storage->getNameList();

    for(int i = 0; i< nlist.count(); i++) {
        action = new QAction(nlist.at(i), this);
        QObject::connect(action, SIGNAL(triggered()), 
                         this, SLOT(slot_session_item_selected()));
        popmenu->addAction(action);        
    }

    QPoint pos = this->ui_dlg.toolButton_2->pos();
    pos = this->mapToGlobal(pos);
    pos.setX(pos.x() + 35);
    popmenu->popup(pos);
}

void SynchronizeOptionDialog::slot_session_item_selected()
{
    QAction *a = static_cast<QAction *>(sender());
    this->ui_dlg.lineEdit_2->setText(a->text());
}

void SynchronizeOptionDialog::slot_option_accepted()
{
    QString str;
    str = this->ui_dlg.lineEdit->text().trimmed();
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
    str = this->ui_dlg.lineEdit_2->text().trimmed();
    if(str.isEmpty()) {
        QMessageBox::warning(this, tr("Invalide parameter"), tr("You must select a session"));        
        this->show();
        return;
    }
    str = this->ui_dlg.lineEdit_3->text().trimmed();
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
    syncwin->set_sync_param(this->ui_dlg.lineEdit->text().trimmed(),
                            this->ui_dlg.lineEdit_2->text().trimmed(),
                            this->ui_dlg.lineEdit_3->text().trimmed(),
                            this->ui_dlg.checkBox->isChecked(),
                            this->ui_dlg.comboBox->currentIndex());
    syncwin->show();
    
}

