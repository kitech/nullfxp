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
 * Last-Updated: 日  8月 10 11:57:51 2008 (CST)
 *           By: 刘光照<liuguangzhao@users.sf.net>
 *     Update #: 1
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

class SynchronizeWindow;
class SyncWalker : public QThread
{
    Q_OBJECT;
public:
    SyncWalker(QObject *parent = 0);
    ~SyncWalker();
    void run();

private:
    SynchronizeWindow *parent;

signals:
    void file_got(QString file_name, LIBSSH2_SFTP_ATTRIBUTES *attr);
    void status_msg(QString msg);
};

class SynchronizeWindow : public QWidget
{
    Q_OBJECT;
public:
    SynchronizeWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~SynchronizeWindow();

    void set_sync_param(QString local_dir, QString sess_name, QString remote_dir, bool recursive, int way);

    public slots:
    void slot_status_msg(QString msg);
    private slots:
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
    SyncWalker *walker;

    QStringList dirs;
    struct Node{
        QString name;
        QString status;
        QString desc;        
        void *ssh2_attr;
    };
    QHash<QString, QHash<QString, Node> > syncer;

    friend class SyncWalker;
};

#endif
