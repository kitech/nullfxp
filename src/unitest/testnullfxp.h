// testnullfxp.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-04-04 12:53:06 +0800
// Version: $Id$
// 

#include <QtTest/QtTest>

class TestNullfxp : public QObject
{
    Q_OBJECT;
private slots:
    void initTestCase();
    
    void myFirstTest();
    
    void mySecondTest();
    
    void cleanupTestCase();
    
    void storageTest();
    
    void testSpecialFileName();
    
    void testSSHFileInfo();

    void testCXXBaseSyntax();
    void testCXX0XSyntax();

    void testCurlFtp();
};

