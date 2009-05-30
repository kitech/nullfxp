// transferthread.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-06-14 09:06:28 +0800
// Last-Updated: 2009-05-30 14:09:12 +0000
// Version: $Id$
// 


#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <sys/types.h>

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#ifdef WIN32
#define jjjjjjjjj
#include <winsock2.h>
#else
#include <sys/uio.h>
#endif

#include <QtCore>

#include "globaloption.h"
#include "transferthread.h"
#include "remotehostconnectthread.h"
#include "utils.h"
#include "sshfileinfo.h"

TransferThread::TransferThread(QObject *parent)
    : QThread(parent), user_canceled(false)
{
    this->file_exist_over_write_method = OW_UNKNOWN;
}


TransferThread::~TransferThread()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
}

int TransferThread::remote_is_dir(LIBSSH2_SFTP *ssh2_sftp, QString path)
{
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    LIBSSH2_SFTP_HANDLE *sftp_handle ;
    
    memset(&ssh2_sftp_attrib, 0, sizeof(ssh2_sftp_attrib));
    
    sftp_handle = libssh2_sftp_opendir(ssh2_sftp, GlobalOption::instance()->remote_codec->fromUnicode(path).data());
    
    if (sftp_handle != NULL) {
        libssh2_sftp_closedir(sftp_handle);
        return 1 ;
    } else {   // == NULL 
        //TODO 可能是一个没有打开权限的目录，这里没有处理这种情况。
        return 0;
    }
    return 0;
}

int TransferThread::remote_is_reg(LIBSSH2_SFTP *ssh2_sftp, QString path)
{
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    LIBSSH2_SFTP_HANDLE *sftp_handle;
    unsigned long flags;
    long mode ;
    int ret = 0 ;
    
    memset(&ssh2_sftp_attrib, 0, sizeof(ssh2_sftp_attrib));
    flags = LIBSSH2_FXF_READ ;
    mode = 022;
    
    sftp_handle = libssh2_sftp_open(ssh2_sftp, GlobalOption::instance()->remote_codec->fromUnicode(path).data(),
                                    flags, mode );
    
    if (sftp_handle != NULL) {
        ret = libssh2_sftp_fstat(sftp_handle, &ssh2_sftp_attrib);
        libssh2_sftp_close(sftp_handle);
        return (S_ISREG( ssh2_sftp_attrib.permissions));
    } else {    // == NULL 
        return 0;
    }
    return 0;
}
//假设这个path的编码方式是远程服务器上所用的编码方式
int TransferThread::fxp_do_ls_dir(LIBSSH2_SFTP *ssh2_sftp, QString path,
                                  QVector<QMap<char, QString> > &fileinfos)
{
    LIBSSH2_SFTP_HANDLE *sftp_handle = 0 ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib ;
    QMap<char, QString> thefile;
    char file_name[PATH_MAX+1];
    QString file_size;
    QString file_date;
    QString file_type;
    
    sftp_handle = libssh2_sftp_opendir(ssh2_sftp, GlobalOption::instance()->remote_codec->fromUnicode(path).data());
    if (sftp_handle == 0) {
        return 0;
    } else {
        fileinfos.clear();
        memset(&ssh2_sftp_attrib,0,sizeof(LIBSSH2_SFTP_ATTRIBUTES));
        while (libssh2_sftp_readdir(sftp_handle, file_name, PATH_MAX, &ssh2_sftp_attrib ) > 0) {
            if (strlen(file_name) == 1 && file_name[0] == '.') continue ;
            if (strlen(file_name) == 2 && file_name[0] == '.' && file_name[1] == '.') continue;
            //不处理隐藏文件? 处理隐藏文件
            
            SSHFileInfo fi(ssh2_sftp_attrib);
            file_size = QString("%1").arg(fi.size());
            file_date = fi.lastModified().toString("yyyy/MM/dd hh:mm:ss");
            file_type = fi.stringMode();

            //printf(" ls dir : %s %s , date=%s , type=%s \n" , file_name , file_size , file_date , file_type );
            thefile.insert('N', GlobalOption::instance()->remote_codec->toUnicode(file_name));
            thefile.insert('T', file_type);
            thefile.insert('S', file_size);
            thefile.insert('D', file_date);  
                      
            fileinfos.push_back(thefile);
            memset(&ssh2_sftp_attrib, 0, sizeof(LIBSSH2_SFTP_ATTRIBUTES));
            thefile.clear();
        }
        libssh2_sftp_closedir(sftp_handle);
        return fileinfos.size();
    }
    
    return 0 ; 
}

