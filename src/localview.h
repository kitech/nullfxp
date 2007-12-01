/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   liuguangzhao@users.sourceforge.net   *
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
#ifndef LOCALVIEW_H
#define LOCALVIEW_H

#include <QtCore>
#include <QtGui>
#include <QWidget>
#include <QMdiSubWindow>
#include <QTreeWidget>
#include <QDirModel>

// #include "localdirfilemodel.h"
#include "localdirsortfiltermodel.h"

#include "ui_localview.h"

class RemoteView ;

/**
	@author liuguangzhao <liuguangzhao@users.sourceforge.net>
*/
class LocalView : public QWidget
{
Q_OBJECT
public:
    LocalView(QWidget *parent = 0);

    ~LocalView();

    QString get_selected_directory();
    
    void update_layout();
    
    signals:
        void new_upload_requested(QStringList local_file_names);
        
    private:
        QStatusBar * status_bar ;
        QDirModel * model ;
        Ui::LocalView localView ;
        LocalDirSortFilterModel * dir_file_model ;
        int   table_row_height ;
        QAbstractItemView * curr_item_view ;    //
        
        void expand_to_home_directory(QModelIndex parent_model,int level );
        
        QMenu * local_dir_tree_context_menu ;
        
        void init_local_dir_tree_context_menu();
        
    public slots:
        
        void slot_local_dir_tree_context_menu_request(const QPoint & pos );
        
        void slot_local_new_upload_requested();
        
        void slot_refresh_directory_tree();
        
        void slot_show_hidden(bool show);
        
    private slots:
        void slot_dir_tree_item_clicked( const QModelIndex & index);
        void slot_dir_file_view_double_clicked( const QModelIndex & index );
        void slot_show_properties();
        void slot_mkdir();
        void slot_rmdir();
        void slot_rename();
        void rm_file_or_directory_recursively();
        
    protected:
        virtual void closeEvent ( QCloseEvent * event );
};

#endif
