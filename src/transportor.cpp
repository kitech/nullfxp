// transportor.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-09-09 11:18:21 +0800
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
#include "transportor.h"
#include "remotehostconnectthread.h"
#include "utils.h"
#include "sshfileinfo.h"
#include "sshconnection.h"
#include "libftp/libftp.h"
#include "ftpconnection.h"

Transportor::Transportor(QObject *parent)
    : QThread(parent), user_canceled(false)
{
    this->file_exist_over_write_method = OW_UNKNOWN;
    this->sconn = 0;
    this->dconn = 0;
    this->src_ssh2_sess = 0;
    this->src_ssh2_sftp = 0;
    this->src_ssh2_sock = 0;
    this->dest_ssh2_sess = 0;
    this->dest_ssh2_sftp = 0;
    this->dest_ssh2_sock = 0;
}


Transportor::~Transportor()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
}

int Transportor::remote_is_dir(LIBSSH2_SFTP *ssh2_sftp, QString path)
{
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    LIBSSH2_SFTP_HANDLE *sftp_handle ;
    QByteArray bpath = GlobalOption::instance()->remote_codec->fromUnicode(path);

    memset(&ssh2_sftp_attrib, 0, sizeof(ssh2_sftp_attrib));
    
    sftp_handle = libssh2_sftp_opendir(ssh2_sftp, bpath.data());
    
    if (sftp_handle != NULL) {
        libssh2_sftp_closedir(sftp_handle);
        return 1 ;
    } else {   // == NULL 
        //TODO 可能是一个没有打开权限的目录，这里没有处理这种情况。
        return 0;
    }
    return 0;
}

int Transportor::remote_is_reg(LIBSSH2_SFTP *ssh2_sftp, QString path)
{
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    LIBSSH2_SFTP_HANDLE *sftp_handle;
    unsigned long flags;
    long mode;
    int ret = 0;
    QByteArray bpath = GlobalOption::instance()->remote_codec->fromUnicode(path);
    
    memset(&ssh2_sftp_attrib, 0, sizeof(ssh2_sftp_attrib));
    flags = LIBSSH2_FXF_READ ;
    mode = 022;
    
    sftp_handle = libssh2_sftp_open(ssh2_sftp, bpath.data(), flags, mode );
    
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
int Transportor::fxp_do_ls_dir(LIBSSH2_SFTP *ssh2_sftp, QString path,
                                  QVector<QMap<char, QString> > &fileinfos)
{
    LIBSSH2_SFTP_HANDLE *sftp_handle = 0 ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
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
int Transportor::isFTPDir(Connection *conn, QString path)
{
    q_debug()<<"test file: "<<path<<conn->ftp;
    // int iret = conn->ftp->stat(path);
    // if (iret != 0) {
    //     q_debug()<<"stat error";
    //     return 0;
    // }
    // QVector<QUrlInfo> dirList = conn->ftp->getDirList();
    // if (dirList.count() == 0) {
    //     q_debug()<<"dirList count == 0";
    //     return 0;
    // }
    // // 根据这个判断看来有很大问题，不同的FTP服务器有不同的结果
    // // 还是要用列出来的属性，dxxxxxxx串来判断
    // QFileInfo fi(path);
    // if (dirList.at(0).name() == path || dirList.at(0).name() == fi.fileName()) {
    //     q_debug()<<"dirList first name == path";
    //     return 0;
    // } else { // 第一个元素不是path的时候，就是目录
    //     q_debug()<<"it must be a dir";
    //     return 1;
    // }
    int iret = conn->ftp->chdir(path);
    if (iret == 0) {
        return 1;
    }
    return 0;
}
int Transportor::isFTPFile(Connection *conn, QString path)
{
    int iret = this->isFTPDir(conn, path);
    if (iret == 0) {
        iret = 1;
    } else {
        iret = 0;
    }
    return iret;
}

void Transportor::run()
{
    int srcProtocol = this->src_pkg.scheme;
    int destProtocol = this->dest_pkg.scheme;
    int iret = -1;
    /*
      FILE -> SFTP  SSHTransportor
      FILE -> FTP   FTPTransportor
      FTP -> FILE   FTPTransportor
      SFTP -> FILE  SSHTransportor
      FTP -> SFTP   ???
      SFTP -> FTP   ???
      FTP -> FTP    FTPTransportor
      SFTP -> SFTP  SSHTransportor
     */
    if (srcProtocol == PROTO_FILE && destProtocol == PROTO_SFTP) {
        iret = this->run_FILE_to_SFTP();
    } else if (srcProtocol == PROTO_FILE && destProtocol == PROTO_FTP) {
        iret = this->run_FILE_to_FTP();
    } else if (srcProtocol == PROTO_FTP && destProtocol == PROTO_FILE) {
        iret = this->run_FTP_to_FILE();
    } else if (srcProtocol == PROTO_SFTP && destProtocol == PROTO_FILE) {
        iret = this->run_SFTP_to_FILE();
    } else if (srcProtocol == PROTO_FTP && destProtocol == PROTO_SFTP) {
        iret = this->run_FTP_to_SFTP();
    } else if (srcProtocol == PROTO_SFTP && destProtocol == PROTO_FTP) {
        iret = this->run_SFTP_to_FTP();
    } else if (srcProtocol == PROTO_FTP && destProtocol == PROTO_FTP) {
        iret = this->run_FTP_to_FTP();
    } else if (srcProtocol == PROTO_SFTP && destProtocol == PROTO_SFTP) {
        iret = this->run_SFTP_to_SFTP();
    } else {
        q_debug()<<"Unsupported file transport type.";
    }
}

// similar to current do_upload method
int Transportor::run_FILE_to_SFTP()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    RemoteHostConnectThread *rhct = 0;

    int rv = -1;
    int transfer_ret = -1;
    //int debug_sleep_time = 5;
    
    TaskPackage src_atom_pkg;
    TaskPackage dest_atom_pkg;

    TaskPackage temp_src_atom_pkg;
    TaskPackage temp_dest_atom_pkg;
        
    QVector<QMap<char, QString> >  fileinfos;
    
    this->error_code = 0 ;
    this->errorString = QString(tr("No error."));
    
    this->dest_ssh2_sess = 0;
    this->dest_ssh2_sftp = 0;
    this->dest_ssh2_sock = 0;
        
    this->src_ssh2_sess = 0;
    this->src_ssh2_sftp = 0;
    this->src_ssh2_sock = 0;
    
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
            // transfer_ret = this->do_upload(this->current_src_file_name, remote_full_path, 0);
            transfer_ret = this->run_FILE_to_SFTP(this->current_src_file_name, remote_full_path);
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
            qDebug()<<"ret:"<<transfer_ret<<"count:"<<fileinfos.size();
               
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
            this->error_code = 1;
            //assert(1 == 2);
            qDebug()<<"Unexpected transfer type: "<<__FILE__<<" in "<< __LINE__;
        }

        this->transfer_ready_queue.erase(this->transfer_ready_queue.begin());
        this->transfer_done_queue.push_back(QPair<TaskPackage, TaskPackage>(src_atom_pkg, dest_atom_pkg));
    } while (this->transfer_ready_queue.size() > 0 && this->user_canceled == false);

    qDebug()<<"transfer_ret :"<< transfer_ret<<" ssh2 sftp shutdown:"<< this->src_ssh2_sftp<<" "<<this->dest_ssh2_sftp;
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
    if (this->user_canceled == true) {
        this->error_code = 3;
    }
    return 0;
}
int Transportor::run_FILE_to_SFTP(QString srcFile, QString destFile)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    qDebug()<<"remote_path = "<<destFile<<" , local_path = "<<srcFile;

    int pcnt = 0;
    int rlen, wlen;
    int file_size, tran_len = 0;
    LIBSSH2_SFTP_HANDLE *sftp_handle ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    char buff[5120] = {0};
    int ret = 0;
    
    //TODO 检测文件是否存在
    memset(&ssh2_sftp_attrib, 0, sizeof(LIBSSH2_SFTP_ATTRIBUTES));
    ret = libssh2_sftp_stat(this->dest_ssh2_sftp,
                            GlobalOption::instance()->remote_codec->fromUnicode(destFile),
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
            QFileInfo fi(srcFile);
            local_file_size = QString("%1").arg(fi.size());
            local_file_date = fi.lastModified ().toString();
            remote_file_size = QString("%1").arg(ssh2_sftp_attrib.filesize);
            QDateTime remote_mtime;
            remote_mtime.setTime_t(ssh2_sftp_attrib.mtime);
            remote_file_date = remote_mtime.toString();
            emit this->dest_file_exists(srcFile, local_file_size, local_file_date, 
                                        destFile, remote_file_size, remote_file_date);
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

    QFileInfo fi = QFileInfo(srcFile);
    SSHFileInfo sfi = SSHFileInfo::fromQFileInfo(fi);
    //TODO 检查文件可写属性
    sftp_handle = libssh2_sftp_open(this->dest_ssh2_sftp,
                                    gOpt->remote_codec->fromUnicode(destFile),
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
	
        return -1;
    }
    
    memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
    QFileInfo local_fi(srcFile);
    file_size = local_fi.size();
    qDebug()<<"local file size:" << file_size ;
    emit this->transfer_got_file_size(file_size);
    
    QFile q_file(srcFile);
    if (!q_file.open( QIODevice::ReadOnly)) {
        //TODO 错误消息通知用户。
        qDebug()<<"open local file error:"<< q_file.errorString()  ;
        //printf("open local file error:%s\n", strerror( errno ) );        
    } else {
        //read local file and then write to remote file
        while (!q_file.atEnd()) {
            rlen = q_file.read(buff, sizeof(buff));
            if (rlen <= 0) {
                //qDebug()<<"errno: "<<errno<<" err msg:"<< strerror( errno) << ftell( local_handle) ;
                break ;
            }
            wlen = libssh2_sftp_write(sftp_handle, buff, rlen);
            Q_ASSERT(wlen == rlen);
            if (wlen < rlen) {
                q_debug()<<"write to server less then need write bytes";
                // TODO 这种情况应该尝试再次写入剩余的数据
            }
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
            if (this->user_canceled == true) {
                break;
            }
        }
        q_file.close();
    }
    qDebug()<<"out cycle, close sftp...";
    libssh2_sftp_close(sftp_handle);
    
    return 0;
}

