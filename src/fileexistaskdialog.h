// fileexistaskdialog.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-08-06 22:00:02 +0800
// Version: $Id$
// 

#ifndef FILEEXISTASKDIALOG_H
#define FILEEXISTASKDIALOG_H

#include <QtCore>
#include <QtGui>

namespace Ui {
    class FileExistAskDialog;
};

class FileExistAskDialog: public QDialog
{
    Q_OBJECT;
public:
    FileExistAskDialog(QWidget *parent);
    virtual ~FileExistAskDialog();

    void set_files(QString src_path, QString src_file_size, QString src_file_date,
                   QString dest_path, QString dest_file_size, QString dest_file_date);

signals:
    void acceptedOne(int which);

protected:
    void close();

private slots:
    void slot_reponse_button_clicked();

private:
    Ui::FileExistAskDialog *uiw;
};

#endif

