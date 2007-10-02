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

#include "remotedirretrivethread.h"

#include "fileproperties.h"

FileProperties::FileProperties(QWidget *parent)
    : QDialog (parent)
{
    this->ui_file_prop_dialog.setupUi(this);
}


FileProperties::~FileProperties()
{
}

void FileProperties::set_file_info_model_list(QModelIndexList &mil)
{
    if( mil.count() == 0 ) return ;
    
    directory_tree_item * item_node = static_cast<directory_tree_item*>(mil.at(0).internalPointer());
    QString file_name = mil.at(0).data().toString();
    QString file_size = mil.at(1).data().toString();
    QString file_modify_time = mil.at(3).data().toString();
    QString file_perm = mil.at(2).data().toString() ;
    
    this->ui_file_prop_dialog.lineEdit->setText( file_name );
    this->ui_file_prop_dialog.lineEdit_2->setText( file_perm.left(1));
    this->ui_file_prop_dialog.lineEdit_3->setText( item_node->strip_path.substr(0,item_node->strip_path.length()-file_name.length()-1).c_str() );
    this->ui_file_prop_dialog.lineEdit_4->setText( file_size );
    this->ui_file_prop_dialog.lineEdit_5->setText( file_modify_time );
    
    
    if( file_perm.length() < strlen("drwxr-xr-x") )
    {
        qDebug()<<" Invalide perm string";
        return ;
    }
    //perm format : drwxr-xr-x
    //for( int i = 1 ; i <= file_perm.length() ; i += 3 )
    {
        QChar rp = file_perm.at(1);
        QChar wp = file_perm.at(1+1);
        QChar xp = file_perm.at(1+2);
        
        this->ui_file_prop_dialog.checkBox->setChecked(rp=='r');
        this->ui_file_prop_dialog.checkBox_2->setChecked(wp=='w');
        this->ui_file_prop_dialog.checkBox_3->setChecked(xp=='x');
    }
    {
        QChar rp = file_perm.at(4);
        QChar wp = file_perm.at(4+1);
        QChar xp = file_perm.at(4+2);
        
        this->ui_file_prop_dialog.checkBox_4->setChecked(rp=='r');
        this->ui_file_prop_dialog.checkBox_5->setChecked(wp=='w');
        this->ui_file_prop_dialog.checkBox_6->setChecked(xp=='x');
    }
    {
        QChar rp = file_perm.at(7);
        QChar wp = file_perm.at(7+1);
        QChar xp = file_perm.at(7+2);
        this->ui_file_prop_dialog.checkBox_7->setChecked(rp=='r');
        this->ui_file_prop_dialog.checkBox_8->setChecked(wp=='w');
        this->ui_file_prop_dialog.checkBox_9->setChecked(xp=='x');
    }        
}


