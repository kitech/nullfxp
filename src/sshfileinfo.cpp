// sshfileinfo.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-01-10 11:10:22 +0800
// Version: $Id$
// 

#include "utils.h"

#include "sshfileinfo.h"

// #include "libssh2_sftp.h" is included in sshfileinfo.h

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
    // return S_ISDIR(this->mAttr.permissions);
    return LIBSSH2_SFTP_S_ISDIR(this->mAttr.permissions);
}
bool SSHFileInfo::isFile () const
{
    return LIBSSH2_SFTP_S_ISREG(this->mAttr.permissions);
}
bool SSHFileInfo::isHidden () const
{
    return false;
}

bool SSHFileInfo::isSymLink () const
{
    return LIBSSH2_SFTP_S_ISLNK(this->mAttr.permissions);
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
    switch (mode & LIBSSH2_SFTP_S_IFMT)
    {
    case LIBSSH2_SFTP_S_IFDIR:			/* directory */
        *p++ = 'd';
        break;
    case LIBSSH2_SFTP_S_IFCHR:			/* character special */
        *p++ = 'c';
        break;
    case LIBSSH2_SFTP_S_IFBLK:			/* block special */
        *p++ = 'b';
        break;
    case LIBSSH2_SFTP_S_IFREG:			/* regular */
        *p++ = '-';
        break;
    case LIBSSH2_SFTP_S_IFLNK:			/* symbolic link */
        *p++ = 'l';
        break;
        //#ifdef S_IFSOCK
    case LIBSSH2_SFTP_S_IFSOCK:			/* socket */
        *p++ = 's';
        break;
        //#endif
        //#ifdef S_IFIFO
    case LIBSSH2_SFTP_S_IFIFO:			/* fifo */
        *p++ = 'p';
        break;
        //#endif
    default:			/* unknown */
        *p++ = '?';
        break;
    }
    /* usr */
    if ( mode & LIBSSH2_SFTP_S_IRUSR )
        *p++ = 'r';
    else
        *p++ = '-';
    if ( mode & LIBSSH2_SFTP_S_IWUSR )
        *p++ = 'w';
    else
        *p++ = '-';
    switch (mode & (LIBSSH2_SFTP_S_IXUSR | S_ISUID))
    {
    case 0:
        *p++ = '-';
        break;
    case LIBSSH2_SFTP_S_IXUSR:
        *p++ = 'x';
        break;
    case S_ISUID:
        *p++ = 'S';
        break;
    case LIBSSH2_SFTP_S_IXUSR | S_ISUID:
        *p++ = 's';
        break;
    }
    /* group */
    if ( mode & LIBSSH2_SFTP_S_IRGRP )
        *p++ = 'r';
    else
        *p++ = '-';
    if ( mode & LIBSSH2_SFTP_S_IWGRP )
        *p++ = 'w';
    else
        *p++ = '-';
    switch (mode & (LIBSSH2_SFTP_S_IXGRP | S_ISGID))
    {
    case 0:
        *p++ = '-';
        break;
    case LIBSSH2_SFTP_S_IXGRP:
        *p++ = 'x';
        break;
    case S_ISGID:
        *p++ = 'S';
        break;
    case LIBSSH2_SFTP_S_IXGRP | S_ISGID:
        *p++ = 's';
        break;
    }
    /* other */
    if ( mode & LIBSSH2_SFTP_S_IROTH )
        *p++ = 'r';
    else
        *p++ = '-';
    if ( mode & LIBSSH2_SFTP_S_IWOTH )
        *p++ = 'w';
    else
        *p++ = '-';
    switch (mode & (LIBSSH2_SFTP_S_IXOTH | S_ISVTX))
    {
    case 0:
        *p++ = '-';
        break;
    case LIBSSH2_SFTP_S_IXOTH:
        *p++ = 'x';
        break;
    case S_ISVTX:
        *p++ = 'T';
        break;
    case LIBSSH2_SFTP_S_IXOTH | S_ISVTX:
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

    if (this->mAttr.permissions & LIBSSH2_SFTP_S_IRUSR) {
        mode |= LIBSSH2_SFTP_S_IRUSR;
    }
    if (this->mAttr.permissions & LIBSSH2_SFTP_S_IWUSR) {
        mode |= LIBSSH2_SFTP_S_IWUSR;
    }
    if (this->mAttr.permissions & LIBSSH2_SFTP_S_IXUSR) {
        mode |= LIBSSH2_SFTP_S_IXUSR;
    }

    if (this->mAttr.permissions & LIBSSH2_SFTP_S_IRGRP) {
        mode |= LIBSSH2_SFTP_S_IRGRP;
    }
    if (this->mAttr.permissions & LIBSSH2_SFTP_S_IWGRP) {
        mode |= LIBSSH2_SFTP_S_IWGRP;
    }
    if (this->mAttr.permissions & LIBSSH2_SFTP_S_IXGRP) {
        mode |= LIBSSH2_SFTP_S_IXGRP;
    }

    if (this->mAttr.permissions & LIBSSH2_SFTP_S_IROTH) {
        mode |= LIBSSH2_SFTP_S_IROTH;
    }
    if (this->mAttr.permissions & LIBSSH2_SFTP_S_IWOTH) {
        mode |= LIBSSH2_SFTP_S_IWOTH;
    }
    if (this->mAttr.permissions & LIBSSH2_SFTP_S_IXOTH) {
        mode |= LIBSSH2_SFTP_S_IXOTH;
    }

    return mode;
}

QFile::Permissions SSHFileInfo::qMode() const
{
    QFile::Permissions perm;

    if (this->mAttr.permissions & LIBSSH2_SFTP_S_IRUSR) {
        perm |= QFile::ReadUser;
        perm |= QFile::ReadOwner;
    }
    if (this->mAttr.permissions & LIBSSH2_SFTP_S_IWUSR) {
        perm |= QFile::WriteUser;
        perm |= QFile::WriteOwner;
    }
    if (this->mAttr.permissions & LIBSSH2_SFTP_S_IXUSR) {
        perm |= QFile::ExeUser;
        perm |= QFile::ExeOwner;
    }

    if (this->mAttr.permissions & LIBSSH2_SFTP_S_IRGRP) {
        perm |= QFile::ReadGroup;
    }
    if (this->mAttr.permissions & LIBSSH2_SFTP_S_IWGRP) {
        perm |= QFile::WriteGroup;
    }
    if (this->mAttr.permissions & LIBSSH2_SFTP_S_IXGRP) {
        perm |= QFile::ExeGroup;
    }

    if (this->mAttr.permissions & LIBSSH2_SFTP_S_IROTH) {
        perm |= QFile::ReadOther;
    }
    if (this->mAttr.permissions & LIBSSH2_SFTP_S_IWOTH) {
        perm |= QFile::WriteOther;
    }
    if (this->mAttr.permissions & LIBSSH2_SFTP_S_IXOTH) {
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
        attr.permissions |= LIBSSH2_SFTP_S_IFREG;
    } else if (fi.isSymLink()) {
        attr.permissions |= LIBSSH2_SFTP_S_IFLNK;
    } else if (fi.isDir()) {
        attr.permissions |= LIBSSH2_SFTP_S_IFDIR;
    } else {
        qDebug()<<"unknown file type:"<<fi.fileName();
    }

    if (perm & QFile::ReadUser) {
        attr.permissions |= LIBSSH2_SFTP_S_IRUSR;
    }
    if (perm & QFile::WriteUser) {
        attr.permissions |= LIBSSH2_SFTP_S_IWUSR;
    }
    if (perm & QFile::ExeUser) {
        attr.permissions |= LIBSSH2_SFTP_S_IXUSR;
    }

    if (perm & QFile::ReadGroup) {
        attr.permissions |= LIBSSH2_SFTP_S_IRGRP;
    }
    if (perm & QFile::WriteGroup) {
        attr.permissions |= LIBSSH2_SFTP_S_IWGRP;
    }
    if (perm & QFile::ExeGroup) {
        attr.permissions |= LIBSSH2_SFTP_S_IXGRP;
    }

    if (perm & QFile::ReadOther) {
        attr.permissions |= LIBSSH2_SFTP_S_IROTH;
    }
    if (perm & QFile::WriteOther) {
        attr.permissions |= LIBSSH2_SFTP_S_IWOTH;
    }
    if (perm & QFile::ExeOther) {
        attr.permissions |= LIBSSH2_SFTP_S_IXOTH;
    }

    SSHFileInfo sshFi(attr);
    return sshFi;
}
