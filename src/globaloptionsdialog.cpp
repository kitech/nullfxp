// globaloptionsdialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-08-29 16:25:43 +0800
// Version: $Id$
// 

#include "utils.h"

#include "ui_globaloptionsdialog.h"
#include "globaloptionsdialog.h"

GlobalOptionsDialog::GlobalOptionsDialog(QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f),
      uiw(new Ui::GlobalOptionsDialog())
{
    this->uiw->setupUi(this);

    this->uiw->listWidget->setIconSize(QSize(50,50));

    QObject::connect(this->uiw->pushButton_2, SIGNAL(clicked()),
                     this, SLOT(slotCancelEdit()));
    QObject::connect(this->uiw->listWidget, SIGNAL(itemClicked(QListWidgetItem*)),
                     this, SLOT(slotCatItemClicked(QListWidgetItem*)));


    QListWidgetItem *item = NULL;
    for (int i = this->uiw->listWidget->count() - 1; i >= 0; --i) {
        item = this->uiw->listWidget->item(i);
        item->setSizeHint(QSize(120, 50));
    }

    this->slotCatItemClicked(this->uiw->listWidget->item(0));
}

GlobalOptionsDialog::~GlobalOptionsDialog()
{
}

void GlobalOptionsDialog::slotCancelEdit()
{
    this->reject();
}

void GlobalOptionsDialog::slotCatItemClicked(QListWidgetItem *item)
{
    q_debug()<<item<<this->uiw->listWidget->row(item);
    QString catName = item->text();
    
    this->uiw->label_2->setText(catName);
    this->uiw->stackedWidget->setCurrentIndex(this->uiw->listWidget->row(item));
}





