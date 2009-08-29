// testnullfxp.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-04-04 13:05:32 +0800
// Version: $Id$
// 

#include "testnullfxp.h"

#include "basestorage.h"
#include "utils.h"
#include "libssh2.h"
#include "libssh2_sftp.h"
#include "sshfileinfo.h"

void TestNullfxp::initTestCase()
{
    QVERIFY(1 == 1);
}

void TestNullfxp::myFirstTest()
{
    QVERIFY(1 == 1);
}

void TestNullfxp::mySecondTest()
{
    QVERIFY(1 == 1);
}

void TestNullfxp::storageTest()
{
    BaseStorage * storage = BaseStorage::instance();

    QMap<QString, QString> host;
    host["show_name"] = "hahaha";
    host["host_name"] = "www.qtchina.net";
    host["user_name"] = "liuguangzhao";
    if(storage->containsHost(host["show_name"]))
        QVERIFY(storage->addHost(host) == false);
    else
        QVERIFY(storage->addHost(host) == true);

    host.clear();
    host["show_name"] = "hahahatttt";
    host["host_name"] = "shell.sf.net";
    host["user_name"] = "liuguangzhao6566";

    QVERIFY(storage->updateHost(host) == true);

}

void TestNullfxp::cleanupTestCase()
{
    QVERIFY(1 == 1);
}

void TestNullfxp::testSpecialFileName()
{
    QUrl u("file:///home/gzl/nxpt//#.newsrc-dribble#");
    int rc = is_reg("/home/gzl/nxpt//#.newsrc-dribble#");
    QVERIFY(rc != 0 ) ;
}

void TestNullfxp::testSSHFileInfo()
{
    LIBSSH2_SFTP_ATTRIBUTES attr;

    memset(&attr, 0, sizeof(LIBSSH2_SFTP_ATTRIBUTES));
    attr.filesize = 123456;
    QVERIFY(SSHFileInfo(attr).size() == 123456);
    attr.filesize = 1234567;
    QVERIFY(SSHFileInfo(&attr).size() != 123456);
    QVERIFY(SSHFileInfo(&attr).size() == 1234567);

    attr.permissions = S_IFDIR;
    QVERIFY(SSHFileInfo(attr).isDir() == true);
    attr.permissions = S_IFREG;
    QVERIFY(SSHFileInfo(attr).isDir() == false);
    QVERIFY(SSHFileInfo(attr).isFile() == true);

    QVERIFY(SSHFileInfo(attr).isSymLink() == false);
    attr.permissions = S_IFLNK;
    QVERIFY(SSHFileInfo(attr).isSymLink() == true);

    QVERIFY(SSHFileInfo(0).isSymLink() == false);
    QVERIFY(SSHFileInfo(0).isDir() == false);
    QVERIFY(SSHFileInfo(0).isFile() == false);
    QVERIFY(SSHFileInfo(0).size() == 0);

}

void TestNullfxp::testCXXBaseSyntax()
{
    char *str = new char[100];
    QVERIFY(str != NULL);
    delete [] str;
    int *ia = new int[50];
    QVERIFY(ia != NULL);
    QVERIFY(ia[0] != 0);
    memset(ia, 0, sizeof(int)*50);
    QVERIFY(ia[0] == 0);
    delete [] ia;
    
    ia = (int*)calloc(50, sizeof(int));
    QVERIFY(ia[0] == 0);
    free(ia);
    ia = (int*)malloc(sizeof(int)*50);
    QVERIFY(ia[0] != 0);
    memset(ia, 0, sizeof(int)*50);
    QVERIFY(ia[0] == 0);
    free(ia);
}

#ifndef _MSC_VER
#include <tr1/array>
#endif

void TestNullfxp::testCXX0XSyntax()
{
#ifndef _MSC_VER
    std::tr1::array<int, 0> tr1arr;
    QVERIFY(tr1arr.size() == 0);
    tr1arr[0] = 5;
    qDebug()<<"array size:"<<tr1arr.size();
    QVERIFY(tr1arr.size() == 0);
    tr1arr[1] = 6;
    QVERIFY(tr1arr.size() == 0);
    qDebug()<<"a[1]="<<tr1arr[1];
    //这个数组模板个数很奇怪啊。
#endif    
}
