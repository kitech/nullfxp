// aboutnullfxp.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-07-15 20:49:06 +0800
// Version: $Id$
// 

#ifndef ABOUTNULLFXP_H
#define ABOUTNULLFXP_H

#include <QDialog>

#include "ui_aboutnullfxp.h"

/**
 * nullfxp关于信息对话框类
 * 
 * @author liuguangzhao
 */
class AboutNullFXP : public QDialog
{
    Q_OBJECT;
public:
    AboutNullFXP(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~AboutNullFXP();

private:
    Ui::AboutNullFXP ui_about_nullfxp;
};

#endif