// similar to current do_download method
int Transportor::run_SFTP_to_FILE()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    RemoteHostConnectThread *rhct = 0;

    int rv = -1;
    int transfer_ret = -1;
    //int debug_sleep_time = 5;
    
    TaskPackage src_atom_pkg;
    TaskPackage dest_atom_pkg;

    TaskPackage temp_src_atom_pkg;
    TaskPackage temp_dest_atom_pkg;
        
    QVector<QMap<char, QString> >  fileinfos;
    
    this->error_code = 0 ;
    this->errorString = QString(tr("No error."));
    
    this->dest_ssh2_sess = 0;
    this->dest_ssh2_sftp = 0;
    this->dest_ssh2_sock = 0;
        
    this->src_ssh2_sess = 0;
    this->src_ssh2_sftp = 0;
    this->src_ssh2_sock = 0;
    
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

            // transfer_ret = this->do_download(this->current_src_file_name, local_full_path, 0);
            transfer_ret = this->run_SFTP_to_FILE(this->current_src_file_name, local_full_path);
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
            this->error_code = 1;
            //assert( 1 == 2 );
            qDebug()<<"Unexpected transfer type: "<<__FILE__<<" in " << __LINE__;
        }
       
        this->transfer_ready_queue.erase(this->transfer_ready_queue.begin());
        this->transfer_done_queue.push_back(QPair<TaskPackage, TaskPackage>(src_atom_pkg, dest_atom_pkg));
    } while (this->transfer_ready_queue.size() > 0 && this->user_canceled == false);

    qDebug()<<"transfer_ret :"<<transfer_ret<<" ssh2 sftp shutdown:"<<this->src_ssh2_sftp<<" "<<this->dest_ssh2_sftp;
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
    if (this->user_canceled == true) {
        this->error_code = 3;
    }
    return 0;
}
int Transportor::run_SFTP_to_FILE(QString srcFile, QString destFile)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    qDebug()<<"remote_path = "<<destFile<<" , local_path = "<<srcFile;

    int pcnt = 0 ;
    int rlen, wlen;
    int file_size, tran_len = 0;
    LIBSSH2_SFTP_HANDLE *sftp_handle ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    char buff[8192] = {0};
    
    sftp_handle = libssh2_sftp_open(this->src_ssh2_sftp, gOpt->remote_codec->fromUnicode(srcFile), LIBSSH2_FXF_READ, 0);
    if (sftp_handle == NULL) {
        //TODO 错误消息通知用户。
        qDebug()<<"open sftp file error :"<<libssh2_sftp_last_error(this->src_ssh2_sftp);
        return -1;
    }
    
    memset(&ssh2_sftp_attrib, 0, sizeof(ssh2_sftp_attrib));
    libssh2_sftp_fstat(sftp_handle, &ssh2_sftp_attrib);
    file_size = ssh2_sftp_attrib.filesize;
    qDebug()<<"remote file size :"<<file_size;
    emit this->transfer_got_file_size(file_size);

    //文件冲突检测
    if (QFile(destFile).exists()) {
        if (this->user_canceled) return 1;
        if (this->user_canceled) return 1;
        if (this->file_exist_over_write_method == OW_UNKNOWN
           || this->file_exist_over_write_method == OW_YES
           || this->file_exist_over_write_method == OW_RESUME
           || this->file_exist_over_write_method == OW_NO) {
            //TODO 通知用户远程文件已经存在，再做处理。                                                                          
            QString local_file_size, local_file_date;
            QString remote_file_size, remote_file_date;
            QFileInfo fi(destFile);
            local_file_size = QString("%1").arg(fi.size());
            local_file_date = fi.lastModified ().toString();
            remote_file_size = QString("%1").arg(ssh2_sftp_attrib.filesize);
            QDateTime remote_mtime;
            remote_mtime.setTime_t(ssh2_sftp_attrib.mtime);
            remote_file_date = remote_mtime.toString();
            emit this->dest_file_exists(destFile, remote_file_size, remote_file_date,
                                        srcFile, local_file_size, local_file_date);
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

    QFile q_file(destFile);
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
            if (this->user_canceled == true) {
                break;
            }
        }
        q_file.setPermissions(SSHFileInfo(ssh2_sftp_attrib).qMode());
        q_file.close();
    }
    
    libssh2_sftp_close(sftp_handle);

    return 0;
}

// similar to current do_nrsftp_exchange method
int Transportor::run_SFTP_to_SFTP()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    RemoteHostConnectThread *rhct = 0;

    int rv = -1;
    int transfer_ret = -1;
    //int debug_sleep_time = 5;
    
    TaskPackage src_atom_pkg;
    TaskPackage dest_atom_pkg;

    TaskPackage temp_src_atom_pkg;
    TaskPackage temp_dest_atom_pkg;
        
    QVector<QMap<char, QString> >  fileinfos;
    
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
            && remote_is_dir(this->dest_ssh2_sftp,this->current_dest_file_name ) ) {
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
                   && remote_is_dir(this->dest_ssh2_sftp,this->current_dest_file_name)) {
            qDebug()<<" nrsftp exchage file to dir...";
            QString dest_full_path = this->current_dest_file_name + "/" + this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1);
            // transfer_ret = this->do_nrsftp_exchange(this->current_src_file_name, dest_full_path);
                transfer_ret = this->run_SFTP_to_SFTP(this->current_src_file_name, dest_full_path);
        } else {
            //其他的情况暂时不考虑处理。跳过
            //TODO return a error value , not only error code
            q_debug()<<"src: "<< src_atom_pkg<<" dest:"<< dest_atom_pkg;
            this->error_code = 1;
            //assert ( 1 == 2 );
            qDebug()<<"Unexpected transfer type: "<<__FILE__<<" in "<< __LINE__;
        }
       
        this->transfer_ready_queue.erase(this->transfer_ready_queue.begin());
        this->transfer_done_queue.push_back(QPair<TaskPackage, TaskPackage>(src_atom_pkg, dest_atom_pkg));
    } while (this->transfer_ready_queue.size() > 0 && this->user_canceled == false);

    qDebug()<<" transfer_ret :"<< transfer_ret<<" ssh2 sftp shutdown:"<< this->src_ssh2_sftp<<" "<<this->dest_ssh2_sftp;
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
    if (this->user_canceled == true) {
        this->error_code = 3;
    }
    q_debug()<<"";
    return 0;
}
int Transportor::run_SFTP_to_SFTP(QString srcFile, QString destFile)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    qDebug()<<"nrsftp from: "<< srcFile <<" to "<<destFile;
    
    LIBSSH2_SFTP_HANDLE *src_sftp_handle = 0;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    LIBSSH2_SFTP_HANDLE *dest_sftp_handle = 0;
    char buff[5120] = {0};
    int rlen, wlen, tran_len = 0;
    int file_size = 0;
    int pcnt = 0;
    
    //检测传输文件的属性

    src_sftp_handle = libssh2_sftp_open(this->src_ssh2_sftp,
                                        gOpt->remote_codec->fromUnicode(srcFile).data(),
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
                                         gOpt->remote_codec->fromUnicode(destFile).data(),
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
        if (this->user_canceled == true) {
            break;
        }
    }

    return 0;
}

