// fileexistaskdialog.cpp --- 
// 
// Filename: fileexistaskdialog.cpp
// Description: 
// Author: liuguangzhao
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sourceforge.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 二  5月  6 21:59:50 2008 (CST)
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


#include "fileexistaskdialog.h"

FileExistAskDialog::FileExistAskDialog(QWidget *parent)
  :QDialog(parent)
{
  this->ui_dlg.setupUi(this);
}

FileExistAskDialog::~FileExistAskDialog()
{

}

void FileExistAskDialog::set_files(QString src_path, QString src_file_size, QString src_file_date, QString dest_path, QString dest_file_size, QString dest_file_date)
{
  QString fmt_prefix = QString("        ");
  this->ui_dlg.label_4->setText(fmt_prefix+src_path);
  this->ui_dlg.label_5->setText(fmt_prefix+src_file_size);
  this->ui_dlg.label_6->setText(fmt_prefix+src_file_date);

  this->ui_dlg.label_8->setText(fmt_prefix+dest_path);
  this->ui_dlg.label_9->setText(fmt_prefix+dest_file_size);
  this->ui_dlg.label_10->setText(fmt_prefix+dest_file_date);
  
}
  