/**
 * 
 */
void TransferThread::run()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib ;
    RemoteHostConnectThread *rhct = 0 ;

    int rv = -1;
    int transfer_ret = -1 ;
    //int debug_sleep_time = 5 ;
    
    TaskPackage src_atom_pkg;
    TaskPackage dest_atom_pkg;

    TaskPackage temp_src_atom_pkg;
    TaskPackage temp_dest_atom_pkg;
        
    QVector<QMap<char, QString> >  fileinfos ;
    
    this->error_code = 0 ;
    this->errorString = QString(tr("No error."));
    
    this->dest_ssh2_sess = 0 ;
    this->dest_ssh2_sftp = 0 ;
    this->dest_ssh2_sock = 0 ;
        
    this->src_ssh2_sess = 0 ;
    this->src_ssh2_sftp = 0 ;
    this->src_ssh2_sock = 0 ;
    
    do {
        src_atom_pkg = this->transfer_ready_queue.front().first;
        dest_atom_pkg = this->transfer_ready_queue.front().second;
        this->current_src_file_name = src_atom_pkg.files.at(0);
        this->current_dest_file_name = dest_atom_pkg.files.at(0);

        //有效协议传输
        //file - > nrsftp
        //nrsftp -> nrsftp
        //nrsftp -> file
  
        qDebug()<<this->current_src_file_name;
        qDebug()<<this->current_dest_file_name;

        //这里有几种情况，全部都列出来
        // 上传:
        // local file type is file               remote file is file     error
        // local file type is file               remote  file is dir      ok
        // local file type is dir                remote file is file     error
        // local file type is dir                remote file is dir      ok
        // 下载：
        // local file type is file               remote file is file     error
        // local file type is file               remote file is dir      error
        // local file type is dir                remote file is file     ok
        // local file type is dir                remote file is dir      ok
       
        if (src_atom_pkg.scheme == PROTO_FILE && dest_atom_pkg.scheme == PROTO_SFTP) {
            //提示开始处理新文件：
            emit this->transfer_new_file_started(this->current_src_file_name);
            
            //连接到目录主机：
             qDebug()<<"connecting to dest ssh host:"<<dest_atom_pkg.username
                     <<":"<<dest_atom_pkg.password<<"@"<<dest_atom_pkg.host <<":"<<dest_atom_pkg.port ;
             if (this->dest_ssh2_sess == 0 || this->dest_ssh2_sftp == 0) {
                 emit  transfer_log("Connecting to destination host ...");
                 QString tmp_passwd = dest_atom_pkg.password;
                 rhct = new RemoteHostConnectThread(dest_atom_pkg.username, tmp_passwd,
                                                    dest_atom_pkg.host, dest_atom_pkg.port.toInt(), dest_atom_pkg.pubkey);
                 rhct->run();
                 //TODO get status code and then ...
                 rv = rhct->get_connect_status();
                 if (rv != RemoteHostConnectThread::CONN_OK) {
                     qDebug()<<"Connect to Host Error: "<<rv<<":"<<rhct->get_status_desc(rv);
                     emit transfer_log("Connect Error: " + rhct->get_status_desc(rv));
                     this->error_code = transfer_ret = 6;
                     this->errorString = rhct->get_status_desc(rv);
                     break;
                 }
                 //get connect return code end
                 this->dest_ssh2_sess = (LIBSSH2_SESSION*)rhct->get_ssh2_sess();
                 this->dest_ssh2_sock = rhct->get_ssh2_sock();
                 this->dest_ssh2_sftp = libssh2_sftp_init(this->dest_ssh2_sess);
                 delete rhct ; rhct = 0 ;
                 emit  transfer_log("Connect done.");
             }

            // 将文件上传到目录
            if (QFileInfo(this->current_src_file_name).isFile()
                && remote_is_dir(this->dest_ssh2_sftp , this->current_dest_file_name)) {
                QString remote_full_path = this->current_dest_file_name + "/"
                    + this->current_src_file_name.split ( "/" ).at ( this->current_src_file_name.split ( "/" ).count()-1 ) ;
                qDebug() << "local file: " << this->current_src_file_name
                         << "remote file:" << this->current_dest_file_name
                         << "remote full file path: "<< remote_full_path ;
                transfer_ret = this->do_upload(this->current_src_file_name, remote_full_path, 0);
            } else if (QFileInfo(this->current_src_file_name).isDir()
                       && remote_is_dir(this->dest_ssh2_sftp, this->current_dest_file_name)) {
                //将目录上传到目录
                qDebug()<<"uploding dir to dir ...";
                //this->sleep(debug_sleep_time);
                //
                //列出本地目录中的文件，加入到队列中，然后继续。
                //如果列出的本地文件是目录，则在这里先确定远程存在此子目录，否则就要创建先。
                fileinfos.clear();

                fxp_local_do_ls(this->current_src_file_name, fileinfos);
                qDebug()<<"ret:"<<transfer_ret<<" count:"<<fileinfos.size() ;
               
                //这个远程目录属性应该和本地属性一样，所以就使用this->current_src_file_type
                //不知道是不是有问题。
                QString remote_full_path = this->current_dest_file_name + "/" 
                    + this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1);

                temp_dest_atom_pkg = dest_atom_pkg;
                temp_dest_atom_pkg.setFile(remote_full_path);

                //为远程建立目录
                memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
                transfer_ret = libssh2_sftp_mkdir(this->dest_ssh2_sftp, 
                                                  GlobalOption::instance()->remote_codec->fromUnicode(remote_full_path), 
                                                  0755);
                qDebug()<<"libssh2_sftp_mkdir : "<< transfer_ret <<" :"<< remote_full_path;

                //添加到队列当中
                for (int i = 0 ; i < fileinfos.size() ; i ++) {
                    temp_src_atom_pkg = src_atom_pkg;
                    temp_src_atom_pkg.setFile(this->current_src_file_name + "/" + fileinfos.at(i)['N']);
                    this->transfer_ready_queue.push_back(QPair<TaskPackage, TaskPackage>
                                                         (temp_src_atom_pkg, temp_dest_atom_pkg));
                }               
            } else {
                //其他的情况暂时不考虑处理。跳过
                //TODO return a error value , not only error code
                this->error_code = 1 ;
                //assert(1 == 2) ;
                qDebug()<<"Unexpected transfer type: "<<__FILE__<<" in " << __LINE__ ;
            }

        } else if (src_atom_pkg.scheme == PROTO_SFTP && dest_atom_pkg.scheme == PROTO_FILE) {
            //提示开始处理新文件：
            emit this->transfer_new_file_started(this->current_src_file_name);
            //连接到目录主机：
            qDebug()<<"connecting to src ssh host:"<<src_atom_pkg.username<<":"<<src_atom_pkg.password
                    <<"@"<<src_atom_pkg.host <<":"<<src_atom_pkg.port ;
            if (this->src_ssh2_sess == 0 || this->src_ssh2_sftp == 0) {
                emit  transfer_log("Connecting to source host ...");
                QString tmp_passwd = src_atom_pkg.password;

                rhct = new RemoteHostConnectThread(src_atom_pkg.username, tmp_passwd, src_atom_pkg.host, 
                                                   src_atom_pkg.port.toInt(), src_atom_pkg.pubkey);
                rhct->run();
                //TODO get status code and then ...
                this->src_ssh2_sess = (LIBSSH2_SESSION*)rhct->get_ssh2_sess();
                this->src_ssh2_sock = rhct->get_ssh2_sock();
                this->src_ssh2_sftp = libssh2_sftp_init(this->src_ssh2_sess);
                delete rhct ; rhct = 0 ;
                emit  transfer_log("Connect done.");
            }
            //将文件下载到目录
            if (remote_is_reg(this->src_ssh2_sftp, this->current_src_file_name) 
                && QFileInfo(this->current_dest_file_name).isDir()) {
                QString local_full_path = this->current_dest_file_name + "/"
                    + this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1);

                qDebug() << "local file: " << this->current_src_file_name
                         << "remote file:" << this->current_dest_file_name
                         << "local full file path: "<< local_full_path ;

                transfer_ret = this->do_download(this->current_src_file_name, local_full_path, 0);
            }
            //将目录下载到目录
            else if (QFileInfo(this->current_dest_file_name).isDir()
                     && remote_is_dir(this->src_ssh2_sftp, this->current_src_file_name)) {
                qDebug()<<"downloading dir to dir ...";
                //this->sleep(debug_sleep_time);
                //列出本远程目录中的文件，加入到队列中，然后继续。
              
                fileinfos.clear();
                transfer_ret = fxp_do_ls_dir(this->src_ssh2_sftp, this->current_src_file_name + "/", fileinfos);
                qDebug()<<"ret:"<<transfer_ret<<" file count:"<<fileinfos.size();
                // local dir = curr local dir +  curr remote dir 的最后一层目录
                temp_dest_atom_pkg = dest_atom_pkg;
                temp_dest_atom_pkg.setFile(this->current_dest_file_name + "/" +
                                          this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1));
                
                //确保本地有这个目录。
                transfer_ret = QDir().mkpath(temp_dest_atom_pkg.files.at(0));
                qDebug()<<" fxp_local_do_mkdir: "<<transfer_ret <<" "<< temp_dest_atom_pkg.files.at(0) ;
                //加入到任务队列
                for (int i = 0 ; i < fileinfos.size() ; i ++) {
                    temp_src_atom_pkg = src_atom_pkg;
                    temp_src_atom_pkg.setFile(this->current_src_file_name + "/" + fileinfos.at(i)['N']);
                    
                    this->transfer_ready_queue.push_back(QPair<TaskPackage, TaskPackage>
                                                         (temp_src_atom_pkg, temp_dest_atom_pkg));
                    //romote is source 
                }
            } else {
                //其他的情况暂时不考虑处理。跳过。
                //TODO return a error value , not only error code 
                this->error_code = 1 ;
                //assert( 1 == 2 ) ; 
                qDebug()<<"Unexpected transfer type: "<<__FILE__<<" in " << __LINE__ ;
            }
        } else if(src_atom_pkg.scheme == PROTO_SFTP && dest_atom_pkg.scheme == PROTO_SFTP) {
            emit this->transfer_new_file_started(this->current_src_file_name);
            //处理nrsftp协议
            if (this->src_ssh2_sess == 0 || this->src_ssh2_sftp == 0) {
                emit  transfer_log("Connecting to destionation host ...");
                QString tmp_passwd = src_atom_pkg.password;

                rhct = new RemoteHostConnectThread(src_atom_pkg.username, tmp_passwd, src_atom_pkg.host, 
                                                   src_atom_pkg.port.toInt(), src_atom_pkg.pubkey);
                rhct->run();
                //TODO get status code and then ...
                this->src_ssh2_sess = (LIBSSH2_SESSION*)rhct->get_ssh2_sess();
                this->src_ssh2_sock = rhct->get_ssh2_sock();
                this->src_ssh2_sftp = libssh2_sftp_init(this->src_ssh2_sess);
                delete rhct ; rhct = 0 ;
                emit  transfer_log("Connect done.");
            }
            if (this->dest_ssh2_sess == 0 || this->dest_ssh2_sftp == 0 ) {
                emit  transfer_log("Connecting to source host ...");
                QString tmp_passwd = dest_atom_pkg.password;

                rhct = new RemoteHostConnectThread(dest_atom_pkg.username, tmp_passwd, dest_atom_pkg.host,
                                                   dest_atom_pkg.port.toInt(), dest_atom_pkg.pubkey);
                rhct->run();
                //TODO get status code and then ...
                this->dest_ssh2_sess = (LIBSSH2_SESSION*)rhct->get_ssh2_sess();
                this->dest_ssh2_sock = rhct->get_ssh2_sock();
                this->dest_ssh2_sftp = libssh2_sftp_init(this->dest_ssh2_sess);
                delete rhct ; rhct = 0 ;
                emit  transfer_log("Connect done.");
            }
            ////////////
            if (remote_is_dir(this->src_ssh2_sftp, this->current_src_file_name) 
                && remote_is_dir(this->dest_ssh2_sftp,this->current_dest_file_name ) )
            {
                qDebug()<<" nrsftp exchage dir to dir...";
                fileinfos.clear();        

                transfer_ret = fxp_do_ls_dir(this->src_ssh2_sftp, this->current_src_file_name + "/", fileinfos);
                qDebug()<<"ret:"<<transfer_ret<<" file count:"<<fileinfos.size();
                temp_dest_atom_pkg = dest_atom_pkg;
                temp_dest_atom_pkg.setFile(this->current_dest_file_name + "/" + 
                                           this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1));    
                                       
                //为远程建立目录
                memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
                transfer_ret = libssh2_sftp_mkdir(this->dest_ssh2_sftp, 
                                                  GlobalOption::instance()->remote_codec->fromUnicode(temp_dest_atom_pkg.files.at(0)), 
                                                  0755);
                qDebug()<<" libssh2_sftp_mkdir : "<< transfer_ret <<" :"<<temp_dest_atom_pkg.files.at(0);

                //添加到队列当中
                for (int i = 0 ; i < fileinfos.size() ; i ++ ) {
                    temp_src_atom_pkg = src_atom_pkg;
                    temp_src_atom_pkg.setFile(this->current_src_file_name + "/" +
                                              fileinfos.at(i)['N']);
                    this->transfer_ready_queue.push_back(QPair<TaskPackage, TaskPackage>
                                                         (temp_src_atom_pkg, temp_dest_atom_pkg));
                }
            } else if (remote_is_reg(this->src_ssh2_sftp, this->current_src_file_name) 
                       && remote_is_dir(this->dest_ssh2_sftp,this->current_dest_file_name))
            {
                qDebug()<<" nrsftp exchage file to dir...";
                QString dest_full_path = this->current_dest_file_name + "/" + this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1);
                transfer_ret = this->do_nrsftp_exchange(this->current_src_file_name, dest_full_path);
            } else {
                //其他的情况暂时不考虑处理。跳过
                //TODO return a error value , not only error code
                q_debug()<<"src: "<< src_atom_pkg<<" dest:"<< dest_atom_pkg;
                this->error_code = 1 ;
                //assert ( 1 == 2 ) ;
                qDebug()<<"Unexpected transfer type: "<<__FILE__<<" in " << __LINE__ ;
            }
        } else {
            q_debug()<<"src: "<< src_atom_pkg<<" dest:"<< dest_atom_pkg;
            this->error_code = 2;
            assert( 1 == 2 );
        }
       
        this->transfer_ready_queue.erase(this->transfer_ready_queue.begin());
        this->transfer_done_queue.push_back(QPair<TaskPackage, TaskPackage>(src_atom_pkg, dest_atom_pkg));
    } while (this->transfer_ready_queue.size() > 0 && user_canceled == false) ;

    qDebug() << " transfer_ret :" << transfer_ret << " ssh2 sftp shutdown:"<< this->src_ssh2_sftp<<" "<<this->dest_ssh2_sftp;
    //TODO 选择性关闭 ssh2 会话，有可能是 src  ,也有可能是dest 
    if (this->src_ssh2_sftp != 0) {
        libssh2_sftp_shutdown(this->src_ssh2_sftp);
        this->src_ssh2_sftp = 0 ;
    }
    if (this->src_ssh2_sess != 0) {
        libssh2_session_free(this->src_ssh2_sess);
        this->src_ssh2_sess = 0 ;
    }
    if (this->src_ssh2_sock > 0) {
#ifdef WIN32
        ::closesocket(this->src_ssh2_sock);
#else  
        ::close(this->src_ssh2_sock);
#endif
        this->src_ssh2_sock = -1;
    }
    if (this->dest_ssh2_sftp != 0) {
        libssh2_sftp_shutdown(this->dest_ssh2_sftp);
        this->dest_ssh2_sftp = 0 ;
    }
    if (this->dest_ssh2_sess != 0) {
        libssh2_session_free(this->dest_ssh2_sess);
        this->dest_ssh2_sess = 0 ;
    }
    if (this->dest_ssh2_sock > 0) {
#ifdef WIN32
        ::closesocket(this->dest_ssh2_sock);
#else  
        ::close(this->dest_ssh2_sock);
#endif
        this->dest_ssh2_sock = -1;
    }
    if (user_canceled == true) {
        this->error_code = 3;
    }
}