int Transportor::run_FILE_to_FTP()
{
    q_debug()<<"";
    int rv = -1;
    int transfer_ret = -1;
    //int debug_sleep_time = 5;
    
    TaskPackage src_atom_pkg;
    TaskPackage dest_atom_pkg;

    TaskPackage temp_src_atom_pkg;
    TaskPackage temp_dest_atom_pkg;
        
    QVector<QMap<char, QString> >  fileinfos;
    QVector<QUrlInfo> fileList;
    
    this->error_code = 0;
    this->errorString = QString(tr("No error."));
        
    do {
        src_atom_pkg = this->transfer_ready_queue.front().first;
        dest_atom_pkg = this->transfer_ready_queue.front().second;
        this->current_src_file_name = src_atom_pkg.files.at(0);
        this->current_dest_file_name = dest_atom_pkg.files.at(0);
 
        qDebug()<<this->current_src_file_name;
        qDebug()<<this->current_dest_file_name;
      
        //提示开始处理新文件：
        emit this->transfer_new_file_started(this->current_src_file_name);
            
        //连接到目录主机：
        if (this->dconn == 0) {
            qDebug()<<"connecting to dest ftp host:"<<dest_atom_pkg.username
                    <<":"<<dest_atom_pkg.password<<"@"<<dest_atom_pkg.host <<":"<<dest_atom_pkg.port ;
            emit  transfer_log("Connecting to destination host ...");
            this->dconn = new FTPConnection();
            this->dconn->setHostInfo(this->getHostInfo(dest_atom_pkg));
            rv = this->dconn->connect();
            q_debug()<<"LibFtp:"<<this->dconn->ftp;
            //TODO get status code and then ...
            if (rv != Connection::CONN_OK) {
                qDebug()<<"Connect to Host Error: "<<rv<<":"<<this->dconn->get_status_desc(rv);
                emit transfer_log("Connect Error: " + this->dconn->get_status_desc(rv));
                this->error_code = transfer_ret = 6;
                this->errorString = this->dconn->get_status_desc(rv);
                break;
            }
            emit  transfer_log("Connect done.");
        }

        // 将文件上传到目录
        if (QFileInfo(this->current_src_file_name).isFile()
            && this->isFTPDir(this->dconn, this->current_dest_file_name)
            ) {
            QString remote_full_path = this->current_dest_file_name + "/"
                + this->current_src_file_name.split ( "/" ).at ( this->current_src_file_name.split ( "/" ).count()-1 ) ;
            qDebug()<<"local file: "<<this->current_src_file_name
                    <<"remote file:"<<this->current_dest_file_name
                    <<"remote full file path: "<<remote_full_path ;

            transfer_ret = this->run_FILE_to_FTP(this->current_src_file_name, remote_full_path);
        } else if (QFileInfo(this->current_src_file_name).isDir()
                   && this->isFTPDir(this->dconn, this->current_dest_file_name)
                   ) {
            //将目录上传到目录
            qDebug()<<"uploding dir to dir ...";
            //this->sleep(debug_sleep_time);
            //
            //列出本地目录中的文件，加入到队列中，然后继续。
            //如果列出的本地文件是目录，则在这里先确定远程存在此子目录，否则就要创建先。
            fileinfos.clear();
            fxp_local_do_ls(this->current_src_file_name, fileinfos);
            qDebug()<<"ret:"<<transfer_ret<<" count:"<<fileinfos.size();

            //这个远程目录属性应该和本地属性一样，所以就使用this->current_src_file_type
            //不知道是不是有问题。
            QString remote_full_path = this->current_dest_file_name + "/" 
                + this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1);

            temp_dest_atom_pkg = dest_atom_pkg;
            temp_dest_atom_pkg.setFile(remote_full_path);

            //为远程建立目录
            rv = this->dconn->ftp->mkdir(GlobalOption::instance()->remote_codec->fromUnicode(remote_full_path));
            if (rv != 0) {
                assert(rv == 0);
                q_debug()<<"ftp mkdir error:";
                this->error_code = 1;
                break;
            }

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
            this->error_code = 1;
            //assert(1 == 2);
            qDebug()<<"Unexpected transfer type: "<<__FILE__<<" in " << __LINE__;
        }
       
        this->transfer_ready_queue.erase(this->transfer_ready_queue.begin());
        this->transfer_done_queue.push_back(QPair<TaskPackage, TaskPackage>(src_atom_pkg, dest_atom_pkg));
    } while (this->transfer_ready_queue.size() > 0 && this->user_canceled == false);

    qDebug()<<"transfer_ret :"<<transfer_ret<<" ssh2 sftp shutdown:"<<this->src_ssh2_sftp<<" "<<this->dest_ssh2_sftp;
    //TODO 选择性关闭 ssh2 会话，有可能是 src  ,也有可能是dest 
    if (this->user_canceled == true) {
        this->error_code = 3;
    }
    return 0;
}
int Transportor::run_FILE_to_FTP(QString srcFile, QString destFile)
{
    q_debug()<<"start :"<<srcFile<<" --> "<<destFile;
    int iret = -1;
    char buf[8192];
    int rlen, wlen, tran_len = 0;
    int file_size = 0;
    int pcnt = 0;

    // 首先转到当前目录为srfFile所在目录，取得文件名
    // FTP 上也转到相应的目录
    // 是在这转呢，还是在LibFtp->put中转呢?
    this->setLocalCurrentDirByFullPath(srcFile);
    this->setFTPCurrentDirByFullPath(this->dconn, destFile);

    iret = this->dconn->ftp->type(LibFtp::TYPE_BIN);
    assert(iret == 0);
    iret = this->dconn->ftp->passive();
    assert(iret == 0);

    iret = this->dconn->ftp->connectDataChannel();
    assert(iret == 0);

    QFileInfo fi(srcFile);
    file_size = fi.size();
    emit this->transfer_got_file_size(file_size);

    iret = this->dconn->ftp->put(fi.fileName());
    assert(iret == 0);

    QFile srcFp(srcFile);
    bool bret = srcFp.open(QIODevice::ReadOnly);
    assert(bret);
    
    QTcpSocket *dsock = this->dconn->ftp->getDataSocket();
    assert(dsock != NULL);

    for (;;) {
        rlen = srcFp.read(buf, sizeof(buf));
        if (rlen <= 0) {
            qDebug()<<"may be read end "<<rlen;
            break;
        }
        wlen = dsock->write(buf, rlen);
        dsock->waitForBytesWritten();
        assert(wlen == rlen);
        tran_len += wlen;

        if (file_size == 0) {
            emit this->transfer_percent_changed(100, tran_len, wlen);
        } else {
            pcnt = 100.0 *((double)tran_len / (double)file_size);
            emit this->transfer_percent_changed(pcnt, tran_len, wlen);
        }
        if (this->user_canceled == true) {
            break;
        }
    }
    this->dconn->ftp->closeDataChannel();
    srcFp.close();

    // 是不是现在应该去读取ftp的ctrl socket的剩余信息了呢
    iret = this->dconn->ftp->swallowResponse();
    
    return 0;
}

int Transportor::setLocalCurrentDirByFullPath(QString path)
{
    QDir dir = QFileInfo(path).absoluteDir();
    dir.cd(dir.dirName());
    return 0;
}

int Transportor::setFTPCurrentDirByFullPath(Connection *conn, QString path)
{
    QDir dir = QFileInfo(path).absoluteDir();
    int iret = conn->ftp->chdir(dir.path());
    QString currDir;

    iret = conn->ftp->pwd(currDir);
    if (currDir == dir.path()) {
        q_debug()<<"chdir ok to:"<<dir.path();
    } else {
        q_debug()<<"chdir error to:"<<dir.path();
    }

    return 0;
}

int Transportor::run_FTP_to_FILE()
{
    q_debug()<<"";
    int rv = -1;
    int transfer_ret = -1;
    //int debug_sleep_time = 5;
    
    TaskPackage src_atom_pkg;
    TaskPackage dest_atom_pkg;

    TaskPackage temp_src_atom_pkg;
    TaskPackage temp_dest_atom_pkg;
        
    QVector<QUrlInfo> fileList;
    
    this->error_code = 0;
    this->errorString = QString(tr("No error."));
        
    do {
        src_atom_pkg = this->transfer_ready_queue.front().first;
        dest_atom_pkg = this->transfer_ready_queue.front().second;
        this->current_src_file_name = src_atom_pkg.files.at(0);
        this->current_dest_file_name = dest_atom_pkg.files.at(0);
 
        qDebug()<<this->current_src_file_name;
        qDebug()<<this->current_dest_file_name;
      
        //提示开始处理新文件：
        emit this->transfer_new_file_started(this->current_src_file_name);

        //连接到目录主机：
        if (this->sconn == 0) {
            qDebug()<<"connecting to src ftp host:"<<src_atom_pkg.username<<":"<<src_atom_pkg.password
                    <<"@"<<src_atom_pkg.host <<":"<<src_atom_pkg.port ;
            
            emit transfer_log("Connecting to source host ...");

            this->sconn = new FTPConnection();
            this->sconn->setHostInfo(this->getHostInfo(src_atom_pkg));
            rv = this->sconn->connect();
            q_debug()<<"LibFtp:"<<this->sconn->ftp;
            if (rv != Connection::CONN_OK) {
                qDebug()<<"Connect to Host Error: "<<rv<<":"<<this->sconn->get_status_desc(rv);
                emit transfer_log("Connect Error: " + this->sconn->get_status_desc(rv));
                this->error_code = transfer_ret = 6;
                this->errorString = this->sconn->get_status_desc(rv);
                break;
            }

            emit transfer_log("Connect done.");
        }
        //将文件下载到目录
        if (this->isFTPFile(this->sconn, this->current_src_file_name)
            && QFileInfo(this->current_dest_file_name).isDir()) {
                QString local_full_path = this->current_dest_file_name + "/"
                    + this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1);

                qDebug()<<"local file: " << this->current_src_file_name
                        <<"remote file:" << this->current_dest_file_name
                        <<"local full file path: "<< local_full_path;

                transfer_ret = this->run_FTP_to_FILE(this->current_src_file_name, local_full_path);
        }
        //将目录下载到目录
        else if (QFileInfo(this->current_dest_file_name).isDir()
                 && this->isFTPDir(this->sconn, this->current_src_file_name)
                 ) {
            qDebug()<<"downloading dir to dir ...";
            //this->sleep(debug_sleep_time);
            //列出本远程目录中的文件，加入到队列中，然后继续。
            // local dir = curr local dir +  curr remote dir 的最后一层目录
            fileList.clear();
            transfer_ret = this->sconn->ftp->passive();
            transfer_ret = this->sconn->ftp->connectDataChannel();
            transfer_ret = this->sconn->ftp->list(this->current_src_file_name + "/");
            assert(transfer_ret == 0);
            
            temp_dest_atom_pkg = dest_atom_pkg;
            temp_dest_atom_pkg.setFile(this->current_dest_file_name + "/" +
                                       this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1));
            
            //确保本地有这个目录。
            transfer_ret = QDir().mkpath(temp_dest_atom_pkg.files.at(0));
            qDebug()<<"fxp_local_do_mkdir: "<<transfer_ret <<" "<< temp_dest_atom_pkg.files.at(0);
            //加入到任务队列
            for (int i = 0; i < fileList.count(); i++) {
                temp_src_atom_pkg = src_atom_pkg;
                temp_src_atom_pkg.setFile(this->current_src_file_name + "/" + fileList.at(i).name());
                
                this->transfer_ready_queue.push_back(QPair<TaskPackage, TaskPackage>
                                                     (temp_src_atom_pkg, temp_dest_atom_pkg));
                //romote is source 
            }
        } else {
            //其他的情况暂时不考虑处理。跳过。
            //TODO return a error value , not only error code 
            this->error_code = 1;
            //assert( 1 == 2 ); 
            qDebug()<<"Unexpected transfer type: "<<__FILE__<<" in "<<__LINE__;
        }
       
        this->transfer_ready_queue.erase(this->transfer_ready_queue.begin());
        this->transfer_done_queue.push_back(QPair<TaskPackage, TaskPackage>(src_atom_pkg, dest_atom_pkg));
    } while (this->transfer_ready_queue.size() > 0 && this->user_canceled == false);

    qDebug()<<"transfer_ret :"<<transfer_ret<<"ssh2 sftp shutdown:"<<this->src_ssh2_sftp<<" "<<this->dest_ssh2_sftp;
    //TODO 选择性关闭 ssh2 会话，有可能是 src  ,也有可能是dest 
    if (this->user_canceled == true) {
        this->error_code = 3;
    }
    return 0;
}

