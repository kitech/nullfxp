// synctransferthread.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-01-11 10:19:22 +0800
// Last-Updated: 2009-05-16 14:25:36 +0800
// Version: $Id$
// 

#define HAVE_SYS_TIME_H
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "utils.h"
#include "globaloption.h"
#include "remotehostconnectthread.h"
#include "basestorage.h"
#include "sshfileinfo.h"

#include "synctransferthread.h"

SyncTransferThread::SyncTransferThread(QObject *parent)
    : QThread(parent)
{
    ssh2_sess = NULL;
    ssh2_sftp = NULL;
    hsftp = NULL;    
}
SyncTransferThread::~SyncTransferThread()
{
}

void SyncTransferThread::run()
{
    SyncTaskPackage task;
    int rc ;
    while (true) {
        this->wcMutex.lock();
        q_debug()<<"before waiting...";
        this->wc.wait(&this->wcMutex);
        q_debug()<<"after waiting...";
        
        while (!this->taskQueue.empty()) {
            this->tMutex.lock();
            task = this->taskQueue.dequeue();
            this->tMutex.unlock();
            
            rc = this->transfer_impl(task); 
            if (rc == TASK_ERROR_CONN_FAILD) {
                q_debug()<<"conn error";
                this->wcMutex.unlock();
                goto out_thread;
            }
            if (rc == TASK_ERROR_OTHER) {
                q_debug()<<"other error";
            }
            if (rc == TASK_OK) {
            }
        }
        this->wcMutex.unlock();
    }
 out_thread:
    return ;
}

int SyncTransferThread::transfer_impl(SyncTaskPackage task)
{
    if (!this->ssh2_sess) {
        if (!this->connectToRemoteHost()) {
            return TASK_ERROR_CONN_FAILD;
        }
    }
    Q_ASSERT(this->ssh2_sess != NULL);
    Q_ASSERT(this->ssh2_sftp != NULL);

    if (task.mnDirect == TASK_DOWNLOAD) {
        return this->do_download(this->remote_base_path + "/" + task.msFileName,
                                 this->local_base_path + "/" + task.msFileName);
    } else if (task.mnDirect == TASK_UPLOAD) {
        return this->do_upload(this->local_base_path + "/" + task.msFileName,
                               this->remote_base_path + "/" + task.msFileName);
    } else {
        Q_ASSERT(1 == 2);
    }
    
    return TASK_OK;
}

int SyncTransferThread::do_download(QString remote_path, QString local_path)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    qDebug()<< "remote_path = "<<  remote_path  << " , local_path = " << local_path ;
    int pcnt = 0 ;
    int  rlen , wlen  ;
    int file_size , tran_len = 0   ;
    LIBSSH2_SFTP_HANDLE * sftp_handle ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    char buff[8192] = {0};
    
    sftp_handle = libssh2_sftp_open(this->ssh2_sftp,
                                    GlobalOption::instance()->remote_codec->fromUnicode(remote_path), LIBSSH2_FXF_READ, 0);
    if (sftp_handle == NULL) {
        //TODO 错误消息通知用户。
        qDebug()<<"open sftp file error :"<< libssh2_sftp_last_error(this->ssh2_sftp);
        return -1 ;
    }
    
    memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
    libssh2_sftp_fstat(sftp_handle,&ssh2_sftp_attrib);
    file_size = ssh2_sftp_attrib.filesize;
    qDebug()<<" remote file size :"<< file_size ;

    // 本地编码 --> Qt 内部编码
    QFile q_file(local_path);
    if (!q_file.open(QIODevice::ReadWrite|QIODevice::Truncate)) {
        //TODO 错误消息通知用户。
        qDebug()<<"open local file error:"<< q_file.errorString() ;        
    } else {
        //read remote file  and then write to local file
        while ((rlen = libssh2_sftp_read(sftp_handle, buff, sizeof(buff))) > 0) {
            wlen = q_file.write( buff, rlen );
            tran_len += wlen ;
            qDebug()<<"Read len :"<<rlen <<" , write len: "<<wlen
                     <<" tran len: "<<tran_len ;
            //my progress signal
            if (file_size == 0) {
                // emit this->transfer_percent_changed ( 100 , tran_len , wlen );
            } else {
                pcnt = 100.0 *((double)tran_len  / (double)file_size);
                // emit this->transfer_percent_changed ( pcnt , tran_len ,wlen );
            }
        }
        q_file.close();
    }
    libssh2_sftp_close(sftp_handle);
    q_debug()<<"syncDownload done.";
    this->do_touch_local_file_with_time(local_path, SSHFileInfo(ssh2_sftp_attrib).lastModified());

    return 0;
}

