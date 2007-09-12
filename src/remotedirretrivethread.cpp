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

#include "sftp.h"
#include "sftp-operation.h"

#include "remotedirretrivethread.h"

//这种声明方法是为什么呢？
int remote_glob(struct sftp_conn *, const char *, int,
                int (*)(const char *, int), glob_t *); /* proto for sftp-glob.c */
                
////////////////////////directory_tree_item
directory_tree_item::~directory_tree_item()
{
    qDebug()<<"tree delete now";
    int line = this->child_items.size();
    for(int i = line -1 ; i >=0 ; i --)
    {
        delete this->child_items[i];
    }
}

///////////////////////////////////
RemoteDirRetriveThread::RemoteDirRetriveThread ( struct sftp_conn * conn , QObject* parent ) : QThread ( parent )
{
    this->sftp_connection = conn ;
}


RemoteDirRetriveThread::~RemoteDirRetriveThread()
{}


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
    
    std::vector<std::map<char,std::string> > fileinfos;
    char file_name[PATH_MAX] ;//= parent_item->tree_node_item['N'];
    char strip_path[PATH_MAX];// = parent_item->strip_path ;    
    int row_count = 0 ;
    int fxp_ls_ret = 0 ;
    
    while ( this->dir_node_process_queue.size() >0 )
    {
        std::map<directory_tree_item*,void * >::iterator mit;
        mit = this->dir_node_process_queue.begin();
        
        parent_item = mit->first ;
        parent_model_internal_pointer = mit->second ;
        fileinfos.clear();
        
        
        int lflag = 0;
        lflag &= ~VIEW_FLAGS;
        lflag |= LS_LONG_VIEW;

        strcpy ( file_name, ( parent_item->strip_path+std::string ( "/" ) ).c_str() );
        strcpy ( strip_path,parent_item->strip_path.c_str() );
		//memset(file_name,0,sizeof(file_name));
		//strcpy(file_name,"/etc");
		//strcpy(strip_path,"/" );
        fxp_ls_ret = fxp_do_globbed_ls ( this->sftp_connection , file_name , strip_path , lflag , fileinfos );
        qDebug() <<"fxp_ls_ret:" << fxp_ls_ret<< ", fileinfos number="<<fileinfos.size() << " use strip:" << strip_path <<" file_name ="<< file_name ;
	
        //将已经存在的项目找出来
        std::map<std::string,int> existed_items ;
        for( int i = 0 ; i < parent_item->child_items.size() ; i ++)
        {
            existed_items.insert(std::make_pair(parent_item->child_items[i]->file_name,8));
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

            thefile->strip_path = std::string ( parent_item->strip_path ) + std::string ( "/" ) +  fileinfos.at ( i ) ['N'] ;

            thefile->file_type = fileinfos.at ( i ) ['T'];
            thefile->file_size = fileinfos.at ( i ) ['S'];
            thefile->file_name = fileinfos.at ( i ) ['N'] ;
            thefile->file_date = fileinfos.at ( i ) ['D'];

			//parent_item->child_items.insert ( std::make_pair ( i,thefile ) ) ;
            parent_item->child_items.insert ( std::make_pair ( curr_count , thefile ) ) ;
        }
        
        this->subtract_existed_model(parent_item,fileinfos);
        
        parent_item->prev_retr_flag = parent_item->retrived ;
        parent_item->retrived = 9 ;
        //         
        //         //////
        this->dir_node_process_queue.erase(parent_item);
        emit this->remote_dir_node_retrived(parent_item,parent_model_internal_pointer);
        
    }
    
    return exec_ret ;
}


void RemoteDirRetriveThread::subtract_existed_model(directory_tree_item * parent_item , std::vector<std::map<char,std::string> > & new_items )
{
    assert( new_items.size() <= parent_item->child_items.size() );
    std::string  tmp_file_name ;
    std::map<std::string,int> new_file_items ;
    
    std::vector<directory_tree_item*> missed_item_handle;
    
    directory_tree_item * missed_item , *temp_item ;
    
    if( new_items.size() != parent_item->child_items.size() )
    {
        qDebug()<<" some item miss in new file list";
        for(int i = 0 ; i < new_items.size() ; i ++ )
        {
            new_file_items.insert(std::make_pair(new_items.at(i)['N'],1));
        }
        for( int i = 0 ; i < parent_item->child_items.size() ; i ++ )
        {
            tmp_file_name = parent_item->child_items[i]->file_name;
            if( new_file_items.count(tmp_file_name) == 0 )
            {
                qDebug()<<" missed file :"<<i<<" "<<tmp_file_name.c_str();
                missed_item = parent_item->child_items[i] ;
                missed_item->delete_flag =1 ;
                
//                 if(i!=parent_item->child_items.size()-1)
//                 {
//                     for( int j = i+1 ; j < parent_item->child_items.size() ; j ++ )
//                     {
//                         temp_item = parent_item->child_items[j];
//                         temp_item->row_number = j-1 ;
//                         parent_item->child_items[j-1] = temp_item ;
//                     }
//                     int c_size = parent_item->child_items.size() ;
//                     parent_item->child_items.erase(c_size-1);
//                 }
//                 else
//                 {
//                     int c_size = parent_item->child_items.size() ;
//                     parent_item->child_items.erase(c_size-1);
//                 }
                // xxxx: 执行下面一句程序会崩溃，不执行会内存潺漏
                //delete missed_item ;missed_item = 0 ;
            }
        }
    }
}


