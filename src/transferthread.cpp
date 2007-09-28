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
#include <sys/uio.h>

#include <cassert>

#include <QtCore>

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
               this->error_code = 1 ;
               assert( 1 == 2 ) ; 
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
               this->error_code = 1 ;
               assert( 1 == 2 ) ; 
           }
       }
       else
       {
            assert( 1 == 2 ) ; 
            this->error_code = 2 ;  
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
TransferThread::do_download (/* struct sftp_conn *conn,*/ char *remote_path, char *local_path,
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
        qDebug()<<"open sftp file error :"<< libssh2_sftp_last_error(ssh2_sftp);
        return -1 ;
    }
    
    memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
    libssh2_sftp_fstat(sftp_handle,&ssh2_sftp_attrib);
    file_size = ssh2_sftp_attrib.filesize;
    qDebug()<<" remote file size :"<< file_size ;
    
    local_fd = ::open(local_path,O_RDWR|O_TRUNC|O_CREAT,0666 );
    if( local_fd < 0 )
    {
        qDebug()<<"open local file error:"<< strerror(errno) ;        
    }
    else
    {
        //read remote file  and then write to local file
        while( (rlen = libssh2_sftp_read(sftp_handle,buff,sizeof(buff) ) ) > 0 )
        {
			wlen = ::write ( local_fd , buff , rlen );
            tran_len += wlen ;
			//qDebug() <<" read len :"<< rlen <<" , write len: "<< wlen 
            //        << " tran len: "<< tran_len ;
			//my progress signal
            if( file_size == 0 )
            {
                emit this->transfer_percent_changed ( 100 );
            }
            else
            {
                emit this->transfer_percent_changed ( tran_len * 100 / file_size );
            }
        }
        ::close(local_fd);
    }
    
    libssh2_sftp_close(sftp_handle);
    
// 	Attrib junk, *a;
// 	Buffer msg;
// 	char *handle;
// 	int local_fd, status = 0, write_error;
// 	int read_error, write_errno;
// 	u_int64_t offset, size;
// 	u_int handle_len, mode, type, id, buflen, num_req, max_req;
// 	off_t progress_counter;
// 	struct request
// 	{
// 		u_int id;
// 		u_int len;
// 		u_int64_t offset;
// 		TAILQ_ENTRY ( request ) tq;
// 	};
// 	TAILQ_HEAD ( reqhead, request ) requests;
// 	struct request *req;
// 
// 	TAILQ_INIT ( &requests );
// 
// 	a = do_stat ( conn, remote_path, 0 );
// 	if ( a == NULL )
// 		return ( -1 );
// 
// 	/* XXX: should we preserve set[ug]id? */
// 	if ( a->flags & SSH2_FILEXFER_ATTR_PERMISSIONS )
// 		mode = a->perm & 0777;
// 	else
// 		mode = 0666;
// 
// 	if ( ( a->flags & SSH2_FILEXFER_ATTR_PERMISSIONS ) &&
// 	        ( !S_ISREG ( a->perm ) ) )
// 	{
// 		printf ( "Cannot download non-regular file: %s\n", remote_path );
// 		return ( -1 );
// 	}
// 
// 	if ( a->flags & SSH2_FILEXFER_ATTR_SIZE )
// 		size = a->size;
// 	else
// 		size = 0;
// 
// 	buflen = conn->transfer_buflen;
// 	buffer_init ( &msg );
// 
// 	/* Send open request */
// 	id = conn->msg_id++;
// 	buffer_put_char ( &msg, SSH2_FXP_OPEN );
// 	buffer_put_int ( &msg, id );
// 	buffer_put_cstring ( &msg, remote_path );
// 	buffer_put_int ( &msg, SSH2_FXF_READ );
// 	attrib_clear ( &junk ); /* Send empty attributes */
// 	encode_attrib ( &msg, &junk );
// 	send_msg ( conn->fd_out, &msg );
// 	printf ( "Sent message SSH2_FXP_OPEN I:%u P:%s\n", id, remote_path );
// 
// 	handle = get_handle ( conn->fd_in, id, &handle_len );
// 	if ( handle == NULL )
// 	{
// 		buffer_free ( &msg );
// 		return ( -1 );
// 	}
// 
// 	local_fd = open ( local_path, O_WRONLY | O_CREAT | O_TRUNC,
// 	                  mode | S_IWRITE );
// 	if ( local_fd == -1 )
// 	{
// 		printf ( "Couldn't open local file \"%s\" for writing: %s\n",
// 		         local_path, strerror ( errno ) );
// 		buffer_free ( &msg );
// 		xfree ( handle );
// 		return ( -1 );
// 	}
// 
// 	/* Read from remote and write to local */
// 	write_error = read_error = write_errno = num_req = offset = 0;
// 	max_req = 1;
// 	progress_counter = 0;
//     
// 	if ( showprogress && size != 0 )
// 		start_progress_meter ( remote_path, size, &progress_counter );
// 
// 	while ( num_req > 0 || max_req > 0 )
// 	{
// 		char *data;
// 		u_int len;
// 
// 		/*
// 		      * Simulate EOF on interrupt: stop sending new requests and
// 		      * allow outstanding requests to drain gracefully
// 		      */
// 		if ( interrupted )
// 		{
// 			if ( num_req == 0 ) /* If we haven't started yet... */
// 				break;
// 			max_req = 0;
// 		}
// 
// 		/* Send some more requests */
// 		while ( num_req < max_req )
// 		{
// 			printf ( "Request range %llu -> %llu (%d/%d)\n",
// 			         ( unsigned long long ) offset,
// 			         ( unsigned long long ) offset + buflen - 1,
// 			         num_req, max_req );
// 			req = ( struct request * ) xmalloc ( sizeof ( *req ) );
// 			req->id = conn->msg_id++;
// 			req->len = buflen;
// 			req->offset = offset;
// 			offset += buflen;
// 			num_req++;
// 			TAILQ_INSERT_TAIL ( &requests, req, tq );
// 			send_read_request ( conn->fd_out, req->id, req->offset,
// 			                    req->len, handle, handle_len );
// 		}
// 
// 		buffer_clear ( &msg );
// 		get_msg ( conn->fd_in, &msg );
// 		type = buffer_get_char ( &msg );
// 		id = buffer_get_int ( &msg );
// 		printf ( "Received reply T:%u I:%u R:%d\n", type, id, max_req );
// 
// 		/* Find the request in our queue */
// 		for ( req = TAILQ_FIRST ( &requests );
// 		        req != NULL && req->id != id;
// 		        req = TAILQ_NEXT ( req, tq ) )
// 			;
// 		if ( req == NULL )
// 		{
// 			printf ( "Unexpected reply %u\n", id );
// 			return ( -87 );
// 		}
// 
// 		switch ( type )
// 		{
// 			case SSH2_FXP_STATUS:
// 				status = buffer_get_int ( &msg );
// 				if ( status != SSH2_FX_EOF )
// 					read_error = 1;
// 				max_req = 0;
// 				TAILQ_REMOVE ( &requests, req, tq );
// 				xfree ( req );
// 				num_req--;
// 				break;
// 			case SSH2_FXP_DATA:
// 				data = ( char* ) buffer_get_string ( &msg, &len );
// 				printf ( "Received data %llu -> %llu\n",
// 				         ( unsigned long long ) req->offset,
// 				         ( unsigned long long ) req->offset + len - 1 );
// 				if ( len > req->len )
// 				{
// 					printf ( "Received more data than asked for "
// 					         "%u > %u\n", len, req->len );
// 					return ( -89 );
// 				}
// 				if ( ( lseek ( local_fd, req->offset, SEEK_SET ) == -1 ||
// 				        atomicio ( vwrite, local_fd, data, len ) != len ) &&
// 				        !write_error )
// 				{
// 					write_errno = errno;
// 					write_error = 1;
// 					max_req = 0;
// 				}
// 				progress_counter += len;
// 				xfree ( data );
//                 //my progress signal
//                 printf("read byte %llu of %llu \n", ( unsigned long long ) req->offset + len  ,  (unsigned long long ) size ) ;
//                 emit this->transfer_percent_changed(
//                         (( unsigned long long ) req->offset + len) * 100 /                    (unsigned long long ) size         
//                                                    );
//                 /////////////
// 				if ( len == req->len )
// 				{
// 					TAILQ_REMOVE ( &requests, req, tq );
// 					xfree ( req );
// 					num_req--;
// 				}
// 				else
// 				{
// 					/* Resend the request for the missing data */
// 					printf ( "Short data block, re-requesting "
// 					         "%llu -> %llu (%2d)\n",
// 					         ( unsigned long long ) req->offset + len,
// 					         ( unsigned long long ) req->offset +
// 					         req->len - 1, num_req );
// 					req->id = conn->msg_id++;
// 					req->len -= len;
// 					req->offset += len;
// 					send_read_request ( conn->fd_out, req->id,
// 					                    req->offset, req->len, handle, handle_len );
// 					/* Reduce the request size */
// 					if ( len < buflen )
// 						buflen = MAX ( MIN_READ_SIZE, len );
// 				}
// 				if ( max_req > 0 )
// 				{ /* max_req = 0 iff EOF received */
// 					if ( size > 0 && offset > size )
// 					{
// 						/* Only one request at a time
// 						                  * after the expected EOF */
// 						printf ( "Finish at %llu (%2d)\n",
// 						         ( unsigned long long ) offset,
// 						         num_req );
// 						max_req = 1;
// 					}
// 					else if ( max_req <= conn->num_requests )
// 					{
// 						++max_req;
// 					}
// 				}
// 				break;
// 			default:
// 				printf ( "Expected SSH2_FXP_DATA(%u) packet, got %u\n",
// 				         SSH2_FXP_DATA, type );
// 				return ( -92 );
// 		}
// 	}
// 
// 	if ( showprogress && size )
// 		stop_progress_meter();
// 
// 	/* Sanity check */
// 	if ( TAILQ_FIRST ( &requests ) != NULL )
// 	{
// 		printf ( "Transfer complete, but requests still in queue\n" );
// 		return ( -93 );
// 	}
// 
// 	if ( read_error )
// 	{
// 		printf ( "Couldn't read from remote file \"%s\" : %s\n",
// 		         remote_path, fx2txt ( status ) );
// 		do_close ( conn, handle, handle_len );
// 	}
// 	else if ( write_error )
// 	{
// 		printf ( "Couldn't write to \"%s\": %s\n", local_path,
// 		         strerror ( write_errno ) );
// 		status = -1;
// 		do_close ( conn, handle, handle_len );
// 	}
// 	else
// 	{
// 		status = do_close ( conn, handle, handle_len );
// 
// 		/* Override umask and utimes if asked */
// #ifdef HAVE_FCHMOD
// 		if ( pflag && fchmod ( local_fd, mode ) == -1 )
// #else
// 		if ( pflag && chmod ( local_path, mode ) == -1 )
// #endif /* HAVE_FCHMOD */
// 		{
// 			printf ( "Couldn't set mode on \"%s\": %s\n", local_path,
// 			         strerror ( errno ) );
// 		}
// 		if ( pflag && ( a->flags & SSH2_FILEXFER_ATTR_ACMODTIME ) )
// 		{
// 			struct timeval tv[2];
// 			tv[0].tv_sec = a->atime;
// 			tv[1].tv_sec = a->mtime;
// 			tv[0].tv_usec = tv[1].tv_usec = 0;
// 			if ( utimes ( local_path, tv ) == -1 )
// 				printf ( "Can't set times on \"%s\": %s\n",
// 				         local_path, strerror ( errno ) );
// 		}
// 	}
// 	close ( local_fd );
// 	buffer_free ( &msg );
// 	xfree ( handle );
// 
// 	return ( status );
// 

	return ( 0 );
}


