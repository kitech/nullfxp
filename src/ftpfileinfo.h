// ftpfileinfo.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-06-06 03:16:58 +0800
// Version: $Id$
// 

#ifndef _FTPFILEINFO_H_
#define _FTPFILEINFO_H_

#include <QUrlInfo>

class FTPFileInfo : public QUrlInfo
{
public:
    FTPFileInfo();
    FTPFileInfo(const QUrlInfo &info);
    FTPFileInfo ( const QString & name, int permissions, const QString & owner, const QString & group, qint64 size, const QDateTime & lastModified, const QDateTime & lastRead, bool isDir, bool isFile, bool isSymLink, bool isWritable, bool isReadable, bool isExecutable );
	FTPFileInfo ( const QUrl & url, int permissions, const QString & owner, const QString & group, qint64 size, const QDateTime & lastModified, const QDateTime & lastRead, bool isDir, bool isFile, bool isSymLink, bool isWritable, bool isReadable, bool isExecutable );

    virtual ~FTPFileInfo();

    QString symlinkTarget();
    void setSymlinkTarget(const QString &target);
private:
    QString m_symlink_target;
};

#endif /* _FTPFILEINFO_H_ */
