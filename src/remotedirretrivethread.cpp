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
#include <vector>
#include <map>
#include <string>

#include <QtCore>

#ifdef WIN32
#include <winsock2.h>
#endif

#include "globaloption.h"
#include "remotedirretrivethread.h"

#include "utils.h"

//这种声明方法是为什么呢？
// int remote_glob(struct sftp_conn *, const char *, int,
//                 int (*)(const char *, int), glob_t *); /* proto for sftp-glob.c */
                
////////////////////////directory_tree_item
directory_tree_item::~directory_tree_item()
{
    //qDebug()<<"tree delete now";
    int line = this->child_items.size();
    for(int i = line -1 ; i >=0 ; i --)
    {
        delete this->child_items[i];
    }
}

///////////////////////////////////
RemoteDirRetriveThread::RemoteDirRetriveThread (QObject* parent ) : QThread ( parent )
{

}


RemoteDirRetriveThread::~RemoteDirRetriveThread()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    libssh2_sftp_shutdown(this->ssh2_sftp);
    libssh2_session_disconnect( this->ssh2_sess,"SSH_DISCONNECT_BY_APPLICATION" );
    libssh2_session_free( this->ssh2_sess );
    #ifdef WIN32    
    //::closesocket( this->ssh2_sock ) ;
    #else
    //::close( this->ssh2_sock );
    #endif
    
    //TODO delete model data , ok , delete it in RemoteDirModel class
}

void RemoteDirRetriveThread::set_ssh2_handler( void * ssh2_sess /*, void * ssh2_sftp, int ssh2_sock*/ )
{
    this->ssh2_sess = (LIBSSH2_SESSION*) ssh2_sess ;
    this->ssh2_sftp = libssh2_sftp_init( this->ssh2_sess ) ;
    qDebug()<<" sftp init: "<< this->ssh2_sftp ;
    assert( this->ssh2_sftp != 0 );
//     this->ssh2_sftp = (LIBSSH2_SFTP * ) ssh2_sftp ;
//     this->ssh2_sock = ssh2_sock ;
}

void RemoteDirRetriveThread::run()
{
    emit enter_remote_dir_retrive_loop();
    
    int exec_ret_code = -1 ;
    command_queue_elem * cmd_elem;
    while( this->command_queue.size() > 0 )
    {
        cmd_elem = this->command_queue.at(0);
        switch(cmd_elem->cmd)
        {
            case SSH2_FXP_READDIR:
                this->dir_node_process_queue.insert(std::make_pair(cmd_elem->parent_item,cmd_elem->parent_model_internal_pointer) );
                
                exec_ret_code = this->retrive_dir();
                
                break;
            case SSH2_FXP_MKDIR:
                exec_ret_code = this->mkdir();
                break;
            case SSH2_FXP_RMDIR:
                exec_ret_code = this->rmdir();
                break;
            case SSH2_FXP_REMOVE:
                exec_ret_code = this->rm_file_or_directory_recursively();
                break;
            case SSH2_FXP_RENAME:
                exec_ret_code = this->rename();
                break ;
            case SSH2_FXP_KEEP_ALIVE:
                exec_ret_code = this->keep_alive();
                break ;
            default:    
                qDebug()<<tr("unknown command:")<<cmd_elem->cmd;
                
                break;
        }
        //delet item form queue , stopping infinite cycle
        this->command_queue.erase(this->command_queue.begin());
        delete cmd_elem ; cmd_elem = 0 ;
    }

    emit this->leave_remote_dir_retrive_loop();
    
}

