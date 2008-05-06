/* fileexistaskdialog.h --- 
 * 
 * Filename: fileexistaskdialog.h
 * Description: 
 * Author: liuguangzhao
 * Maintainer: 
 * Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sourceforge.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 二  5月  6 22:00:02 2008 (CST)
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


#ifndef FILEEXISTASKDIALOG_H
#define FILEEXISTASKDIALOG_H

#include <QtCore>
#include <QtGui>

#include "ui_fileexistaskdialog.h"

class FileExistAskDialog:public QDialog
{
Q_OBJECT
  public:
  FileExistAskDialog(QWidget *parent);
  ~FileExistAskDialog();

  void set_files(QString src_path, QString src_file_size, QString src_file_date, QString dest_path, QString dest_file_size, QString dest_file_date);
  private:
  Ui::FileExistAskDialog ui_dlg;
};

#endif

