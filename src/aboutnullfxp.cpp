// aboutnullfxp.cpp --- 
// 
// Filename: aboutnullfxp.cpp
// Description: 
// Author: 刘光照<liuguangzhao@users.sf.net>
// Maintainer: 
// Copyright (C) 2007-2010 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 二  7月 15 20:48:56 2008 (CST)
// Version: 
// Last-Updated: 
//           By: 
//     Update #: 0
// URL: 
// Keywords: 
// Compatibility: 
// 
// 

// Commentary: 
// 
// 
// 
// 

// Change log:
// 
// 
// 

#include "nullfxp-version.h"

#include "aboutnullfxp.h"


AboutNullFXP::AboutNullFXP(QWidget* parent, Qt::WindowFlags f): QDialog(parent, f)
{
    ui_about_nullfxp.setupUi(this);
    QString NULLFXP_RELEASE_AND_QT_VERION = QString("%1 (Using Qt %2, %3)").arg(NULLFXP_RELEASE).arg(qVersion()).arg(GCC_MV);
    this->ui_about_nullfxp.label_2->setText( NULLFXP_RELEASE_AND_QT_VERION );
    
    QString about_info = ""
            "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\np, li { white-space: pre-wrap; }\n</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">\n<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br>   Project Home: <a href=\"http://nullget.sourceforge.net/\">http://nullget.sourceforge.net/</a>    <br><br>   NullFXP : It is a SFTP Client based on Qt Library. I hope it is useful for you. It can be compiled and run on Linux/X11, FreeBSD/X11 and Windows Mingw platform now.       </body></p></body></html>"
            
            "";
    
    QString about_author = ""
            "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\np, li { white-space: pre-wrap; }\n</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">\n<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">   Visit <a href=\"http://nullget.sourceforge.net/bug\">http://nullget.sourceforge.net/bug</a> to report bugs.     <br>   <a href=\"mailto:liuguangzhao@users.sf.net\" title=\"liuguangzhao@users.sf.net\">Send me email</a> if you want to add this project.     <br><br>  Liuguangzhao:   <br>      <a href=\"mailto:liuguangzhao@users.sf.net\">liuguangzhao@users.sf.net</a>   <br>      Main developer.      </body></p></body></html>"
            
            "";
    
    this->ui_about_nullfxp.label->setPixmap(QPixmap(":/icons/nullget-1.png").scaledToHeight(50));
    this->ui_about_nullfxp.textBrowser_2->setHtml(about_info);
    this->ui_about_nullfxp.textBrowser->setHtml(about_author);
}


AboutNullFXP::~AboutNullFXP()
{
}


