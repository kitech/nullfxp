/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   liuguangzhao@users.sourceforge.net   *
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

#ifdef WIN32
#include <winsock2.h>
#endif

#include <cassert>

#include <QtCore>
#include "globaloption.h"
#include "transferthread.h"
#include "remotehostconnectthread.h"
#include "utils.h"


TransferThread::TransferThread ( QObject *parent )
		: QThread ( parent ),user_canceled(false)
{}


TransferThread::~TransferThread()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
}

int TransferThread::remote_is_dir( LIBSSH2_SFTP * ssh2_sftp, QString path )
{
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    LIBSSH2_SFTP_HANDLE * sftp_handle ;
    
    memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
    
	sftp_handle = libssh2_sftp_opendir( ssh2_sftp ,GlobalOption::instance()->remote_codec->fromUnicode( path ) .data() );
    
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

int TransferThread::remote_is_reg(LIBSSH2_SFTP * ssh2_sftp, QString path )
{
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    LIBSSH2_SFTP_HANDLE * sftp_handle ;
    unsigned long flags;
    long mode ;
    int ret = 0 ;
    
    memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
    flags = LIBSSH2_FXF_READ ;
    mode = 022;
    
    sftp_handle = libssh2_sftp_open( ssh2_sftp , GlobalOption::instance()->remote_codec->fromUnicode( path ) .data() , flags , mode );
    
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
//假设这个path的编码方式是远程服务器上所用的编码方式
int TransferThread::fxp_do_ls_dir ( LIBSSH2_SFTP * ssh2_sftp, QString path  , QVector<QMap<char, QString> > & fileinfos    )
{
    LIBSSH2_SFTP_HANDLE * sftp_handle = 0 ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib ;
    QMap<char, QString> thefile;
    char file_name[PATH_MAX+1];
    char file_size[PATH_MAX+1];
    char file_type[PATH_MAX+1];
    char file_date[PATH_MAX+1];
    int file_count = 0 ;
    
	sftp_handle = libssh2_sftp_opendir(ssh2_sftp, GlobalOption::instance()->remote_codec->fromUnicode(path).data() );
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
            //if( file_name[0] == '.' ) continue ;
            
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
			thefile.insert( 'N', GlobalOption::instance()->remote_codec->toUnicode(file_name ) );
            thefile.insert( 'T', file_type );
            thefile.insert( 'S', file_size );
            thefile.insert( 'D',  file_date );  
                      
            fileinfos.push_back(thefile);
            memset(&ssh2_sftp_attrib,0,sizeof(LIBSSH2_SFTP_ATTRIBUTES) );
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
    QUrl current_src_url ;
    QUrl current_dest_url ;
    RemoteHostConnectThread * rhct = 0 ;
    
	int transfer_ret ;
    int debug_sleep_time = 5 ;
    
    QPair<QString , QString> local_file_pair;
    QPair<QString , QString> remote_file_pair;
    
    QPair< QString , QString> temp_local_file_pair;
    QPair< QString , QString> temp_remote_file_pair;
        
    QVector<QMap<char, QString> >  fileinfos ;
    
    this->error_code = 0 ;
    
    this->dest_ssh2_sess = 0 ;
    this->dest_ssh2_sftp = 0 ;
    this->dest_ssh2_sock = 0 ;
        
    this->src_ssh2_sess = 0 ;
    this->src_ssh2_sftp = 0 ;
    this->src_ssh2_sock = 0 ;
    
    do{
       local_file_pair = this->transfer_ready_queue.front().first;
       remote_file_pair = this->transfer_ready_queue.front().second ;
       this->current_src_file_name = local_file_pair.first ;
       this->current_src_file_type = local_file_pair.second ; 
       this->current_dest_file_name = remote_file_pair.first ;
       this->current_dest_file_type = remote_file_pair.second  ;
       
       //有效协议传输
       //file - > nrsftp
       //nrsftp -> nrsftp
       //nrsftp -> file
       if(this->current_src_file_name.indexOf("?")!= -1){
          this->current_src_file_name = this->current_src_file_name.replace("?","_whywenhao_");
       }
       if(this->current_dest_file_name.indexOf("?") != -1){
          this->current_dest_file_type = this->current_dest_file_name.replace("?","_whywenhao_");
       }
	   qDebug()<<this->current_src_file_name;
	   qDebug()<<this->current_dest_file_name;
       current_src_url = this->current_src_file_name ;
       current_dest_url = this->current_dest_file_name ;
       this->current_src_file_name = current_src_url.path() ;
       this->current_dest_file_name = current_dest_url.path() ;
	   if(this->current_src_file_name.at(2) == ':'){
		   this->current_src_file_name = this->current_src_file_name.right(this->current_src_file_name.length()-1);
	   }
	   if(this->current_dest_file_name.at(2) == ':'){
		   this->current_dest_file_name = this->current_dest_file_name.right(this->current_dest_file_name.length()-1);
	   }
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
       
       //if( this->transfer_type == TransferThread::TRANSFER_PUT )
       if( current_src_url.scheme() == "file" && current_dest_url.scheme() == "nrsftp")
       {
           //提示开始处理新文件：
           emit this->transfer_new_file_started(this->current_src_file_name);
            
           //连接到目录主机：
           qDebug()<<"connecting to dest ssh host:"<<current_dest_url.userName()<<":"<<current_dest_url.password()<<"@"<<current_dest_url.host() <<":"<<current_dest_url.port() ;
           if( this->dest_ssh2_sess == 0 || this->dest_ssh2_sftp == 0 )
           {
               emit  transfer_log("Connecting to destination host ...");
               QString tmp_passwd = current_dest_url.password();
               if(tmp_passwd.indexOf("_whywenhao_") != -1){
                  tmp_passwd = tmp_passwd.replace("_whywenhao_","?");
               }
                rhct = new RemoteHostConnectThread ( current_dest_url.userName() , tmp_passwd ,current_dest_url.host() );
                rhct->run();
                //TODO get status code and then ...
                this->dest_ssh2_sess = (LIBSSH2_SESSION*)rhct->get_ssh2_sess();
                this->dest_ssh2_sock = rhct->get_ssh2_sock();
                this->dest_ssh2_sftp = libssh2_sftp_init(this->dest_ssh2_sess);
                delete rhct ; rhct = 0 ;
                emit  transfer_log("Connect done.");
           }
           
           //将文件上传到目录
		   if( is_reg( GlobalOption::instance()->locale_codec->fromUnicode( this->current_src_file_name ).data() )
              && remote_is_dir( this->dest_ssh2_sftp , this->current_dest_file_name ) )
           {
               QString remote_full_path = this->current_dest_file_name + "/"
                       + this->current_src_file_name.split ( "/" ).at ( this->current_src_file_name.split ( "/" ).count()-1 ) ;

               qDebug() << "local file: " << this->current_src_file_name
                       << "remote file:" << this->current_dest_file_name
                       << "remote full file path: "<< remote_full_path ;

               transfer_ret = this->do_upload (  this->current_src_file_name , remote_full_path ,0 );
           }
           //将目录上传到目录
           else if(is_dir( GlobalOption::instance()->locale_codec->fromUnicode( this->current_src_file_name ).data() )
                   && remote_is_dir(  this->dest_ssh2_sftp , this->current_dest_file_name ) )
           {
               qDebug()<<"uploding dir to dir ...";
               //this->sleep(debug_sleep_time);
               //
               //列出本地目录中的文件，加入到队列中，然后继续。
               //如果列出的本地文件是目录，则在这里先确定远程存在此子目录，否则就要创建先。
               fileinfos.clear();
               fxp_local_do_ls(   this->current_src_file_name  ,fileinfos );
               qDebug()<<"ret:"<<transfer_ret<<" count:"<<fileinfos.size() ;
               
               //这个远程目录属性应该和本地属性一样，所以就使用this->current_src_file_type
               //不知道是不是有问题。
               temp_remote_file_pair = QPair<QString,QString>( current_dest_url.toString() + "/" + this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1)   ,  this->current_src_file_type  );
               //为远程建立目录
               memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
			   transfer_ret = libssh2_sftp_mkdir(this->dest_ssh2_sftp, GlobalOption::instance()->remote_codec->fromUnicode( QUrl(temp_remote_file_pair.first).path() ).data(), 0755);
               qDebug()<<" libssh2_sftp_mkdir : "<< transfer_ret <<" :"<< temp_remote_file_pair.first;

               //添加到队列当中
               for( int i = 0 ; i < fileinfos.size() ; i ++ )
               {
                   temp_local_file_pair = QPair<QString,QString>( "file://"
#ifdef WIN32
					   "/"
#endif					   
					   +this->current_src_file_name+"/"+ fileinfos.at(i)['N']  , fileinfos.at(i)['T'] ) ;
                   
                  this->transfer_ready_queue.push_back( QPair<QPair<QString ,QString>,QPair<QString,QString> >( temp_local_file_pair,temp_remote_file_pair ) ); 
               }
               
           }
            else
            {
                //其他的情况暂时不考虑处理。跳过
                //TODO return a error value , not only error code
                this->error_code = 1 ;
                //assert( 1 == 2 ) ;
                qDebug()<<"Unexpected transfer type: "<<__FILE__<<" in " << __LINE__ ;
            }
		}
        else if( current_src_url.scheme() == "nrsftp" && current_dest_url.scheme() == "file")
                //if( this->transfer_type == TransferThread::TRANSFER_GET )
       {
           //提示开始处理新文件：
           emit this->transfer_new_file_started(this->current_src_file_name);
           //连接到目录主机：
           qDebug()<<"connecting to src ssh host:"<<current_src_url.userName()<<":"<<current_src_url.password()<<"@"<<current_src_url.host() <<":"<<current_src_url.port() ;
           if( this->src_ssh2_sess == 0 || this->src_ssh2_sftp == 0 )
           {
               emit  transfer_log("Connecting to source host ...");
               QString tmp_passwd = current_src_url.password();
               if(tmp_passwd.indexOf("_whywenhao_") != -1){
                  tmp_passwd = tmp_passwd.replace("_whywenhao_","?");
               }
               rhct = new RemoteHostConnectThread ( current_src_url.userName() , tmp_passwd ,current_src_url.host() );
               rhct->run();
                //TODO get status code and then ...
               this->src_ssh2_sess = (LIBSSH2_SESSION*)rhct->get_ssh2_sess();
               this->src_ssh2_sock = rhct->get_ssh2_sock();
               this->src_ssh2_sftp = libssh2_sftp_init(this->src_ssh2_sess);
               delete rhct ; rhct = 0 ;
               emit  transfer_log("Connect done.");
           }
           
           //将文件下载到目录
           //if(is_dir( GlobalOption::instance()->locale_codec->fromUnicode( this->current_src_file_name ).data() )
           //   && remote_is_reg(  this->dest_ssh2_sftp ,  this->current_dest_file_name ) )
           if( remote_is_reg(this->src_ssh2_sftp, this->current_src_file_name) 
               && is_dir( GlobalOption::instance()->locale_codec->fromUnicode( this->current_dest_file_name ).data() ) )
           {
               QString local_full_path = this->current_dest_file_name + "/"
                       + this->current_src_file_name.split ( "/" ).at ( this->current_src_file_name.split ( "/" ).count()-1 ) ;

               qDebug() << "local file: " << this->current_src_file_name
                       << "remote file:" << this->current_dest_file_name
                       << "local full file path: "<< local_full_path ;

               transfer_ret = this->do_download ( this->current_src_file_name ,  local_full_path , 0 );
           }
           //将目录下载到目录
           else if( is_dir( GlobalOption::instance()->locale_codec->fromUnicode( this->current_dest_file_name ).data() )
                    && remote_is_dir(  this->src_ssh2_sftp ,  this->current_src_file_name ) )
           {
               qDebug()<<"downloading dir to dir ...";
               //this->sleep(debug_sleep_time);
               //列出本远程目录中的文件，加入到队列中，然后继续。
              
               fileinfos.clear();
               transfer_ret = fxp_do_ls_dir(  this->src_ssh2_sftp ,  this->current_src_file_name+"/" ,  fileinfos);
               qDebug()<<"ret:"<<transfer_ret<<" file count:"<<fileinfos.size();
               
               // local dir = curr local dir +  curr remote dir 的最后一层目录
               temp_local_file_pair = QPair<QString,QString>( QString("file://"
#ifdef WIN32
					   "/"
#endif	
				   )+ this->current_dest_file_name+"/"+this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1) ,this->current_src_file_type );
               //确保本地有这个目录。
			   transfer_ret = fxp_local_do_mkdir( GlobalOption::instance()->locale_codec->fromUnicode( QUrl(temp_local_file_pair.first).path() ).data() ) ;
               qDebug()<<" fxp_local_do_mkdir: "<<transfer_ret <<" "<< temp_local_file_pair.first  ;
               //加入到任务队列
               for(int i = 0 ; i < fileinfos.size() ; i ++)
               {
                   temp_remote_file_pair = QPair<QString,QString>(  QString("nrsftp://%1:%2@%3:%4").arg(current_src_url.userName()).arg(current_src_url.password()).arg(current_src_url.host()).arg(current_src_url.port())+this->current_src_file_name+"/"+fileinfos.at(i)['N']  , fileinfos.at(i)['T'] );

                   this->transfer_ready_queue.push_back( QPair<QPair<QString ,QString>,QPair<QString,QString> >(  temp_remote_file_pair, temp_local_file_pair ) ) ;
                   //romote is source 
               }
           }
           else
           {
               //其他的情况暂时不考虑处理。跳过。
               //TODO return a error value , not only error code 
               this->error_code = 1 ;
               //assert( 1 == 2 ) ; 
               qDebug()<<"Unexpected transfer type: "<<__FILE__<<" in " << __LINE__ ;
           }
       }
       else  if( current_src_url.scheme() == "nrsftp" && current_dest_url.scheme() == "nrsftp")
       {
           emit this->transfer_new_file_started(this->current_src_file_name);
               //处理nrsftp协议
           if( this->src_ssh2_sess == 0 || this->src_ssh2_sftp == 0 )
           {
               emit  transfer_log("Connecting to destionation host ...");
               QString tmp_passwd = current_src_url.password();
               if(tmp_passwd.indexOf("_whywenhao_") != -1){
                  tmp_passwd = tmp_passwd.replace("_whywenhao_","?");
               }
               rhct = new RemoteHostConnectThread ( current_src_url.userName() , tmp_passwd ,current_src_url.host() );
               rhct->run();
                //TODO get status code and then ...
               this->src_ssh2_sess = (LIBSSH2_SESSION*)rhct->get_ssh2_sess();
               this->src_ssh2_sock = rhct->get_ssh2_sock();
               this->src_ssh2_sftp = libssh2_sftp_init(this->src_ssh2_sess);
               delete rhct ; rhct = 0 ;
               emit  transfer_log("Connect done.");
           }
           if( this->dest_ssh2_sess == 0 || this->dest_ssh2_sftp == 0 )
           {
               emit  transfer_log("Connecting to source host ...");
               QString tmp_passwd = current_dest_url.password();
               if(tmp_passwd.indexOf("_whywenhao_") != -1){
                  tmp_passwd = tmp_passwd.replace("_whywenhao_","?");
               }
               rhct = new RemoteHostConnectThread ( current_dest_url.userName() , tmp_passwd ,current_dest_url.host() );
               rhct->run();
                //TODO get status code and then ...
               this->dest_ssh2_sess = (LIBSSH2_SESSION*)rhct->get_ssh2_sess();
               this->dest_ssh2_sock = rhct->get_ssh2_sock();
               this->dest_ssh2_sftp = libssh2_sftp_init(this->dest_ssh2_sess);
               delete rhct ; rhct = 0 ;
               emit  transfer_log("Connect done.");
           }
           ////////////
           if( remote_is_dir(this->src_ssh2_sftp, this->current_src_file_name) 
               && remote_is_dir(this->dest_ssh2_sftp,this->current_dest_file_name ) )
           {
                qDebug()<<" nrsftp exchage dir to dir...";
               fileinfos.clear();
//                this->current_src_file_name = url.path() ;
//                this->current_dest_file_name = dest_path ;
        
               transfer_ret = fxp_do_ls_dir(  this->src_ssh2_sftp ,  this->current_src_file_name+"/" ,  fileinfos);
               qDebug()<<"ret:"<<transfer_ret<<" file count:"<<fileinfos.size();
               
               temp_remote_file_pair = QPair<QString,QString>( QString("nrsftp://%1:%2@%3:%4").arg(current_dest_url.userName()).arg(current_dest_url.password()).arg(current_dest_url.host()).arg(current_dest_url.port()) + this->current_dest_file_name + "/" + this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1)   ,  this->current_src_file_type  );
               //为远程建立目录
               memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
               transfer_ret = libssh2_sftp_mkdir(this->dest_ssh2_sftp, GlobalOption::instance()->remote_codec->fromUnicode( QUrl(temp_remote_file_pair.first).path() ).data(), 0755);
               qDebug()<<" libssh2_sftp_mkdir : "<< transfer_ret <<" :"<< temp_remote_file_pair.first;

               //添加到队列当中
               for( int i = 0 ; i < fileinfos.size() ; i ++ )
               {
                   temp_local_file_pair = QPair<QString,QString>( QString("nrsftp://%1:%2@%3:%4").arg(current_src_url.userName()).arg(current_src_url.password()).arg(current_src_url.host()).arg(current_src_url.port()) +  this->current_src_file_name+"/"+ fileinfos.at(i)['N']  , fileinfos.at(i)['T'] ) ;
                   
                   this->transfer_ready_queue.push_back( QPair<QPair<QString ,QString>,QPair<QString,QString> >( temp_local_file_pair,temp_remote_file_pair ) ); 
               }
           }
           else if ( remote_is_reg(this->src_ssh2_sftp, this->current_src_file_name) 
                     && remote_is_dir(this->dest_ssh2_sftp,this->current_dest_file_name ) )
           {
               qDebug()<<" nrsftp exchage file to dir...";
               QString dest_full_path = this->current_dest_file_name + "/" + this->current_src_file_name.split("/").at(this->current_src_file_name.split("/").count()-1);
               transfer_ret = this->do_nrsftp_exchange(this->current_src_file_name,dest_full_path );
           }
           else
           {
				//其他的情况暂时不考虑处理。跳过
				//TODO return a error value , not only error code
                qDebug()<<"src: "<< current_src_url<<" dest:"<< current_dest_url ;
				this->error_code = 1 ;
				//assert ( 1 == 2 ) ;
                qDebug()<<"Unexpected transfer type: "<<__FILE__<<" in " << __LINE__ ;
           }
       }
       else
       {
           qDebug()<<"src: "<< current_src_url<<" dest:"<< current_dest_url ;
           this->error_code = 2 ;
           assert( 1 == 2 ) ;
       }
       
       this->transfer_ready_queue.erase(this->transfer_ready_queue.begin());
       this->transfer_done_queue.push_back( QPair<QPair<QString ,QString>,QPair<QString,QString> >( local_file_pair,remote_file_pair)); 
    }while(this->transfer_ready_queue.size() > 0 && user_canceled == false) ;

    
    qDebug() << " transfer_ret :" << transfer_ret << " ssh2 sftp shutdown:"<< this->src_ssh2_sftp<<" "<<this->dest_ssh2_sftp;//libssh2_sftp_shutdown( this->dest_ssh2_sftp );
    //TODO 选择性关闭 ssh2 会话，有可能是 src  ,也有可能是dest 
    if(this->src_ssh2_sftp != 0 ){
        libssh2_sftp_shutdown(this->src_ssh2_sftp);
        this->src_ssh2_sftp = 0 ;
    }
    if(this->src_ssh2_sess != 0 ){
        libssh2_session_free(this->src_ssh2_sess);
        this->src_ssh2_sess = 0 ;
    }
    if(this->src_ssh2_sock > 0 ) {
#ifdef WIN32
            ::closesocket(this->src_ssh2_sock);
#else  
            ::close(this->src_ssh2_sock);
#endif
            this->src_ssh2_sock = -1;
    }
    if(this->dest_ssh2_sftp != 0 ){
        libssh2_sftp_shutdown(this->dest_ssh2_sftp);
        this->dest_ssh2_sftp = 0 ;
    }
    if(this->dest_ssh2_sess != 0 ){
        libssh2_session_free(this->dest_ssh2_sess);
        this->dest_ssh2_sess = 0 ;
    }
    if(this->dest_ssh2_sock > 0 ) {
#ifdef WIN32
            ::closesocket(this->dest_ssh2_sock);
#else  
            ::close(this->dest_ssh2_sock);
#endif
            this->dest_ssh2_sock = -1;
    }
    if(user_canceled == true){
        this->error_code = 3;
    }
}

