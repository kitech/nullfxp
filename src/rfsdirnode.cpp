
#include <QtCore>
#include <QtGui>

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
    return S_ISDIR(this->attrib.permissions);
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
    for(int i = 0 ; i < this->child_items.size(); i++) {
        if(child_items.at(i)->file_name == name) {
            return true;
        }
    }
    return false;
}
bool directory_tree_item::setMeet(QString name, bool meet)
{
    for(int i = 0 ; i < this->child_items.size(); i++) {
        if(child_items.at(i)->file_name == name) {
            child_items.at(i)->meet = meet;
            return true;
        }
    }    
    return false;
}
bool directory_tree_item::setDeleteFlag(QString name, bool del)
{
    for(int i = 0 ; i < this->child_items.size(); i++) {
        if(child_items.at(i)->file_name == name) {
            this->child_items.at(i)->delete_flag = del;
            return true;
        }
    }    
    return false;
}
directory_tree_item *directory_tree_item::childAt(int index)
{
    return this->child_items.at(index);
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

