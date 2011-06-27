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

void SynchronizeOptionDialog::slot_show_session_list()
{
    q_debug()<<"";
    QMenu * popmenu = new QMenu(this);
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