int Transportor::run_FTP_to_FILE(QString srcFile, QString destFile)
{
    q_debug()<<"start :"<<srcFile<<" --> "<<destFile;
    
    int iret = -1;
    char buf[8192];
    int rlen, wlen, tran_len = 0;
    quint64 file_size = 0;
    int pcnt = 0;

    // 首先转到当前目录为srfFile所在目录，取得文件名
    // FTP 上也转到相应的目录
    // 是在这转呢，还是在LibFtp->put中转呢?
    this->setLocalCurrentDirByFullPath(destFile);
    this->setFTPCurrentDirByFullPath(this->sconn, srcFile);

    {
        iret = this->sconn->ftp->type(LibFtp::TYPE_BIN);
        assert(iret == 0);

        // 获取源文件大小, 必须在BIN模式下才能调用
        iret = this->sconn->ftp->size(srcFile, file_size);
        assert(iret == 0);
        emit this->transfer_got_file_size(file_size);

        iret = this->sconn->ftp->passive();
        assert(iret == 0);

        iret = this->sconn->ftp->connectDataChannel();
        assert(iret == 0);

        QFileInfo fi(srcFile);
        iret = this->sconn->ftp->get(fi.fileName());
        assert(iret == 0);
    }

    QFile destFp(destFile);
    bool bret = destFp.open(QIODevice::ReadWrite);
    assert(bret);
    
    QTcpSocket *dsock = this->sconn->ftp->getDataSocket();
    assert(dsock != NULL);

    for (;;) {
        dsock->waitForReadyRead();
        rlen = dsock->read(buf, sizeof(buf));
        if (rlen <= 0) {
            qDebug()<<" may be read end "<<rlen;
            break;
        }
        wlen = destFp.write(buf, rlen);
        destFp.waitForBytesWritten(-1);
        assert(wlen == rlen);

        tran_len += wlen;

        if (file_size == 0) {
            emit this->transfer_percent_changed(100, tran_len, wlen);
        } else {
            pcnt = 100.0 *((double)tran_len / (double)file_size);
            emit this->transfer_percent_changed(pcnt, tran_len, wlen);
        }
        if (this->user_canceled == true) {
            break;
        }
    }

    this->sconn->ftp->closeDataChannel();
    destFp.close();

    // 是不是现在应该去读取ftp的ctrl socket的剩余信息了呢
    iret = this->sconn->ftp->swallowResponse();
    
    return 0;
}

int Transportor::run_FTP_to_FTP()        // 负责根据情况调用下面的两种方式进行文件传输
{
    q_debug()<<"";
    int rv = -1;
    int transfer_ret = -1;
    //int debug_sleep_time = 5;
    
    TaskPackage src_atom_pkg;
    TaskPackage dest_atom_pkg;

    TaskPackage temp_src_atom_pkg;
    TaskPackage temp_dest_atom_pkg;
        
    QVector<QUrlInfo> fileList;
    
    this->error_code = 0 ;
    this->errorString = QString(tr("No error."));
        
    do {
        src_atom_pkg = this->transfer_ready_queue.front().first;
        dest_atom_pkg = this->transfer_ready_queue.front().second;
        this->current_src_file_name = src_atom_pkg.files.at(0);
        this->current_dest_file_name = dest_atom_pkg.files.at(0);
 
        qDebug()<<this->current_src_file_name;
        qDebug()<<this->current_dest_file_name;
      
        //提示开始处理新文件：
        emit this->transfer_new_file_started(this->current_src_file_name);

        //处理nrftp协议
        //连接到目的主机：
        if (this->dconn == 0) {
            qDebug()<<"connecting to dest ftp host:"<<dest_atom_pkg.username
                    <<":"<<dest_atom_pkg.password<<"@"<<dest_atom_pkg.host <<":"<<dest_atom_pkg.port ;
            emit  transfer_log("Connecting to destination host ...");
            this->dconn = new FTPConnection();
            this->dconn->setHostInfo(this->getHostInfo(dest_atom_pkg));
            rv = this->dconn->connect();
            q_debug()<<"LibFtp:"<<this->dconn->ftp;

            if (rv != Connection::CONN_OK) {
                qDebug()<<"Connect to Host Error: "<<rv<<":"<<this->dconn->get_status_desc(rv);
                emit transfer_log("Connect Error: " + this->dconn->get_status_desc(rv));
                this->error_code = transfer_ret = 6;
                this->errorString = this->dconn->get_status_desc(rv);
                break;
            }
            //get connect return code end
            emit  transfer_log("Connect done.");
        }

        // 连接到源FTP主机
        if (this->sconn == 0) {
            qDebug()<<"connecting to src ftp host:"<<src_atom_pkg.username<<":"<<src_atom_pkg.password
                    <<"@"<<src_atom_pkg.host <<":"<<src_atom_pkg.port ;
            
            emit transfer_log("Connecting to source host ...");

            this->sconn = new FTPConnection();
            this->sconn->setHostInfo(this->getHostInfo(src_atom_pkg));
            rv = this->sconn->connect();
            q_debug()<<"LibFtp:"<<this->sconn->ftp;
            if (rv != Connection::CONN_OK) {
                qDebug()<<"Connect to Host Error: "<<rv<<":"<<this->sconn->get_status_desc(rv);
                emit transfer_log("Connect Error: " + this->sconn->get_status_desc(rv));
                this->error_code = transfer_ret = 6;
                this->errorString = this->sconn->get_status_desc(rv);
                break;
            }

            emit transfer_log("Connect done.");
        }

        ////////////
        if (
            this->isFTPDir(this->sconn, this->current_src_file_name)
            && this->isFTPDir(this->dconn, this->current_dest_file_name)
            ) {
            qDebug()<<"nrftp exchage dir to dir...";

            fileList.clear();
            transfer_ret = this->sconn->ftp->list(this->current_src_file_name + "/");
            assert(transfer_ret == 0);
            
            temp_dest_atom_pkg = dest_atom_pkg;
            temp_dest_atom_pkg.setFile(this->current_dest_file_name + "/" +
                                       this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1));
            
            //确保目标FTP上有这个目录。
            rv = this->dconn->ftp->mkdir(temp_dest_atom_pkg.files.at(0));
            qDebug()<<"fxp_dest ftp_do_mkdir: "<<transfer_ret <<" "<< temp_dest_atom_pkg.files.at(0);
            if (rv != 0) {
                assert(rv == 0);
                q_debug()<<"ftp mkdir error:";
                this->error_code = 1;
                break;
            }

            //加入到任务队列           
            for (int i = 0; i < fileList.count(); i++) {
                temp_src_atom_pkg = src_atom_pkg;
                temp_src_atom_pkg.setFile(this->current_src_file_name + "/" + fileList.at(i).name());
                
                this->transfer_ready_queue.push_back(QPair<TaskPackage, TaskPackage>
                                                     (temp_src_atom_pkg, temp_dest_atom_pkg));
                //romote is source 
            }
        } else if (
                   this->isFTPFile(this->sconn, this->current_src_file_name)
                   && this->isFTPDir(this->dconn, this->current_dest_file_name)
                   ) {
            qDebug()<<" nrsftp exchage file to dir...";
            QString dest_full_path = this->current_dest_file_name + "/" + this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1);
            if (0) {
                transfer_ret = this->run_FTP_to_FTP_relay(this->current_src_file_name, dest_full_path);
            } else {
                transfer_ret = this->run_FTP_to_FTP_fxp(this->current_src_file_name, dest_full_path);
            }
        } else {
            //其他的情况暂时不考虑处理。跳过
            //TODO return a error value , not only error code
            q_debug()<<"src: "<< src_atom_pkg<<" dest:"<<dest_atom_pkg;
            this->error_code = 1;
            //assert(1 == 2);
            qDebug()<<"Unexpected transfer type: "<<__FILE__<<" in "<<__LINE__;
        }
       
        this->transfer_ready_queue.erase(this->transfer_ready_queue.begin());
        this->transfer_done_queue.push_back(QPair<TaskPackage, TaskPackage>(src_atom_pkg, dest_atom_pkg));
    } while (this->transfer_ready_queue.size() > 0 && this->user_canceled == false);

    qDebug()<<"transfer_ret :"<<transfer_ret<<"ssh2 sftp shutdown:"<< this->src_ssh2_sftp<<" "<<this->dest_ssh2_sftp;
    //TODO 选择性关闭 ssh2 会话，有可能是 src  ,也有可能是dest 
    if (this->user_canceled == true) {
        this->error_code = 3;
    }
    return 0;
}

int Transportor::run_FTP_to_FTP_relay(QString srcFile, QString destFile) // 通过中继方式传数据, 不需要服务器支持。
{
    q_debug()<<"start :"<<srcFile<<" --> "<<destFile;
    
    int iret = -1;
    char buf[8192];
    int rlen, wlen, tran_len = 0;
    quint64 file_size = 0;
    int pcnt = 0;
    QTcpSocket *srcDataSock = NULL, *destDataSock = NULL;

    // FTP 上转到相应的当前工作目录
    this->setFTPCurrentDirByFullPath(this->sconn, srcFile);
    this->setFTPCurrentDirByFullPath(this->dconn, destFile);

    // src cmd sequence
    {
        iret = this->sconn->ftp->type(LibFtp::TYPE_BIN);
        assert(iret == 0);

        // 获取源文件大小, 必须在BIN模式下才能调用
        iret = this->sconn->ftp->size(srcFile, file_size);
        assert(iret == 0);
        emit this->transfer_got_file_size(file_size);

        iret = this->sconn->ftp->passive();
        assert(iret == 0);

        iret = this->sconn->ftp->connectDataChannel();
        assert(iret == 0);

        QFileInfo fi(srcFile);
        iret = this->sconn->ftp->get(fi.fileName());
        assert(iret == 0);

        srcDataSock = this->sconn->ftp->getDataSocket();
        assert(srcDataSock != NULL);
    }

    // dest cmd sequence
    {
        iret = this->dconn->ftp->type(LibFtp::TYPE_BIN);
        assert(iret == 0);
        iret = this->dconn->ftp->passive();
        assert(iret == 0);

        iret = this->dconn->ftp->connectDataChannel();
        assert(iret == 0);

        QFileInfo fi(srcFile);
        iret = this->dconn->ftp->put(fi.fileName());
        assert(iret == 0);
    
        destDataSock = this->dconn->ftp->getDataSocket();
        assert(destDataSock != NULL);
    }

    for (;;) {
        srcDataSock->waitForReadyRead();
        rlen = srcDataSock->read(buf, sizeof(buf));
        if (rlen <= 0) {
            qDebug()<<" may be read end "<<rlen;
            break;
        }
        wlen = destDataSock->write(buf, rlen);
        destDataSock->waitForBytesWritten();
        assert(wlen == rlen);

        tran_len += wlen;

        if (file_size == 0) {
            emit this->transfer_percent_changed(100, tran_len, wlen);
        } else {
            pcnt = 100.0 *((double)tran_len / (double)file_size);
            emit this->transfer_percent_changed(pcnt, tran_len, wlen);
        }
        if (this->user_canceled == true) {
            break;
        }
    }

    iret = this->sconn->ftp->closeDataChannel();
    iret = this->dconn->ftp->closeDataChannel();

    // 是不是现在应该去读取ftp的ctrl socket的剩余信息了呢
    iret = this->sconn->ftp->swallowResponse();
    iret = this->dconn->ftp->swallowResponse();
   
    return 0;
}

