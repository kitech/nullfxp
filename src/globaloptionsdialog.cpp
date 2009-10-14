// globaloptionsdialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-08-29 16:25:43 +0800
// Version: $Id$
// 

#include "utils.h"

#include "globaloptionsdialog.h"

GlobalOptionsDialog::GlobalOptionsDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f)
{
    this->ui_win.setupUi(this);

    this->ui_win.listWidget->setIconSize(QSize(50,50));

    QObject::connect(this->ui_win.pushButton_2, SIGNAL(clicked()),
                     this, SLOT(slotCancelEdit()));
    QObject::connect(this->ui_win.listWidget, SIGNAL(itemClicked(QListWidgetItem*)),
                     this, SLOT(slotCatItemClicked(QListWidgetItem*)));

    this->slotCatItemClicked(this->ui_win.listWidget->item(0));
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
    q_debug()<<item<<this->ui_win.listWidget->row(item);
    QString catName = item->text();
    
    this->ui_win.label_2->setText(catName);
    this->ui_win.stackedWidget->setCurrentIndex(this->ui_win.listWidget->row(item));
}





