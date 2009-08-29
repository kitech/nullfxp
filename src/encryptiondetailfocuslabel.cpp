// encryptiondetailfocuslabel.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-05-05 21:22:01 +0800
// Version: $Id$
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