// http://en.wikipedia.org/wiki/File_eXchange_Protocol
int Transportor::run_FTP_to_FTP_fxp(QString srcFile, QString destFile)   // 通过FTP协议中的FXP方式传数据，需要服务器支持。
{
    q_debug()<<"start :"<<srcFile<<" --> "<<destFile;
    
    quint16 pasvPort = 0;
    QString pasvHost;
    int iret = -1;
    int rlen, wlen, tran_len = 0;
    quint64 file_size = 0;
    int pcnt = 0;

    // FTP 上转到相应的当前工作目录
    this->setFTPCurrentDirByFullPath(this->sconn, srcFile);
    this->setFTPCurrentDirByFullPath(this->dconn, destFile);

    // dest cmd sequence
    {
        iret = this->dconn->ftp->type(LibFtp::TYPE_BIN);
        assert(iret == 0);
        iret = this->dconn->ftp->passive();
        assert(iret == 0);

        pasvPort = this->dconn->ftp->pasvPeer(pasvHost);
    }

    // src cmd sequence
    {
        iret = this->sconn->ftp->type(LibFtp::TYPE_BIN);
        assert(iret == 0);

        // 获取源文件大小, 必须在BIN模式下才能调用
        iret = this->sconn->ftp->size(srcFile, file_size);
        assert(iret == 0);
        emit this->transfer_got_file_size(file_size);

        iret = this->sconn->ftp->portNoWaitResponse(pasvHost, pasvPort);
        // assert(iret == 0);
        if (iret != 0) {
            // 是可能服务器不支持port命令, 关闭目标ftp的passive连接
            // iret = this->dconn->ftp->connectDataChannel();
            // assert(iret == 0);
            iret = this->dconn->ftp->closeDataChannel();
            q_debug()<<"maybe the source ftp do not suppert port command.";
            return -1;
        }
    }

    // dest 
    {
        QFileInfo fi(srcFile);
        iret = this->dconn->ftp->putNoWaitResponse(fi.fileName());
        assert(iret == 0);
    }

    // src 
    {
        QFileInfo fi(srcFile);
        iret = this->sconn->ftp->getNoWaitResponse(fi.fileName());
        assert(iret == 0);        
    }
    // TODO fxp数据传输进度计算。
    // 是不是现在应该去读取ftp的ctrl socket的剩余信息了呢
    iret = this->sconn->ftp->swallowResponse(); // src port ok
    iret = this->dconn->ftp->swallowResponse(); // dest bin mode ready
    iret = this->sconn->ftp->swallowResponse(); // src bin mode ready
    // 这从开始，等待fxp数据传输完成，之后源和目的FTP都会返回，226 Transfer complete
    
    iret = this->sconn->ftp->waitForCtrlResponse();
    iret = this->dconn->ftp->waitForCtrlResponse();

    emit this->transfer_percent_changed(100, file_size, file_size);

    // 这儿应该有什么方法检测到两个服务器的传输进度及完成状态。
    q_debug()<<"fxp transport done.";

    return 0;
}

int Transportor::run_SFTP_to_FTP()
{
    q_debug()<<"";
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    RemoteHostConnectThread *rhct = 0;

    int rv = -1;
    int transfer_ret = -1 ;
    //int debug_sleep_time = 5 ;
    
    TaskPackage src_atom_pkg;
    TaskPackage dest_atom_pkg;

    TaskPackage temp_src_atom_pkg;
    TaskPackage temp_dest_atom_pkg;
        
    QVector<QMap<char, QString> >  fileinfos;
    QVector<QUrlInfo> fileList;
    
    this->error_code = 0;
    this->errorString = QString(tr("No error."));
        
    do {
        src_atom_pkg = this->transfer_ready_queue.front().first;
        dest_atom_pkg = this->transfer_ready_queue.front().second;
        this->current_src_file_name = src_atom_pkg.files.at(0);
        this->current_dest_file_name = dest_atom_pkg.files.at(0);
 
        qDebug()<<this->current_src_file_name;
        qDebug()<<this->current_dest_file_name;
      
        //提示开始处理新文件：
        emit this->transfer_new_file_started(this->current_src_file_name);

        //处理nrftp协议
        // 连接到源SFTP主机
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

        //连接到目的FTP主机：
        if (this->dconn == 0) {
            qDebug()<<"connecting to dest ftp host:"<<dest_atom_pkg.username
                    <<":"<<dest_atom_pkg.password<<"@"<<dest_atom_pkg.host <<":"<<dest_atom_pkg.port ;
            emit  transfer_log("Connecting to destination host ...");
            this->dconn = new FTPConnection();
            this->dconn->setHostInfo(this->getHostInfo(dest_atom_pkg));
            rv = this->dconn->connect();
            q_debug()<<"LibFtp:"<<this->dconn->ftp;

            if (rv != Connection::CONN_OK) {
                qDebug()<<"Connect to Host Error: "<<rv<<":"<<this->dconn->get_status_desc(rv);
                emit transfer_log("Connect Error: " + this->dconn->get_status_desc(rv));
                this->error_code = transfer_ret = 6;
                this->errorString = this->dconn->get_status_desc(rv);
                break;
            }
            //get connect return code end
            emit  transfer_log("Connect done.");
        }

        ////////////
        if (
            remote_is_dir(this->src_ssh2_sftp, this->current_src_file_name) 
            // && remote_is_dir(this->dest_ssh2_sftp,this->current_dest_file_name )
            && this->isFTPDir(this->dconn, this->current_dest_file_name)
            ) {
            qDebug()<<"sftp -> ftp exchage dir to dir...";

            fileinfos.clear();
            transfer_ret = fxp_do_ls_dir(this->src_ssh2_sftp, this->current_src_file_name + "/", fileinfos);
            qDebug()<<"ret:"<<transfer_ret<<" file count:"<<fileinfos.size();
            
            temp_dest_atom_pkg = dest_atom_pkg;
            temp_dest_atom_pkg.setFile(this->current_dest_file_name + "/" +
                                       this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1));
            
            //确保目标FTP上有这个目录。
            rv = this->dconn->ftp->mkdir(temp_dest_atom_pkg.files.at(0));
            assert(rv == 0);
            qDebug()<<"ftp_dest ftp_do_mkdir: "<<transfer_ret <<" "<< temp_dest_atom_pkg.files.at(0) ;

            //加入到任务队列           
            for (int i = 0 ; i < fileinfos.size() ; i ++) {
                temp_src_atom_pkg = src_atom_pkg;
                temp_src_atom_pkg.setFile(this->current_src_file_name + "/" + fileinfos.at(i)['N']);
                this->transfer_ready_queue.push_back(QPair<TaskPackage, TaskPackage>
                                                     (temp_src_atom_pkg, temp_dest_atom_pkg));
            }
        } else if (
                   remote_is_reg(this->src_ssh2_sftp, this->current_src_file_name) 
                   && this->isFTPDir(this->dconn, this->current_dest_file_name)
                   ) {
            qDebug()<<" nrsftp sftp -> ftp exchage file to dir...";
            QString dest_full_path = this->current_dest_file_name + "/" + this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1);
            transfer_ret = this->run_SFTP_to_FTP(this->current_src_file_name, dest_full_path);
        } else {
            //其他的情况暂时不考虑处理。跳过
            //TODO return a error value , not only error code
            q_debug()<<"src: "<< src_atom_pkg<<" dest:"<< dest_atom_pkg;
            this->error_code = 1;
            //assert ( 1 == 2 );
            qDebug()<<"Unexpected transfer type: "<<__FILE__<<" in "<<__LINE__;
        }
       
        this->transfer_ready_queue.erase(this->transfer_ready_queue.begin());
        this->transfer_done_queue.push_back(QPair<TaskPackage, TaskPackage>(src_atom_pkg, dest_atom_pkg));
    } while (this->transfer_ready_queue.size() > 0 && this->user_canceled == false);

    qDebug()<<"transfer_ret :"<<transfer_ret<<"ssh2 sftp shutdown:"<<this->src_ssh2_sftp<<" "<<this->dest_ssh2_sftp;
    //TODO 选择性关闭 ssh2 会话，有可能是 src  ,也有可能是dest 
    if (this->user_canceled == true) {
        this->error_code = 3;
    }
    return 0;
}
int Transportor::run_SFTP_to_FTP(QString srcFile, QString destFile)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    qDebug()<<"remote_path = "<<destFile<<" , local_path = "<<srcFile;

    int iret = 0;
    int pcnt = 0 ;
    int rlen, wlen;
    int file_size, tran_len = 0;
    LIBSSH2_SFTP_HANDLE *sftp_handle ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    char buff[8192] = {0};
    QTcpSocket *destDataSock = NULL;
    
    sftp_handle = libssh2_sftp_open(this->src_ssh2_sftp, gOpt->remote_codec->fromUnicode(srcFile), LIBSSH2_FXF_READ, 0);
    if (sftp_handle == NULL) {
        //TODO 错误消息通知用户。
        qDebug()<<"open sftp file error :"<<libssh2_sftp_last_error(this->src_ssh2_sftp);
        return -1;
    }
    
    memset(&ssh2_sftp_attrib, 0, sizeof(ssh2_sftp_attrib));
    libssh2_sftp_fstat(sftp_handle, &ssh2_sftp_attrib);
    file_size = ssh2_sftp_attrib.filesize;
    qDebug()<<"remote file size :"<<file_size;
    emit this->transfer_got_file_size(file_size);

    // FTP 上转到相应的当前工作目录
    this->setFTPCurrentDirByFullPath(this->dconn, destFile);

    // dest cmd sequence
    {
        iret = this->dconn->ftp->type(LibFtp::TYPE_BIN);
        assert(iret == 0);
        iret = this->dconn->ftp->passive();
        assert(iret == 0);

        iret = this->dconn->ftp->connectDataChannel();
        assert(iret == 0);

        QFileInfo fi(srcFile);
        iret = this->dconn->ftp->put(fi.fileName());
        assert(iret == 0);
    
        destDataSock = this->dconn->ftp->getDataSocket();
        assert(destDataSock != NULL);
    }

    //read remote file  and then write to local file
    while ((rlen = libssh2_sftp_read(sftp_handle, buff, sizeof(buff))) > 0) {
        wlen = destDataSock->write(buff, rlen);
        tran_len += wlen;
        //qDebug() <<" read len :"<< rlen <<" , write len: "<< wlen                     << " tran len: "<< tran_len ;
        //my progress signal
        if (file_size == 0) {
            emit this->transfer_percent_changed(100, tran_len, wlen);
        } else {
            pcnt = 100.0 *((double)tran_len / (double)file_size);
            emit this->transfer_percent_changed(pcnt, tran_len, wlen);
        }
        destDataSock->waitForBytesWritten();
        if (this->user_canceled == true) {
            break;
        }
    }

    iret = this->dconn->ftp->closeDataChannel();
    // 是不是现在应该去读取ftp的ctrl socket的剩余信息了呢
    iret = this->dconn->ftp->swallowResponse();
   
    libssh2_sftp_close(sftp_handle);

    return 0;
}

