// sshfileinfo.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-01-10 11:10:22 +0800
// Last-Updated: 2009-05-30 13:39:04 +0000
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
    QString strMode;
    uint mode = this->mAttr.permissions;
    char buf[16] = {0};
    char *p = buf;

    /* print type */
    switch (mode & S_IFMT)
    {
    case S_IFDIR:			/* directory */
        *p++ = 'd';
        break;
    case S_IFCHR:			/* character special */
        *p++ = 'c';
        break;
    case S_IFBLK:			/* block special */
        *p++ = 'b';
        break;
    case S_IFREG:			/* regular */
        *p++ = '-';
        break;
    case S_IFLNK:			/* symbolic link */
        *p++ = 'l';
        break;
        //#ifdef S_IFSOCK
    case S_IFSOCK:			/* socket */
        *p++ = 's';
        break;
        //#endif
        //#ifdef S_IFIFO
    case S_IFIFO:			/* fifo */
        *p++ = 'p';
        break;
        //#endif
    default:			/* unknown */
        *p++ = '?';
        break;
    }
    /* usr */
    if ( mode & S_IRUSR )
        *p++ = 'r';
    else
        *p++ = '-';
    if ( mode & S_IWUSR )
        *p++ = 'w';
    else
        *p++ = '-';
    switch (mode & (S_IXUSR | S_ISUID))
    {
    case 0:
        *p++ = '-';
        break;
    case S_IXUSR:
        *p++ = 'x';
        break;
    case S_ISUID:
        *p++ = 'S';
        break;
    case S_IXUSR | S_ISUID:
        *p++ = 's';
        break;
    }
    /* group */
    if ( mode & S_IRGRP )
        *p++ = 'r';
    else
        *p++ = '-';
    if ( mode & S_IWGRP )
        *p++ = 'w';
    else
        *p++ = '-';
    switch (mode & (S_IXGRP | S_ISGID))
    {
    case 0:
        *p++ = '-';
        break;
    case S_IXGRP:
        *p++ = 'x';
        break;
    case S_ISGID:
        *p++ = 'S';
        break;
    case S_IXGRP | S_ISGID:
        *p++ = 's';
        break;
    }
    /* other */
    if ( mode & S_IROTH )
        *p++ = 'r';
    else
        *p++ = '-';
    if ( mode & S_IWOTH )
        *p++ = 'w';
    else
        *p++ = '-';
    switch (mode & (S_IXOTH | S_ISVTX))
    {
    case 0:
        *p++ = '-';
        break;
    case S_IXOTH:
        *p++ = 'x';
        break;
    case S_ISVTX:
        *p++ = 'T';
        break;
    case S_IXOTH | S_ISVTX:
        *p++ = 't';
        break;
    }
    *p++ = ' ';		/* will be a '+' if ACL's implemented */
    *p = '\0';
    
    strMode = QByteArray(buf);
    return strMode;
}

uint SSHFileInfo::mode() const
{
    uint mode = 0;

    if (this->mAttr.permissions & S_IRUSR) {
        mode |= S_IRUSR;
    }
    if (this->mAttr.permissions & S_IWUSR) {
        mode |= S_IWUSR;
    }
    if (this->mAttr.permissions & S_IXUSR) {
        mode |= S_IXUSR;
    }

    if (this->mAttr.permissions & S_IRGRP) {
        mode |= S_IRGRP;
    }
    if (this->mAttr.permissions & S_IWGRP) {
        mode |= S_IWGRP;
    }
    if (this->mAttr.permissions & S_IXGRP) {
        mode |= S_IXGRP;
    }

    if (this->mAttr.permissions & S_IROTH) {
        mode |= S_IROTH;
    }
    if (this->mAttr.permissions & S_IWOTH) {
        mode |= S_IWOTH;
    }
    if (this->mAttr.permissions & S_IXOTH) {
        mode |= S_IXOTH;
    }

    return mode;
}

QFile::Permissions SSHFileInfo::qMode() const
{
    QFile::Permissions perm;

    if (this->mAttr.permissions & S_IRUSR) {
        perm |= QFile::ReadUser;
        perm |= QFile::ReadOwner;
    }
    if (this->mAttr.permissions & S_IWUSR) {
        perm |= QFile::WriteUser;
        perm |= QFile::WriteOwner;
    }
    if (this->mAttr.permissions & S_IXUSR) {
        perm |= QFile::ExeUser;
        perm |= QFile::ExeOwner;
    }

    if (this->mAttr.permissions & S_IRGRP) {
        perm |= QFile::ReadGroup;
    }
    if (this->mAttr.permissions & S_IWGRP) {
        perm |= QFile::WriteGroup;
    }
    if (this->mAttr.permissions & S_IXGRP) {
        perm |= QFile::ExeGroup;
    }

    if (this->mAttr.permissions & S_IROTH) {
        perm |= QFile::ReadOther;
    }
    if (this->mAttr.permissions & S_IWOTH) {
        perm |= QFile::WriteOther;
    }
    if (this->mAttr.permissions & S_IXOTH) {
        perm |= QFile::ExeOther;
    }

    return perm;
}

SSHFileInfo SSHFileInfo::fromQFileInfo(QFileInfo &fi)
{
    QFile::Permissions perm;
    LIBSSH2_SFTP_ATTRIBUTES attr;

    memset(&attr, 0, sizeof(attr));
    attr.filesize = fi.size();

    perm = fi.permissions();

    attr.atime = fi.lastRead().toTime_t();
    attr.mtime = fi.lastModified().toTime_t();

    if (fi.isFile() && !fi.isSymLink()) {
        attr.permissions |= S_IFREG;
    }
    if (fi.isDir()) {
        attr.permissions |= S_IFDIR;
    }
    if (fi.isSymLink()) {
        attr.permissions |= S_IFLNK;
    }

    if (perm & QFile::ReadUser) {
        attr.permissions |= S_IRUSR;
    }
    if (perm & QFile::QFile::WriteUser) {
        attr.permissions |= S_IWUSR;
    }
    if (perm & QFile::ExeUser) {
        attr.permissions |= S_IXUSR;
    }

    if (perm & QFile::ReadGroup) {
        attr.permissions |= S_IRGRP;
    }
    if (perm & QFile::WriteGroup) {
        attr.permissions |= S_IWGRP;
    }
    if (perm & QFile::ExeGroup) {
        attr.permissions |= S_IXGRP;
    }

    if (perm & QFile::ReadOther) {
        attr.permissions |= S_IROTH;
    }
    if (perm & QFile::WriteOther) {
        attr.permissions |= S_IWOTH;
    }
    if (perm & QFile::ExeOther) {
        attr.permissions |= S_IXOTH;
    }

    SSHFileInfo sshFi(attr);
    return sshFi;
}
