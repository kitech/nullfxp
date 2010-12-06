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

// #include "ui_aboutnullfxp.h"

namespace Ui {
    class AboutNullFXP;
};
/**
 * nullfxp关于信息对话框类
 * 
 * @author liuguangzhao
 */
class AboutNullFXP : public QDialog
{
    Q_OBJECT;
public:
    explicit AboutNullFXP(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~AboutNullFXP();

    void dummyDepend();
private:
    Ui::AboutNullFXP *uiw;
};

#endif
