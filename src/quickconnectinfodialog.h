// quickconnectinfodialog.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-06-08 11:29:15 +0800
// Version: $Id: quickconnectinfodialog.h 475 2009-09-05 14:58:13Z liuguangzhao $
// 

#ifndef QUICKCONNECTINFODIALOG_H
#define QUICKCONNECTINFODIALOG_H

#include <QtCore>

#include <QDialog>


namespace Ui {
    class QuickConnectInfoDialog;
};

/**
 *
 */
class QuickConnectInfoDialog : public QDialog
{
    Q_OBJECT;
public:
    explicit QuickConnectInfoDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~QuickConnectInfoDialog();

    QString get_protocol();
    QString get_user_name();
    QString get_host_name();
    QString get_password();
    short  get_port();
    QString get_pubkey();
    QMap<QString,QString> get_host_map();
    
public slots:
    void set_active_host(QMap<QString,QString> host);

private slots:
    void slot_pubkey_checked(int state);
    void slot_select_pubkey();
    void slot_protocol_changed(int index);

private:
    Ui::QuickConnectInfoDialog *uiw;
    QString show_name;
    QString pubkey_path;
};

#endif
