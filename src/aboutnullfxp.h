/* aboutnullfxp.h --- 
 * 
 * Filename: aboutnullfxp.h
 * Description: 
 * Author: 刘光照<liuguangzhao@users.sf.net>
 * Maintainer: 
 * Copyright (C) 2007-2010 liuguangzhao <liuguangzhao@users.sf.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 二  7月 15 20:49:06 2008 (CST)
 * Version: 
 * Last-Updated: 
 *           By: 
 *     Update #: 0
 * URL: 
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change log:
 * 
 * 
 */

#ifndef ABOUTNULLFXP_H
#define ABOUTNULLFXP_H

#include <QDialog>

#include "ui_aboutnullfxp.h"

/**
	@author liuguangzhao <gzl@localhost>
*/
class AboutNullFXP : public QDialog
{
Q_OBJECT
public:
    AboutNullFXP(QWidget* parent=0, Qt::WindowFlags f=0);

    ~AboutNullFXP();

    private:
        Ui::AboutNullFXP ui_about_nullfxp ;
        
};

#endif
