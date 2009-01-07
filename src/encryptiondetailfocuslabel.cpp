// encryptiondetailfocuslabel.cpp --- 
// 
// Filename: encryptiondetailfocuslabel.cpp
// Description: 
// Author: liuguangzhao
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 一  5月  5 21:22:01 2008 (CST)
// Version: 
// Last-Updated: 三  1月  7 14:19:25 2009 (+0000)
//           By: <liuguangzhao@users.sf.net>
//     Update #: 3
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


//////////////////
HostInfoDetailFocusLabel::HostInfoDetailFocusLabel ( const QString & text, QWidget * parent , Qt::WindowFlags f  )
  :QLabel(text, parent, f)
{
  this->setPixmap(QPixmap(":/icons/computer.png").scaledToHeight(20));
  this->setToolTip(tr("Show SSH host info."));
}
HostInfoDetailFocusLabel::~HostInfoDetailFocusLabel()
{
}

void HostInfoDetailFocusLabel::mouseDoubleClickEvent ( QMouseEvent * event )
{
  emit this->mouseDoubleClick();
  QLabel::mouseDoubleClickEvent(event);
}
