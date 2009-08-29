// globaloption.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2007-05-17 11:05:08 +0800
// Version: $Id$
// 

#include <cassert>

#include <QtCore>
#include <QTextCodec>

#include "globaloption.h"

GlobalOption * gOpt = GlobalOption::instance();

GlobalOption * GlobalOption::mInstance = 0 ;

GlobalOption * GlobalOption::instance()
{
    if (GlobalOption::mInstance == 0) {
        GlobalOption::mInstance = new GlobalOption();
        GlobalOption::mInstance->remote_codec = QTextCodec::codecForName("UTF-8");
        //GlobalOption::mInstance->locale_codec = "UTF-8";
        GlobalOption::mInstance->locale_codec = QTextCodec::codecForLocale();
		// qDebug()<<"Locale codec Mib Enum:"<< GlobalOption::mInstance->locale_codec->mibEnum()
		// 	<<"Locale codec name: "<< GlobalOption::mInstance->locale_codec->name()
		// 	<<GlobalOption::mInstance->locale_codec->aliases()
		// 	<<"Remote codec name:" << GlobalOption::mInstance->remote_codec->name() 
		// 	<<GlobalOption::mInstance->remote_codec->aliases() ;
			//<< GlobalOption::mInstance->remote_codec->availableCodecs () ;
        GlobalOption::mInstance->keep_alive = true ;
        GlobalOption::mInstance->kepp_alive_internal = 180; //S
        GlobalOption::mInstance->test_codec =  QTextCodec::codecForName("C");
        //Q_CHECK_PTR(GlobalOption::mInstance->test_codec);
		//qDebug()<<"GBK: "<<  GlobalOption::mInstance->test_codec->aliases();
    }
    
    return GlobalOption::mInstance;
}
GlobalOption::GlobalOption()
{
}


GlobalOption::~GlobalOption()
{
}

void GlobalOption::set_remote_codec(const char *codec)
{
    this->remote_codec = QTextCodec::codecForName(codec);
    if (this->remote_codec == 0) {
        this->remote_codec = QTextCodec::codecForName("UTF-8");
        assert( 1 == 2 );
    }
}


