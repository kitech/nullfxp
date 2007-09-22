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
#ifndef REMOTEDIRMODEL_H
#define REMOTEDIRMODEL_H

#include <QtCore>
#include <QtGui>
#include <QAbstractItemModel>

#include "sftp-operation.h"
#include "sftp-client.h"
#include "sftp-wrapper.h"

#include "remotedirretrivethread.h"


/**
	@author liuguangzhao <gzl@localhost>

	这个model必须做成lazy模式的，因为它是远程目录，只有需要显示的时候才去取来远程目录结构
*/
class RemoteDirModel : public QAbstractItemModel
{
		Q_OBJECT
	public:
		RemoteDirModel ( struct sftp_conn * conn , QObject *parent = 0 );

		virtual ~RemoteDirModel();
        //仅需要调用一次的函数,并且是在紧接着该类的初始化之后调用。
        void set_user_home_path(std::string user_home_path);
        
                
        ////model 函数
		QVariant data ( const QModelIndex &index, int role ) const;
		Qt::ItemFlags flags ( const QModelIndex &index ) const;
		QVariant headerData ( int section, Qt::Orientation orientation,
		                      int role = Qt::DisplayRole ) const;
		QModelIndex index ( int row, int column,
		                    const QModelIndex &parent = QModelIndex() ) const;
		QModelIndex parent ( const QModelIndex &child ) const;
		int rowCount ( const QModelIndex &parent = QModelIndex() ) const;
		int columnCount ( const QModelIndex &parent = QModelIndex() ) const;

		virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
		virtual bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() ) ;
		virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() ) ;
        
		bool rowMoveTo ( const QModelIndex & from , const QModelIndex & to );
        bool insertColumns ( int column, int count, const QModelIndex & parent = QModelIndex() ) ;
        
        bool setItemData ( const QModelIndex & index, const QMap<int, QVariant> & roles )  ;
		
// 		int rowCount(const QModelIndex &parent = QModelIndex()) const;
// 		int columnCount(const QModelIndex &parent = QModelIndex()) const;
// 
// 		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
// 		bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
// 
// 		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
// 
// 		bool hasChildren(const QModelIndex &index = QModelIndex()) const;
// 		Qt::ItemFlags flags(const QModelIndex &index) const;
// 
// 		void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
// 
		QStringList mimeTypes() const;
		QMimeData *mimeData(const QModelIndexList &indexes) const;
		bool dropMimeData(const QMimeData *data, Qt::DropAction action,
		                  int row, int column, const QModelIndex &parent);
		Qt::DropActions supportedDropActions() const;    
// 
// 		
// 		//QDirModel specific API
// 		
// 		void setNameFilters(const QStringList &filters);
// 		QStringList nameFilters() const;
// 
// 		void setFilter(QDir::Filters filters);
// 		QDir::Filters filter() const;
// 
// 		void setSorting(QDir::SortFlags sort);
// 		QDir::SortFlags sorting() const;
// 
// 		void setResolveSymlinks(bool enable);
// 		bool resolveSymlinks() const;
// 
// 		void setReadOnly(bool enable);
// 		bool isReadOnly() const;
// 
// 		void setLazyChildCount(bool enable);
// 		bool lazyChildCount() const;
// 
// 		QModelIndex index(const QString &path, int column = 0) const;
// 
// 		bool isDir(const QModelIndex &index) const;
// 		QModelIndex mkdir(const QModelIndex &parent, const QString &name);
// 		bool rmdir(const QModelIndex &index);
// 		bool remove(const QModelIndex &index);
// 
// 		QString filePath(const QModelIndex &index) const;
// 		QString fileName(const QModelIndex &index) const;
// 		QIcon fileIcon(const QModelIndex &index) const;
// 		QFileInfo fileInfo(const QModelIndex &index) const;    
// 
// 		public slots:
// 		    void refresh(const QModelIndex &parent = QModelIndex());
// 		
        
        
        
    public slots:
        void slot_remote_dir_node_retrived(directory_tree_item* parent_item,void *  parent_model_internal_pointer );
      
        
        void slot_remote_dir_node_clicked(const QModelIndex & index);
        
        void slot_execute_command( directory_tree_item* parent_item , void * parent_model_internal_pointer, int cmd , std::string params );
        
        //keep_alive
        void set_keep_alive(bool keep_alive,int time_out=150);
    private slots:
        /// time_out 秒                
        void slot_keep_alive_time_out();
    signals:
        //void new_transfer_requested(QString local_file_name,QString local_file_type,                                    QString remote_file_name,QString remote_file_type);
        void new_transfer_requested(QStringList local_file_names,                                    QStringList remote_file_names);
        
        //for wait option
        void enter_remote_dir_retrive_loop();
        void leave_remote_dir_retrive_loop();
        
	private:

		directory_tree_item * tree_root ;
		struct sftp_conn * sftp_connection ;
        RemoteDirRetriveThread * remote_dir_retrive_thread ;
        
		void dump_tree_node_item ( directory_tree_item * node_item ) const ;

        std::string user_home_path ;
        
        bool    keep_alive ;
        QTimer  * keep_alive_timer ;
        int     keep_alive_interval;        
};

#endif