int  RemoteDirRetriveThread::retrive_dir()
{

    int exec_ret = -1;
    
    directory_tree_item * parent_item;
    void *  parent_model_internal_pointer ;
    
    QVector< QMap< char, QString > > fileinfos;
    char file_name[PATH_MAX+1] ;
    //char strip_path[PATH_MAX+1];
    int row_count = 0 ;
    int fxp_ls_ret = 0 ;
    char file_size[20] = {0};
    char file_date[32] = {0};
    char file_type[20] = {0};

    
    while ( this->dir_node_process_queue.size() >0 )
    {
        std::map<directory_tree_item*,void * >::iterator mit;
        mit = this->dir_node_process_queue.begin();

        parent_item = mit->first ;
        parent_model_internal_pointer = mit->second ;
       
        //strcpy ( file_name, ( parent_item->strip_path+ ( "/" ) ).toAscii().data() );
		//strcpy ( strip_path,parent_item->strip_path .toAscii().data()  );

        fileinfos.clear();

        LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib ;
        memset(&ssh2_sftp_attrib,0,sizeof(LIBSSH2_SFTP_ATTRIBUTES));
		LIBSSH2_SFTP_HANDLE * ssh2_sftp_handle = libssh2_sftp_opendir(this->ssh2_sftp,GlobalOption::instance()->remote_codec->fromUnicode(parent_item->strip_path+ ( "/" )).data());

        // ssh2_sftp_handle == 0 是怎么回事呢？ 返回值 应该是
        // 1 . 这个file_name 是一个链接，但这个链接指向的是一个普通文件而不是目录时libssh2_sftp_opendir返回0 , 而 libssh2_sftp_last_error 返回值为 2 == SSH2_FX_NO_SUCH_FILE
        if( ssh2_sftp_handle == 0 )
        {
            qDebug()<<" sftp last error: "<< libssh2_sftp_last_error( this->ssh2_sftp )
				<<(parent_item->strip_path+ ( "/" ))<<GlobalOption::instance()->remote_codec->fromUnicode(parent_item->strip_path+ ( "/" )).data();
        }
        while( ssh2_sftp_handle != 0 &&
               libssh2_sftp_readdir( ssh2_sftp_handle ,file_name , PATH_MAX ,& ssh2_sftp_attrib ) > 0 )
        {

            QMap<char,QString> thefile ;
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
			thefile.insert( 'N',QString( GlobalOption::instance()->remote_codec->toUnicode(file_name )) );
            thefile.insert( 'T',QString( file_type ) );
            thefile.insert( 'S',QString( file_size ) );
            thefile.insert( 'D',QString( file_date ) );  
                      
            fileinfos.push_back(thefile);
            memset(&ssh2_sftp_attrib,0,sizeof(LIBSSH2_SFTP_ATTRIBUTES) );
        }  

        //qDebug() <<"fxp_ls_ret:" << fxp_ls_ret<< ", fileinfos number="<<fileinfos.size() << " use strip:" << strip_path <<" file_name ="<< file_name ;
	
        //将已经存在的项目找出来
        QMap<QString,int> existed_items ;
        for( int i = 0 ; i < parent_item->child_items.size() ; i ++)
        {
            existed_items.insert( parent_item->child_items[i]->file_name,8 );
        }
        int  curr_count = 0 ;

        ////////////
        for ( int i = 0 ; i < fileinfos.size() ; i ++ )
        {
            curr_count = parent_item->child_items.size();
            
            if( existed_items.count(fileinfos.at(i)['N']) > 0 )
            {
                qDebug()<<"also in model,skipped";
                continue ;
            }
            
            directory_tree_item * thefile = new directory_tree_item();
            
            
            if(fileinfos.at ( i ) ['T'][0] == 'd' ||
               fileinfos.at ( i ) ['T'][0] == 'l' )
                thefile->retrived = 0;
            else    //对非目录就不需要再让它再次列目录了
                thefile->retrived = 9 ;
            
            thefile->parent_item = parent_item ;
            thefile->row_number= curr_count ;
 
            thefile->strip_path =   parent_item->strip_path  +  QString( "/" ) +  fileinfos.at ( i ) ['N'] ;

            thefile->file_type = fileinfos.at ( i ) ['T'];
            thefile->file_size = fileinfos.at ( i ) ['S'];
            thefile->file_name = fileinfos.at ( i ) ['N'] ;
            thefile->file_date = fileinfos.at ( i ) ['D'];

			//parent_item->child_items.insert ( std::make_pair ( i,thefile ) ) ;
            parent_item->child_items.insert ( std::make_pair ( curr_count , thefile ) ) ;
        }
        
        this->subtract_existed_model( parent_item , fileinfos );

        parent_item->prev_retr_flag = parent_item->retrived ;
        parent_item->retrived = 9 ;

        //         
        //         //////
        this->dir_node_process_queue.erase(parent_item);
        emit this->remote_dir_node_retrived(parent_item,parent_model_internal_pointer);

        if(ssh2_sftp_handle!=0) //TODO 应该在循环上面检测到为0就continue才对啊。
            libssh2_sftp_closedir(ssh2_sftp_handle);

    }

    return exec_ret ;
}


