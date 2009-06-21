// sessiondialog.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-07-16 22:06:33 +0000
// Last-Updated: 2009-06-21 11:35:09 +0000
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>

#include "basestorage.h"
#include "ui_hostlistdialog.h"

class SessionDirModel : public QDirModel
{
    Q_OBJECT;
public:
    SessionDirModel(const QStringList & nameFilters, QDir::Filters filters, QDir::SortFlags sort, QObject * parent = 0 );
    SessionDirModel(QObject *parent = 0 );
    ~SessionDirModel();

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
};

class SessionDialog: public QDialog
{
    Q_OBJECT;
public:
    SessionDialog(QWidget *parent = 0);
    ~SessionDialog();

public slots:
    void slot_conntect_selected_host();
    void slot_conntect_selected_host(const QModelIndex & index);
    void slot_edit_selected_host();
    void slot_rename_selected_host();
    void slot_remove_selected_host();
    void slot_quick_connect();

    QMap<QString,QString>  get_host_map();

signals:
    void connect_remote_host_requested(QMap<QString,QString> host);
    void quick_connect();

private slots:
    void slot_ctx_menu_requested(const QPoint & pos);
    void slot_show_no_item_tip();

private:
    Ui::HostListDialog ui_win;
    BaseStorage *storage;
    // QStringListModel *host_list_model;
    QMenu *host_list_ctx_menu;
    QAction *action_connect;
    QAction *action_edit;
    QAction *action_rename;
    QAction *action_remove;
    QAction *action_sep;
    QMap<QString, QString> selected_host;

    void *info_dlg;

    QString sessPath;
    SessionDirModel *sessTree;
};


/* sessiondialog.h ends here */