int  RemoteDirRetriveThread::mkdir()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
    Attrib ab ;
    
    command_queue_elem * cmd_item = this->command_queue.at(0);
    
    std::string abs_path = cmd_item->parent_item->strip_path + "/" +
            cmd_item->params ;
    char  remote_path[PATH_MAX] = {0};
    strcpy(remote_path, abs_path.c_str());
    
    qDebug()<< "abs  path :"<< abs_path .c_str() ;
    
    attrib_clear(&ab);
    ab.flags |= SSH2_FILEXFER_ATTR_PERMISSIONS;
    ab.perm = 0777;
    
    exec_ret = do_mkdir(this->sftp_connection , remote_path ,&ab );
    
    //cmd_item->parent_item->retrived = 2 ;   //让上层视图更新这个结点
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
    
    std::string abs_path = cmd_item->parent_item->strip_path + "/" +
            cmd_item->params ;
    char  remote_path[PATH_MAX] = {0};
    strcpy(remote_path, abs_path.c_str());
    
    qDebug()<< "abs  path :"<< abs_path .c_str() ;
    
    if( sys_dirs.contains( QString(abs_path.c_str())) )
    {
        qDebug()<<" rm system directory , this is danger.";
    }
    else
    {
        exec_ret = do_rmdir(this->sftp_connection , remote_path   );
    }
    //cmd_item->parent_item->retrived = 2 ;   //让上层视图更新这个结点
    this->add_node( cmd_item->parent_item , cmd_item->parent_model_internal_pointer );
    
    return exec_ret ;
}

int  RemoteDirRetriveThread::rm_file_or_directory_recursively()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
    glob_t g;
    QStringList  sys_dirs;
    sys_dirs<<"/usr"<<"/bin"<<"/sbin"<<"/lib"<<"/etc"<<"/dev"<<"/proc"
            <<"/mnt"<<"/sys"<<"/var";
    
    command_queue_elem * cmd_item = this->command_queue.at(0);
    
    std::string abs_path = cmd_item->parent_item->strip_path + "/" +
            cmd_item->params ;
    char  remote_path[PATH_MAX] = {0};
    strcpy(remote_path, abs_path.c_str());
    
    qDebug()<< "abs  path :"<< abs_path .c_str() ;
    
    if( sys_dirs.contains( QString(abs_path.c_str())) )
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
        remote_glob(this->sftp_connection , remote_path , GLOB_NOCHECK , NULL ,& g);
        for( int i = 0 ; g.gl_pathv[i] != NULL ; i ++ )
        {
            qDebug()<<QString(tr("Removing %1")).arg(g.gl_pathv[i]);
            exec_ret = do_rm( this->sftp_connection,g.gl_pathv[i] );
            if( exec_ret != 0 )
            {
                //break ;
            }
        }
    }
    //cmd_item->parent_item->retrived = 2 ;   //让上层视图更新这个结点
    this->add_node( cmd_item->parent_item , cmd_item->parent_model_internal_pointer );
    
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
    
    size_t sep_pos = cmd_item->params.find('!');
    
    std::string abs_path = cmd_item->parent_item->strip_path + "/" +
            cmd_item->params.substr(0,sep_pos) ;
    std::string abs_path_rename_to = cmd_item->parent_item->strip_path + "/" +
            cmd_item->params.substr(sep_pos+1) ;
    
    char  remote_path[PATH_MAX] = {0};
    char  remote_path_rename_to[PATH_MAX] = {0};
    
    strcpy(remote_path, abs_path.c_str());
    strcpy( remote_path_rename_to,abs_path_rename_to.c_str() );
    
    qDebug()<< "abs  path :"<< abs_path .c_str()
            << " abs path rename to ;"<< abs_path_rename_to.c_str();
    
    if( sys_dirs.contains( QString(abs_path.c_str())) )
    {
        qDebug()<<" rm system directory , this is danger.";
    }
    else
    {
        exec_ret = do_rename(this->sftp_connection , remote_path  , remote_path_rename_to  );
    }
    //cmd_item->parent_item->retrived = 2 ;   //让上层视图更新这个结点
    this->add_node( cmd_item->parent_item , cmd_item->parent_model_internal_pointer );
    
    return exec_ret ;
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
        this->command_queue.push_back(cmd_elem);
        
		if ( !this->isRunning() )
		{
			this->start();
		}
    
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

void RemoteDirRetriveThread::slot_execute_command(directory_tree_item* parent_item , void * parent_model_internal_pointer,  int cmd , std::string params )
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

