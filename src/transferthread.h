/* transferthread.h --- 
 * 
 * Author: liuguangzhao
 * Copyright (C) 2007-2010 liuguangzhao@users.sf.net
 * URL: http://www.qtchina.net http://nullget.sourceforge.net
 * Created: 2008-06-14 09:02:05 +0800
 * Last-Updated: 2009-05-09 09:06:21 +0800
 * Version: $Id$
 */

#ifndef TRANSFERTHREAD_H
#define TRANSFERTHREAD_H

#include <vector>
#include <map>

#include <QtCore>
#include <QThread>

#include "libssh2.h"
#include "libssh2_sftp.h"

#include "taskpackage.h"
/**
 * @author liuguangzhao <liuguangzhao@users.sf.net>
 * 
 * 此类实现不同主机之间的文件传输功能
 * 由于libssh2协议库的限制，程序无法在一个ssh连接上打开两个或者以上SFTP传输，在此类中选择每次传输
 * 都会创建一个新的SSH连接会话（注：并不是每个文件创建一个连接），这样就不会与目录操作冲突了。
 * 引起的问题是连接速度不容乐观，对于小文件及少量文件传输很不合适，大多数时间都用在了创建新的
 * SSH连接上了。
 * 另一个选择是使用连接池，预先创建一定数量的SSH连接，在此类中选择空闲的连接即可。如何定义预连接的数目，
 * 如果管理这些连接，相当的复杂。
 * 最后，使用最先使用的方式，对某远程主机，同一时刻只准执行一个SSH命令操作，也就是在这个唯一的SSH连接
 * 上执行同步操作，顺序操作，所有的命令序列及响应都是顺序的，不会发生死锁问题。
 * 还有其他的方案吗？
 */

class TransferThread : public QThread
{
    Q_OBJECT
public:
    
    enum {TRANSFER_MIN, TRANSFER_GET,TRANSFER_PUT,TRANSFER_EXCHANGE,TRANSFER_RETRIVE_TO_LOCAL,TRANSFER_RETRIVE_TO_REMOTE ,TRANSFER_MAX };
    enum {OW_UNKNOWN,OW_CANCEL, OW_YES,OW_YES_ALL,OW_RESUME,OW_NO, OW_NO_ALL};
    enum {ERRNO_BASE=100};
    
    TransferThread(QObject *parent = 0);
    ~TransferThread();

    void run();
    
    //说明，在上传的时候local_file_names.count()可以大于1个，而remote_file_names.count()必须等于1
    //在下载的时候：local_file_names.count()必须等于1,而remote_file_names.count()可以大于1个
    void set_transfer_info(TaskPackage src_pkg, TaskPackage dest_pkg);
    
    int do_upload(QString src_path, QString dest_path, int pflag);
    int do_download(QString src_path, QString dest_path, int pflag);
    int do_nrsftp_exchange(QString src_path, QString dest_path);
    
    int get_error_code () { return this->error_code ;} 
    QString get_error_message(int error_code){ (void)error_code; return this->errorString;}
    void set_user_cancel( bool cancel );

    void user_response_result(int result);

private :
    int remote_is_dir(LIBSSH2_SFTP * ssh2_sftp, QString path);
    int remote_is_reg(LIBSSH2_SFTP * ssh2_sftp, QString path); 
    int fxp_do_ls_dir(LIBSSH2_SFTP * ssh2_sftp, QString parent_path, QVector<QMap<char, QString> > & fileinfos);

    void wait_user_response();

signals:
    void  transfer_percent_changed(int percent, int total_transfered, int transfer_delta);
    void  transfer_new_file_started(QString new_file_name);
    void  transfer_got_file_size(int size);
    void  transfer_log(QString log);
    void dest_file_exists(QString src_file, QString src_file_size,
                          QString src_file_date, QString dest_file,
                          QString dest_file_size, QString dest_file_date);
        
private:
    LIBSSH2_SESSION * dest_ssh2_sess;
    LIBSSH2_SFTP * dest_ssh2_sftp;
    int dest_ssh2_sock ;
        
    LIBSSH2_SESSION * src_ssh2_sess ;
    LIBSSH2_SFTP * src_ssh2_sftp ;
    int src_ssh2_sock ;
        
    bool user_canceled ;
    int file_exist_over_write_method;
        
    TaskPackage src_pkg;
    TaskPackage dest_pkg;
        
    quint64 total_file_size ;
    quint64 total_transfered_file_length ; 
    quint32 total_file_count ;
    quint32 total_transfered_file_count ;
    quint64 current_file_size ;
    quint64 current_file_transfered_length ;
    QString  current_src_file_name;
    QString  current_dest_file_name;

    QVector<QPair<TaskPackage, TaskPackage> > transfer_ready_queue;
    QVector<QPair<TaskPackage, TaskPackage> > transfer_done_queue;
    //
    int error_code ;
    QString errorString;
    //
    QWaitCondition  wait_user_response_cond;
    QMutex     wait_user_response_mutex;
};

#endif
