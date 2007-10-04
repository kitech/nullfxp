/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   gzl@localhost   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 

#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#ifdef WIN32
#define jjjjjjjjj
#else
#include <sys/uio.h>
#endif

#include <cassert>

#include <QtCore>
#include "globaloption.h"
#include "transferthread.h"

#include "utils.h"


TransferThread::TransferThread ( QObject *parent )
		: QThread ( parent )
{}


TransferThread::~TransferThread()
{}

int TransferThread::remote_is_dir(  char *path )
{
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    LIBSSH2_SFTP_HANDLE * sftp_handle ;
    
    memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
    
    sftp_handle = libssh2_sftp_opendir( this->ssh2_sftp , path );
    
    if( sftp_handle != NULL )
    {
        libssh2_sftp_closedir(sftp_handle);
        return 1 ;
    }
    else    // == NULL 
    {
        //TODO 可能是一个没有打开权限的目录，这里没有处理这种情况。
        return 0;
    }
    return 0;
}

int TransferThread::remote_is_reg(  char *path )
{
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    LIBSSH2_SFTP_HANDLE * sftp_handle ;
    unsigned long flags;
    long mode ;
    int ret = 0 ;
    
    memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
    flags = LIBSSH2_FXF_READ ;
    mode = 022;
    
    sftp_handle = libssh2_sftp_open( this->ssh2_sftp , path , flags , mode );
    
    if( sftp_handle != NULL )
    {
        ret = libssh2_sftp_fstat( sftp_handle , & ssh2_sftp_attrib );
        libssh2_sftp_close(sftp_handle);
        return ( S_ISREG( ssh2_sftp_attrib.permissions ) );
    }
    else    // == NULL 
    {
        return 0;
    }
    return 0;
}

int TransferThread::fxp_do_ls_dir ( char * path,std::vector<std::map<char, std::string> > & fileinfos      )
{
    LIBSSH2_SFTP_HANDLE * sftp_handle = 0 ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib ;
    std::map<char,std::string> thefile;
    char file_name[PATH_MAX+1];
    char file_size[PATH_MAX+1];
    char file_type[PATH_MAX+1];
    char file_date[PATH_MAX+1];
    int file_count = 0 ;
    
    sftp_handle = libssh2_sftp_opendir(ssh2_sftp,path);
    if( sftp_handle == 0 )
    {
        return 0;
    }
    else
    {
        fileinfos.clear();
        memset(&ssh2_sftp_attrib,0,sizeof(LIBSSH2_SFTP_ATTRIBUTES));
        while( libssh2_sftp_readdir( sftp_handle , file_name , PATH_MAX , & ssh2_sftp_attrib ) > 0 )
        {
            if( strlen ( file_name ) == 1 && file_name[0] == '.' ) continue ;
            if( strlen ( file_name ) == 2 && file_name[0] == '.' && file_name[1] == '.') continue ;
            //不处理隐藏文件? 处理隐藏文件
            if( file_name[0] == '.' ) continue ;
            
            memset(file_size,0,sizeof(file_size )) ;
            snprintf(file_size,sizeof(file_size) , "%llu",ssh2_sftp_attrib.filesize );
            
            struct tm *ltime = localtime((time_t*)&ssh2_sftp_attrib.mtime);
            if (ltime != NULL) 
            {
                if (time(NULL) - ssh2_sftp_attrib.mtime < (365*24*60*60)/2)
                    strftime(file_date, sizeof file_date, "%Y/%m/%d %H:%M:%S", ltime);
                else
                    strftime(file_date, sizeof file_date, "%Y/%m/%d %H:%M:%S", ltime);
            }

            strmode(ssh2_sftp_attrib.permissions,file_type );
            //printf(" ls dir : %s %s , date=%s , type=%s \n" , file_name , file_size , file_date , file_type );
            thefile.insert(std::make_pair('N',std::string(file_name)));
            thefile.insert(std::make_pair('T',std::string(file_type)));
            thefile.insert(std::make_pair('S',std::string(file_size)));
            thefile.insert(std::make_pair('D',std::string( file_date )));  
                      
            fileinfos.push_back(thefile);
            memset(&ssh2_sftp_attrib,0,sizeof(LIBSSH2_SFTP_ATTRIBUTES) );
            thefile.clear();
        }
        libssh2_sftp_closedir(sftp_handle);
        return fileinfos.size();
    }
    
    return 0 ; 
}