int SyncTransferThread::do_upload(QString local_path, QString remote_path)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    qDebug()<< "remote_path = "<<  remote_path  << " , local_path = " << local_path ;

    int pcnt = 0 ;
    int rlen , wlen  ;
    int file_size , tran_len = 0   ;
    LIBSSH2_SFTP_HANDLE * sftp_handle ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    char buff[5120] = {0};
    
    //TODO 检查文件可写属性
    sftp_handle = libssh2_sftp_open(this->ssh2_sftp,
                                    GlobalOption::instance()->remote_codec->fromUnicode(remote_path),
                                    LIBSSH2_FXF_READ|LIBSSH2_FXF_WRITE|LIBSSH2_FXF_CREAT|LIBSSH2_FXF_TRUNC, 0666);
    if (sftp_handle == NULL) {
        //TODO 错误消息通知用户。
        qDebug()<<"open sftp file error :"<< libssh2_sftp_last_error(this->ssh2_sftp);
        if (libssh2_sftp_last_error(this->ssh2_sftp) == LIBSSH2_FX_PERMISSION_DENIED) {
            // this->errorString = QString(tr("Open file faild, Permission denied"));
            // qDebug()<<this->errorString;
        }
        // this->error_code = ERRNO_BASE + libssh2_sftp_last_error(this->ssh2_sftp);
        return -1 ;
    }
    
    memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
    QFileInfo local_fi(local_path);
    file_size = local_fi.size();
    qDebug()<<"local file size:" << file_size ;
    // emit this->transfer_got_file_size(file_size);
    
    // 本地编码 --> Qt 内部编码
    QFile q_file(local_path);
    if (!q_file.open( QIODevice::ReadOnly)) {
        //TODO 错误消息通知用户。
        qDebug()<<"open local file error:"<< q_file.errorString()  ;
        //printf("open local file error:%s\n", strerror( errno ) );        
    } else {
        //read local file and then write to remote file
        while (!q_file.atEnd()) {
            qDebug()<<"Read local ... ";
            rlen = q_file.read(buff, sizeof(buff));
            qDebug()<<"Read local done ";
            if (rlen <= 0) {
                //qDebug()<<"errno: "<<errno<<" err msg:"<< strerror( errno) << ftell( local_handle) ;
                break ;
            }
            qDebug()<<"write to sftp ... ";
            wlen = libssh2_sftp_write(sftp_handle, buff, rlen);
            qDebug()<<"write to sftp done ";
            Q_ASSERT(wlen == rlen);
            tran_len += wlen ;
            
            //qDebug()<<" local read : "<< rlen << " sftp write :"<<wlen <<" up len :"<< tran_len ;
            // 			qDebug() <<" read len :"<< rlen <<" , write len: "<< wlen 
            //                    << " tran len: "<< tran_len ;
            if (file_size == 0 ) {
                // emit this->transfer_percent_changed(100, tran_len, wlen);
            } else {
                pcnt = 100.0 *((double)tran_len  / (double)file_size);
                // qDebug()<< QString("100.0 *((double)%1  / (double)%2)").arg(tran_len).arg(file_size)<<" = "<<pcnt ;
                // emit this->transfer_percent_changed(pcnt, tran_len, wlen);
            }
        }
        q_file.close();
    }
    // qDebug()<<"out cycle, close sftp...";
    libssh2_sftp_close(sftp_handle);
    q_debug()<<"syncUpload done.";
    this->do_touch_sftp_file_with_time(remote_path, QFileInfo(local_path).lastModified());

    return 0;
}

