/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   liuguangzhao@users.sourceforge.net   *
 *                                                                         *
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

#include "nullfxp-version.h"

#include "aboutnullfxp.h"


AboutNullFXP::AboutNullFXP(QWidget* parent, Qt::WindowFlags f): QDialog(parent, f)
{
    ui_about_nullfxp.setupUi(this);
    this->ui_about_nullfxp.label_2->setText( NULLFXP_RELEASE );
    
    QString about_info = ""
            "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\np, li { white-space: pre-wrap; }\n</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">\n<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br>   Project Home: <a href=\"http://nullget.sourceforge.net/\">http://nullget.sourceforge.net/</a>    <br><br>   NullFXP : It is a SFTP Client based on Qt Library. I hope it is useful for you. It can be compiled and run on Linux/X11 and Windows Mingw platform now.       </body></p></body></html>"
            
            "";
    
    QString about_author = ""
            "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\np, li { white-space: pre-wrap; }\n</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">\n<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">   Visit <a href=\"http://nullget.sourceforge.net/bug\">http://nullget.sourceforge.net/bug</a> to report bugs.     <br>   <a href=\"mailto:liuguangzhao@users.sourceforge.net\" title=\"liuguangzhao@users.sourceforge.net\">Send me email</a> if you want to add this project.     <br><br>  Liuguangzhao:   <br>      <a href=\"mailto:liuguangzhao@users.sourceforge.net\">liuguangzhao@users.sourceforge.net</a>   <br>      Main developer.      </body></p></body></html>"
            
            "";
    
    this->ui_about_nullfxp.textBrowser_2->setHtml(about_info);
    this->ui_about_nullfxp.textBrowser->setHtml(about_author);
}


AboutNullFXP::~AboutNullFXP()
{
}