void TransferThread::set_transfer_info (TaskPackage src_pkg, TaskPackage dest_pkg)
{
    this->src_pkg = src_pkg;
    this->dest_pkg = dest_pkg;

    QString src_file_name ;
    QString dest_file_name ;

    TaskPackage src_atom_pkg;
    TaskPackage dest_atom_pkg;

    for (int i = 0 ; i < src_pkg.files.count() ; i ++) {
        src_atom_pkg = src_pkg;
        src_atom_pkg.files.clear();
        src_atom_pkg.files<<src_pkg.files.at(i);

        for (int j = 0 ; j < dest_pkg.files.count() ; j ++) {
            dest_atom_pkg = dest_pkg;
            dest_atom_pkg.files.clear();
            dest_atom_pkg.files<<dest_pkg.files.at(j);

            this->transfer_ready_queue.push_back(QPair<TaskPackage, TaskPackage>(src_atom_pkg, dest_atom_pkg));
        }
    }
}

int TransferThread::do_download(QString remote_path, QString local_path, int pflag)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    qDebug()<< "remote_path = "<<  remote_path  << " , local_path = " << local_path ;
    Q_UNUSED(pflag);
    int pcnt = 0 ;
    int rlen, wlen;
    int file_size, tran_len = 0;
    LIBSSH2_SFTP_HANDLE *sftp_handle ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    char buff[8192] = {0};
    
    sftp_handle = libssh2_sftp_open(this->src_ssh2_sftp, gOpt->remote_codec->fromUnicode(remote_path), LIBSSH2_FXF_READ, 0);
    if (sftp_handle == NULL) {
        //TODO 错误消息通知用户。
        qDebug()<<"open sftp file error :"<<libssh2_sftp_last_error(this->src_ssh2_sftp);
        return -1 ;
    }
    
    memset(&ssh2_sftp_attrib, 0, sizeof(ssh2_sftp_attrib));
    libssh2_sftp_fstat(sftp_handle, &ssh2_sftp_attrib);
    file_size = ssh2_sftp_attrib.filesize;
    qDebug()<<" remote file size :"<<file_size ;
    emit this->transfer_got_file_size(file_size);

    //文件冲突检测
    if (QFile(local_path).exists()) {
        if (this->user_canceled) return 1;
        if (this->user_canceled) return 1;
        if (this->file_exist_over_write_method == OW_UNKNOWN
           || this->file_exist_over_write_method == OW_YES
           || this->file_exist_over_write_method == OW_RESUME
           || this->file_exist_over_write_method == OW_NO) {
            //TODO 通知用户远程文件已经存在，再做处理。                                                                          
            QString local_file_size, local_file_date;
            QString remote_file_size, remote_file_date;
            QFileInfo fi(local_path);
            local_file_size = QString("%1").arg(fi.size());
            local_file_date = fi.lastModified ().toString();
            remote_file_size = QString("%1").arg(ssh2_sftp_attrib.filesize);
            QDateTime remote_mtime ;
            remote_mtime.setTime_t(ssh2_sftp_attrib.mtime);
            remote_file_date = remote_mtime.toString();
            emit this->dest_file_exists(remote_path, remote_file_size, remote_file_date,
                                        local_path,local_file_size,local_file_date);
            this->wait_user_response();
        }

        if (this->user_canceled || this->file_exist_over_write_method== OW_CANCEL) return 1;
        if (this->file_exist_over_write_method == OW_YES){}   //go on 
        if (this->file_exist_over_write_method == OW_NO) return 1;
        if (this->file_exist_over_write_method == OW_NO_ALL) {
            this->user_canceled = true;
            return 1;
        }
    }

    // 本地编码 --> Qt 内部编码
    QFile q_file(local_path);
    if (!q_file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        //TODO 错误消息通知用户。
        qDebug()<<"open local file error:"<<q_file.errorString();        
    } else {
        //read remote file  and then write to local file
        while ((rlen = libssh2_sftp_read(sftp_handle, buff, sizeof(buff))) > 0) {
            wlen = q_file.write(buff, rlen);
            tran_len += wlen;
            //qDebug() <<" read len :"<< rlen <<" , write len: "<< wlen                     << " tran len: "<< tran_len ;
            //my progress signal
            if (file_size == 0) {
                emit this->transfer_percent_changed(100, tran_len, wlen);
            } else {
                pcnt = 100.0 *((double)tran_len / (double)file_size);
                emit this->transfer_percent_changed(pcnt, tran_len, wlen);
            }
            if (user_canceled == true) {
                break;
            }
        }
        q_file.setPermissions(SSHFileInfo(ssh2_sftp_attrib).qMode());
        q_file.close();
    }
    
    libssh2_sftp_close(sftp_handle);

    return 0;
}


