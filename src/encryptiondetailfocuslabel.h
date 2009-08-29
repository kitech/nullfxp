// encryptiondetailfocuslabel.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-05-05 21:23:13 +0800
// Version: $Id$
// 

#ifndef ENCRYPTIONDETAILFOCUSLABEL_H
#define ENCRYPTIONDETAILFOCUSLABEL_H


#include <QtCore>
#include <QtGui>

class EncryptionDetailFocusLabel : public QLabel
{
Q_OBJECT
  public:
  EncryptionDetailFocusLabel ( const QString & text, QWidget * parent = 0, Qt::WindowFlags f = 0 );
  ~EncryptionDetailFocusLabel();

  signals:
  void mouseDoubleClick();
  protected:
  virtual void mouseDoubleClickEvent ( QMouseEvent * event );
};


class HostInfoDetailFocusLabel : public QLabel
{
Q_OBJECT
  public:
  HostInfoDetailFocusLabel ( const QString & text, QWidget * parent = 0, Qt::WindowFlags f = 0 );
  ~HostInfoDetailFocusLabel();

  signals:
  void mouseDoubleClick();
  protected:
  virtual void mouseDoubleClickEvent ( QMouseEvent * event );
};

#endif