void RemoteDirRetriveThread::subtract_existed_model(directory_tree_item * parent_item ,
													QVector<QMap<char,QString> > & new_items )
{
    assert( new_items.size() <= parent_item->child_items.size() );
    QString  tmp_file_name ;
    QMap<QString,int> new_file_items ;
    
    std::vector<directory_tree_item*> missed_item_handle;
    
    directory_tree_item * missed_item , *temp_item ;
    
    if( new_items.count() != parent_item->child_items.size() )
    {
        qDebug()<<" some item miss in new file list";
        for(int i = 0 ; i < new_items.count() ; i ++ )
        {
            new_file_items.insert( new_items.at(i)['N'],1 );
        }
        for( int i = 0 ; i < parent_item->child_items.size() ; i ++ )
        {
            tmp_file_name = parent_item->child_items[i]->file_name;
            if( new_file_items.count(tmp_file_name) == 0 )
            {
                qDebug()<<" missed file :"<<i<<" "<<tmp_file_name ;
                missed_item = parent_item->child_items[i] ;
                missed_item->delete_flag =1 ;	//使用delete 标记来处理，让谁来删掉它呢？
            }
        }
    }
}


int  RemoteDirRetriveThread::mkdir()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
    command_queue_elem * cmd_item = this->command_queue.at(0);
    
    QString abs_path = cmd_item->parent_item->strip_path + QString("/") + cmd_item->params ;
    
    qDebug()<< "abs  path :"<< abs_path  ;
    
	exec_ret = libssh2_sftp_mkdir( ssh2_sftp , GlobalOption::instance()->remote_codec->fromUnicode(abs_path).data() , 0777 );

    this->add_node( cmd_item->parent_item , cmd_item->parent_model_internal_pointer );
    
    return exec_ret ;
}

int  RemoteDirRetriveThread::rmdir()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
    QStringList  sys_dirs;
    sys_dirs<<"/usr"<<"/bin"<<"/sbin"<<"/lib"<<"/etc"<<"/dev"<<"/proc"
            <<"/mnt"<<"/sys"<<"/var";
    
    command_queue_elem * cmd_item = this->command_queue.at(0);
    
    QString abs_path = cmd_item->parent_item->strip_path + "/" + cmd_item->params ;

    qDebug()<< "abs  path :"<< abs_path  ;
    
    if( sys_dirs.contains( abs_path ) )
    {
        qDebug()<<" rm system directory , this is danger.";
    }
    else
    {
		exec_ret = libssh2_sftp_rmdir(ssh2_sftp, GlobalOption::instance()->remote_codec->fromUnicode(abs_path).data()  );
    }
    //cmd_item->parent_item->retrived = 2 ;   //让上层视图更新这个结点
    this->add_node( cmd_item->parent_item , cmd_item->parent_model_internal_pointer );
    
    return exec_ret ;
}

int  RemoteDirRetriveThread::rm_file_or_directory_recursively()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
    QStringList  sys_dirs;
    sys_dirs<<"/usr"<<"/bin"<<"/sbin"<<"/lib"<<"/etc"<<"/dev"<<"/proc"
            <<"/mnt"<<"/sys"<<"/var";
    
    directory_tree_item * child_item = 0 ;
    directory_tree_item * parent_item = 0 ;
    command_queue_elem * cmd_item = this->command_queue.at(0);
    parent_item = cmd_item->parent_item ;
    
    QString abs_path = cmd_item->parent_item->strip_path + "/" +  cmd_item->params ;
    //char  remote_path[PATH_MAX] = {0};
	//strcpy(remote_path, abs_path.toAscii().data());
    
    qDebug()<< "abs  path :"<< abs_path  ;
    
    if( sys_dirs.contains(  abs_path) )
    {
        qDebug()<<" rm  system directory recusively , this is danger.";
    }
    else
    {
//         path1 = make_absolute(path1, *pwd);
//         remote_glob(conn, path1, GLOB_NOCHECK, NULL, &g);
//         for (i = 0; g.gl_pathv[i] && !interrupted; i++) {
//             printf("Removing %s\n", g.gl_pathv[i]);
//             err = do_rm(conn, g.gl_pathv[i]);
//             if (err != 0 && err_abort)
//                 break;
//         }        
        //exec_ret = do_rmdir(this->sftp_connection , remote_path   );
        //TODO 这个地方实现递归删除功能，现在还没有实现。
//         remote_glob(this->sftp_connection , remote_path , GLOB_NOCHECK , NULL ,& g);
//         for( int i = 0 ; g.gl_pathv[i] != NULL ; i ++ )
//         {
//             qDebug()<<QString(tr("Removing %1")).arg(g.gl_pathv[i]);
//             exec_ret = do_rm( this->sftp_connection,g.gl_pathv[i] );
//             if( exec_ret != 0 )
//             {
//                 //break ;
//             }
//         }
        //找到这个要删除的结点并删除
        for( int  i = 0 ; i < parent_item->child_items.size() ; i ++ )
        {
            child_item = parent_item->child_items[i];
            if( child_item->file_name.compare( cmd_item->params ) == 0 )
            {
                this->rm_file_or_directory_recursively_ex(child_item->strip_path);
                break ;
            }
        }
    }

	this->add_node( cmd_item->parent_item , cmd_item->parent_model_internal_pointer );
    
    return exec_ret ;
}

