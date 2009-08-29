// globaloptionsdialog.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-08-29 16:25:33 +0800
// Version: $Id$
// 

#ifndef GLOBALOPTIONSDIALOG_H
#define GLOBALOPTIONSDIALOG_H

#include <QDialog>

/**
	@author liuguangzhao <gzl@localhost>
*/
class GlobalOptionsDialog : public QDialog
{
Q_OBJECT
public:
    GlobalOptionsDialog(QWidget* parent, Qt::WindowFlags f=0);

    ~GlobalOptionsDialog();

};

#endif
