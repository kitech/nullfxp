// testnullfxp.cpp --- 
// 
// Filename: testnullfxp.cpp
// Description: 
// Author: liuguangzhao
// Maintainer: 
// Created: 五  4月  4 13:05:32 2008 (CST)
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
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth
// Floor, Boston, MA 02110-1301, USA.
// 
// 

// Code:

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
    QVERIFY(storage->open()==true);
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

    QVERIFY(storage->save() == true);
  
    QVERIFY(storage->close() == true);
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

// 
// testnullfxp.cpp ends here
