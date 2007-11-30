/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   liuguangzhao@users.sourceforge.net   *
 *
 *   http://www.qtchina.net                                                *
 *   http://nullget.sourceforge.net                                        *
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
#ifndef FILEPROPERTIES_H
#define FILEPROPERTIES_H

#include <QtCore>
#include <QtGui>
#include <QDialog>

#include "libssh2.h"
#include "libssh2_sftp.h"

#include "ui_fileproperties.h"

class FilePropertiesRetriveThread : public QThread
{
    Q_OBJECT
    public:
        FilePropertiesRetriveThread( LIBSSH2_SFTP * ssh2_sftp , QString file_path , QObject * parent = 0 );
        ~FilePropertiesRetriveThread();
        virtual void run ();
    signals:
        void file_attr_abtained(QString file_name, void * attr );
    private:
        LIBSSH2_SFTP * ssh2_sftp ;
        QString file_path ;
};

/**
	@author liuguangzhao <gzl@localhost>
*/
class FileProperties : public  QDialog
{
Q_OBJECT
public:
    FileProperties(QWidget *parent = 0);

    ~FileProperties();
    void set_ssh2_sftp( void * ssh2_sftp );
    
    void set_file_info_model_list(QModelIndexList &mil);

    public slots:
        void slot_prop_thread_finished();
        void slot_file_attr_abtained(QString file_name, void * attr );
        
    private:
        void update_perm_table( QString file_perm );
        QString type(QString file_name);
        Ui::FileProperties ui_file_prop_dialog;
        LIBSSH2_SFTP * ssh2_sftp ;
};

class LocalFileProperties: public QDialog
{
Q_OBJECT
public:
    LocalFileProperties(QWidget *parent = 0);

    ~LocalFileProperties();
    //void set_file_info_model_list(QModelIndexList &mil);
    void set_file_info_model_list(QString file_name);

    public slots:

    private:
        void update_perm_table( QString file_name );
        QString type(QString file_name);
        Ui::FileProperties ui_file_prop_dialog;
        QString file_name;
};

#endif
