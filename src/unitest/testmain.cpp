// testmain.cpp --- 
// 
// Filename: testmain.cpp
// Description: 
// Author: liuguangzhao
// Maintainer: 
// Created: 五  4月  4 12:50:57 2008 (CST)
// Version: 
// Last-Updated: 五  4月  4 13:14:26 2008 (CST)
//           By: liuguangzhao
//     Update #: 1
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
// 4-Apr-2008    liuguangzhao  
//    Last-Updated: 五  4月  4 13:14:26 2008 (CST) #1 (liuguangzhao)
//    添加测试框架代码
// 
// 
// 
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// ///////////////////////////////////////////////////////////////
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

int main(int argc, char ** argv)
{
	QCoreApplication a(argc, argv);
    TestNullfxp mytest;
    QTest::qExec(&mytest);
    return 0;
}

// 
// testmain.cpp ends here

