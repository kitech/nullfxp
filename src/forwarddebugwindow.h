// forwarddebugwindow.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2007-08-29 16:41:34 +0800
// Version: $Id$
// 

#ifndef FORWARDDEBUGWINDOW_H
#define FORWARDDEBUGWINDOW_H

#include <QtCore>
#include <QtGui>

namespace Ui {
    class ForwardDebugWindow;
};

class DebugMessage
{
    public:
        DebugMessage(){}
        DebugMessage(QString key, int level, QString msg)
        {
            this->key = key;
            this->level = level;
            this->msg = msg;
        }
        QString key;
        int  level;
        QString msg;
};
/**
Port Forward Connection Debug Message Window Class

	@author liuguangzhao <liuguangzhao@users.sf.net>
*/
class ForwardDebugWindow : public QDialog
{
    Q_OBJECT;
public:
    ForwardDebugWindow(QWidget *parent = 0);
    virtual ~ForwardDebugWindow();
private:
    Ui::ForwardDebugWindow *uiw;
    char curr_show_level;
    QString curr_show_key;
    //QMap<QString, QVector<QPair<int, QStringList> > > msg_vec;
    QVector<DebugMessage> msg_vec;

public slots:
    void slot_log_debug_message(QString key, int level, QString msg);
    void slot_reload_message(QString key, int level);
    void slot_currentIndexChanged ( int index );
};

#endif