int Transportor::run_FTP_to_SFTP()
{
    q_debug()<<"";
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    RemoteHostConnectThread *rhct = 0;

    int rv = -1;
    int transfer_ret = -1 ;
    //int debug_sleep_time = 5 ;
    
    TaskPackage src_atom_pkg;
    TaskPackage dest_atom_pkg;

    TaskPackage temp_src_atom_pkg;
    TaskPackage temp_dest_atom_pkg;
        
    QVector<QUrlInfo> fileList;
    
    this->error_code = 0;
    this->errorString = QString(tr("No error."));
        
    do {
        src_atom_pkg = this->transfer_ready_queue.front().first;
        dest_atom_pkg = this->transfer_ready_queue.front().second;
        this->current_src_file_name = src_atom_pkg.files.at(0);
        this->current_dest_file_name = dest_atom_pkg.files.at(0);
 
        qDebug()<<this->current_src_file_name;
        qDebug()<<this->current_dest_file_name;
      
        //提示开始处理新文件：
        emit this->transfer_new_file_started(this->current_src_file_name);

        //连接到源主机：
        if (this->sconn == 0) {
            qDebug()<<"connecting to src ftp host:"<<src_atom_pkg.username<<":"<<src_atom_pkg.password
                    <<"@"<<src_atom_pkg.host <<":"<<src_atom_pkg.port;
            
            emit transfer_log("Connecting to source host ...");

            this->sconn = new FTPConnection();
            this->sconn->setHostInfo(this->getHostInfo(src_atom_pkg));
            rv = this->sconn->connect();
            q_debug()<<"LibFtp:"<<this->sconn->ftp;
            if (rv != Connection::CONN_OK) {
                qDebug()<<"Connect to Host Error: "<<rv<<":"<<this->sconn->get_status_desc(rv);
                emit transfer_log("Connect Error: " + this->sconn->get_status_desc(rv));
                this->error_code = transfer_ret = 6;
                this->errorString = this->sconn->get_status_desc(rv);
                break;
            }

            emit transfer_log("Connect done.");
        }

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

        //将文件传到目录
        if (this->isFTPFile(this->sconn, this->current_src_file_name)
            && remote_is_dir(this->dest_ssh2_sftp, this->current_dest_file_name) 
            ) {
            QString local_full_path = this->current_dest_file_name + "/"
                + this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1);
            
            qDebug()<<"local file: "<<this->current_src_file_name
                    <<"remote file:"<<this->current_dest_file_name
                    <<"local full file path: "<<local_full_path;
            
            transfer_ret = this->run_FTP_to_SFTP(this->current_src_file_name, local_full_path);
        }
        //将目录传到目录
        else if (
                 this->isFTPDir(this->sconn, this->current_src_file_name)
                 && remote_is_dir(this->dest_ssh2_sftp, this->current_dest_file_name)
                 ) {
            qDebug()<<"downloading dir to dir ...";
            //this->sleep(debug_sleep_time);
            //列出本远程目录中的文件，加入到队列中，然后继续。
            // local dir = curr local dir +  curr remote dir 的最后一层目录
            fileList.clear();
            transfer_ret = this->sconn->ftp->list(this->current_src_file_name + "/");
            assert(transfer_ret == 0);
            
            temp_dest_atom_pkg = dest_atom_pkg;
            temp_dest_atom_pkg.setFile(this->current_dest_file_name + "/" +
                                       this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1));
            
            //确保sftp上有这个目录。
            memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
            transfer_ret = libssh2_sftp_mkdir(this->dest_ssh2_sftp, 
                                              GlobalOption::instance()->remote_codec->fromUnicode(temp_dest_atom_pkg.files.at(0)), 
                                              0755);
            qDebug()<<"fxp_local_do_mkdir: "<<transfer_ret <<" "<< temp_dest_atom_pkg.files.at(0);

            //加入到任务队列           
            for (int i = 0; i < fileList.count(); i++) {
                temp_src_atom_pkg = src_atom_pkg;
                temp_src_atom_pkg.setFile(this->current_src_file_name + "/" + fileList.at(i).name());
                
                this->transfer_ready_queue.push_back(QPair<TaskPackage, TaskPackage>
                                                     (temp_src_atom_pkg, temp_dest_atom_pkg));
                //romote is source 
            }
        } else {
            //其他的情况暂时不考虑处理。跳过。
            //TODO return a error value , not only error code 
            this->error_code = 1;
            //assert( 1 == 2 ); 
            qDebug()<<"Unexpected transfer type:"<<__FILE__<<" in "<<__LINE__;
        }
       
        this->transfer_ready_queue.erase(this->transfer_ready_queue.begin());
        this->transfer_done_queue.push_back(QPair<TaskPackage, TaskPackage>(src_atom_pkg, dest_atom_pkg));
    } while (this->transfer_ready_queue.size() > 0 && this->user_canceled == false);

    qDebug()<<"transfer_ret :"<<transfer_ret<<"ssh2 sftp shutdown:"<<this->src_ssh2_sftp<<" "<<this->dest_ssh2_sftp;
    //TODO 选择性关闭 ssh2 会话，有可能是 src  ,也有可能是dest 
    if (user_canceled == true) {
        this->error_code = 3;
    }
    return 0;    
}
int Transportor::run_FTP_to_SFTP(QString srcFile, QString destFile)
{
    q_debug()<<"start :"<<srcFile<<" --> "<<destFile;
    
    int iret = -1;
    QTcpSocket *srcDataSock = NULL;
    int pcnt = 0;
    int rlen, wlen;
    quint64 file_size, tran_len = 0;
    LIBSSH2_SFTP_HANDLE *sftp_handle;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    char buff[5120] = {0};

    // FTP 上转到相应的当前工作目录
    this->setFTPCurrentDirByFullPath(this->sconn, srcFile);

    // src cmd sequence
    {
        iret = this->sconn->ftp->type(LibFtp::TYPE_BIN);
        assert(iret == 0);

        // 获取源文件大小, 必须在BIN模式下才能调用
        iret = this->sconn->ftp->size(srcFile, file_size);
        assert(iret == 0);
        emit this->transfer_got_file_size(file_size);

        iret = this->sconn->ftp->passive();
        assert(iret == 0);

        iret = this->sconn->ftp->connectDataChannel();
        assert(iret == 0);

        QFileInfo fi(srcFile);
        iret = this->sconn->ftp->get(fi.fileName());
        assert(iret == 0);

        srcDataSock = this->sconn->ftp->getDataSocket();
        assert(srcDataSock != NULL);
    }

    QFileInfo fi = QFileInfo(srcFile);
    SSHFileInfo sfi = SSHFileInfo::fromQFileInfo(fi);
    //TODO 检查文件可写属性
    sftp_handle = libssh2_sftp_open(this->dest_ssh2_sftp,
                                    gOpt->remote_codec->fromUnicode(destFile),
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
	
        return -1;
    }
    
    memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
    
    //read ftp file and then write to sftp file
    srcDataSock->waitForReadyRead();
    while ((rlen = srcDataSock->read(buff, sizeof(buff))) > 0) {
        wlen = libssh2_sftp_write(sftp_handle, buff, rlen);
        Q_ASSERT(wlen == rlen);
        if (wlen < rlen) {
            q_debug()<<"write to server less then need write bytes";
            // TODO 这种情况应该尝试再次写入剩余的数据
        }
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
        srcDataSock->waitForReadyRead();
        if (this->user_canceled == true) {
            break;
        }
    }
    qDebug()<<"out cycle, close sftp...";
    libssh2_sftp_close(sftp_handle);

    iret = this->sconn->ftp->closeDataChannel();
    // 是不是现在应该去读取ftp的ctrl socket的剩余信息了呢
    iret = this->sconn->ftp->swallowResponse();
    
    return 0;
}

