/* synchronizeoptiondialog.h --- 
 * 
 * Filename: synchronizeoptiondialog.h
 * Description: 
 * Author: 刘光照<liuguangzhao@users.sf.net>
 * Maintainer: 
 * Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 五  8月  8 13:45:15 2008 (CST)
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
#ifndef SYNCHRONIZEOPTIONDIALOG_H
#define SYNCHRONIZEOPTIONDIALOG_H


#include <QtGlobal>
#include <QtCore>
#include <QtGui>


#include "ui_synchronizeoptiondialog.h"

class SynchronizeOptionDialog : public QDialog
{
    Q_OBJECT;
public:
    SynchronizeOptionDialog(QWidget *parent = 0,Qt::WindowFlags flags = 0);
    ~SynchronizeOptionDialog();

private:
    Ui::SynchronizeOptionDialog ui_dlg;

private slots:
    void slot_select_local_base_directory();
    void slot_show_session_list();
    void slot_session_item_selected();
    void slot_option_accepted();
};

#endif
