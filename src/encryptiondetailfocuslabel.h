/* encryptiondetailfocuslabel.h --- 
 * 
 * Filename: encryptiondetailfocuslabel.h
 * Description: 
 * Author: liuguangzhao
 * Maintainer: 
 * Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sourceforge.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 一  5月  5 21:23:13 2008 (CST)
 * Version: 
 * Last-Updated: 
 *           By: 
 *     Update #: 0
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

#endif
