// rfsdirnode.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-08-09 11:50:49 +0000
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>

#include "utils.h"
#include "sshfileinfo.h"

#include "rfsdirnode.h"

////////////////////////NetDirNode
NetDirNode::~NetDirNode()
{
    //qDebug()<<"tree delete now";
    NetDirNode *node = NULL;
    int line = this->childNodes.count();
    for (int i = line -1 ; i >=0 ; i --) {
        Q_ASSERT(this->childNodes.contains(i));
        node = this->childNodes.value(i);
        delete node;
    }
    this->childNodes.clear();
}

bool NetDirNode::isDir()
{
    return LIBSSH2_SFTP_S_ISDIR(this->attrib.permissions);
    return LIBSSH2_SFTP_S_ISDIR(this->attrib.permissions) 
        || LIBSSH2_SFTP_S_ISLNK(this->attrib.permissions);
}

bool NetDirNode::isSymLink()
{
    return LIBSSH2_SFTP_S_ISLNK(this->attrib.permissions);
}
bool NetDirNode::isSymLinkToDir()
{
    return this->linkToDir;
    return false;
}
int NetDirNode::childCount()
{
    return this->childNodes.count();
    return 0;
}

NetDirNode *NetDirNode::parent()
{
    return this->pNode;
}

bool NetDirNode::hasChild(QString name)
{
    // for (unsigned int i = 0 ; i < this->childItems.count(); i++) {
    //     if (this->childItems.at(i)->file_name == name) {
    //         return true;
    //     }
    // }
    
    return false;
}

NetDirNode *NetDirNode::findChindByName(QString name)
{
    NetDirNode *child = NULL;
    // for (unsigned int i = 0 ; i < this->childItems.count(); i++) {
    //     if (this->childItems.at(i)->file_name == name) {
    //         child = childItems.at(i);
    //         break;
    //     }
    // } 
    return child;
}
bool NetDirNode::matchChecksum(QDateTime mdate, quint64 fsize)
{
    if (this->attrib.mtime == mdate.toTime_t()
        && fsize == this->attrib.filesize) {
        return true;
    }
    return false;
}

bool NetDirNode::matchChecksum(LIBSSH2_SFTP_ATTRIBUTES *attr)
{
    Q_ASSERT(attr != NULL);
    if (this->attrib.mtime == attr->mtime
        && this->attrib.filesize == attr->filesize) {
        return true;
    }
    return false;
}

bool NetDirNode::setDeleteFlag(QString name, bool del)
{
    // for (unsigned int i = 0 ; i < this->childItems.count(); i++) {
    //     if (this->childItems.at(i)->file_name == name) {
    //         this->childItems.at(i)->delete_flag = del;
    //         return true;
    //     }
    // } 
    
    return false;
}
bool NetDirNode::setDeleteFlag(bool deleted)
{
    if (this->deleted) {
        q_debug()<<"flag is already deleted state";
    }
    this->deleted = deleted;
    return true;
}

NetDirNode *NetDirNode::childAt(int index)
{
    if (this->childNodes.contains(index)) {
        return this->childNodes.value(index);
    } else {
        q_debug()<<"can find child at:"<<index;
    }
    return NULL;
}
QString NetDirNode::filePath()
{
    return this->fullPath;
}
QString NetDirNode::fileName()
{
    return this->_fileName;
}
QString NetDirNode::fileMode()
{
    char mem[32] = {0};
    strmode(this->attrib.permissions, mem);
    return QString(mem);
}
QString NetDirNode::fileMDate()
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
QString NetDirNode::fileADate()
{
    return QString();
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

quint64 NetDirNode::fileSize()
{
    return this->attrib.filesize;
}

QString NetDirNode::strFileSize()
{
    return QString("%1").arg(this->attrib.filesize);
}

QString NetDirNode::fileType()
{
    SSHFileInfo fi(this->attrib);
    return fi.stringMode();
}

bool NetDirNode::copyFrom(NetDirNode *node)
{
    this->fullPath = node->fullPath;
    this->_fileName = node->_fileName;
    this->linkToDir = node->linkToDir;
    
    // this->onRow = node->onRow;
    // this->pNode = node->pNode;
    
    memcpy(&this->attrib, &node->attrib, sizeof(this->attrib));

    return true;
}

void NetDirNode::dumpTreeRecursive()
{
    NetDirNode *pnode = this->pNode;
    if (pnode != NULL) {
        pnode->dumpTreeRecursive();
    }
    int depth = this->fullPath.split('/').count();
    QString prepad;
    prepad.fill(' ', depth);
    qDebug()<<prepad<<this->_fileName;
}



