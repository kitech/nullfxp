/* encryptiondetaildialog.h --- 
 * 
 * Filename: encryptiondetaildialog.h
 * Description: 
 * Author: liuguangzhao
 * Maintainer: 
 * Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sourceforge.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 一  5月  5 21:26:14 2008 (CST)
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


#ifndef ENCRYPTIONDETAILDIALOG_H
#define ENCRYPTIONDETAILDIALOG_H

#include <QtCore>
#include <QtGui>

#include "ui_encryptiondetaildialog.h"

class EncryptionDetailDialog : public QDialog
{
Q_OBJECT
  public:
  EncryptionDetailDialog(QWidget * parent = 0, Qt::WindowFlags f = 0);
  ~EncryptionDetailDialog();

  public slots:
  void why();
 signals:

 private:
  Ui::EncryptionDetailDialog ui_dlg;
};

#endif

