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
#include <cassert>
#include <QtCore>


#include "remotedirmodel.h"

#define REMOTE_CODEC "UTF-8"

//////////////////////////
/////////////////////////////////
/////////////////////////////////////
RemoteDirModel::RemoteDirModel ( struct sftp_conn * conn , QObject *parent )
		:QAbstractItemModel ( parent )
{
	this->sftp_connection = conn ;

	this->remote_dir_retrive_thread = new RemoteDirRetriveThread ( conn );
	QObject::connect ( this->remote_dir_retrive_thread,SIGNAL ( remote_dir_node_retrived ( directory_tree_item *,void * ) ),
	                   this,SLOT ( slot_remote_dir_node_retrived ( directory_tree_item*,   void * ) ) );

	//
	QObject::connect ( this->remote_dir_retrive_thread,SIGNAL ( enter_remote_dir_retrive_loop() ),
	                   this,SIGNAL ( enter_remote_dir_retrive_loop() ) );
	QObject::connect ( this->remote_dir_retrive_thread,SIGNAL ( leave_remote_dir_retrive_loop() ),
	                   this,SIGNAL ( leave_remote_dir_retrive_loop() ) );
}

void RemoteDirModel::set_user_home_path ( std::string user_home_path )
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	this->user_home_path = user_home_path ;

	qDebug() <<" i know remote home path: "<< this->user_home_path.c_str() ;

	//Todo: 使用这个初始化路径来初始化这个树，而不是用默认的根路径 .
	//初始化目录案例：
	// 1.      /home/users/l/li/liuguangzhao
	// 2.      /root
	// 3.       /home/gzl

	this->tree_root = new  directory_tree_item ();

	//{创建初始化目录树
	directory_tree_item * first_item ;
	//= new directory_tree_item();
	//first_item->tree_node_item.insert ( std::make_pair ( 'N',"/" ) );
	//first_item->tree_node_item.insert ( std::make_pair ( 'T',"D" ) ) ;
	//first_item->parent_item = this->tree_root;
	//first_item->row_number = 1;
	//first_item->retrived = 1 ;  //半满结点
	//first_item->strip_path = "";    //对根，不需要strip前缀，现在程序的表现就好了。

	directory_tree_item * temp_parent_tree_item = 0 , * temp_tree_item =0 ;
	std::string temp_strip_path , temp_path_name ;
	char   buff[PATH_MAX+1] = {0};
	char    buff2[PATH_MAX+1] = {0};
	char    buff3[PATH_MAX+1] = {0} ;
	char * sep_pos = 0 ,* pre_sep_pos ;
	strcpy ( buff,this->user_home_path.c_str() );
	strcpy ( buff2,this->user_home_path.c_str() );
	pre_sep_pos = buff ;
	while ( 1 )
	{
		sep_pos = strchr ( buff,'/' ) ;
		if ( sep_pos == NULL )
		{
			temp_strip_path = std::string ( buff2 );
			memset ( buff3,0,PATH_MAX+1 );
			strcpy ( buff3,buff2+ ( pre_sep_pos-buff ) +1 );
			temp_path_name = std::string ( buff3 );
			temp_parent_tree_item = temp_tree_item ;
			temp_tree_item = new directory_tree_item();
		}
		else
		{
			*sep_pos = '&'; //将这个字符替换掉，防止重复查找这个位置
			if ( sep_pos == buff )
			{
				strncpy ( buff3,buff2,int ( sep_pos-buff ) );
				temp_strip_path = std::string ( buff3 );
				temp_path_name = std::string ( "/" );
				first_item   = new directory_tree_item();
				temp_tree_item = first_item ;
				temp_parent_tree_item = this->tree_root ;
			}
			else
			{
				strncpy ( buff3,buff2,int ( sep_pos-buff ) );
				temp_strip_path = std::string ( buff3 );
				memset ( buff3,0,PATH_MAX+1 );
				strncpy ( buff3,buff2+ ( pre_sep_pos-buff ) +1, ( sep_pos-pre_sep_pos-1 ) );
				temp_path_name = std::string ( buff3 );
				temp_parent_tree_item = temp_tree_item ;
				temp_tree_item = new directory_tree_item();
			}


		}
		qDebug() <<"distance to begin:   strip path:"<< temp_strip_path.c_str() << "dir name:"<< temp_path_name.c_str() ;
		//assign
		//temp_tree_item->tree_node_item.insert ( std::make_pair ( 'N',temp_path_name ) );
		//temp_tree_item->tree_node_item.insert ( std::make_pair ( 'T',"D" ) ) ;
		temp_tree_item->parent_item = temp_parent_tree_item ;
		temp_tree_item->row_number = 0 ;    //指的是此结点在父结点中的第几个结点，在这里预置的只能为0
		temp_tree_item->retrived = ( sep_pos == NULL ) ?0:1 ;  //半满结点
		temp_tree_item->strip_path = temp_strip_path;
		temp_tree_item->file_name = temp_path_name;
		temp_tree_item->file_size = std::string ( "0" );
		temp_tree_item->file_type = std::string ( "D" );
		temp_tree_item->prev_retr_flag = -1 ;

		//对根，不需要strip前缀，现在程序的表现就好了。
		temp_parent_tree_item->child_items.insert ( std::make_pair ( 0,temp_tree_item ) );

		//pre_sep_pos
		pre_sep_pos = sep_pos ;
		if ( sep_pos == NULL ) break ;
	}
	qDebug() <<" seach end :"<< buff ;
	//}

	this->tree_root->child_items.insert ( std::make_pair ( 0, first_item ) );
	this->tree_root->row_number = 0;
}



