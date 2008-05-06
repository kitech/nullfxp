// encryptiondetailfocuslabel.cpp --- 
// 
// Filename: encryptiondetailfocuslabel.cpp
// Description: 
// Author: liuguangzhao
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sourceforge.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 一  5月  5 21:22:01 2008 (CST)
// Version: 
// Last-Updated: 二  5月  6 09:16:05 2008 (UTC)
//           By: 刘光照<liuguangzhao@comsenz.com>
//     Update #: 1
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

#include "encryptiondetailfocuslabel.h"


EncryptionDetailFocusLabel::EncryptionDetailFocusLabel ( const QString & text, QWidget * parent , Qt::WindowFlags f  )
  :QLabel(text, parent, f)
{
  this->setPixmap(QPixmap(":/icons/document-encrypt.png").scaledToHeight(20));
  this->setToolTip(tr("Show encryption detail."));
}
EncryptionDetailFocusLabel::~EncryptionDetailFocusLabel()
{

}

void EncryptionDetailFocusLabel::mouseDoubleClickEvent ( QMouseEvent * event )
{
  emit this->mouseDoubleClick();
  QLabel::mouseDoubleClickEvent(event);
}

