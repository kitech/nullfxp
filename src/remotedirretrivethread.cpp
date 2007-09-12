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

#include "remotedirretrivethread.h"

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

	directory_tree_item * parent_item;
    void *  parent_model_internal_pointer ;
    
    std::vector<std::map<char,std::string> > fileinfos;
    char file_name[PATH_MAX] ;//= parent_item->tree_node_item['N'];
    char strip_path[PATH_MAX];// = parent_item->strip_path ;    
    int row_count = 0 ;
    int fxp_ls_ret = 0 ;
    
    emit enter_remote_dir_retrive_loop();
    
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
    emit this->leave_remote_dir_retrive_loop();
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

void RemoteDirRetriveThread::add_node ( directory_tree_item* parent_item ,void * parent_model_internal_pointer )
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    if ( this->dir_node_process_queue.count ( parent_item ) == 0 )
	{
        parent_item->prev_retr_flag = parent_item->retrived ;
        parent_item->retrived = 8 ;
                
        this->dir_node_process_queue.insert ( std::make_pair ( parent_item,parent_model_internal_pointer ) );
		if ( !this->isRunning() )
		{
			this->start();
		}
	}
	else
	{
		qDebug() <<" this node has already in process queue , omitted";
	}
    assert ( this->dir_node_process_queue.count ( parent_item ) == 1 );

}
