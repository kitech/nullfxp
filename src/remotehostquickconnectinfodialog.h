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
#ifndef REMOTEHOSTQUICKCONNECTINFODIALOG_H
#define REMOTEHOSTQUICKCONNECTINFODIALOG_H

#include <QtCore>

#include <QDialog>

#include "ui_remotehostquickconnectfinfodailog.h"

/**
	@author liuguangzhao <gzl@localhost>
*/
class RemoteHostQuickConnectInfoDialog : public QDialog
{
    Q_OBJECT
public:
    RemoteHostQuickConnectInfoDialog(QWidget* parent=0, Qt::WindowFlags f=0);

    ~RemoteHostQuickConnectInfoDialog();
    
    QString get_user_name ();
    QString get_host_name ();
    QString get_password();
    short  get_port();
    
    public slots:
        void slot_test_remote_host_changed(int value);
        
    private:
        Ui::RemoteHostQuickConnectInfoDialog quick_connect_info_dialog;
        
};

#endif