// void TransferThread::set_remote_connection ( void* ssh2_sess )
// {
//     this->dest_ssh2_sess = (LIBSSH2_SESSION*) ssh2_sess;
//     //使用共用的ssh2_session 并重新打开一个新的 ssh2_sftp channel , 并在此实例结束的时候关闭这个 sftp channel 
//     this->dest_ssh2_sftp = libssh2_sftp_init( this->dest_ssh2_sess );
//     qDebug()<<" sftp init: "<< this->dest_ssh2_sftp ;
//     assert( this->dest_ssh2_sftp != 0 );
//     //this->dest_ssh2_sftp = (LIBSSH2_SFTP * ) ssh2_sftp ;
// //     this->dest_ssh2_sock = ssh2_sock ;
// }

void TransferThread::set_transfer_info ( /*int type ,*/ QStringList src_file_names,QStringList dest_file_names  )
{
	//this->transfer_type = type ;

    this->src_file_names = src_file_names ;
    this->dest_file_names = dest_file_names ;

    QString src_file_name ;
    QString dest_file_name ;
    
    //std::pair<std::string , std::string> local_file_pair;
    //std::pair<std::string , std::string> remote_file_pair;
    QPair<QString , QString > src_file_pair ;
	QPair<QString , QString > dest_file_pair ;
    
    for( int i = 0 ; i < src_file_names.count() ; i ++ )
    {
        src_file_name = src_file_names.at(i);
        src_file_pair = QPair<QString,QString>(src_file_name,"");
        for(int j = 0 ; j < dest_file_names.count() ; j ++ )
        {
            dest_file_name = dest_file_names.at(j);
            dest_file_pair = QPair<QString,QString>(dest_file_name,"");
            this->transfer_ready_queue.	push_back( QPair<QPair<QString,QString>,QPair<QString,QString> > (src_file_pair,dest_file_pair));            
        }
    }

//     if( type == TransferThread::TRANSFER_PUT )
//     {
//         assert( remote_file_names.count() == 1 );
//         remote_file_name = remote_file_names.at(0);
//         for(int i = 0 ; i < local_file_names.count() ; i ++ )
//         {
//             local_file_name = local_file_names.at(i);
//             local_file_pair = QPair<QString,QString>(local_file_name,"");
//             remote_file_pair = QPair<QString,QString>(remote_file_name,"");
//         
//             this->transfer_ready_queue.	push_back( QPair<QPair<QString,QString>,QPair<QString,QString> > (local_file_pair,remote_file_pair));
//         }
//     }
//     else if( type == TransferThread::TRANSFER_GET )
//     {
//         assert( local_file_names.count() == 1 );
//         local_file_name = local_file_names.at(0);
//         for(int i = 0 ; i < remote_file_names.count() ; i ++ )
//         {
//             remote_file_name = remote_file_names.at(i);            
//             local_file_pair = QPair<QString,QString>(local_file_name ,"");
//             remote_file_pair = QPair<QString,QString>(remote_file_name ,"");
//         
//             this->transfer_ready_queue.	push_back( QPair<QPair<QString,QString>,QPair<QString,QString> >  (local_file_pair,remote_file_pair) );
//         }
//     }
//     else
//     {
//         assert( 1 == 2 );   
//     }
}