/**
 * 
 */
// void Transportor::run_backup()
// {
//     qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

//     LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
//     RemoteHostConnectThread *rhct = 0 ;

//     int rv = -1;
//     int transfer_ret = -1 ;
//     //int debug_sleep_time = 5 ;
    
//     TaskPackage src_atom_pkg;
//     TaskPackage dest_atom_pkg;

//     TaskPackage temp_src_atom_pkg;
//     TaskPackage temp_dest_atom_pkg;
        
//     QVector<QMap<char, QString> >  fileinfos ;
    
//     this->error_code = 0 ;
//     this->errorString = QString(tr("No error."));
    
//     this->dest_ssh2_sess = 0 ;
//     this->dest_ssh2_sftp = 0 ;
//     this->dest_ssh2_sock = 0 ;
        
//     this->src_ssh2_sess = 0 ;
//     this->src_ssh2_sftp = 0 ;
//     this->src_ssh2_sock = 0 ;
    
//     do {
//         src_atom_pkg = this->transfer_ready_queue.front().first;
//         dest_atom_pkg = this->transfer_ready_queue.front().second;
//         this->current_src_file_name = src_atom_pkg.files.at(0);
//         this->current_dest_file_name = dest_atom_pkg.files.at(0);

//         //有效协议传输
//         //file - > nrsftp
//         //nrsftp -> nrsftp
//         //nrsftp -> file
  
//         qDebug()<<this->current_src_file_name;
//         qDebug()<<this->current_dest_file_name;

//         //这里有几种情况，全部都列出来
//         // 上传:
//         // local file type is file               remote file is file     error
//         // local file type is file               remote  file is dir      ok
//         // local file type is dir                remote file is file     error
//         // local file type is dir                remote file is dir      ok
//         // 下载：
//         // local file type is file               remote file is file     error
//         // local file type is file               remote file is dir      error
//         // local file type is dir                remote file is file     ok
//         // local file type is dir                remote file is dir      ok
       
//         if (src_atom_pkg.scheme == PROTO_FILE && dest_atom_pkg.scheme == PROTO_SFTP) {
//             //提示开始处理新文件：
//             emit this->transfer_new_file_started(this->current_src_file_name);
            
//             //连接到目录主机：
//              qDebug()<<"connecting to dest ssh host:"<<dest_atom_pkg.username
//                      <<":"<<dest_atom_pkg.password<<"@"<<dest_atom_pkg.host <<":"<<dest_atom_pkg.port ;
//              if (this->dest_ssh2_sess == 0 || this->dest_ssh2_sftp == 0) {
//                  emit  transfer_log("Connecting to destination host ...");
//                  QString tmp_passwd = dest_atom_pkg.password;
//                  rhct = new RemoteHostConnectThread(dest_atom_pkg.username, tmp_passwd,
//                                                     dest_atom_pkg.host, dest_atom_pkg.port.toInt(), dest_atom_pkg.pubkey);
//                  rhct->run();
//                  //TODO get status code and then ...
//                  rv = rhct->get_connect_status();
//                  if (rv != RemoteHostConnectThread::CONN_OK) {
//                      qDebug()<<"Connect to Host Error: "<<rv<<":"<<rhct->get_status_desc(rv);
//                      emit transfer_log("Connect Error: " + rhct->get_status_desc(rv));
//                      this->error_code = transfer_ret = 6;
//                      this->errorString = rhct->get_status_desc(rv);
//                      break;
//                  }
//                  //get connect return code end
//                  this->dest_ssh2_sess = (LIBSSH2_SESSION*)rhct->get_ssh2_sess();
//                  this->dest_ssh2_sock = rhct->get_ssh2_sock();
//                  this->dest_ssh2_sftp = libssh2_sftp_init(this->dest_ssh2_sess);
//                  delete rhct ; rhct = 0 ;
//                  emit  transfer_log("Connect done.");
//              }

//             // 将文件上传到目录
//             if (QFileInfo(this->current_src_file_name).isFile()
//                 && remote_is_dir(this->dest_ssh2_sftp , this->current_dest_file_name)) {
//                 QString remote_full_path = this->current_dest_file_name + "/"
//                     + this->current_src_file_name.split ( "/" ).at ( this->current_src_file_name.split ( "/" ).count()-1 ) ;
//                 qDebug() << "local file: " << this->current_src_file_name
//                          << "remote file:" << this->current_dest_file_name
//                          << "remote full file path: "<< remote_full_path ;
//                 transfer_ret = this->do_upload(this->current_src_file_name, remote_full_path, 0);
//             } else if (QFileInfo(this->current_src_file_name).isDir()
//                        && remote_is_dir(this->dest_ssh2_sftp, this->current_dest_file_name)) {
//                 //将目录上传到目录
//                 qDebug()<<"uploding dir to dir ...";
//                 //this->sleep(debug_sleep_time);
//                 //
//                 //列出本地目录中的文件，加入到队列中，然后继续。
//                 //如果列出的本地文件是目录，则在这里先确定远程存在此子目录，否则就要创建先。
//                 fileinfos.clear();

//                 fxp_local_do_ls(this->current_src_file_name, fileinfos);
//                 qDebug()<<"ret:"<<transfer_ret<<" count:"<<fileinfos.size() ;
               
//                 //这个远程目录属性应该和本地属性一样，所以就使用this->current_src_file_type
//                 //不知道是不是有问题。
//                 QString remote_full_path = this->current_dest_file_name + "/" 
//                     + this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1);

//                 temp_dest_atom_pkg = dest_atom_pkg;
//                 temp_dest_atom_pkg.setFile(remote_full_path);

//                 //为远程建立目录
//                 memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
//                 transfer_ret = libssh2_sftp_mkdir(this->dest_ssh2_sftp, 
//                                                   GlobalOption::instance()->remote_codec->fromUnicode(remote_full_path), 
//                                                   0755);
//                 qDebug()<<"libssh2_sftp_mkdir : "<< transfer_ret <<" :"<< remote_full_path;

//                 //添加到队列当中
//                 for (int i = 0 ; i < fileinfos.size() ; i ++) {
//                     temp_src_atom_pkg = src_atom_pkg;
//                     temp_src_atom_pkg.setFile(this->current_src_file_name + "/" + fileinfos.at(i)['N']);
//                     this->transfer_ready_queue.push_back(QPair<TaskPackage, TaskPackage>
//                                                          (temp_src_atom_pkg, temp_dest_atom_pkg));
//                 }               
//             } else {
//                 //其他的情况暂时不考虑处理。跳过
//                 //TODO return a error value , not only error code
//                 this->error_code = 1 ;
//                 //assert(1 == 2) ;
//                 qDebug()<<"Unexpected transfer type: "<<__FILE__<<" in " << __LINE__ ;
//             }

//         } else if (src_atom_pkg.scheme == PROTO_SFTP && dest_atom_pkg.scheme == PROTO_FILE) {
//             //提示开始处理新文件：
//             emit this->transfer_new_file_started(this->current_src_file_name);
//             //连接到目录主机：
//             qDebug()<<"connecting to src ssh host:"<<src_atom_pkg.username<<":"<<src_atom_pkg.password
//                     <<"@"<<src_atom_pkg.host <<":"<<src_atom_pkg.port ;
//             if (this->src_ssh2_sess == 0 || this->src_ssh2_sftp == 0) {
//                 emit  transfer_log("Connecting to source host ...");
//                 QString tmp_passwd = src_atom_pkg.password;

//                 rhct = new RemoteHostConnectThread(src_atom_pkg.username, tmp_passwd, src_atom_pkg.host, 
//                                                    src_atom_pkg.port.toInt(), src_atom_pkg.pubkey);
//                 rhct->run();
//                 //TODO get status code and then ...
//                 this->src_ssh2_sess = (LIBSSH2_SESSION*)rhct->get_ssh2_sess();
//                 this->src_ssh2_sock = rhct->get_ssh2_sock();
//                 this->src_ssh2_sftp = libssh2_sftp_init(this->src_ssh2_sess);
//                 delete rhct ; rhct = 0 ;
//                 emit  transfer_log("Connect done.");
//             }
//             //将文件下载到目录
//             if (remote_is_reg(this->src_ssh2_sftp, this->current_src_file_name) 
//                 && QFileInfo(this->current_dest_file_name).isDir()) {
//                 QString local_full_path = this->current_dest_file_name + "/"
//                     + this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1);

//                 qDebug() << "local file: " << this->current_src_file_name
//                          << "remote file:" << this->current_dest_file_name
//                          << "local full file path: "<< local_full_path ;

//                 transfer_ret = this->do_download(this->current_src_file_name, local_full_path, 0);
//             }
//             //将目录下载到目录
//             else if (QFileInfo(this->current_dest_file_name).isDir()
//                      && remote_is_dir(this->src_ssh2_sftp, this->current_src_file_name)) {
//                 qDebug()<<"downloading dir to dir ...";
//                 //this->sleep(debug_sleep_time);
//                 //列出本远程目录中的文件，加入到队列中，然后继续。
              
//                 fileinfos.clear();
//                 transfer_ret = fxp_do_ls_dir(this->src_ssh2_sftp, this->current_src_file_name + "/", fileinfos);
//                 qDebug()<<"ret:"<<transfer_ret<<" file count:"<<fileinfos.size();
//                 // local dir = curr local dir +  curr remote dir 的最后一层目录
//                 temp_dest_atom_pkg = dest_atom_pkg;
//                 temp_dest_atom_pkg.setFile(this->current_dest_file_name + "/" +
//                                           this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1));
                
//                 //确保本地有这个目录。
//                 transfer_ret = QDir().mkpath(temp_dest_atom_pkg.files.at(0));
//                 qDebug()<<" fxp_local_do_mkdir: "<<transfer_ret <<" "<< temp_dest_atom_pkg.files.at(0) ;
//                 //加入到任务队列
//                 for (int i = 0 ; i < fileinfos.size() ; i ++) {
//                     temp_src_atom_pkg = src_atom_pkg;
//                     temp_src_atom_pkg.setFile(this->current_src_file_name + "/" + fileinfos.at(i)['N']);
                    