//TODO 现在删除隐藏文件或者目录还有问题：即以  .  字符开头的项
int  RemoteDirRetriveThread::rm_file_or_directory_recursively_ex( QString parent_path )  // <==> rm -rf
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
   
    QString abs_path  ;
    //char  remote_path[PATH_MAX+1] = {0};
    //char  strip_path[PATH_MAX+1] = {0};
    //char  file_name[PATH_MAX+1] = {0};
    QVector<QMap<char, QString> > fileinfos ;
    directory_tree_item * child_item = 0 ;
    
     //再次从服务器列出目录，然后处理
    //int lflag = 0 ;    
//     lflag = LS_LONG_VIEW;
//     lflag |= LS_SHOW_ALL ;
    
    //strcpy ( file_name, ( parent_path+  ( "/" ) ).toAscii().data() );
	//strcpy ( strip_path,parent_path.toAscii().data() );
    
//     exec_ret = fxp_do_globbed_ls(this->sftp_connection,file_name,strip_path,lflag , fileinfos);
    exec_ret = fxp_do_ls_dir( parent_path+  ( "/" ) , fileinfos ) ;
    
    int file_count = fileinfos.size();
    //qDebug()<<" rm ex :" << file_count ;
    
    for( int i = file_count -1 ; i >= 0 ; i --)
    {
        //qDebug()<<" lsed file:"<< fileinfos.at(i)['N'].c_str() ;
//         path1 = make_absolute(path1, *pwd);
//         remote_glob(conn, path1, GLOB_NOCHECK, NULL, &g);
//         for (i = 0; g.gl_pathv[i] && !interrupted; i++) {
//             printf("Removing %s\n", g.gl_pathv[i]);
//             err = do_rm(conn, g.gl_pathv[i]);
//             if (err != 0 && err_abort)
//                 break;
//         }        
            if( fileinfos.at(i)['T'].at(0) == 'd')
            {
                if( fileinfos.at(i)['N'].compare(".") == 0 
                    || fileinfos.at(i)['N'].compare("..") == 0 )
                {
                    //qDebug()<<"do ... ls shown . and .. ???";
                    continue ;
                }
                else
                {
                    this->rm_file_or_directory_recursively_ex( parent_path + ("/") + fileinfos.at(i)['N'] );
                }
            }
            else if( fileinfos.at(i)['T'].at(0) == 'l'
                     ||  fileinfos.at(i)['T'].at(0) == '-' )
            {
                abs_path = parent_path + "/" + fileinfos.at(i)['N'] ;//+ "/" + child_item->file_name ;
				//strcpy( remote_path , abs_path.toAscii().data() );
                //qDebug()<<QString(tr("Removing %1")).arg( remote_path );
				exec_ret = libssh2_sftp_unlink( ssh2_sftp , GlobalOption::instance()->remote_codec->fromUnicode(abs_path) );
                                
//                 remote_glob(this->sftp_connection , remote_path , GLOB_NOCHECK , NULL ,& g);
//                 for( int i = 0 ; g.gl_pathv[i] != NULL ; i ++ )
//                 {
//                     //qDebug()<<QString(tr("Removing %1")).arg(g.gl_pathv[i]);
//                     exec_ret = do_rm( this->sftp_connection,g.gl_pathv[i] );
//                     if( exec_ret != 0 )
//                     {
//                         //break ;
//                     }
//                 }
            }
            else
            {
                qDebug()<<" unknow file type ,don't know how to remove it";
            }
    }
    
    //删除这个目录
    abs_path = parent_path ;//+ "/" + parent_item->file_name ;
	//strcpy( remote_path , abs_path.toAscii().data() );
//     exec_ret = do_rmdir(this->sftp_connection , remote_path   );
//     if(exec_ret != 0 )  //可能这是一个文件，不是目录，那么使用删除文件的指令
//         exec_ret = do_rm(this->sftp_connection , remote_path   );
	exec_ret = libssh2_sftp_rmdir(ssh2_sftp, GlobalOption::instance()->remote_codec->fromUnicode(abs_path) );
    if( exec_ret != 0 ) //可能这是一个文件，不是目录，那么使用删除文件的指令
    {
        exec_ret = libssh2_sftp_unlink(ssh2_sftp,GlobalOption::instance()->remote_codec->fromUnicode(abs_path) );
        if( exec_ret != 0 )
        {
            qDebug()<< " count remove file or directory ("<< libssh2_sftp_last_error(ssh2_sftp) <<"): "<< abs_path ;
        }
    }
    
    return exec_ret ;
}