int TransferThread::do_download ( QString remote_path, QString local_path,  int pflag )
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    qDebug()<< "remote_path = "<<  remote_path  << " , local_path = " << local_path ;
    int pcnt = 0 ;
    int  rlen , wlen  ;
    int file_size , tran_len = 0   ;
    LIBSSH2_SFTP_HANDLE * sftp_handle ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    char buff[5120] = {0};
    
	sftp_handle = libssh2_sftp_open(this->src_ssh2_sftp, GlobalOption::instance()->remote_codec->fromUnicode(remote_path),LIBSSH2_FXF_READ,0);
    if( sftp_handle == NULL )
    {
        //TODO 错误消息通知用户。
        qDebug()<<"open sftp file error :"<< libssh2_sftp_last_error(this->src_ssh2_sftp);
        return -1 ;
    }
    
    memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
    libssh2_sftp_fstat(sftp_handle,&ssh2_sftp_attrib);
    file_size = ssh2_sftp_attrib.filesize;
    qDebug()<<" remote file size :"<< file_size ;
    emit this->transfer_got_file_size( file_size );
    
    // 本地编码 --> Qt 内部编码
    QFile q_file (  local_path  ) ;
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
                pcnt = 100.0 *((double)tran_len  / (double)file_size);
                emit this->transfer_percent_changed ( pcnt , tran_len ,wlen );
            }
            if(user_canceled == true){
                break;
            }
        }
        //::close(local_fd);

        q_file.close();
    }
    
    libssh2_sftp_close(sftp_handle);

	return ( 0 );
}