void TransferThread::run()
{

	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib ;
    
	int transfer_ret ;
    int debug_sleep_time = 5 ;
    
    std::pair<std::string , std::string> local_file_pair;
    std::pair<std::string , std::string> remote_file_pair;
    
    std::pair<std::string , std::string> temp_local_file_pair;
    std::pair<std::string , std::string> temp_remote_file_pair;
        
    std::vector<std::map<char, std::string> >  fileinfos ;
//     Attrib a ;  //用于创建远程目录
    
    this->error_code = 0 ;
    
    do{
       local_file_pair = this->transfer_ready_queue.front().first;
       remote_file_pair = this->transfer_ready_queue.front().second ;
       this->current_local_file_name = local_file_pair.first.c_str();
       this->current_local_file_type = local_file_pair.second.c_str(); 
       this->current_remote_file_name = remote_file_pair.first.c_str();
       this->current_remote_file_type = remote_file_pair.second.c_str() ;
       
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
       
       if( this->transfer_type == TransferThread::TRANSFER_PUT )
       {
           //提示开始处理新文件：
           emit this->transfer_new_file_started(this->current_local_file_name);
           is_reg(this->current_local_file_name.toAscii().data());
           //将文件上传到目录
           if(is_reg(this->current_local_file_name.toAscii().data())
              && remote_is_dir(/*this->sftp_connection,*/this->current_remote_file_name.toAscii().data()) )
           {
               QString remote_full_path = this->current_remote_file_name + "/"
                       + this->current_local_file_name.split ( "/" ).at ( this->current_local_file_name.split ( "/" ).count()-1 ) ;

               qDebug() << "local file: " << this->current_local_file_name
                       << "remote file:" << this->current_remote_file_name
                       << "remote full file path: "<< remote_full_path ;

               transfer_ret = this->do_upload ( /*this->sftp_connection,*/this->current_local_file_name.toAscii().data(),
                       remote_full_path.toAscii().data(),0 );
           }
           //将目录上传到目录
           else if(is_dir(this->current_local_file_name.toAscii().data())
                   && remote_is_dir(/*this->sftp_connection,*/this->current_remote_file_name.toAscii().data()) )
           {
               qDebug()<<"uploding dir to dir ...";
               this->sleep(debug_sleep_time);
               //
               //列出本地目录中的文件，加入到队列中，然后继续。
               //如果列出的本地文件是目录，则在这里先确定远程存在此子目录，否则就要创建先。
               fileinfos.clear();
               fxp_local_do_ls(this->current_local_file_name.toAscii().data(),fileinfos);
               qDebug()<<"ret:"<<transfer_ret<<" count:"<<fileinfos.size() ;
               
               //这个远程目录属性应该和本地属性一样，所以就使用this->current_local_file_type
               //不知道是不是有问题。
               temp_remote_file_pair = std::make_pair( QString( this->current_remote_file_name + "/" + this->current_local_file_name.split("/").at(this->current_local_file_name.split("/").count()-1)).toAscii().data() ,QString( this->current_local_file_type).toAscii().data() );
               //为远程建立目录
               memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
               transfer_ret = libssh2_sftp_mkdir(ssh2_sftp,temp_remote_file_pair.first.c_str(),0755);
               qDebug()<<" libssh2_sftp_mkdir : "<< transfer_ret <<" :"<< temp_remote_file_pair.first.c_str();
//                attrib_clear(&a);
//                a.flags |= SSH2_FILEXFER_ATTR_PERMISSIONS;
//                a.perm = 0777;
//                transfer_ret = do_mkdir(this->sftp_connection, QString(temp_remote_file_pair.first.c_str()).toAscii().data(), &a);
               
               //添加到队列当中
               for( int i = 0 ; i < fileinfos.size() ; i ++ )
               {
                   temp_local_file_pair = std::make_pair( QString( this->current_local_file_name+"/"+
                           fileinfos.at(i)['N'].c_str()) .toAscii().data() , fileinfos.at(i)['T']) ;
                   
                  this->transfer_ready_queue.push_back(std::make_pair(temp_local_file_pair,temp_remote_file_pair)); 
               }
               
           }
           else
           {
               //其他的情况暂时不考虑处理。跳过
               //TODO return a error value , not only error code 
               this->error_code = 1 ;
               //assert( 1 == 2 ) ; 
           }
       }
       else if( this->transfer_type == TransferThread::TRANSFER_GET )
       {
           //提示开始处理新文件：
           emit this->transfer_new_file_started(this->current_remote_file_name);
                      
           //将文件下载到目录
           if(is_dir(this->current_local_file_name.toAscii().data())
              && remote_is_reg(/*this->sftp_connection,*/this->current_remote_file_name.toAscii().data()) )
           {
               QString local_full_path = this->current_local_file_name + "/"
                       + this->current_remote_file_name.split ( "/" ).at ( this->current_remote_file_name.split ( "/" ).count()-1 ) ;

               qDebug() << "local file: " << this->current_local_file_name
                       << "remote file:" << this->current_remote_file_name
                       << "local full file path: "<< local_full_path ;

               transfer_ret = this->do_download ( /*this->sftp_connection,*/
                       this->current_remote_file_name.toAscii().data(),
                               local_full_path.toAscii().data(), 0 );
           }
           //将目录下载到目录
           else if( is_dir(this->current_local_file_name.toAscii().data())
                    && remote_is_dir(this->current_remote_file_name.toAscii().data()) )
           {
               qDebug()<<"downloading dir to dir ...";
               this->sleep(debug_sleep_time);
               //列出本远程目录中的文件，加入到队列中，然后继续。
               //QString current_remote_strip_path = this->current_remote_file_name;
               //QString current_remote_full_path = this->current_remote_file_name + "/";
//                int ls_flag = 0 ;
//                ls_flag &= ~VIEW_FLAGS;
//                ls_flag |= LS_LONG_VIEW;
               
               fileinfos.clear();
               transfer_ret = fxp_do_ls_dir(/*this->sftp_connection,*/
                                            QString(this->current_remote_file_name+"/").toAscii().data(),
                                            fileinfos);
               qDebug()<<"ret:"<<transfer_ret<<" file count:"<<fileinfos.size();
               
               // local dir = curr local dir +  curr remote dir 的最后一层目录
               temp_local_file_pair = std::make_pair(QString( this->current_local_file_name+"/"+this->current_remote_file_name.split("/").at(this->current_remote_file_name.split("/").count()-1)) .toAscii().data(),this->current_remote_file_type.toAscii().data());
               //确保本地有这个目录。
               transfer_ret = fxp_local_do_mkdir(temp_local_file_pair.first.c_str());
               qDebug()<<" fxp_local_do_mkdir: "<<transfer_ret <<" "<< temp_local_file_pair.first.c_str() ;
               //加入到任务队列
               for(int i = 0 ; i < fileinfos.size() ; i ++)
               {
                   temp_remote_file_pair = std::make_pair( QString(this->current_remote_file_name+"/"+fileinfos.at(i)['N'].c_str()).toAscii().data() , fileinfos.at(i)['T'] );
                                      
                   this->transfer_ready_queue.push_back(std::make_pair( temp_local_file_pair,temp_remote_file_pair));
                   
               }
           }
           else
           {
               //其他的情况暂时不考虑处理。跳过。
               //TODO return a error value , not only error code 
               this->error_code = 1 ;
               //assert( 1 == 2 ) ; 
           }
       }
       else
       {
           this->error_code = 2 ;
           assert( 1 == 2 ) ;
       }
       
       this->transfer_ready_queue.erase(this->transfer_ready_queue.begin());
       this->transfer_done_queue.push_back(std::make_pair(local_file_pair,remote_file_pair)); 
    }while(this->transfer_ready_queue.size() > 0 ) ;

	qDebug() << " transfer_ret :" << transfer_ret ;
}

