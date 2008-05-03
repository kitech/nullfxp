
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

  private:
  Ui::FileExistAskDialog ui_dlg;
};

#endif

