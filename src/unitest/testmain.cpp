// testmain.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-04-04 12:50:57 +0800
// Version: $Id$
// 

#include "testnullfxp.h"

int main(int argc, char ** argv)
{
	QCoreApplication a(argc, argv);
    TestNullfxp mytest;
    QTest::qExec(&mytest);
    return 0;
}

