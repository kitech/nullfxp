/* nullfxp.h --- 
 * 
 * Author: liuguangzhao
 * Copyright (C) 2007-2010 liuguangzhao@users.sf.net
 * URL: http://www.qtchina.net http://nullget.sourceforge.net
 * Created: 2008-05-06 22:02:06 +0800
 * Version: $Id$
 */

#ifndef NULLFXP_H
#define NULLFXP_H

#include <QObject>
#include <QtCore>
#include <QtGui>
#include <QMainWindow>
#include <QMessageBox>
#include <QTreeWidget>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QTime>
#include <QDate>
#include <QDateTime>
#include <QImage>
#include <QPixmap>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QPalette>

class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class MdiChild;
class QSignalMapper;

#include "taskpackage.h"
#include "aboutnullfxp.h" 
#include "ui_nullfxp.h"

class LocalView;
class RemoteView;
class RemoteHostConnectingStatusDialog;
class RemoteHostQuickConnectInfoDialog;
// class RemoteHostConnectThread ;
class ForwardConnectDaemon;
class Connection;
class Connector;

/**
	@author liuguangzhao <gzl@localhost>
*/
class NullFXP : public QMainWindow
{
    Q_OBJECT;
public:
    NullFXP(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~NullFXP();
    
public slots:
    void slot_about_nullfxp();
        
    void connect_to_remote_host();
    void connect_to_remote_host(QMap<QString, QString> host);
    void connect_to_remote_host2(QMap<QString, QString> host);
    void slot_disconnect_from_remote_host();
    void slot_cancel_connect();

    void slot_connect_remote_host_finished(int status, Connection *conn);
        
    void slot_new_upload_requested(TaskPackage local_pkg);
        
    void slot_show_local_view(bool);
    void slot_show_remote_view(bool);
    void slot_cascade_sub_windows(bool triggered);
    void slot_tile_sub_windows(bool triggered);
    void slot_show_transfer_queue(bool show);
    void slot_show_fxp_command_log(bool show);
        
private slots:
    void slot_forward_connect(bool show);
    void slot_synchronize_file();
	void slot_show_session_dialog();
    void slot_check_for_updates();

private:
    MdiChild *activeMdiChild();        
    QMdiArea *mdiArea;
    QSignalMapper *windowMapper;                
    RemoteView *get_top_most_remote_view();
        
private:
    QSplitter *central_splitter_widget;
    QListView *transfer_queue_list_view;
                
    LocalView *localView;
        
    Ui::MainWindow mUIMain;
    AboutNullFXP  *about_nullfxp_dialog;

    RemoteHostConnectingStatusDialog *connect_status_dailog;
    RemoteHostQuickConnectInfoDialog *quick_connect_info_dailog;

    Connector *connector;

    ForwardConnectDaemon *fcd;
};

#endif
