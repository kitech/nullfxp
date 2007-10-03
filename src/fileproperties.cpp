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

#include "utils.h"

#include "remotedirretrivethread.h"

#include "fileproperties.h"

FilePropertiesRetriveThread::FilePropertiesRetriveThread(LIBSSH2_SFTP * ssh2_sftp ,QString file_path , QObject * parent): QThread(parent)
{
    this->ssh2_sftp = ssh2_sftp ;
    this->file_path = file_path ;
}
FilePropertiesRetriveThread::~FilePropertiesRetriveThread()
{
}
void FilePropertiesRetriveThread::run()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    LIBSSH2_SFTP_ATTRIBUTES * sftp_attrib = (LIBSSH2_SFTP_ATTRIBUTES*)malloc( sizeof( LIBSSH2_SFTP_ATTRIBUTES));
    memset( sftp_attrib,0,sizeof( LIBSSH2_SFTP_ATTRIBUTES ));
	libssh2_sftp_stat ( ssh2_sftp , file_path.toAscii().data() ,  sftp_attrib );
    emit file_attr_abtained( sftp_attrib );
}

///////////////////////////////////////////////////
FileProperties::FileProperties ( QWidget *parent )
		: QDialog ( parent )
{
	this->ui_file_prop_dialog.setupUi ( this );

    //connect(this,SIGNAL(finished()),this,SLOT(slot_this_thread_finished()));
}

FileProperties::~FileProperties()
{

}
void FileProperties::set_ssh2_sftp ( void * ssh2_sftp )
{
	this->ssh2_sftp = ( LIBSSH2_SFTP* ) ssh2_sftp ;
}

void FileProperties::set_file_info_model_list ( QModelIndexList &mil )
{
	if ( mil.count() == 0 ) return ;

	directory_tree_item * item_node = static_cast<directory_tree_item*> ( mil.at ( 0 ).internalPointer() );
	QString file_name = mil.at ( 0 ).data().toString();
	QString file_size = mil.at ( 1 ).data().toString();
	QString file_modify_time = mil.at ( 3 ).data().toString();
	QString file_perm = mil.at ( 2 ).data().toString() ;
    
    //qDebug()<<item_node->strip_path.c_str();
	this->ui_file_prop_dialog.lineEdit->setText ( file_name );
	this->ui_file_prop_dialog.lineEdit_2->setText ( file_perm.left ( 1 ) );
    this->ui_file_prop_dialog.lineEdit_3->setText ( item_node->strip_path.length()-file_name.length() ==1 ? "/" : ( item_node->strip_path.substr ( 0,item_node->strip_path.length()-file_name.length()-1 ).c_str() ) );
	this->ui_file_prop_dialog.lineEdit_4->setText ( file_size );
	this->ui_file_prop_dialog.lineEdit_5->setText ( file_modify_time );


	if ( file_perm.length() < strlen ( "drwxr-xr-x" ) )
	{
		//qDebug() <<" Invalide perm string";
		//return ;
	}
	else
    {//perm format : drwxr-xr-x
	   this->update_perm_table ( file_perm );
    }
    
    QString file_path =  this->ui_file_prop_dialog.lineEdit_3->text() +QString ( "/" ) +this->ui_file_prop_dialog.lineEdit->text() ;
    FilePropertiesRetriveThread * rt = new FilePropertiesRetriveThread(this->ssh2_sftp,file_path , this);
    QObject::connect( rt ,SIGNAL( file_attr_abtained(void*)),this,SLOT(slot_file_attr_abtained(void*)) );
    rt->start();
}