RemoteDirModel::~RemoteDirModel()
{
	if ( this->remote_dir_retrive_thread->isRunning() )
	{
		qDebug() <<" remote_dir_retrive_thread is run , how stop ?";
	}
	else
	{
		delete this->remote_dir_retrive_thread ;
	}
	//Todo: 删除model中的现有数据
}

QModelIndex RemoteDirModel::index ( int row, int column, const QModelIndex &parent ) const
{
	//qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	//qDebug() << "row :" << row << " column:" << column ;

	directory_tree_item  *parent_item;

	if ( !parent.isValid() )
		parent_item = this->tree_root ;
	else
		parent_item = static_cast<directory_tree_item*> ( parent.internalPointer() );

	directory_tree_item *child_item = 0;
	if ( parent_item->child_items.count ( row ) ==1 )
		child_item = parent_item->child_items[row];

	if ( child_item )
	{
		//qDebug()<< "createIndex ( row, column, child_item );"<< row << " " << column << " " << child_item ;
		return createIndex ( row, column, child_item );
	}
	else
	{
		//qDebug()<< "! child_item , QModelIndex();" ;
		return QModelIndex();
	}

	//return createIndex( row , column , 1 ) ;
}

QModelIndex RemoteDirModel::parent ( const QModelIndex &child ) const
{
	//qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

	if ( !child.isValid() )
	{
		//qDebug()<<" ! child.isValid()";
		return QModelIndex();
	}

	directory_tree_item  *child_item = static_cast< directory_tree_item *> ( child.internalPointer() );
	directory_tree_item  *parent_item = child_item->parent_item;

	if ( !parent_item || parent_item == this->tree_root )
	{
		//qDebug()<<"  !parent_item || parent_item == this->tree_root ";
		return QModelIndex();
	}

	//qDebug()<< " parent_item->row_number ";
	return createIndex ( parent_item->row_number, 0, parent_item );

	return QModelIndex();
}

QVariant RemoteDirModel::data ( const QModelIndex &index, int role ) const
{
	//qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	QVariant ret_var ;
	QTextCodec * codec = 0 ;
	QString unicode_name ;

	if ( role != Qt::DisplayRole )
		return QVariant();

	//ret_var = QVariant("hahaa");
	//return ret_var ;

	directory_tree_item *item = static_cast< directory_tree_item*> ( index.internalPointer() );

	//QDomNode node = item->node();
	//QStringList attributes;
	//QDomNamedNodeMap attributeMap = node.attributes();
	//qDebug()<< "item = "<< item << "  tree_root="<<this->tree_root ;

	switch ( index.column() )
	{
		case 0:
			//return node.nodeName();
			codec = QTextCodec::codecForName ( REMOTE_CODEC );
			unicode_name = codec->toUnicode ( item->file_name.c_str() );
			//ret_var = QVariant ( item->tree_node_item['N'].c_str() );
			ret_var = QVariant ( unicode_name );
			break;
		case 2:

			ret_var = QVariant ( item->file_type .c_str () );
			break;
		case 1:
			//return node.nodeValue().split("\n").join(" ");
			ret_var = QVariant ( item->file_size .c_str() );
			break;
		case 3:
			ret_var = QVariant ( item->file_date.c_str() );
			break;
		default:
			return QVariant();
			break;
	}

	return ret_var ;
}

