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


//////////////////////////
/////////////////////////////////
/////////////////////////////////////
RemoteDirModel::RemoteDirModel ( struct sftp_conn * conn , QObject *parent )
		:QAbstractItemModel ( parent )
{
	this->sftp_connection = conn ;

	this->remote_dir_retrive_thread = new RemoteDirRetriveThread ( conn );
	QObject::connect ( this->remote_dir_retrive_thread,SIGNAL ( remote_dir_node_retrived ( directory_tree_item *,const QModelIndex* ) ),
	                   this,SLOT ( slot_remote_dir_node_retrived ( directory_tree_item*,const QModelIndex* ) ) );


	this->tree_root = new  directory_tree_item ();
	//memset ( this->tree_root,0,sizeof ( this->tree_root ) );

	directory_tree_item * first_item  = new directory_tree_item();
	first_item->tree_node_item.insert ( std::make_pair ( 'N',"/" ) );
	first_item->tree_node_item.insert ( std::make_pair ( 'T',"D" ) ) ;
	first_item->parent_item = this->tree_root;
	first_item->row_number = 0;
	first_item->retrived = 0;
	first_item->strip_path = "";    //对根，不需要strip前缀，现在程序的表现就好了。

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
			ret_var = QVariant ( item->tree_node_item['N'].c_str() );
			break;
		case 2:

			ret_var = QVariant ( item->tree_node_item['T'] .c_str () );
			break;
		case 1:
			//return node.nodeValue().split("\n").join(" ");
			ret_var = QVariant ( item->tree_node_item['S'] .c_str() );
			break;
		case 3:
			ret_var = QVariant ( item->tree_node_item['D'].c_str() );
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
		        || parent_item->retrived == 1
		        || parent_item->retrived == 2 ) //为了lazy模式的需要，才做一个假数据。
		{
			//或者是不是要在这里取子结点的行数，即去远程找数据呢。
			//row_count =1 ;
			this->remote_dir_retrive_thread->add_node ( parent_item,&parent );
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
	qDebug() <<"name="<<QString ( node_item->tree_node_item['N'].c_str() );
	qDebug() <<"Type="<<QString ( node_item->tree_node_item['T'].c_str() );
	qDebug() <<"Size="<<QString ( node_item->tree_node_item['S'].c_str() );
	qDebug() <<"Date="<<QString ( node_item->tree_node_item['D'].c_str() );
	qDebug() <<"Retrived="<<node_item->retrived;
	qDebug() <<"ChildCount="<<node_item->child_items.size();
	qDebug() <<"<<<<====================";
}

void RemoteDirModel::slot_remote_dir_node_retrived (
    directory_tree_item*  parent_item,const QModelIndex * parent_model )
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

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
    directory_tree_item * selected_item = static_cast<directory_tree_item*> (indexes.at(0).internalPointer());
    
    assert( selected_item != NULL );
    
    file_name = selected_item->strip_path.c_str() ;
    file_type = selected_item->file_type.c_str() ;
    
    //格式说明：sftp://file_name||file_type
    file_name = QString( "sftp://" + file_name+"||"+file_type ).toAscii() ;
    encodedData = file_name.toAscii();
    //QList<QUrl> urls ;
    //application/x-qabstractitemmodeldatalist
    md->setData("text/uri-list",encodedData);
    //urls.append(QUrl(file_name));
    
    //md->setUrls(urls);

	return md ;
}
bool RemoteDirModel::dropMimeData ( const QMimeData *data, Qt::DropAction action,
                                    int row, int column, const QModelIndex &parent )
{
	bool ret = true ;

    directory_tree_item * aim_item = static_cast<directory_tree_item*>(parent.internalPointer());
    
    QString remote_file_name = aim_item->strip_path.c_str();
    QString remote_file_type = aim_item->file_type.c_str();
    
    QList<QUrl> urls = data->urls( ) ;
    
    qDebug()<< urls << " action: " << action <<" "<< parent ;
    
    if(urls.count() == 0 )
    {
        qDebug()<<" no url droped";
        return false ;    
    }
        
    if(urls.count() >1 )
    {
        qDebug()<<"more than one file should process , but not supported now,only one processed";        
    }
    QString file_name = urls.at(0).toString().right(urls.at(0).toString().length()-7);
    qDebug()<< file_name ;
    
    emit this->new_transfer_requested(file_name,QString("-rw-r--r--"),
                                      remote_file_name,remote_file_type );
    qDebug()<<"signals emited";
    return true ;
	return ret ;
}