int SyncTransferThread::do_touch_local_file_with_time(QString fileName, QDateTime time)
{
    // Linux 上可以使用 utime 或者 utimes 函数, - change file last access and modification times
    // Windows 上文件 SetFileTime（）函数设置文件的创建时间、最近一次访问时间以及最近一次修改的时间等
    // Windows 目录 两个函数GetDirTime()和SetDirTime()来实现对文件夹时间信息
    // Qt 中好像是没有修改文件时间属性的方法。

    int ret;
    QFileInfo fi(fileName);

#ifdef WIN32    
    // SYSTEMTIME systime;
    // FILETIME ft, ftUTC;
    // HANDLE hFile;

    // systime.wYear = l_year;
    // systime.wMonth = l_month;
    // systime.wDay = l_day;
    // systime.wHour = l_hour;
    // systime.wMinute = l_minute;
    // systime.wSecond = l_second;
    // systime.wMilliseconds = l_millsecond;                

    // SystemTimeToFileTime(&systime, &ft);
    // LocalFileTimeToFileTime(&ft,&ftUTC);

    // hFile = CreateFile( filePathName, GENERIC_READ | GENERIC_WRITE,
    //                     FILE_SHARE_READ| FILE_SHARE_WRITE,
    //                     NULL,
    //                     OPEN_EXISTING,
    //                     FILE_ATTRIBUTE_NORMAL,
    //                     NULL);            
    // SetFileTime(hFile, (LPFILETIME) NULL, (LPFILETIME) NULL, &ftUTC);
    // CloseHandle(hFile);
#else
    struct timeval tv[2] = {{0,0}, {0,0}};
    tv[0].tv_sec = fi.lastRead().toTime_t();
    tv[1].tv_sec = time.toTime_t();
    ret = utimes(GlobalOption::instance()->remote_codec->fromUnicode(fileName), tv);
    assert(ret == 0);    
#endif

    return 0;
}

int SyncTransferThread::do_touch_sftp_file_with_time(QString fileName, QDateTime time)
{
    // sftp_open, 可以得到当前的文件属性
    // sftp_close,
    // 修改LIBSSH2_SFTP_ATTRIBUTE中的最后修改日期
    // sftp_set_stat, 修改文件的最后修改日期。

    LIBSSH2_SFTP_ATTRIBUTES attr;
    int ret;

    ret = libssh2_sftp_stat(this->ssh2_sftp, GlobalOption::instance()->remote_codec->fromUnicode(fileName), &attr);
    if (ret != 0) {
        //int libssh2_session_last_error(LIBSSH2_SESSION *session, char **errmsg, int *errmsg_len, int want_buf
        char errmsg[200] = {0};
        int emlen = 0;
        libssh2_session_last_error(this->ssh2_sess, (char **)&errmsg, &emlen, 0);
        q_debug()<<"sftp set stat error: "<<errmsg;
    }

    attr.flags = LIBSSH2_SFTP_ATTR_ACMODTIME;
    attr.mtime = time.toTime_t();

    ret = libssh2_sftp_setstat(this->ssh2_sftp, GlobalOption::instance()->remote_codec->fromUnicode(fileName), &attr);
    if (ret != 0) {
        char errmsg[200] = {0};
        int emlen = 0;
        libssh2_session_last_error(this->ssh2_sess, (char **)&errmsg, &emlen, 0);
        q_debug()<<"sftp set stat error: "<<errmsg;
    }
    return 0;
}

bool SyncTransferThread::setRemoteSession(QString sess)
{
    this->sess_name = sess;
    this->start();
    return true;
}

bool SyncTransferThread::setBasePath(QString local, QString remote)
{
    this->local_base_path = local;
    this->remote_base_path = remote;
    return true;
}

bool SyncTransferThread::addTask(SyncTaskPackage task)
{
    if (task.mpAttr != NULL) {
        this->tMutex.lock();
        this->taskQueue.enqueue(task);
        this->tMutex.unlock();
        this->wc.wakeOne();
    } else {
        q_debug()<<"Invalid task:"<<task.msFileName<<task.mpAttr<<task.mnDirect;
    }
    return true;
}

bool SyncTransferThread::connectToRemoteHost()
{
    //this->remoteBasePath = this->parent->remote_dir;

    QMap<QString, QString> host;
    BaseStorage *storage = BaseStorage::instance();
    storage->open();
    this->remoteHost = host = storage->getHost(sess_name);
    q_debug()<<host;
    
    RemoteHostConnectThread *conn 
        = new RemoteHostConnectThread(host["user_name"], host["password"], host["host_name"],
                                      host["port"].toShort(), host["pubkey"]);
    conn->run();
    ssh2_sess = (LIBSSH2_SESSION*)conn->get_ssh2_sess();
    delete conn; conn = NULL;
    Q_ASSERT(ssh2_sess != NULL);

    ssh2_sftp = libssh2_sftp_init(ssh2_sess);
    
    if (ssh2_sftp != NULL) {
        Q_ASSERT(ssh2_sftp != NULL);
        q_debug()<<"connect ok";
        return true;
    }
    
    return false;
}

void SyncTransferThread::slot_syncDownload(QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> task)
{
    q_debug()<<"";
    SyncTaskPackage pkg(task.first, task.second, TASK_DOWNLOAD);
    this->addTask(pkg);
}

void SyncTransferThread::slot_syncUpload(QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> task)
{
    SyncTaskPackage pkg(task.first, task.second, TASK_UPLOAD);
    this->addTask(pkg);
}

