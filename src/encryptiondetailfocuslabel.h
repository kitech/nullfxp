/* encryptiondetailfocuslabel.h --- 
 * 
 * Filename: encryptiondetailfocuslabel.h
 * Description: 
 * Author: liuguangzhao
 * Maintainer: 
 * Copyright (C) 2007-2010 liuguangzhao <liuguangzhao@users.sf.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 一  5月  5 21:23:13 2008 (CST)
 * Version: 
 * Last-Updated: 三  1月  7 14:19:47 2009 (+0000)
 *           By: <liuguangzhao@users.sf.net>
 *     Update #: 1
 * URL: 
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change log:
 * 
 * 
 */

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
