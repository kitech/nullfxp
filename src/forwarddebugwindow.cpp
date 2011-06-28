// forwarddebugwindow.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2007-08-29 16:41:20 +0800
// Version: $Id$
// 

#include "ui_forwarddebugwindow.h"
#include "forwarddebugwindow.h"

ForwardDebugWindow::ForwardDebugWindow(QWidget *parent)
    : QDialog(parent)
    , uiw(new Ui::ForwardDebugWindow())
{
    uiw->setupUi(this);
    curr_show_level = 0;
    this->uiw->textEdit->setUndoRedoEnabled(false);
    
    QObject::connect(this->uiw->comboBox, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(slot_currentIndexChanged(int)));
    QObject::connect(this->uiw->comboBox_2, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(slot_currentIndexChanged(int)));
}


ForwardDebugWindow::~ForwardDebugWindow()
{
}

void ForwardDebugWindow::slot_log_debug_message(QString key, int level, QString msg)
{
    this->msg_vec.append(DebugMessage(key,level,msg));

    if (-1 == this->uiw->comboBox_2->findText(key)) {
        this->uiw->comboBox_2->addItem(key);
        this->curr_show_key = this->uiw->comboBox_2->currentText();
    }
    if (curr_show_key==key 
        && (curr_show_level == level || curr_show_level == 0)
        && this->isVisible()) {
        this->uiw->textEdit->insertPlainText(msg+"\n");
    }
    if (this->msg_vec.count() > 10) {
        this->msg_vec.remove(0,10);
        if (curr_show_key==key 
            && (curr_show_level == level || curr_show_level == 0)
            && this->isVisible()) {
            this->slot_reload_message(key, level);
        }
    }
    //qDebug()<<key<<":"<<level<<":"<<msg<<this->msg_vec[key][level]<<"\n";
}

void ForwardDebugWindow::slot_reload_message(QString key, int level)
{
    this->uiw->textEdit->clear();

    for (int i =0 ; i < this->msg_vec.count() ;i ++) {
        if (key == this->msg_vec.at(i).key
            && (level == 0 || level == this->msg_vec.at(i).level)){
            this->uiw->textEdit->insertPlainText(QString("%1: %2").arg(this->msg_vec.at(i).level).arg(this->msg_vec.at(i).msg));        }
    }
}

void ForwardDebugWindow::slot_currentIndexChanged ( int index )
{
    //qDebug()<<__FILE__<<__LINE__;
    
    QString key = this->uiw->comboBox_2->currentText();
    int level = this->uiw->comboBox->currentIndex();
    if (key.length() == 0) return;
    this->slot_reload_message(key, level);
}


