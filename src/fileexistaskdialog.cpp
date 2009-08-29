// fileexistaskdialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-05-05 21:59:50 +0800
// Version: $Id$
// 

#include "transferthread.h"
#include "fileexistaskdialog.h"

FileExistAskDialog::FileExistAskDialog(QWidget *parent)
  :QDialog(parent)
{
  this->ui_dlg.setupUi(this);

  QObject::connect(this->ui_dlg.pushButton, SIGNAL(clicked()), this, SLOT(slot_reponse_button_clicked()));
  QObject::connect(this->ui_dlg.pushButton_2, SIGNAL(clicked()), this, SLOT(slot_reponse_button_clicked()));
  QObject::connect(this->ui_dlg.pushButton_3, SIGNAL(clicked()), this, SLOT(slot_reponse_button_clicked()));
  QObject::connect(this->ui_dlg.pushButton_4, SIGNAL(clicked()), this, SLOT(slot_reponse_button_clicked()));
  QObject::connect(this->ui_dlg.pushButton_5, SIGNAL(clicked()), this, SLOT(slot_reponse_button_clicked()));
  QObject::connect(this->ui_dlg.pushButton_6, SIGNAL(clicked()), this, SLOT(slot_reponse_button_clicked()));
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
  
void FileExistAskDialog::slot_reponse_button_clicked()
{
  QWidget * sender_widget = (QWidget*)sender();
  if(sender_widget == this->ui_dlg.pushButton){
    emit this->acceptedOne(TransferThread::OW_YES);
  }else if(sender_widget == this->ui_dlg.pushButton_2){
    emit this->acceptedOne(TransferThread::OW_YES_ALL); 
  }else if(sender_widget == this->ui_dlg.pushButton_3){
    emit this->acceptedOne(TransferThread::OW_RESUME); 
  }else if(sender_widget == this->ui_dlg.pushButton_4){
    emit this->acceptedOne(TransferThread::OW_NO);
  }else if(sender_widget == this->ui_dlg.pushButton_5){
    emit    this->acceptedOne(TransferThread::OW_NO_ALL);
  }else if(sender_widget == this->ui_dlg.pushButton_6){
    emit this->acceptedOne(TransferThread::OW_CANCEL);
  }else{
    emit this->acceptedOne(TransferThread::OW_CANCEL);
  }
  this->reject();
}

void FileExistAskDialog::close()
{
  emit  this->acceptedOne(TransferThread::OW_CANCEL);
  this->reject();
}