Qt::ItemFlags RemoteDirModel::flags ( const QModelIndex &index ) const
{
	//qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

//     Qt::ItemFlags defaultFlags = QStringListModel::flags(index);
//
//     if (index.isValid())
//     return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
//     else
//     return Qt::ItemIsDropEnabled | defaultFlags;

	if ( !index.isValid() )
		return Qt::ItemIsEnabled;

	//if ( index.column() == 0 )
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled  ;
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled  ;
	//else
	//	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

}
QVariant RemoteDirModel::headerData ( int section, Qt::Orientation orientation,
                                      int role /*= Qt::DisplayRole*/ ) const
{

	//qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	//qDebug() <<"section:" << section ;

	if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
	{
		switch ( section )
		{
			case 0:
				return QVariant ( "Name" );
			case 1:
				return QVariant ( "Size" );
			case 2:
				return QVariant ( "Type" );
			case 3:
				return QVariant ( "Date" );
			default:
				return QVariant ( "what are you want?" );
		}
	}

	return QVariant();

}

int RemoteDirModel::columnCount ( const QModelIndex &/*parent*/ ) const
{
	//qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	// return 3 ;
	return 4 ;
}

int RemoteDirModel::rowCount ( const QModelIndex &parent/* = QModelIndex()*/ ) const
{
	//qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

	int row_count = 0;

	if ( ! parent.isValid() )
	{
		row_count = 1 ;
	}
	else
	{
		directory_tree_item * parent_item = static_cast<directory_tree_item*> ( parent.internalPointer() );

		if ( parent_item->retrived == 0
		        // || parent_item->retrived == 1
		        || parent_item->retrived == 2 ) //为了lazy模式的需要，才做一个假数据。
		{
			//或者是不是要在这里取子结点的行数，即去远程找数据呢。
			//row_count =1 ;
			this->remote_dir_retrive_thread->add_node ( parent_item,parent.internalPointer() );
			/*
			this->dump_tree_node_item(  parent_item ) ;
			//row_count = parent_item->child_items.size();
			int lflag = 0;
			lflag &= ~VIEW_FLAGS;
			lflag |= LS_LONG_VIEW;
			std::vector<std::map<char,std::string> > fileinfos;
			char file_name[PATH_MAX] ;//= parent_item->tree_node_item['N'];
			char strip_path[PATH_MAX];// = parent_item->strip_path ;
			strcpy(file_name,(parent_item->strip_path+std::string("/")).c_str());
			strcpy(strip_path,parent_item->strip_path.c_str() ); 
			//memset(file_name,0,sizeof(file_name));
			//strcpy(file_name,"/etc");
			//strcpy(strip_path,"/" );   
			fxp_do_globbed_ls( this->sftp_connection , file_name , strip_path , lflag , fileinfos );
			qDebug()<< "fileinfos number="<<fileinfos.size()<< " use strip:" << strip_path <<" file_name ="<< file_name ;
			row_count = fileinfos.size();
			parent_item->retrived = 1 ;
			////////////
			for(int i = 0 ; i < fileinfos.size() ; i ++ )
			{
			    directory_tree_item * thefile = new directory_tree_item();
			    thefile->retrived = 0;
			    thefile->parent_item = parent_item ;
			    thefile->tree_node_item.insert(std::make_pair('N',fileinfos.at(i)['N']) ) ;
			    thefile->tree_node_item.insert(std::make_pair('S',fileinfos.at(i)['S']) ) ;
			    thefile->tree_node_item.insert(std::make_pair('T',fileinfos.at(i)['T']) ) ;
			    thefile->tree_node_item.insert(std::make_pair('D',fileinfos.at(i)['D']) ) ;
			    thefile->row_number=i;

			    thefile->strip_path = std::string(parent_item->strip_path) + std::string("/") +  fileinfos.at(i)['N'] ;
			   
			    thefile->file_type = fileinfos.at(i)['T'];
			    thefile->file_size = fileinfos.at(i)['S'];
			    thefile->file_name = fileinfos.at(i)['N'] ;
			    thefile->file_date = fileinfos.at(i)['D'];
			   
			    parent_item->child_items.insert(std::make_pair(i,thefile) ) ;
			}
			*/
		}
		else
		{
			row_count = parent_item->child_items.size();
		}
		row_count = parent_item->child_items.size();
	}
	//qDebug()<< "row_count="<<row_count ;
	return row_count ;
}

