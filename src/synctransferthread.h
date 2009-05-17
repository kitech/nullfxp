/* synctransferthread.h --- 
 * 
 * Author: liuguangzhao
 * Copyright (C) 2007-2010 liuguangzhao@users.sf.net
 * URL: http://www.qtchina.net http://nullget.sourceforge.net
 * Created: 2009-01-11 10:19:18 +0800
 * Last-Updated: 2009-05-17 15:04:25 +0800
 * Version: $Id$
 */

#ifndef SYNC_TRANSFER_THREAD_H
#define SYNC_TRANSFER_THREAD_H

#include <QtCore>

#include "libssh2.h"
#include "libssh2_sftp.h"

class SyncTaskPackage
{
 public:
    SyncTaskPackage(QString fileName, LIBSSH2_SFTP_ATTRIBUTES * pattr, int direct) {
        this->msFileName = fileName;
        this->mpAttr = pattr;
        this->mnDirect = direct;
    }
    // 拷贝构造函数
    SyncTaskPackage(const SyncTaskPackage &pkg) {
        *this = pkg;
    }
    // 赋值操作符重载
    SyncTaskPackage & operator=(const SyncTaskPackage &pkg) {
        this->msFileName = pkg.msFileName;
        this->mpAttr = pkg.mpAttr;
        this->mnDirect = pkg.mnDirect;

        return *this;
    }
    SyncTaskPackage(){}
    ~SyncTaskPackage(){}
    
    QString msFileName;
    LIBSSH2_SFTP_ATTRIBUTES *mpAttr;
    int mnDirect;
};

class SyncTransferThread : public QThread
{
    Q_OBJECT;
public:
    SyncTransferThread(QObject *parent = 0);
    ~SyncTransferThread();

    void run();
    
    // must call before run
    bool setRemoteSession(QString sess);
    bool setBasePath(QString local, QString remote);
    
    enum {TASK_UPLOAD = 1, TASK_DOWNLOAD=2, TASK_BOTH=3, TASK_UPLOAD_LOCAL_ONLY, TASK_DOWNLOAD_REMOTE_ONLY};
    enum {TASK_OK = 0, TASK_ERROR_CONN_FAILD=1, TASK_ERROR_OTHER=2};

public slots:
    void slot_syncDownload(QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> task);
    void slot_syncUpload(QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> task);
    
private:
    bool connectToRemoteHost();
    bool addTask(SyncTaskPackage task);

    int transfer_impl(SyncTaskPackage task);
    int do_download(QString remote_path, QString local_path);
    int do_upload(QString local_path, QString remote_path);
    int do_touch_local_file_with_time(QString fileName, QDateTime time);
    int do_touch_sftp_file_with_time(QString fileName, QDateTime time);

private:
    QQueue<SyncTaskPackage> taskQueue;
    QWaitCondition wc;
    QMutex   wcMutex;
    QMutex   tMutex; // task modify mutex

    LIBSSH2_SESSION *ssh2_sess ;
    LIBSSH2_SFTP *ssh2_sftp ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_attr;
    LIBSSH2_SFTP_HANDLE *hsftp ;

    QMap<QString, QString> remoteHost;
    QString sess_name;
    QString local_base_path;
    QString remote_base_path;

signals:
    void syncFileStarted(QString fileName, quint64 fileSize);
    void syncFileStopped(QString fileName, int status);
    void transfer_percent_changed(QString fileName, int percent, quint64 transferLength, quint64 lastBlockLength);
    void syncTaskFinished(int status);
};


#endif


