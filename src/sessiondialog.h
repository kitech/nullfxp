/* sessiondialog.h --- 
 * 
 * Filename: sessiondialog.h
 * Description: 
 * Author: 刘光照<liuguangzhao@users.sf.net>
 * Maintainer: 
 * Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 三  7月 16 22:06:33 2008 (CST)
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


#include <QtCore>
#include <QtGui>

#include "basestorage.h"
#include "ui_hostlistdialog.h"

class SessionDialog: public QDialog
{
  Q_OBJECT

public:
    SessionDialog(QWidget * parent = 0);
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
    bool loadHost();
    void slot_ctx_menu_requested(const QPoint & pos);
    void slot_show_no_item_tip();

private:
    Ui::HostListDialog sess_dlg;
    BaseStorage * storage;
    QStringListModel * host_list_model;
    QMenu * host_list_ctx_menu;
    QAction * action_connect;
    QAction * action_edit;
    QAction * action_rename;
    QAction * action_remove;
    QAction * action_sep;
    QMap<QString, QString> selected_host;

    void * info_dlg;
};


/* sessiondialog.h ends here */
