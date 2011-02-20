// sessiondialog.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-07-16 22:06:33 +0000
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>

#include "basestorage.h"

namespace Ui {
    class HostListDialog;
};

class SessionDirModel : public QDirModel
{
    Q_OBJECT;
public:
    SessionDirModel(const QStringList & nameFilters, QDir::Filters filters,
                    QDir::SortFlags sort, QObject * parent = 0);
    SessionDirModel(QObject *parent = 0 );
    virtual ~SessionDirModel();

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
};

class SessionDialog: public QDialog
{
    Q_OBJECT;
public:
    SessionDialog(QWidget *parent = 0);
    virtual ~SessionDialog();

public slots:
    void slot_conntect_selected_host();
    void slot_conntect_selected_host(const QModelIndex & index);
    void slot_edit_selected_host();
    void slot_rename_selected_host();
    void slot_remove_selected_host();
    void slot_quick_connect();
    void slot_cut_selected();
    void slot_copy_selected();
    void slot_paste_selected();
    void slot_new_folder();

    QMap<QString, QString>  get_host_map();

signals:
    void connect_remote_host_requested(QMap<QString, QString> host);
    void quick_connect();

private slots:
    void slot_ctx_menu_requested(const QPoint &pos);
    void slot_show_no_item_tip();
    void slot_item_clicked(const QModelIndex &index);

private:
    Ui::HostListDialog *uiw;
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

    enum {OP_COPY=1, OP_CUT=2};
    int optype; // OP_COPY, OP_CUT
    QString opLeftFile;
    // QModelIndex oppidx;
    // QModelIndex opidx;
};


/* sessiondialog.h ends here */
