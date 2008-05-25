/* remotedirretrivethread.h --- 
 * 
 * Filename: remotedirretrivethread.h
 * Description: 
 * Author: liuguangzhao
 * Maintainer: 
 * Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sourceforge.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 日  5月 25 09:50:13 2008 (CST)
 * Version: 
 * Last-Updated: 
 *           By: 
 *     Update #: 0
 * URL: 
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change log:
 * 
 * 
 */

#ifndef REMOTEDIRRETRIVETHREAD_H
#define REMOTEDIRRETRIVETHREAD_H

#include <cassert>
#include <vector>
#include <map>

#include <queue>

#include <QtCore>
#include <QThread>

#include "sftp.h"
#include "libssh2.h"
#include "libssh2_sftp.h"

#define SSH2_FXP_KEEP_ALIVE 8888

//这个类中存储的字符串改为Qt内部使用的Unicode编码。
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
                
    QString strip_path ;
    QString file_name ;
    QString file_size ;
    QString file_date ;
    QString file_type ;
    QString file_perm ;  
};


/**
   @author liuguangzhao <gzl@localhost>
*/
class RemoteDirRetriveThread : public QThread
{
    Q_OBJECT
	public:
    RemoteDirRetriveThread(/*struct sftp_conn * conn , */QObject* parent=0);

    ~RemoteDirRetriveThread();
    //在实例初始化后马上调用，否则会导致程序崩溃
    void set_ssh2_handler( void * ssh2_sess /*, void * ssh2_sftp, int ssh2_sock*/ );
    
    virtual void run();
        
    void add_node(directory_tree_item* parent_item , void * parent_model_internal_pointer );
    
    void slot_execute_command(directory_tree_item* parent_item , void * parent_model_internal_pointer, 
			      int cmd , QString params );

private:
    int  retrive_dir();
    int  mkdir();
    int  rmdir();
    int  rm_file_or_directory_recursively();  // <==> rm -rf
    int  rm_file_or_directory_recursively_ex(QString parent_path);  // <==> rm -rf
    int  rename();
        
    int keep_alive() ;
    int fxp_do_ls_dir ( QString parent_path  , QVector<QMap<char, QString> > & fileinfos  );
signals:
    void enter_remote_dir_retrive_loop();
    void leave_remote_dir_retrive_loop();
        
    void remote_dir_node_retrived(directory_tree_item* parent_item , void * parent_model_internal_pointer );
        
    void execute_command_finished( directory_tree_item* parent_item , void * parent_model_internal_pointer, int cmd ,int status );
        
private:

    std::map<directory_tree_item*,void * >   dir_node_process_queue ;
    class command_queue_elem
    {
    public:
	command_queue_elem()
	{  
	    this->parent_item=0;
	    this->parent_model_internal_pointer = 0 ;
	    this->cmd = -1 ;
	    this->retry_times = 0 ;
	}
               
	directory_tree_item* parent_item;
	void * parent_model_internal_pointer;
	int  cmd;
	QString  params;
	int  retry_times ;
    };       
    std::vector<command_queue_elem*>  command_queue;
       
    LIBSSH2_SESSION * ssh2_sess;
    LIBSSH2_SFTP * ssh2_sftp ;
//        int ssh2_sock ;
       
    //void subtract_existed_model(directory_tree_item * parent_item , std::vector<std::map<char,std::string> > & new_items );
    void subtract_existed_model(directory_tree_item * parent_item , QVector<QMap<char,QString> > & new_items );
       
};

#endif
