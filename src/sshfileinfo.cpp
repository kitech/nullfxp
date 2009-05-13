// sshfileinfo.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-01-10 11:10:22 +0800
// Last-Updated: 2009-05-13 23:06:20 +0800
// Version: $Id$
// 

#include "utils.h"

#include "sshfileinfo.h"

SSHFileInfo::SSHFileInfo(LIBSSH2_SFTP_ATTRIBUTES attr)
{
    new(this)SSHFileInfo(&attr); // hehe, trick

    memcpy(&mAttr, &attr, sizeof(LIBSSH2_SFTP_ATTRIBUTES));
}
SSHFileInfo::SSHFileInfo(LIBSSH2_SFTP_ATTRIBUTES *pattr)
{
    if (pattr == NULL) {
        // qDebug()<<"Null attr";
        memset(&mAttr, 0, sizeof(mAttr));
    } else {
        memcpy(&mAttr, pattr, sizeof(LIBSSH2_SFTP_ATTRIBUTES));
    }
}
SSHFileInfo::~SSHFileInfo()
{
}

uint SSHFileInfo::groupId () const
{
    return this->mAttr.gid;
}
bool SSHFileInfo::isDir () const
{
    return S_ISDIR(this->mAttr.permissions);
}
bool SSHFileInfo::isFile () const
{
    return S_ISREG(this->mAttr.permissions);
}
bool SSHFileInfo::isHidden () const
{
    return false;
}

bool SSHFileInfo::isSymLink () const
{
    return S_ISLNK(this->mAttr.permissions);
}
QDateTime SSHFileInfo::lastModified () const
{
    return QDateTime::fromTime_t(this->mAttr.mtime);
    return QDateTime::currentDateTime();
}
QDateTime SSHFileInfo::lastRead () const
{
    return QDateTime::fromTime_t(this->mAttr.atime);
    return QDateTime::currentDateTime();
}
uint SSHFileInfo::ownerId () const
{
    return this->mAttr.uid;
}
qint64 SSHFileInfo::size () const
{
    return this->mAttr.filesize;
}

QString SSHFileInfo::stringMode() const
{
    QString mode;
    
    return mode;
}

SSHFileInfo SSHFileInfo::fromQFileInfo(QFileInfo &fi)
{
    LIBSSH2_SFTP_ATTRIBUTES attr;
    memset(&attr, 0, sizeof(attr));

    attr.filesize = fi.size();

    SSHFileInfo sshFi(attr);
    return sshFi;
}