bool RemoteDirModel::setData ( const QModelIndex & index, const QVariant & value, int role/* = Qt::EditRole*/ )
{

	return true;
}

bool RemoteDirModel::insertRows ( int row, int count, const QModelIndex & parent /*= QModelIndex() */ )
{
	return true;
}
bool RemoteDirModel::removeRows ( int row, int count, const QModelIndex & parent /*= QModelIndex()*/ )
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	qDebug() <<" row = "<< row << " count = "<< count;
	if ( parent.isValid() )
		qDebug() << parent;
	else
		qDebug() << parent.isValid()  ;
	directory_tree_item * parent_item = static_cast<directory_tree_item*> ( parent.internalPointer() );
	qDebug() << parent_item ;
	directory_tree_item * delete_item = 0 ;
    directory_tree_item * temp_item  = 0 ;
    
	this->beginRemoveRows ( parent, row, row+count );
	this->dump_tree_node_item ( parent_item );
    
    for( int i = row + count -1 ; i >= row ; i --)
    {
        delete_item = parent_item->child_items[i];
        if ( i !=parent_item->child_items.size()-1 )
        {
            for ( int j = i+1 ; j < parent_item->child_items.size() ; j ++ )
            {
                temp_item = parent_item->child_items[j];
                temp_item->row_number = j-1 ;
                parent_item->child_items[j-1] = temp_item ;
            }
            int c_size = parent_item->child_items.size() ;
            parent_item->child_items.erase ( c_size-1 );
        }
        else
        {
            int c_size = parent_item->child_items.size() ;
            parent_item->child_items.erase ( c_size-1 );    
        }
        delete delete_item ; delete_item = 0 ;
    }
	this->endRemoveRows ();
	return true;
}
bool RemoteDirModel::rowMoveTo ( const QModelIndex & from , const QModelIndex & to )
{
	return true;
}

bool RemoteDirModel::insertColumns ( int column, int count, const QModelIndex & parent /*= QModelIndex() */ )
{
	return true ;
}
bool RemoteDirModel::setItemData ( const QModelIndex & index, const QMap<int, QVariant> & roles )
{
	return true ;
}

void RemoteDirModel::dump_tree_node_item ( directory_tree_item * node_item ) const
{
	directory_tree_item * item = ( directory_tree_item * ) item ;
	assert ( node_item != 0 );
	qDebug() <<"====================>>>>";
	qDebug() <<"name="<<QString ( node_item->file_name.c_str() );
	qDebug() <<"Type="<<QString ( node_item->file_type.c_str() );
	qDebug() <<"Size="<<QString ( node_item->file_size.c_str() );
	qDebug() <<"Date="<<QString ( node_item->file_date.c_str() );
	qDebug() <<"Retrived="<<node_item->retrived;
	qDebug() <<"prev_retr_flag="<<node_item->prev_retr_flag;
	qDebug() <<"ChildCount="<<node_item->child_items.size();
	qDebug() << "DeleteFlag="<<node_item->delete_flag;
	qDebug() <<"<<<<====================";
}

void RemoteDirModel::slot_remote_dir_node_retrived (
    directory_tree_item*  parent_item,void *  parent_model_internal_pointer )
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

	int row , col = 0;
	for ( int i = parent_item->child_items.size()-1 ; i >=0  ; i -- )
	{
		if ( parent_item->child_items[i]->delete_flag == 1 )
		{
			row = parent_item->child_items[i]->row_number ;
			qDebug() << "find should delete item "<< i
			<<" row num:"<< row ;

			this->removeRows ( row,1,
			                   this->createIndex ( parent_item->row_number,0
			                                       ,parent_model_internal_pointer ) );
		}
	}

	emit layoutChanged();

	/*
	void columnsAboutToBeInserted ( const QModelIndex & parent, int start, int end )
	void columnsAboutToBeRemoved ( const QModelIndex & parent, int start, int end )
	void columnsInserted ( const QModelIndex & parent, int start, int end )
	void columnsRemoved ( const QModelIndex & parent, int start, int end )
	void dataChanged ( const QModelIndex & topLeft, const QModelIndex & bottomRight )
	void headerDataChanged ( Qt::Orientation orientation, int first, int last )
	void layoutAboutToBeChanged ()
	void layoutChanged ()
	void modelAboutToBeReset ()
	void modelReset ()
	void rowsAboutToBeInserted ( const QModelIndex & parent, int start, int end )
	void rowsAboutToBeRemoved ( const QModelIndex & parent, int start, int end )
	void rowsInserted ( const QModelIndex & parent, int start, int end )
	void rowsRemoved ( const QModelIndex & parent, int start, int end ) 
	*/
}


