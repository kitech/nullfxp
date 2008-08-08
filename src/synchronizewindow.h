/* synchronizewindow.h --- 
 * 
 * Filename: synchronizewindow.h
 * Description: 
 * Author: 刘光照<liuguangzhao@users.sf.net>
 * Maintainer: 
 * Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 五  8月  8 13:44:51 2008 (CST)
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
#ifndef SYNCHRONIZEWINDOW_H
#define SYNCHRONIZEWINDOW_H

#include <QtGlobal>
#include <QtCore>
#include <QtGui>

#include "libssh2.h"
#include "libssh2_sftp.h"

#include "ui_synchronizewindow.h"

class SyncWalker : public QThread
{
    Q_OBJECT;
public:
    SyncWalker(QObject *parent = 0);
    ~SyncWalker();
    void run();

signals:
    void file_got(QString file_name, LIBSSH2_SFTP_ATTRIBUTES *attr);
};

class SynchronizeWindow : public QWidget
{
    Q_OBJECT;
public:
    SynchronizeWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~SynchronizeWindow();

    void set_sync_param(QString local_dir, QString sess_name, QString remote_dir, bool recursive, int way);
    void start();
    void stop();

private:
    Ui::SynchronizeWindow  ui_win;
    QString local_dir;
    QString sess_name;
    QString remote_dir;
    bool recursive;
    int way;
    QTimer progress_timer;
    bool running;
};

#endif
