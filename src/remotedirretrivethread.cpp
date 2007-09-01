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
#include "remotedirretrivethread.h"

RemoteDirRetriveThread::RemoteDirRetriveThread ( struct sftp_conn * conn , QObject* parent ) : QThread ( parent )
{
    this->sftp_connection = conn ;
}


RemoteDirRetriveThread::~RemoteDirRetriveThread()
{}


void RemoteDirRetriveThread::run()
{

	directory_tree_item * parent_item;
	const QModelIndex * parent_model ;
    
    std::vector<std::map<char,std::string> > fileinfos;
    char file_name[PATH_MAX] ;//= parent_item->tree_node_item['N'];
    char strip_path[PATH_MAX];// = parent_item->strip_path ;    
    int row_count = 0 ;
    int fxp_ls_ret = 0 ;
    
	while ( this->dir_node_process_queue.size() >0 )
	{
        std::map<directory_tree_item*,const QModelIndex*>::iterator mit;
        mit = this->dir_node_process_queue.begin();
        
        parent_item = mit->first ;
        parent_model = mit->second ;
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
				
		////////////
		for ( int i = 0 ; i < fileinfos.size() ; i ++ )
		{
			directory_tree_item * thefile = new directory_tree_item();
			thefile->retrived = 0;
			thefile->parent_item = parent_item ;
			thefile->tree_node_item.insert ( std::make_pair ( 'N',fileinfos.at ( i ) ['N'] ) ) ;
			thefile->tree_node_item.insert ( std::make_pair ( 'S',fileinfos.at ( i ) ['S'] ) ) ;
			thefile->tree_node_item.insert ( std::make_pair ( 'T',fileinfos.at ( i ) ['T'] ) ) ;
			thefile->tree_node_item.insert ( std::make_pair ( 'D',fileinfos.at ( i ) ['D'] ) ) ;
			thefile->row_number=i;

			thefile->strip_path = std::string ( parent_item->strip_path ) + std::string ( "/" ) +  fileinfos.at ( i ) ['N'] ;

			thefile->file_type = fileinfos.at ( i ) ['T'];
			thefile->file_size = fileinfos.at ( i ) ['S'];
			thefile->file_name = fileinfos.at ( i ) ['N'] ;
			thefile->file_date = fileinfos.at ( i ) ['D'];

			parent_item->child_items.insert ( std::make_pair ( i,thefile ) ) ;
		}
        
        parent_item->prev_retr_flag = parent_item->retrived ;
        parent_item->retrived = 9 ;
        
        //////
        this->dir_node_process_queue.erase(parent_item);
        emit this->remote_dir_node_retrived(parent_item,parent_model);
        
	}
}

void RemoteDirRetriveThread::add_node ( directory_tree_item* parent_item , const QModelIndex * parent_model )
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    if ( this->dir_node_process_queue.count ( parent_item ) == 0 )
	{
        parent_item->prev_retr_flag = parent_item->retrived ;
        parent_item->retrived = 8 ;
                
        this->dir_node_process_queue.insert ( std::make_pair ( parent_item,parent_model ) );
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