Qt::DropActions RemoteDirModel::supportedDropActions () const
{
	return Qt::CopyAction | Qt::MoveAction;
}

QStringList RemoteDirModel::mimeTypes() const
{
	QStringList mtypes;
	mtypes<< "text/uri-list";
	//mtypes << "text/plain";
	mtypes << QAbstractItemModel::mimeTypes();
	//qDebug()<< mtypes ;
	//mtypes = QAbstractItemModel::mimeTypes();
	//qDebug()<< mtypes ;
	//return QAbstractItemModel::mimeTypes();
	return mtypes ;

}
QMimeData *RemoteDirModel::mimeData ( const QModelIndexList &indexes ) const
{
	QMimeData * md = new QMimeData();
	QByteArray encodedData;

	//读取出来indexes中的目录路径信息，封装成特定mine 类型的数据即可以了。
	QString file_name = "file:///hahahha";
	QString file_type ;

	//现在还没有支持多选择，那么我们就只处理一个即可,不用考虑那么多
	directory_tree_item * selected_item = static_cast<directory_tree_item*> ( indexes.at ( 0 ).internalPointer() );

	assert ( selected_item != NULL );

	file_name = selected_item->strip_path.c_str() ;
	file_type = selected_item->file_type.c_str() ;

	//格式说明：sftp://file_name||file_type
	file_name = QString ( "sftp://" + file_name+"||"+file_type ).toAscii() ;
	encodedData = file_name.toAscii();
	//QList<QUrl> urls ;
	//application/x-qabstractitemmodeldatalist
	md->setData ( "text/uri-list",encodedData );
	//urls.append(QUrl(file_name));

	//md->setUrls(urls);

	return md ;
}
bool RemoteDirModel::dropMimeData ( const QMimeData *data, Qt::DropAction action,
                                    int row, int column, const QModelIndex &parent )
{
	bool ret = true ;
	QStringList local_file_names;
	QStringList remote_file_names ;

	QTextCodec * codec = QTextCodec::codecForName ( REMOTE_CODEC );
	QByteArray ba ;

	directory_tree_item * aim_item = static_cast<directory_tree_item*> ( parent.internalPointer() );
    
	QString remote_file_name = aim_item->strip_path.c_str();
	//QString remote_file_type = aim_item->file_type.c_str();
	remote_file_names << remote_file_name ;

	QList<QUrl> urls = data->urls( ) ;

	qDebug() << urls << " action: " << action <<" "<< parent ;

	if ( urls.count() == 0 )
	{
		qDebug() <<" no url droped";
		return false ;
	}

	//if(urls.count() >1 )
	{
		//    qDebug()<<"more than one file should process , but not supported now,only one processed";
	}
	QString file_name;
	for ( int i = 0 ; i < urls.count() ; i ++ )
	{
		file_name = urls.at ( i ).toString().right ( urls.at ( i ).toString().length()-7 );
		if ( file_name.trimmed().length() == 0 ) continue ;
		ba = codec->fromUnicode ( file_name );
		//qDebug()<< file_name <<" ---> :" REMOTE_CODEC << ba ;
		file_name = ba ;
		local_file_names << file_name ;
	}
	emit this->new_transfer_requested ( local_file_names,remote_file_names );

	qDebug() <<"signals emited";
	return true ;
	return ret ;
}

void RemoteDirModel::slot_remote_dir_node_clicked ( const QModelIndex & index )
{
	//qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	directory_tree_item * clicked_item = 0;
	clicked_item = static_cast<directory_tree_item*> ( index.internalPointer() );

	//this->dump_tree_node_item(clicked_item);

	if ( clicked_item->retrived == 1 ) // 半满状态结点
	{
		this->remote_dir_retrive_thread->add_node ( clicked_item,index.internalPointer() );
	}
	else
	{
		//no op needed
	}
}
