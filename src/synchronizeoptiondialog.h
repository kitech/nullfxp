// synchronizeoptiondialog.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-08-08 13:45:15 +0000
// Version: $Id$
// 

#ifndef SYNCHRONIZEOPTIONDIALOG_H
#define SYNCHRONIZEOPTIONDIALOG_H


#include <QtGlobal>
#include <QtCore>
#include <QtGui>


// #include "ui_synchronizeoptiondialog.h"
namespace Ui {
    class SynchronizeOptionDialog;
};

class SynchronizeOptionDialog : public QDialog
{
    Q_OBJECT;
public:
    SynchronizeOptionDialog(QWidget *parent = 0,Qt::WindowFlags flags = 0);
    virtual ~SynchronizeOptionDialog();

private:
    Ui::SynchronizeOptionDialog *uiw;

private slots:
    void slot_select_local_base_directory();
    void slot_show_session_list();
    void slot_session_item_selected();
    void slot_option_accepted();
    void slot_session_list_menu_hide();
};

#endif
