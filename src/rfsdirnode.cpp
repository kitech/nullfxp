// rfsdirnode.cpp --- 
// 
// Filename: rfsdirnode.cpp
// Description: 
// Author: 刘光照<liuguangzhao@users.sf.net>
// Maintainer: 
// Copyright (C) 2007-2010 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 六  8月  9 11:50:49 2008 (CST)
// Version: 
// Last-Updated: 六  8月  9 11:51:05 2008 (CST)
//           By: 刘光照<liuguangzhao@users.sf.net>
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

#include <QtCore>
#include <QtGui>

#include "utils.h"

#include "rfsdirnode.h"

////////////////////////directory_tree_item
directory_tree_item::~directory_tree_item()
{
    //qDebug()<<"tree delete now";
    int line = this->child_items.size();
    for(int i = line -1 ; i >=0 ; i --)
    {
        delete this->child_items[i];
    }
}

bool directory_tree_item::isDir()
{
    return S_ISDIR(this->attrib.permissions) || S_ISLNK(this->attrib.permissions);
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
    /*
    std::map<int, directory_tree_item*>::iterator it;
    int i = 0;
    for(it = this->child_items.begin(); it != this->child_items.end(); it++)
    {
        if(it->second->file_name == name) {
            return true;
        }
    }
    */
    
    for(int i = 0 ; i < this->child_items.size(); i++) {
        if(child_items[i]->file_name == name) {
            return true;
        }
    }
    
    return false;
}

bool directory_tree_item::setDeleteFlag(QString name, bool del)
{
    /*
    std::map<int, directory_tree_item*>::iterator it;
    int i = 0;
    for(it = this->child_items.begin(); it != this->child_items.end(); it++)
    {
        if(it->second->file_name == name) {
            it->second->delete_flag = del;
            return true;
        }
    }
    */
    
    for(int i = 0 ; i < this->child_items.size(); i++) {
        if(child_items[i]->file_name == name) {
            this->child_items[i]->delete_flag = del;
            return true;
        }
    } 
    
    return false;
}
directory_tree_item *directory_tree_item::childAt(int index)
{
    /*
    std::map<int, directory_tree_item*>::iterator it;
    int i = 0;
    for(it = this->child_items.begin(); it != this->child_items.end(); it++)
    {
        if(i++ == index) {
            return it->second;
        }
    }
    
    return NULL;
    */
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
    return QString();
}
QString directory_tree_item::fileMDate()
{
    return QString();
}
QString directory_tree_item::fileADate()
{
    return QString();
}

