/* remotehostquickconnectinfodialog.h --- 
 * 
 * Filename: remotehostquickconnectinfodialog.h
 * Description: 
 * Author: 刘光照<liuguangzhao@users.sf.net>
 * Maintainer: 
 * Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 日  6月  8 11:29:15 2008 (CST)
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

#ifndef REMOTEHOSTQUICKCONNECTINFODIALOG_H
#define REMOTEHOSTQUICKCONNECTINFODIALOG_H

#include <QtCore>

#include <QDialog>

#include "ui_remotehostquickconnectfinfodailog.h"

/**
	@author liuguangzhao <gzl@localhost>
*/
class RemoteHostQuickConnectInfoDialog : public QDialog
{
    Q_OBJECT
public:
    RemoteHostQuickConnectInfoDialog(QWidget* parent=0, Qt::WindowFlags f=0);

    ~RemoteHostQuickConnectInfoDialog();
    
    QString get_user_name ();
    QString get_host_name ();
    QString get_password();
    short  get_port();
    QString get_pubkey();
    QMap<QString,QString> get_host_map();
    
    public slots:
        void set_active_host(QMap<QString,QString> host);
    private slots:
        void slot_pubkey_checked(int state);
        void slot_select_pubkey();
    private:
        Ui::RemoteHostQuickConnectInfoDialog quick_connect_info_dialog;
        QString show_name;
        QString pubkey_path;
};

#endif
