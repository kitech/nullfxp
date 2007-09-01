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
#ifndef LOCALVIEW_H
#define LOCALVIEW_H

#include <QtCore>
#include <QtGui>
#include <QWidget>
#include <QMdiSubWindow>
#include <QTreeWidget>
#include <QDirModel>

#include "sftp-client.h"
#include "ui_localview.h"

/**
	@author liuguangzhao <gzl@localhost>
*/
class LocalView : public QWidget
{
Q_OBJECT
public:
    LocalView(QWidget *parent = 0);

    ~LocalView();

    void set_sftp_connection(struct sftp_conn* conn);
    
    QString get_selected_directory();
    
    signals:
        void new_upload_requested(QString local_file_name,QString local_file_type );
        
    private:
        QDirModel * model ;
        Ui::LocalView localView ;
        struct sftp_conn * sftp_connection ;
        
        QMenu * local_dir_tree_context_menu ;
        
        void init_local_dir_tree_context_menu();
        
    public slots:
        
        //void slot_remote_new_transfer_requested(QString filename);
        
        void slot_local_dir_tree_context_menu_request(const QPoint & pos );
        
        void slot_local_new_upload_requested();
        
        void slot_refresh_directory_tree();
        
};

#endif
