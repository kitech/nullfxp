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
#ifndef REMOTEHOSTCONNECTINGSTATUSDIALOG_H
#define REMOTEHOSTCONNECTINGSTATUSDIALOG_H

#include <QtCore>
#include <QDialog>

#include "ui_remotehostconnectingstatusdialog.h"

/**
	@author liuguangzhao <gzl@localhost>
*/
class RemoteHostConnectingStatusDialog : public QDialog
{
Q_OBJECT
public:
    RemoteHostConnectingStatusDialog(QString user_name,QString host_name,QWidget* parent, Qt::WindowFlags f);

    ~RemoteHostConnectingStatusDialog();
    
    public slots:
        void  slot_connect_state_changed(QString state_desc );
        
    private slots:
        void slot_time_out();
    
    private:
        QString host_name;
        QString user_name ;
            
        QTimer  timer ;
        
        Ui::RemoteHostConnectingStatusDialog connect_status_dialog;
    
};

#endif