//                     this->transfer_ready_queue.push_back(QPair<TaskPackage, TaskPackage>
//                                                          (temp_src_atom_pkg, temp_dest_atom_pkg));
//                     //romote is source 
//                 }
//             } else {
//                 //其他的情况暂时不考虑处理。跳过。
//                 //TODO return a error value , not only error code 
//                 this->error_code = 1 ;
//                 //assert( 1 == 2 ) ; 
//                 qDebug()<<"Unexpected transfer type: "<<__FILE__<<" in " << __LINE__ ;
//             }
//         } else if(src_atom_pkg.scheme == PROTO_SFTP && dest_atom_pkg.scheme == PROTO_SFTP) {
//             emit this->transfer_new_file_started(this->current_src_file_name);
//             //处理nrsftp协议
//             if (this->src_ssh2_sess == 0 || this->src_ssh2_sftp == 0) {
//                 emit  transfer_log("Connecting to destionation host ...");
//                 QString tmp_passwd = src_atom_pkg.password;

//                 rhct = new RemoteHostConnectThread(src_atom_pkg.username, tmp_passwd, src_atom_pkg.host, 
//                                                    src_atom_pkg.port.toInt(), src_atom_pkg.pubkey);
//                 rhct->run();
//                 //TODO get status code and then ...
//                 this->src_ssh2_sess = (LIBSSH2_SESSION*)rhct->get_ssh2_sess();
//                 this->src_ssh2_sock = rhct->get_ssh2_sock();
//                 this->src_ssh2_sftp = libssh2_sftp_init(this->src_ssh2_sess);
//                 delete rhct ; rhct = 0 ;
//                 emit  transfer_log("Connect done.");
//             }
//             if (this->dest_ssh2_sess == 0 || this->dest_ssh2_sftp == 0 ) {
//                 emit  transfer_log("Connecting to source host ...");
//                 QString tmp_passwd = dest_atom_pkg.password;

//                 rhct = new RemoteHostConnectThread(dest_atom_pkg.username, tmp_passwd, dest_atom_pkg.host,
//                                                    dest_atom_pkg.port.toInt(), dest_atom_pkg.pubkey);
//                 rhct->run();
//                 //TODO get status code and then ...
//                 this->dest_ssh2_sess = (LIBSSH2_SESSION*)rhct->get_ssh2_sess();
//                 this->dest_ssh2_sock = rhct->get_ssh2_sock();
//                 this->dest_ssh2_sftp = libssh2_sftp_init(this->dest_ssh2_sess);
//                 delete rhct ; rhct = 0 ;
//                 emit  transfer_log("Connect done.");
//             }
//             ////////////
//             if (remote_is_dir(this->src_ssh2_sftp, this->current_src_file_name) 
//                 && remote_is_dir(this->dest_ssh2_sftp,this->current_dest_file_name ) )
//             {
//                 qDebug()<<" nrsftp exchage dir to dir...";
//                 fileinfos.clear();        

//                 transfer_ret = fxp_do_ls_dir(this->src_ssh2_sftp, this->current_src_file_name + "/", fileinfos);
//                 qDebug()<<"ret:"<<transfer_ret<<" file count:"<<fileinfos.size();
//                 temp_dest_atom_pkg = dest_atom_pkg;
//                 temp_dest_atom_pkg.setFile(this->current_dest_file_name + "/" + 
//                                            this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1));    
                                       
//                 //为远程建立目录
//                 memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
//                 transfer_ret = libssh2_sftp_mkdir(this->dest_ssh2_sftp, 
//                                                   GlobalOption::instance()->remote_codec->fromUnicode(temp_dest_atom_pkg.files.at(0)), 
//                                                   0755);
//                 qDebug()<<" libssh2_sftp_mkdir : "<< transfer_ret <<" :"<<temp_dest_atom_pkg.files.at(0);

//                 //添加到队列当中
//                 for (int i = 0 ; i < fileinfos.size() ; i ++ ) {
//                     temp_src_atom_pkg = src_atom_pkg;
//                     temp_src_atom_pkg.setFile(this->current_src_file_name + "/" +
//                                               fileinfos.at(i)['N']);
//                     this->transfer_ready_queue.push_back(QPair<TaskPackage, TaskPackage>
//                                                          (temp_src_atom_pkg, temp_dest_atom_pkg));
//                 }
//             } else if (remote_is_reg(this->src_ssh2_sftp, this->current_src_file_name) 
//                        && remote_is_dir(this->dest_ssh2_sftp,this->current_dest_file_name))
//             {
//                 qDebug()<<" nrsftp exchage file to dir...";
//                 QString dest_full_path = this->current_dest_file_name + "/" + this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1);
//                 transfer_ret = this->do_nrsftp_exchange(this->current_src_file_name, dest_full_path);
//             } else {
//                 //其他的情况暂时不考虑处理。跳过
//                 //TODO return a error value , not only error code
//                 q_debug()<<"src: "<< src_atom_pkg<<" dest:"<< dest_atom_pkg;
//                 this->error_code = 1 ;
//                 //assert ( 1 == 2 ) ;
//                 qDebug()<<"Unexpected transfer type: "<<__FILE__<<" in " << __LINE__ ;
//             }
//         } else {
//             q_debug()<<"src: "<< src_atom_pkg<<" dest:"<< dest_atom_pkg;
//             this->error_code = 2;
//             assert( 1 == 2 );
//         }
       
//         this->transfer_ready_queue.erase(this->transfer_ready_queue.begin());
//         this->transfer_done_queue.push_back(QPair<TaskPackage, TaskPackage>(src_atom_pkg, dest_atom_pkg));
//     } while (this->transfer_ready_queue.size() > 0 && this->user_canceled == false);

//     qDebug() << " transfer_ret :" << transfer_ret << " ssh2 sftp shutdown:"<< this->src_ssh2_sftp<<" "<<this->dest_ssh2_sftp;
//     //TODO 选择性关闭 ssh2 会话，有可能是 src  ,也有可能是dest 
//     if (this->src_ssh2_sftp != 0) {
//         libssh2_sftp_shutdown(this->src_ssh2_sftp);
//         this->src_ssh2_sftp = 0 ;
//     }
//     if (this->src_ssh2_sess != 0) {
//         libssh2_session_free(this->src_ssh2_sess);
//         this->src_ssh2_sess = 0 ;
//     }
//     if (this->src_ssh2_sock > 0) {
// #ifdef WIN32
//         ::closesocket(this->src_ssh2_sock);
// #else  
//         ::close(this->src_ssh2_sock);
// #endif
//         this->src_ssh2_sock = -1;
//     }
//     if (this->dest_ssh2_sftp != 0) {
//         libssh2_sftp_shutdown(this->dest_ssh2_sftp);
//         this->dest_ssh2_sftp = 0 ;
//     }
//     if (this->dest_ssh2_sess != 0) {
//         libssh2_session_free(this->dest_ssh2_sess);
//         this->dest_ssh2_sess = 0 ;
//     }
//     if (this->dest_ssh2_sock > 0) {
// #ifdef WIN32
//         ::closesocket(this->dest_ssh2_sock);
// #else  
//         ::close(this->dest_ssh2_sock);
// #endif
//         this->dest_ssh2_sock = -1;
//     }
//     if (user_canceled == true) {
//         this->error_code = 3;
//     }
// }

void Transportor::set_transport_info(TaskPackage src_pkg, TaskPackage dest_pkg)
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

int Transportor::do_download(QString remote_path, QString local_path, int pflag)
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


int Transportor::do_upload(QString local_path, QString remote_path, int pflag)
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
    QFileInfo local_fi(local_path);
    file_size = local_fi.size();
    qDebug()<<"local file size:" << file_size ;
    emit this->transfer_got_file_size(file_size);
    
    QFile q_file(local_path);
    if (!q_file.open( QIODevice::ReadOnly)) {
        //TODO 错误消息通知用户。
        qDebug()<<"open local file error:"<< q_file.errorString()  ;
        //printf("open local file error:%s\n", strerror( errno ) );        
    } else {
        //read local file and then write to remote file
        while (!q_file.atEnd()) {
            rlen = q_file.read(buff, sizeof(buff));
            if (rlen <= 0) {
                //qDebug()<<"errno: "<<errno<<" err msg:"<< strerror( errno) << ftell( local_handle) ;
                break ;
            }
            wlen = libssh2_sftp_write(sftp_handle, buff, rlen);
            Q_ASSERT(wlen == rlen);
            if (wlen < rlen) {
                q_debug()<<"write to server less then need write bytes";
                // TODO 这种情况应该尝试再次写入剩余的数据
            }
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
int Transportor::do_nrsftp_exchange(QString src_path, QString dest_path)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
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
        if (this->user_canceled == true) {
            break;
        }
    }

    return 0;
}

QMap<QString, QString> Transportor::getHostInfo(TaskPackage &pkg)
{
    QMap<QString, QString> host;

    if (pkg.scheme == PROTO_FTP) {
        host["protocol"] = "FTP";
    } else if (pkg.scheme == PROTO_SFTP) {
        host["protocol"] = "SFTP";
    } else if (pkg.scheme == PROTO_FILE) {
        host["protocol"] = "FILE";
    } else {
        assert(0);
    }
    host["host_name"] = pkg.host;
    host["port"] = pkg.port;
    host["user_name"] = pkg.username;
    host["password"] = pkg.password;
    host["pubkey"] = pkg.pubkey;

    return host;
}

void Transportor::set_user_cancel(bool cancel)
{
    this->user_canceled = cancel;
}

void Transportor::wait_user_response()
{
    this->wait_user_response_mutex.lock();
    this->wait_user_response_cond.wait(&this->wait_user_response_mutex);
}
void Transportor::user_response_result(int result)
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
// Assertion failed: *lock == MUTEX_UNLOCKED, file ath.c, line 184
// 这是mingw32平台上的libgcrypt相关的问题。
// 这个问题应该不存在了吧