void TransferThread::set_remote_connection (void* ssh2_sess,void* ssh2_sftp,int ssh2_sock)
{
    this->ssh2_sess = (LIBSSH2_SESSION*) ssh2_sess;
    this->ssh2_sftp = (LIBSSH2_SFTP * ) ssh2_sftp ;
    this->ssh2_sock = ssh2_sock ;
}

void TransferThread::set_transfer_info ( int type,QStringList local_file_names,QStringList remote_file_names  )
{
	this->transfer_type = type ;

    this->local_file_names = local_file_names ;
    this->remote_file_names = remote_file_names ;

    QString local_file_name ;
    QString remote_file_name ;
    
    std::pair<std::string , std::string> local_file_pair;
    std::pair<std::string , std::string> remote_file_pair;
    
    if( type == TransferThread::TRANSFER_PUT )
    {
        assert( remote_file_names.count() == 1 );
        remote_file_name = remote_file_names.at(0);
        for(int i = 0 ; i < local_file_names.count() ; i ++ )
        {
            local_file_name = local_file_names.at(i);
            local_file_pair = std::make_pair(local_file_name.toAscii().data(),"");
            remote_file_pair = std::make_pair(remote_file_name.toAscii().data(),"");
        
            this->transfer_ready_queue.	push_back(std::make_pair(local_file_pair,remote_file_pair));
        }
    }
    else if( type == TransferThread::TRANSFER_GET )
    {
        assert( local_file_names.count() == 1 );
        local_file_name = local_file_names.at(0);
        for(int i = 0 ; i < remote_file_names.count() ; i ++ )
        {
            remote_file_name = remote_file_names.at(i);            
            local_file_pair = std::make_pair(local_file_name.toAscii().data(),"");
            remote_file_pair = std::make_pair(remote_file_name.toAscii().data(),"");
        
            this->transfer_ready_queue.	push_back(std::make_pair(local_file_pair,remote_file_pair));
        }
    }
    else
    {
        assert( 1 == 2 );   
    }
}

