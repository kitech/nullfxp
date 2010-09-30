// aboutnullfxp.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-07-15 20:48:56 +0800
// Version: $Id$
// 

#include <QtCore>

#include "nullfxp-version.h"
#include "aboutnullfxp.h"

AboutNullFXP::AboutNullFXP(QWidget *parent, Qt::WindowFlags f)
  : QDialog(parent, f)
{
    this->ui_about_nullfxp.setupUi(this);
    QString NULLFXP_RELEASE_AND_QT_VERION = QString("%1 (Using Qt %2)\nCompile with Qt %3, %4")
        .arg(NULLFXP_RELEASE).arg(qVersion())
        .arg(QT_VERSION_STR).arg(GCC_MV);
    this->ui_about_nullfxp.label_2->setText( NULLFXP_RELEASE_AND_QT_VERION );
    
    QString about_info;    
    QString about_author;

    QFile fp;
    fp.setFileName(":/data/about_info.html");
    fp.open(QIODevice::ReadOnly);
    about_info = fp.readAll();
    fp.close();
    
    fp.setFileName(":/data/about_author.html");
    fp.open(QIODevice::ReadOnly);
    about_author = fp.readAll();
    fp.close();
    
    this->ui_about_nullfxp.label->setPixmap(QPixmap(":/icons/nullget-1.png").scaledToHeight(50));
    this->ui_about_nullfxp.textBrowser_2->setHtml(about_info);
    this->ui_about_nullfxp.textBrowser->setHtml(about_author);

}

AboutNullFXP::~AboutNullFXP()
{
}

#ifndef O_OS_WIN
#include <fontconfig/fontconfig.h>
#endif

void AboutNullFXP::dummyDepend()
{

#ifndef Q_OS_WIN
    // force link fontconfig lib, can not strip 
    FcObjectSet * fco = FcObjectSetCreate();
    FcObjectSetDestroy(fco);
#endif

}