int
TransferThread::do_upload (/* struct sftp_conn *conn,*/ char *local_path, char *remote_path,
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
    
    sftp_handle = libssh2_sftp_open( ssh2_sftp,remote_path,LIBSSH2_FXF_READ|LIBSSH2_FXF_WRITE|LIBSSH2_FXF_CREAT|LIBSSH2_FXF_TRUNC,0666 ) ;
    if( sftp_handle == NULL )
    {
        qDebug()<<"open sftp file error :"<< libssh2_sftp_last_error(ssh2_sftp);
        return -1 ;
    }
    
    memset(&ssh2_sftp_attrib,0,sizeof(ssh2_sftp_attrib));
    //libssh2_sftp_fstat(sftp_handle,&ssh2_sftp_attrib);
    //file_size = ssh2_sftp_attrib.filesize;
    //qDebug()<<" remote file size :"<< file_size ;
    memset(&file_stat,0,sizeof(struct stat));
    ::stat(local_path,&file_stat);
    file_size = file_stat.st_size;
    qDebug()<<"local file size:" << file_size ;
    
    local_fd = ::open(local_path,O_RDWR );
    if( local_fd < 0 )
    {
        qDebug()<<"open local file error:"<< strerror(errno) ;        
    }
    else
    {
        //read local file and then write to remote file
        while( ( rlen = ::read(local_fd,buff,sizeof(buff))) > 0 )
        {
            wlen = libssh2_sftp_write(sftp_handle,buff,rlen);
            tran_len += wlen ;
// 			qDebug() <<" read len :"<< rlen <<" , write len: "<< wlen 
//                    << " tran len: "<< tran_len ;
            if( file_size == 0 )
            {
                emit this->transfer_percent_changed ( 100 );
            }
            else
            {
                emit this->transfer_percent_changed ( tran_len * 100 / file_size );
            }
        }
    }
    
    libssh2_sftp_close(sftp_handle);
    
// 	int local_fd, status;
// 	u_int handle_len, id, type;
// 	u_int64_t offset;
// 	char *handle, *data;
// 	Buffer msg;
// 	struct stat sb;
// 	Attrib a;
// 	u_int32_t startid;
// 	u_int32_t ackid;
// 	struct outstanding_ack
// 	{
// 		u_int id;
// 		u_int len;
// 		u_int64_t offset;
// 		TAILQ_ENTRY ( outstanding_ack ) tq;
// 	};
// 	TAILQ_HEAD ( ackhead, outstanding_ack ) acks;
// 	struct outstanding_ack *ack = NULL;
// 
// 	TAILQ_INIT ( &acks );
// 
// 	if ( ( local_fd = open ( local_path, O_RDONLY, 0 ) ) == -1 )
// 	{
// 		printf ( "Couldn't open local file \"%s\" for reading: %s\n",
// 		         local_path, strerror ( errno ) );
// 		return ( -1 );
// 	}
// 
// 	if ( fstat ( local_fd, &sb ) == -1 )
// 	{
// 		printf ( "Couldn't fstat local file \"%s\": %s\n",
// 		         local_path, strerror ( errno ) );
// 		close ( local_fd );
// 		return ( -2 );
// 	}
// 	if ( !S_ISREG ( sb.st_mode ) )
// 	{
// 		printf ( "%s is not a regular file\n", local_path );
// 		close ( local_fd );
// 		return ( -3 );
// 	}
// 	stat_to_attrib ( &sb, &a );
// 
// 	a.flags &= ~SSH2_FILEXFER_ATTR_SIZE;
// 	a.flags &= ~SSH2_FILEXFER_ATTR_UIDGID;
// 	a.perm &= 0777;
// 	if ( !pflag )
// 		a.flags &= ~SSH2_FILEXFER_ATTR_ACMODTIME;
// 
// 	buffer_init ( &msg );
// 
// 	/* Send open request */
// 	id = conn->msg_id++;
// 	buffer_put_char ( &msg, SSH2_FXP_OPEN );
// 	buffer_put_int ( &msg, id );
// 	buffer_put_cstring ( &msg, remote_path );
// 	buffer_put_int ( &msg, SSH2_FXF_WRITE|SSH2_FXF_CREAT|SSH2_FXF_TRUNC );
// 	encode_attrib ( &msg, &a );
// 	send_msg ( conn->fd_out, &msg );
// 	printf ( "Sent message SSH2_FXP_OPEN I:%u P:%s\n", id, remote_path );
// 
// 	buffer_clear ( &msg );
// 
// 	handle = get_handle ( conn->fd_in, id, &handle_len );
// 	if ( handle == NULL )
// 	{
// 		close ( local_fd );
// 		buffer_free ( &msg );
// 		return ( -4 );
// 	}
// 
// 	startid = ackid = id + 1;
// 	data = ( char* ) xmalloc ( conn->transfer_buflen );
// 
// 	/* Read from local and write to remote */
// 	offset = 0;
// 	if ( showprogress )
// 		start_progress_meter ( local_path, sb.st_size, ( off_t* ) &offset );
// 
// 	for ( ;; )
// 	{
// 		int len;
// 
// 		/*
// 		      * Can't use atomicio here because it returns 0 on EOF,
// 		      * thus losing the last block of the file.
// 		      * Simulate an EOF on interrupt, allowing ACKs from the
// 		      * server to drain.
// 		      */
// 		if ( interrupted )
// 			len = 0;
// 		else do
// 				len = read ( local_fd, data, conn->transfer_buflen );
// 			while ( ( len == -1 ) && ( errno == EINTR || errno == EAGAIN ) );
// 
// 		if ( len == -1 )
// 		{
// 			printf ( "Couldn't read from \"%s\": %s\n", local_path,
// 			         strerror ( errno ) );
// 		}
// 
// 		if ( len != 0 )
// 		{
// 			ack = ( struct outstanding_ack * ) xmalloc ( sizeof ( *ack ) );
// 			ack->id = ++id;
// 			ack->offset = offset;
// 			ack->len = len;
// 			TAILQ_INSERT_TAIL ( &acks, ack, tq );
// 
// 			buffer_clear ( &msg );
// 			buffer_put_char ( &msg, SSH2_FXP_WRITE );
// 			buffer_put_int ( &msg, ack->id );
// 			buffer_put_string ( &msg, handle, handle_len );
// 			buffer_put_int64 ( &msg, offset );
// 			buffer_put_string ( &msg, data, len );
// 			send_msg ( conn->fd_out, &msg );
// 			printf ( "Sent message SSH2_FXP_WRITE I:%u O:%llu S:%u\n",
// 			         id, ( unsigned long long ) offset, len );
// 		}
// 		else if ( TAILQ_FIRST ( &acks ) == NULL )
// 			break;
// 
// 		if ( ack == NULL )
// 		{
// 			printf ( "Unexpected ACK %u\n", id );
// 			return -6;
// 		}
// 
// 		if ( id == startid || len == 0 ||
// 		        id - ackid >= conn->num_requests )
// 		{
// 			u_int r_id;
// 
// 			buffer_clear ( &msg );
// 			get_msg ( conn->fd_in, &msg );
// 			type = buffer_get_char ( &msg );
// 			r_id = buffer_get_int ( &msg );
// 
// 			if ( type != SSH2_FXP_STATUS )
// 			{
// 				printf ( "Expected SSH2_FXP_STATUS(%d) packet, "
// 				         "got %d\n", SSH2_FXP_STATUS, type );
// 			}
// 
// 			status = buffer_get_int ( &msg );
// 			printf ( "SSH2_FXP_STATUS %d\n", status );
// 
// 			/* Find the request in our queue */
// 			for ( ack = TAILQ_FIRST ( &acks );
// 			        ack != NULL && ack->id != r_id;
// 			        ack = TAILQ_NEXT ( ack, tq ) )
// 				;
// 			if ( ack == NULL )
// 			{
// 				printf ( "Can't find request for ID %u\n", r_id );
// 			}
// 			TAILQ_REMOVE ( &acks, ack, tq );
// 
// 			if ( status != SSH2_FX_OK )
// 			{
// 				printf ( "Couldn't write to remote file \"%s\": %s\n",
// 				         remote_path, fx2txt ( status ) );
// 				if ( showprogress )
// 					stop_progress_meter();
// 				do_close ( conn, handle, handle_len );
// 				close ( local_fd );
// 				xfree ( data );
// 				xfree ( ack );
// 				status = -1;
// 				goto done;
// 			}
// 			printf ( "In write loop, ack for %u %u bytes at %llu\n",
// 			         ack->id, ack->len, ( unsigned long long ) ack->offset );
// 			printf ( " total size: %llu , conn buff len: %d\n",
// 			         ( unsigned long long ) sb.st_size,
// 			         conn->transfer_buflen );
// 			emit transfer_percent_changed ( ( ( unsigned long long ) ack->offset ) *100/ ( unsigned long long ) sb.st_size );
// 			++ackid;
// 			xfree ( ack );
// 		}
// 		offset += len;
// 	}
// 	if ( showprogress )
// 		stop_progress_meter();
// 	xfree ( data );
// 
// 	if ( close ( local_fd ) == -1 )
// 	{
// 		printf ( "Couldn't close local file \"%s\": %s\n", local_path,
// 		         strerror ( errno ) );
// 		do_close ( conn, handle, handle_len );
// 		status = -1;
// 		goto done;
// 	}
// 
// 	/* Override umask and utimes if asked */
// 	if ( pflag )
// 		do_fsetstat ( conn, handle, handle_len, &a );
// 
// 	status = do_close ( conn, handle, handle_len );
// 
// done:
// 	xfree ( handle );
// 	buffer_free ( &msg );
// 	return ( status );
// 

	return ( 0 );
}




