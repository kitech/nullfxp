// rfsdirnode.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-08-09 11:50:49 +0000
// Last-Updated: 2009-08-23 13:36:37 +0000
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>

#include "utils.h"
#include "sshfileinfo.h"

#include "rfsdirnode.h"

////////////////////////directory_tree_item
directory_tree_item::~directory_tree_item()
{
    //qDebug()<<"tree delete now";
    int line = this->child_items.size();
    for (int i = line -1 ; i >=0 ; i --) {
        delete this->child_items[i];
    }
}

bool directory_tree_item::isDir()
{
    return S_ISDIR(this->attrib.permissions) || S_ISLNK(this->attrib.permissions);
}

bool directory_tree_item::isSymbolLink()
{
    return this->attrib.permissions & LIBSSH2_SFTP_S_IFLNK;
}

int directory_tree_item::childCount()
{
    return this->child_items.size();
    return 0;
}

directory_tree_item *directory_tree_item::parent()
{
    return this->parent_item;
}

bool directory_tree_item::hasChild(QString name)
{
    for (unsigned int i = 0 ; i < this->child_items.size(); i++) {
        if (child_items[i]->file_name == name) {
            return true;
        }
    }
    
    return false;
}

bool directory_tree_item::setDeleteFlag(QString name, bool del)
{
    for (unsigned int i = 0 ; i < this->child_items.size(); i++) {
        if (child_items[i]->file_name == name) {
            this->child_items[i]->delete_flag = del;
            return true;
        }
    } 
    
    return false;
}
directory_tree_item *directory_tree_item::childAt(int index)
{
    return this->child_items[index];
}
QString directory_tree_item::filePath()
{
    return this->strip_path;
}
QString directory_tree_item::fileName()
{
    return this->file_name;
}
QString directory_tree_item::fileMode()
{
    char file_date[PATH_MAX+1];
#ifndef _MSC_VER
    struct tm *ltime = localtime((time_t*)&this->attrib.atime);
    if (ltime != NULL) {
        if (time(NULL) - this->attrib.atime < (365*24*60*60) / 2)
            strftime(file_date, sizeof file_date, "%Y/%m/%d %H:%M:%S", ltime);
        else
            strftime(file_date, sizeof file_date, "%Y/%m/%d %H:%M:%S", ltime);
    }
#else
    _snprintf(file_date, sizeof(file_date) - 1, "0000/00/00 00:00:00");
#endif    
    return QString(file_date);
}
QString directory_tree_item::fileMDate()
{
    char file_date[PATH_MAX+1];
#ifndef _MSC_VER
    struct tm *ltime = localtime((time_t*)&this->attrib.mtime);
    if (ltime != NULL) {
        if (time(NULL) - this->attrib.mtime < (365*24*60*60) / 2)
            strftime(file_date, sizeof file_date, "%Y/%m/%d %H:%M:%S", ltime);
        else
            strftime(file_date, sizeof file_date, "%Y/%m/%d %H:%M:%S", ltime);
    }
#else
    _snprintf(file_date, sizeof(file_date) - 1, "0000/00/00 00:00:00");
#endif    
    return QString(file_date);
}
QString directory_tree_item::fileADate()
{
    return QString();
}

quint64 directory_tree_item::fileSize()
{
    return this->attrib.filesize;
}

QString directory_tree_item::strFileSize()
{
    return QString("%1").arg(this->attrib.filesize);
}

QString directory_tree_item::fileType()
{
    SSHFileInfo fi(this->attrib);
    return fi.stringMode();
}