int
TransferThread::do_download ( char *remote_path, char *local_path,
                              int pflag )
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    qDebug()<< "remote_path = "<<  remote_path  << " , local_path = " << local_path ;
    int local_fd , rlen , wlen  ;
    int file_size , tran_len = 0   ;
    LIBSSH2_SFTP_HANDLE * sftp_handle ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    char buff[5120] = {0};
    
    sftp_handle = libssh2_sftp_open(ssh2_sftp,remote_path,LIBSSH2_FXF_READ,0);
    if( sftp_handle == NULL )
    {
        //TODO 错误消息通知用户。
        qDebug()<<"open sftp file error :"<< libssh2_sftp_last_error(ssh2_sftp);
        return -1 ;
    }
    
    memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
    libssh2_sftp_fstat(sftp_handle,&ssh2_sftp_attrib);
    file_size = ssh2_sftp_attrib.filesize;
    qDebug()<<" remote file size :"<< file_size ;
    emit this->transfer_got_file_size( file_size );
    
    // 本地编码 --> Qt 内部编码
    QFile q_file ( GlobalOption::instance()->locale_codec->toUnicode( local_path ) ) ;
    //local_fd = ::open(local_path,O_RDWR|O_TRUNC|O_CREAT,0666 );
    if( ! q_file.open(QIODevice::ReadWrite|QIODevice::Truncate))
    //if( local_fd < 0 )
    {
        //TODO 错误消息通知用户。
        qDebug()<<"open local file error:"<< q_file.errorString() ;        
    }
    else
    {
        //read remote file  and then write to local file
        while( (rlen = libssh2_sftp_read(sftp_handle,buff,sizeof(buff) ) ) > 0 )
        {
						//wlen = ::write ( local_fd , buff , rlen );
						wlen = q_file.write( buff, rlen );
            tran_len += wlen ;
			//qDebug() <<" read len :"<< rlen <<" , write len: "<< wlen                     << " tran len: "<< tran_len ;
			//my progress signal
            if( file_size == 0 )
            {
                emit this->transfer_percent_changed ( 100 , tran_len , wlen );
            }
            else
            {
                emit this->transfer_percent_changed ( tran_len * 100 / file_size , tran_len ,wlen );
            }
        }
        //::close(local_fd);

        q_file.close();
    }
    
    libssh2_sftp_close(sftp_handle);

	return ( 0 );
}