int TransferThread::do_upload ( QString local_path, QString remote_path, int pflag )
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    qDebug()<< "remote_path = "<<  remote_path  << " , local_path = " << local_path ;

    int pcnt = 0 ;
    int local_fd , rlen , wlen  ;
    int file_size , tran_len = 0   ;
    LIBSSH2_SFTP_HANDLE * sftp_handle ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib;
    struct stat     file_stat ;
    char buff[5120] = {0};

    //char native_path [PATH_MAX+1] = {0};
    
    sftp_handle = libssh2_sftp_open( this->dest_ssh2_sftp,
		GlobalOption::instance()->remote_codec->fromUnicode( remote_path ) ,
		LIBSSH2_FXF_READ|LIBSSH2_FXF_WRITE|LIBSSH2_FXF_CREAT|LIBSSH2_FXF_TRUNC,0666 ) ;
    if( sftp_handle == NULL )
    {
        //TODO 错误消息通知用户。
        qDebug()<<"open sftp file error :"<< libssh2_sftp_last_error(this->dest_ssh2_sftp);
        return -1 ;
    }
    
    memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
    //libssh2_sftp_fstat(sftp_handle,&ssh2_sftp_attrib);
    //file_size = ssh2_sftp_attrib.filesize;
    //qDebug()<<" remote file size :"<< file_size ;
    memset(&file_stat,0,sizeof(struct stat));
	::stat( GlobalOption::instance()->locale_codec->fromUnicode(local_path) , &file_stat );
    file_size = file_stat.st_size;
    qDebug()<<"local file size:" << file_size ;
    emit this->transfer_got_file_size( file_size ) ;
    
    //local_fd = ::open(native_path,O_RDONLY );
    //local_handle = ::fopen( native_path , "r");
    // 本地编码 --> Qt 内部编码
    QFile q_file(  local_path  ) ;
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
                pcnt = 100.0 *((double)tran_len  / (double)file_size);
                //qDebug()<< QString("100.0 *((double)%1  / (double)%2)").arg(tran_len).arg(file_size)<<" = "<<pcnt ;
                emit this->transfer_percent_changed ( pcnt , tran_len ,wlen  );
            }
            if(user_canceled == true){
                break;
            }
        }
        q_file.close();
    }
    libssh2_sftp_close(sftp_handle);
    
	return ( 0 );
}
int TransferThread::do_nrsftp_exchange( QString src_path , QString dest_path )
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    qDebug()<<"nrsftp from: "<< src_path <<" to "<< dest_path ;
    
    LIBSSH2_SFTP_HANDLE * src_sftp_handle = 0 ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib ;
    LIBSSH2_SFTP_HANDLE * dest_sftp_handle = 0 ;
    char buff[5120] = {0};
    int rlen , wlen , tran_len = 0 ;
    int file_size=0;
    int pcnt = 0 ;
    
    //检测传输文件的属性

    src_sftp_handle = libssh2_sftp_open( this->src_ssh2_sftp ,GlobalOption::instance()->remote_codec->fromUnicode(  src_path ).data()  ,LIBSSH2_FXF_READ,0666);
    if( src_sftp_handle == 0 )
    {
        qDebug()<<"sftp open error: "<< libssh2_session_last_error((LIBSSH2_SESSION*)(src_ssh2_sess),0,0,0) ; 
        assert( src_sftp_handle != 0 );
    }
    
    libssh2_sftp_fstat(src_sftp_handle,&ssh2_sftp_attrib);
    file_size = ssh2_sftp_attrib.filesize;
    qDebug()<<" source file size :"<< file_size ;
    emit this->transfer_got_file_size( file_size ) ;
    
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    
    assert( this->dest_ssh2_sftp != 0 ) ;
    dest_sftp_handle = libssh2_sftp_open( this->dest_ssh2_sftp ,
                                          GlobalOption::instance()->remote_codec->fromUnicode( dest_path  ).data() ,
                                          LIBSSH2_FXF_WRITE | LIBSSH2_FXF_CREAT | LIBSSH2_FXF_TRUNC , 0666 );
    if( src_sftp_handle == 0 )
    {
        qDebug()<<"sftp open error: "<< libssh2_session_last_error((LIBSSH2_SESSION*)(dest_ssh2_sess),0,0,0) ; 
        assert( dest_sftp_handle != 0 );
    }
    
    //TODO 如果是同一台SSH服务器，可以用libssh2_sftp_copy,不过现在还没这个函数
    for ( ; ; )
    {
		rlen = libssh2_sftp_read ( src_sftp_handle,buff,sizeof ( buff ) );
        if( rlen <= 0 )
        {
            qDebug()<<" may be read end "<< rlen << libssh2_session_last_error((LIBSSH2_SESSION*)(this->src_ssh2_sess),0,0,0) ;
            break;
        }
		wlen = libssh2_sftp_write ( dest_sftp_handle,buff,rlen );
		tran_len += wlen ;
		//qDebug() <<" read len :"<< rlen <<" , write len: "<< wlen << " tran len: "<< tran_len ;
        
        if( file_size == 0 )
        {
            emit this->transfer_percent_changed ( 100, tran_len , wlen );
        }
        else
        {
            pcnt = 100.0 *((double)tran_len  / (double)file_size);
            emit this->transfer_percent_changed ( pcnt , tran_len ,wlen  );
        }
        if(user_canceled == true){
            break;
        }
    }

    return (0);
}

void TransferThread::set_user_cancel( bool cancel )
{
    this->user_canceled = cancel ;
}

// on windows 有一个问题：当两个从本地到同一远程主机的目录上传时，导致下面的错误：
//Assertion failed: *lock == MUTEX_UNLOCKED, file ath.c, line 184
//这是mingw32平台上的libgcrypt相关的问题。