void FileProperties::slot_prop_thread_finished()
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
}
void FileProperties::slot_file_attr_abtained( void * attr )
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
	char file_date[60] = {0};
	char file_perm[60] = {0};
    char file_size[60] = {0} ;
	LIBSSH2_SFTP_ATTRIBUTES * sftp_attrib = (LIBSSH2_SFTP_ATTRIBUTES *) attr ;
    
	struct tm *ltime = localtime ( ( time_t* ) &sftp_attrib->atime );
	if ( ltime != NULL )
	{
		if ( time ( NULL ) - sftp_attrib->atime < ( 365*24*60*60 ) /2 )
			strftime ( file_date, sizeof file_date, "%Y/%m/%d %H:%M:%S", ltime );
		else
			strftime ( file_date, sizeof file_date, "%Y/%m/%d %H:%M:%S", ltime );
	}
    this->ui_file_prop_dialog.lineEdit_6->setText ( file_date );
    ltime = localtime ( ( time_t* ) &sftp_attrib->mtime );
    if ( ltime != NULL )
    {
        if ( time ( NULL ) - sftp_attrib->mtime < ( 365*24*60*60 ) /2 )
            strftime ( file_date, sizeof file_date, "%Y/%m/%d %H:%M:%S", ltime );
        else
            strftime ( file_date, sizeof file_date, "%Y/%m/%d %H:%M:%S", ltime );
    }
    this->ui_file_prop_dialog.lineEdit_5->setText ( file_date );	
    
	if ( this->ui_file_prop_dialog.lineEdit_2->text() == "D" )
	{
        memset(file_size,0,sizeof(file_size )) ;
        snprintf(file_size,sizeof(file_size) , "%llu",sftp_attrib->filesize );
        this->ui_file_prop_dialog.lineEdit_4->setText ( file_size );
        this->ui_file_prop_dialog.lineEdit_2->setText(tr("directory"));
	}
	else if ( this->ui_file_prop_dialog.lineEdit_2->text() == "d" )
	{
        this->ui_file_prop_dialog.lineEdit_2->setText(tr("directory"));
	}
	else if ( this->ui_file_prop_dialog.lineEdit_2->text() == "l" )
	{
		//qDebug() <<" open link , not process now";
        //TODO 写一个更好的，根据文件后缀判断文件类型的类库
        this->ui_file_prop_dialog.lineEdit_2->setText(tr("symlink"));
	}
	else
	{
		// reg file??
        this->ui_file_prop_dialog.lineEdit_2->setText(tr("regular file"));
	}
	strmode ( sftp_attrib->permissions,file_perm );
	this->update_perm_table ( file_perm );
    free( sftp_attrib );
    
}

void FileProperties::update_perm_table ( QString file_perm )
{
    //在一个线程中操作UI元素很不安全，容易导致程序死锁
	//perm format : drwxr-xr-x
	{
		QChar rp = file_perm.at ( 1 );
		QChar wp = file_perm.at ( 1+1 );
		QChar xp = file_perm.at ( 1+2 );

		this->ui_file_prop_dialog.checkBox->setChecked ( rp=='r' );
		this->ui_file_prop_dialog.checkBox_2->setChecked ( wp=='w' );
		this->ui_file_prop_dialog.checkBox_3->setChecked ( xp=='x' );
	}
	{
		QChar rp = file_perm.at ( 4 );
		QChar wp = file_perm.at ( 4+1 );
		QChar xp = file_perm.at ( 4+2 );

		this->ui_file_prop_dialog.checkBox_4->setChecked ( rp=='r' );
		this->ui_file_prop_dialog.checkBox_5->setChecked ( wp=='w' );
		this->ui_file_prop_dialog.checkBox_6->setChecked ( xp=='x' );
	}
	{
		QChar rp = file_perm.at ( 7 );
		QChar wp = file_perm.at ( 7+1 );
		QChar xp = file_perm.at ( 7+2 );
		this->ui_file_prop_dialog.checkBox_7->setChecked ( rp=='r' );
		this->ui_file_prop_dialog.checkBox_8->setChecked ( wp=='w' );
		this->ui_file_prop_dialog.checkBox_9->setChecked ( xp=='x' );
	}
}

