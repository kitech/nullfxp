// encryptiondetaildialog.cpp --- 
// 
// Filename: encryptiondetaildialog.cpp
// Description: 
// Author: liuguangzhao
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sourceforge.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 一  5月  5 21:26:45 2008 (CST)
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


#include "encryptiondetaildialog.h"

EncryptionDetailDialog::EncryptionDetailDialog(QWidget * parent, Qt::WindowFlags f)
  : QDialog(parent, f)
{
  this->ui_dlg.setupUi(this);
  this->ui_dlg.label_8->setPixmap(QPixmap(":/icons/document-encrypt.png").scaledToHeight(64));
}

EncryptionDetailDialog::~EncryptionDetailDialog()
{

}

void EncryptionDetailDialog::why()
{

}