int
TransferThread::do_upload ( char *local_path, char *remote_path,
                            int pflag )
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    qDebug()<< "remote_path = "<<  remote_path  << " , local_path = " << local_path ;

    int local_fd , rlen , wlen  ;
    int file_size , tran_len = 0   ;
    LIBSSH2_SFTP_HANDLE * sftp_handle ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    struct stat     file_stat ;
    char buff[5120] = {0};
    FILE * local_handle = 0;
    char native_path [PATH_MAX+1] = {0};
    
    sftp_handle = libssh2_sftp_open( ssh2_sftp,remote_path,LIBSSH2_FXF_READ|LIBSSH2_FXF_WRITE|LIBSSH2_FXF_CREAT|LIBSSH2_FXF_TRUNC,0666 ) ;
    if( sftp_handle == NULL )
    {
        //TODO 错误消息通知用户。
        qDebug()<<"open sftp file error :"<< libssh2_sftp_last_error(ssh2_sftp);
        return -1 ;
    }
    
    memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
    //libssh2_sftp_fstat(sftp_handle,&ssh2_sftp_attrib);
    //file_size = ssh2_sftp_attrib.filesize;
    //qDebug()<<" remote file size :"<< file_size ;
    strcpy( native_path, QDir::toNativeSeparators( local_path ).toAscii().data());
    memset(&file_stat,0,sizeof(struct stat));
    ::stat(native_path,&file_stat);
    file_size = file_stat.st_size;
    qDebug()<< "remote_path = "<<  remote_path  << " , local_path = " << native_path ;
    qDebug()<<"local file size:" << file_size ;
    emit this->transfer_got_file_size( file_size ) ;
    
    //local_fd = ::open(native_path,O_RDONLY );
    //local_handle = ::fopen( native_path , "r");
    // 本地编码 --> Qt 内部编码
    QFile q_file( GlobalOption::instance()->locale_codec->toUnicode(native_path ) ) ;
    if( ! q_file.open( QIODevice::ReadOnly) )
    //if( local_fd <= 0 )
    //if( local_handle == 0 )
    {
        //TODO 错误消息通知用户。
        qDebug()<<"open local file error:"<< q_file.errorString()  ;
        //printf("open local file error:%s\n", strerror( errno ) );        
    }
    else
    {
    		//fseek(local_handle, 0L, SEEK_SET)	;
        //read local file and then write to remote file
        while( ! q_file.atEnd()  )
        {
        		//rlen = ::read(local_fd,buff,sizeof(buff));
        		//rlen = ::fread(  buff , 1  , sizeof(buff) , local_handle );
        		rlen = q_file.read( buff, sizeof( buff ) );
        		if( rlen <= 0 )
        			{
        				//qDebug()<<"errno: "<<errno<<" err msg:"<< strerror( errno) << ftell( local_handle) ;
        				break ;
        			}
            wlen = libssh2_sftp_write(sftp_handle,buff,rlen);
            
            tran_len += wlen ;
            
            //qDebug()<<" local read : "<< rlen << " sftp write :"<<wlen <<" up len :"<< tran_len ;
// 			qDebug() <<" read len :"<< rlen <<" , write len: "<< wlen 
//                    << " tran len: "<< tran_len ;
            if( file_size == 0 )
            {
                emit this->transfer_percent_changed ( 100, tran_len , wlen );
            }
            else
            {
                emit this->transfer_percent_changed ( tran_len * 100 / file_size , tran_len ,wlen  );
            }
        }
        q_file.close();
    }
    libssh2_sftp_close(sftp_handle);
    
	return ( 0 );
}




