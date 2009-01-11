/* synctransferthread.h --- 
 * 
 * Filename: synctransferthread.h
 * Description: 
 * Author: 刘光照<liuguangzhao@users.sf.net>
 * Maintainer: 
 * Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 日  1月 11 10:19:18 2009 (CST)
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
#ifndef SYNC_TRANSFER_THREAD_H
#define SYNC_TRANSFER_THREAD_H

#include <QtCore>

#include "libssh2.h"
#include "libssh2_sftp.h"

class SyncTransferThread : public QThread
{
    Q_OBJECT;
public:
    SyncTransferThread(QObject *parent = 0);
    ~SyncTransferThread();

    void run();
    
    // must call before run
    bool setRemoteSession(QString sess);
    bool addTask(QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> task);
    bool connectToRemoteHost();
    
    enum {TASK_UPLOAD = 1, TASK_DOWNLOAD=2};
    enum {TASK_OK = 0, TASK_ERROR_CONN_FAILD=1, TASK_ERROR_OTHER=2};

private:
    int transfer_impl(QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> task);
    
private:
    QQueue<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > taskQueue;
    QWaitCondition wc;
    QMutex   wcMutex;
    QMutex   tMutex; // task modify mutex

    LIBSSH2_SESSION *ssh2_sess ;
    LIBSSH2_SFTP *ssh2_sftp ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_attr;
    LIBSSH2_SFTP_HANDLE *hsftp ;

    QMap<QString, QString> remoteHost;
    QString sess_name;
};


#endif


