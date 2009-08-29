// encryptiondetaildialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-05-05 21:26:45 +0800
// Version: $Id$
// 

#include "encryptiondetaildialog.h"

EncryptionDetailDialog::EncryptionDetailDialog(char **server_info, QWidget * parent, Qt::WindowFlags f)
  : QDialog(parent, f)
{
  this->ui_dlg.setupUi(this);
  this->ui_dlg.label_8->setPixmap(QPixmap(":/icons/document-encrypt.png").scaledToHeight(64));

  //设置信息：
  this->ui_dlg.label_6->setText(QString("%1 %2").arg(this->ui_dlg.label_6->text()).arg(server_info[1]));
  this->ui_dlg.label_7->setText(QString("%1 %2").arg(this->ui_dlg.label_7->text()).arg(server_info[2]));
  this->ui_dlg.label_5->setText(QString("%1 %2").arg(this->ui_dlg.label_5->text()).arg(server_info[3]));

  this->ui_dlg.label->setText(QString("%1 %2").arg(this->ui_dlg.label->text()).arg(server_info[4]));
  this->ui_dlg.label_2->setText(QString("%1 %2").arg(this->ui_dlg.label_2->text()).arg(server_info[5]));
  this->ui_dlg.label_4->setText(QString("%1 %2").arg(this->ui_dlg.label_4->text()).arg(server_info[6]));
  this->ui_dlg.label_3->setText(QString("%1 %2").arg(this->ui_dlg.label_3->text()).arg(server_info[7]));

  QList<QAbstractButton *> ok_cancel_btns = this->ui_dlg.buttonBox->buttons();
  this->ui_dlg.buttonBox->removeButton(ok_cancel_btns.at(1));
}

EncryptionDetailDialog::~EncryptionDetailDialog()
{

}

void EncryptionDetailDialog::why()
{

}
