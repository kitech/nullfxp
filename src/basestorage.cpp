// basestorage.cpp --- 
// 
// Filename: basestorage.cpp
// Description: 
// Author: liuguangzhao
// Maintainer: 
// Created: 五  4月  4 14:39:04 2008 (CST)
// Version: 
// Last-Updated: 五  4月  4 16:28:26 2008 (CST)
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

#include "basestorage.h"
BaseStorage::BaseStorage()
{
}

BaseStorage::~BaseStorage()
{
  if(this->opened)
    this->close();
}

bool BaseStorage::open()
{
#ifdef Q_OS_WIN
  QString config_path = QCoreApplication::applicationDirPath() + QString("/.nullfxp");
#elif Q_OS_MAC
  //TODO should where?
  QString config_path = QDir::homePath()+QString("/.nullfxp");
#else
  QString config_path = QDir::homePath()+QString("/.nullfxp");
#endif

  if(!QDir().exists(config_path))
    if(!QDir().mkdir(config_path)) return false;

  QFile *fp = new QFile(config_path + QString("/hosts.db"));
  fp->open(QIODevice::ReadWrite|QIODevice::Unbuffered);
  this->opened = true;
  this->changed = false;
  this->ioStream.setDevice(fp);
  this->ioStream>>this->hosts;
  return true;
}

bool BaseStorage::close()
{
  QFile * fp = (QFile*)this->ioStream.device();
  if(this->changed)
    this->save();
  if(!fp->isOpen())
    fp->close();
  this->opened = false;
  return true;
  delete fp;
}

bool BaseStorage::save()
{
  if(!this->changed) return true;
  QFile * fp = (QFile*)this->ioStream.device();
  fp->resize(0);
  this->ioStream<<this->hosts;

  return true;
}
bool BaseStorage::addHost(QMap<QString,QString> host)
{
  QMap<QString, QMap<QString,QString> >::iterator it;

  for(it = this->hosts.begin(); it != this->hosts.end(); it ++)
    {
      if((*it)["show_name"] == host["show_name"]) return false;
    }
  this->hosts[host["show_name"]] = host;
  this->changed = true;
  return true;
}
bool BaseStorage::removeHost(QString show_name)
{
  if(this->containsHost(show_name))
    {
      this->hosts.remove(show_name);
      this->changed = true;
      return true;
    }
  return false;
}

bool BaseStorage::updateHost(QMap<QString,QString> host)
{
  if(this->containsHost(host["show_name"]))
    this->hosts[host["show_name"]] = host;
  else
    this->hosts[host["show_name"]] = host;
  this->changed = true;
  return true;
}

bool BaseStorage::containsHost(QString show_name)
{
  QMap<QString, QMap<QString,QString> >::iterator it;

  for(it = this->hosts.begin(); it != this->hosts.end(); it ++)
    {
      if((*it)["show_name"] == show_name) return true;
    }
  return false;
}

QMap<QString, QMap<QString,QString> > & BaseStorage::getAllHost()
{
  return this->hosts;
}

QMap<QString,QString> BaseStorage::getHost(QString show_name)
{
  QMap<QString,QString> host;

  if(this->containsHost(show_name))
    return this->hosts[show_name];
  else
    return host;
}

// 
// basestorage.cpp ends here

