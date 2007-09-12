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
#ifndef REMOTEDIRRETRIVETHREAD_H
#define REMOTEDIRRETRIVETHREAD_H

#include <cassert>
#include <vector>
#include <map>

#include <queue>

#include <QtCore>
#include <QThread>

#include "sftp-operation.h"
#include "sftp-client.h"
#include "sftp-wrapper.h"

//从remotedirmodel.h 移动过来的,
class directory_tree_item
{
    public:
        directory_tree_item()
        {
            this->type = 0 ;
            this->prev_retr_flag = 0;
            this->retrived = 0;
            this->parent_item = 0;
            this->row_number = -1;
            this->delete_flag = 0 ;
        }
        ~directory_tree_item();

        int type ;  // dir , not dir    // depcreated
        int prev_retr_flag ;    // 前一步的retrived 状态值。在改变retrived的状态的时候使用。
        int retrived ;  // 1 , 0
        int delete_flag ;   // 1 , 0 ;
        /*
        0 表示UI没有要求过的，所以我们也没有取过数据的            
        1 表示UI没有请求过，我们自己填入了部分数据（如用户指定他的初始化目录的时候），即这表示需要更新的结点
        2 表示UI已经请求过，我们只取回来了一部分数据，即这表示需要更新的结点
        8 表示UI已经请求过，我们已经放到处理队列的，但还没有获取来数据的
        9 表示UI已经请求过，并且更新到了最新目录结构状态。
        
        状态转换：
        0-----> 8 ------ > 9 
        1------> 8 -------> 9
        2------>  8 -------> 9
        */

        std::map< int, directory_tree_item *> child_items ;
        directory_tree_item *parent_item;
        int row_number ;    //指的是所包含的子结点个数
				//N , S , T , D
				//T = D , F , L
        //std::map< char  , std::string > tree_node_item ;
                
        std::string strip_path ;
        std::string file_name ;
        std::string file_size ;
        std::string file_date ;
        std::string file_type ;
        std::string file_perm ;
        
                
};


/**
	@author liuguangzhao <gzl@localhost>
*/
class RemoteDirRetriveThread : public QThread
{
Q_OBJECT
public:
    RemoteDirRetriveThread(struct sftp_conn * conn , QObject* parent=0);

    ~RemoteDirRetriveThread();

    virtual void run();

    void add_node(directory_tree_item* parent_item , void * parent_model_internal_pointer );
            
    signals:
        void enter_remote_dir_retrive_loop();
        void leave_remote_dir_retrive_loop();
        
        void remote_dir_node_retrived(directory_tree_item* parent_item , void * parent_model_internal_pointer );
        
         
    private:
       std::map<directory_tree_item*,void * >   dir_node_process_queue ;
       struct sftp_conn * sftp_connection;
       
       void subtract_existed_model(directory_tree_item * parent_item , std::vector<std::map<char,std::string> > & new_items );
       
};

#endif