int TransferThread::do_upload(QString local_path, QString remote_path, int pflag)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    qDebug()<< "remote_path = "<<  remote_path  << " , local_path = " <<local_path;
    Q_UNUSED(pflag);

    int pcnt = 0 ;
    int rlen, wlen;
    int file_size, tran_len = 0;
    LIBSSH2_SFTP_HANDLE *sftp_handle ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    char buff[5120] = {0};
    int ret = 0;
    
    //TODO 检测文件是否存在
    memset(&ssh2_sftp_attrib, 0, sizeof(LIBSSH2_SFTP_ATTRIBUTES));
    ret = libssh2_sftp_stat(this->dest_ssh2_sftp,
                            GlobalOption::instance()->remote_codec->fromUnicode(remote_path),
                            &ssh2_sftp_attrib);
    if (ret == 0) {
        if (this->user_canceled) return 1;
        if (this->file_exist_over_write_method == OW_UNKNOWN 
           || this->file_exist_over_write_method == OW_YES
           || this->file_exist_over_write_method == OW_RESUME
           || this->file_exist_over_write_method == OW_NO) {
            //TODO 通知用户远程文件已经存在，再做处理。
            QString local_file_size, local_file_date;
            QString remote_file_size, remote_file_date;
            QFileInfo fi(local_path);
            local_file_size = QString("%1").arg(fi.size());
            local_file_date = fi.lastModified ().toString();
            remote_file_size = QString("%1").arg(ssh2_sftp_attrib.filesize);
            QDateTime remote_mtime;
            remote_mtime.setTime_t(ssh2_sftp_attrib.mtime);
            remote_file_date = remote_mtime.toString();
            emit this->dest_file_exists(local_path, local_file_size, local_file_date, 
                                        remote_path, remote_file_size, remote_file_date);
            this->wait_user_response();
        }
        if (this->user_canceled || this->file_exist_over_write_method == OW_CANCEL) return 1;
        if (this->file_exist_over_write_method == OW_YES) {}//go on
        if (this->file_exist_over_write_method == OW_NO) return 1;
        if (this->file_exist_over_write_method == OW_NO_ALL) {
            this->user_canceled = true;
            return 1;
        }
        qDebug()<<"Remote file exists, cover it.";
    } else {
        //文件不存在
    }

    QFileInfo fi = QFileInfo(local_path);
    SSHFileInfo sfi = SSHFileInfo::fromQFileInfo(fi);
    //TODO 检查文件可写属性
    sftp_handle = libssh2_sftp_open(this->dest_ssh2_sftp,
                                    gOpt->remote_codec->fromUnicode(remote_path),
                                    LIBSSH2_FXF_READ|LIBSSH2_FXF_WRITE|LIBSSH2_FXF_CREAT|LIBSSH2_FXF_TRUNC, 
                                    sfi.mode());
    if (sftp_handle == NULL) {
        //TODO 错误消息通知用户。
        qDebug()<<"open sftp file error :"<< libssh2_sftp_last_error(this->dest_ssh2_sftp);
        if (libssh2_sftp_last_error(this->dest_ssh2_sftp) == LIBSSH2_FX_PERMISSION_DENIED) {
            this->errorString = QString(tr("Open file faild, Permission denied"));
            qDebug()<<this->errorString;
        }
        this->error_code = ERRNO_BASE + libssh2_sftp_last_error(this->dest_ssh2_sftp);
	
        return -1 ;
    }
    
    memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
    //libssh2_sftp_fstat(sftp_handle,&ssh2_sftp_attrib);
    //file_size = ssh2_sftp_attrib.filesize;
    //qDebug()<<" remote file size :"<< file_size ;
    QFileInfo local_fi(local_path);
    file_size = local_fi.size();
    qDebug()<<"local file size:" << file_size ;
    emit this->transfer_got_file_size(file_size);
    
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
                emit this->transfer_percent_changed(100, tran_len, wlen);
            } else {
                pcnt = 100.0 *((double)tran_len  / (double)file_size);
                // qDebug()<< QString("100.0 *((double)%1  / (double)%2)").arg(tran_len).arg(file_size)<<" = "<<pcnt ;
                emit this->transfer_percent_changed(pcnt, tran_len, wlen);
            }
            if (user_canceled == true) {
                break;
            }
        }
        q_file.close();
    }
    qDebug()<<"out cycle, close sftp...";
    libssh2_sftp_close(sftp_handle);
    
    return 0;
}
int TransferThread::do_nrsftp_exchange(QString src_path, QString dest_path)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    qDebug()<<"nrsftp from: "<< src_path <<" to "<< dest_path ;
    
    LIBSSH2_SFTP_HANDLE *src_sftp_handle = 0;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    LIBSSH2_SFTP_HANDLE *dest_sftp_handle = 0;
    char buff[5120] = {0};
    int rlen, wlen, tran_len = 0;
    int file_size = 0;
    int pcnt = 0;
    
    //检测传输文件的属性

    src_sftp_handle = libssh2_sftp_open(this->src_ssh2_sftp,
                                        gOpt->remote_codec->fromUnicode(src_path).data(),
                                        LIBSSH2_FXF_READ, 0666);
    if (src_sftp_handle == 0) {
        qDebug()<<"sftp open error: "<<libssh2_session_last_error((LIBSSH2_SESSION*)(src_ssh2_sess), 0, 0, 0); 
        assert(src_sftp_handle != 0);
    }
    
    libssh2_sftp_fstat(src_sftp_handle, &ssh2_sftp_attrib);
    file_size = ssh2_sftp_attrib.filesize;
    qDebug()<<" source file size :"<<file_size;
    emit this->transfer_got_file_size(file_size);
    
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    
    assert(this->dest_ssh2_sftp != 0);
    dest_sftp_handle = libssh2_sftp_open(this->dest_ssh2_sftp,
                                         gOpt->remote_codec->fromUnicode(dest_path).data(),
                                         LIBSSH2_FXF_WRITE | LIBSSH2_FXF_CREAT | LIBSSH2_FXF_TRUNC, 0666 );
    if (src_sftp_handle == 0 ) {
        qDebug()<<"sftp open error: "<<libssh2_session_last_error((LIBSSH2_SESSION*)(dest_ssh2_sess), 0, 0, 0); 
        assert(dest_sftp_handle != 0);
    }
    
    //TODO 如果是同一台SSH服务器，可以用libssh2_sftp_copy,不过现在还没这个函数
    for (;;) {
        rlen = libssh2_sftp_read(src_sftp_handle, buff, sizeof(buff));
        if (rlen <= 0 ) {
            qDebug()<<" may be read end "<<rlen
                    <<libssh2_session_last_error((LIBSSH2_SESSION*)(this->src_ssh2_sess), 0, 0, 0);
            break;
        }
        wlen = libssh2_sftp_write(dest_sftp_handle, buff, rlen);
        tran_len += wlen;
        //qDebug() <<" read len :"<< rlen <<" , write len: "<< wlen << " tran len: "<< tran_len ;
        
        if (file_size == 0) {
            emit this->transfer_percent_changed(100, tran_len, wlen);
        } else {
            pcnt = 100.0 *((double)tran_len / (double)file_size);
            emit this->transfer_percent_changed(pcnt, tran_len, wlen);
        }
        if (user_canceled == true) {
            break;
        }
    }

    return 0;
}

void TransferThread::set_user_cancel(bool cancel)
{
    this->user_canceled = cancel ;
}

void TransferThread::wait_user_response()
{
    this->wait_user_response_mutex.lock();
    this->wait_user_response_cond.wait(&this->wait_user_response_mutex);
}
void TransferThread::user_response_result(int result)
{
    if (result >= OW_CANCEL && result <= OW_NO_ALL) {
        this->file_exist_over_write_method = result;
    } else {
        //未知处理方式的情况下，不覆盖原有文件，所以就取消传输任务
        this->set_user_cancel(true);
    }
    this->wait_user_response_cond.wakeAll();
    this->wait_user_response_mutex.unlock();
}

// on windows 有一个问题：当两个从本地到同一远程主机的目录上传时，导致下面的错误：
//Assertion failed: *lock == MUTEX_UNLOCKED, file ath.c, line 184
//这是mingw32平台上的libgcrypt相关的问题。