// linux 路径名中不能出现的字符： ! 
int  RemoteDirRetriveThread::rename()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
    QStringList  sys_dirs;
    sys_dirs<<"/usr"<<"/bin"<<"/sbin"<<"/lib"<<"/etc"<<"/dev"<<"/proc"
            <<"/mnt"<<"/sys"<<"/var";
    
    command_queue_elem * cmd_item = this->command_queue.at(0);
    
	size_t sep_pos = cmd_item->params.indexOf('!');
    
    QString abs_path = cmd_item->parent_item->strip_path + "/" +  cmd_item->params.mid(0,sep_pos) ;
    QString abs_path_rename_to = cmd_item->parent_item->strip_path + "/" + cmd_item->params.mid(sep_pos+1,-1) ;
    
    qDebug()<< "abs  path :"<< abs_path  
            << " abs path rename to ;"<< abs_path_rename_to ;
    
    if( sys_dirs.contains(  abs_path ))
    {
        qDebug()<<" rm system directory , this is danger.";
    }
    else
    {
        exec_ret = libssh2_sftp_rename(ssh2_sftp,
			GlobalOption::instance()->remote_codec->fromUnicode(abs_path).data() ,
			GlobalOption::instance()->remote_codec->fromUnicode(abs_path_rename_to) );
    }

    this->add_node( cmd_item->parent_item , cmd_item->parent_model_internal_pointer );
    
    return exec_ret ;
}

int RemoteDirRetriveThread::keep_alive()
{
    int exec_ret;
    char full_path [PATH_MAX+1] = {0};
    char strip_path [PATH_MAX+1] = {0};

    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib ;
    
    strcpy(full_path,"/nullfxp_keep_alive_dummy_directory");
    strcpy(strip_path,"/");

    exec_ret = libssh2_sftp_stat(ssh2_sftp,full_path,&ssh2_sftp_attrib);
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<< " stat : "<< exec_ret <<" sftp errno:"<< libssh2_sftp_last_error(ssh2_sftp) ;
    //TODO 在网络失去连接的时候如何向上层类通知，并进行重新连接
    return exec_ret ;
}

int RemoteDirRetriveThread::fxp_do_ls_dir ( QString path , QVector<QMap<char, QString> > & fileinfos      )
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

void RemoteDirRetriveThread::add_node ( directory_tree_item* parent_item ,void * parent_model_internal_pointer )
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	
	parent_item->prev_retr_flag = parent_item->retrived ;
	parent_item->retrived = 8 ;
	command_queue_elem * cmd_elem = new command_queue_elem();
	cmd_elem->parent_item = parent_item;
	cmd_elem->parent_model_internal_pointer = parent_model_internal_pointer;
	cmd_elem->cmd = SSH2_FXP_READDIR;
	this->command_queue.push_back ( cmd_elem );
	
	if (  !this->isRunning()  )
	{
		this->start();
	}

    //TODO 检测重复命令
//     if ( this->dir_node_process_queue.count ( parent_item ) == 0 )
// 	{
//         parent_item->prev_retr_flag = parent_item->retrived ;
//         parent_item->retrived = 8 ;
//                 
//         this->dir_node_process_queue.insert ( std::make_pair ( parent_item,parent_model_internal_pointer ) );
// 		if ( !this->isRunning() )
// 		{
// 			this->start();
// 		}
// 	}
// 	else
// 	{
// 		qDebug() <<" this node has already in process queue , omitted";
// 	}
    //assert ( this->dir_node_process_queue.count ( parent_item ) == 1 );

}

void RemoteDirRetriveThread::slot_execute_command(directory_tree_item* parent_item , 
												  void * parent_model_internal_pointer,  int cmd , QString params )
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    command_queue_elem * cmd_elem = new command_queue_elem();
    cmd_elem->parent_item = parent_item;
    cmd_elem->parent_model_internal_pointer = parent_model_internal_pointer;
    cmd_elem->cmd = cmd;
    cmd_elem->params = params;
    
    this->command_queue.push_back(cmd_elem);
    
    if ( !this->isRunning() )
    {
        this->start();
    }
        
}

