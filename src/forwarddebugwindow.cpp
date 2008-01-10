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
#include "forwarddebugwindow.h"

ForwardDebugWindow::ForwardDebugWindow(QWidget *parent)
    : QDialog(parent)
{
    fdw.setupUi(this);
    curr_show_level = 0;
    this->fdw.textEdit->setUndoRedoEnabled(false);
}


ForwardDebugWindow::~ForwardDebugWindow()
{
}

void ForwardDebugWindow::slot_log_debug_message(QString key, int level, QString msg)
{
    QMap<int, QStringList> debug_msg;
    QStringList sl;
    if(!this->msg_vec.contains(key))
    {
        sl<< msg;
        debug_msg.insert(level, sl);
        this->msg_vec[key] = debug_msg;
    }
    else
    {
        debug_msg = this->msg_vec[key];
        if(debug_msg.contains(level))
        {
            this->msg_vec[key][level]<<msg;
        }
        else
        {
            sl<<msg;
            this->msg_vec[key][level] = sl;
        }
    }
    if(-1 == this->fdw.comboBox_2->findText(key))
    {
        this->fdw.comboBox_2->addItem(key);
        this->curr_show_key = this->fdw.comboBox_2->currentText();
    }
    if(curr_show_key==key 
       && (curr_show_level == level || curr_show_level == 0)
       && this->isVisible())
    {
        this->fdw.textEdit->insertPlainText(msg+"\n");
    }
    if(this->msg_vec[key][level].count() > 10)
    {
        for(int i=0; i < 5; i ++)
        {
            this->msg_vec[key][level].pop_front ();
        }
        if(curr_show_key==key 
           && (curr_show_level == level || curr_show_level == 0)
           && this->isVisible())
        {
            this->slot_reload_message();
        }
    }
    //qDebug()<<key<<":"<<level<<":"<<msg<<this->msg_vec[key][level]<<"\n";
}

void ForwardDebugWindow::slot_reload_message()
{
    this->fdw.textEdit->clear();
}


