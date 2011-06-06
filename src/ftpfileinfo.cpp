// ftpfileinfo.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-06-06 03:17:23 +0800
// Version: $Id$
// 


#include "ftpfileinfo.h"

FTPFileInfo::FTPFileInfo()
    : QUrlInfo()
{
}
FTPFileInfo::FTPFileInfo(const QUrlInfo &info)
    : QUrlInfo(info)
{
}

FTPFileInfo::FTPFileInfo ( const QString & name, int permissions, const QString & owner, const QString & group, qint64 size, const QDateTime & lastModified, const QDateTime & lastRead, bool isDir, bool isFile, bool isSymLink, bool isWritable, bool isReadable, bool isExecutable )
    : QUrlInfo(name, permissions, owner, group, size, lastModified, lastRead, isDir, isFile, isSymLink, isWritable, isReadable, isExecutable)
{
}

FTPFileInfo::FTPFileInfo ( const QUrl & url, int permissions, const QString & owner, const QString & group, qint64 size, const QDateTime & lastModified, const QDateTime & lastRead, bool isDir, bool isFile, bool isSymLink, bool isWritable, bool isReadable, bool isExecutable )
    : QUrlInfo(url, permissions, owner, group, size, lastModified, lastRead, isDir, isFile, isSymLink, isWritable, isReadable, isExecutable)
{
}

FTPFileInfo::~FTPFileInfo()
{
}

QString FTPFileInfo::symlinkTarget()
{
    return this->m_symlink_target;
}
void FTPFileInfo::setSymlinkTarget(const QString &target)
{
    this->m_symlink_target = target;
}

static void test_FTPFileInfo()
{
    FTPFileInfo fi, fi2;
    fi = fi2;

    fi.setSymlinkTarget("aaaaaaaaaaaaaaaaa");
    fi2.setSymlinkTarget(fi.symlinkTarget());
}

