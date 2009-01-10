/* sshfileinfo.h --- 
 * 
 * Filename: sshfileinfo.h
 * Description: 
 * Author: 刘光照<liuguangzhao@users.sf.net>
 * Maintainer: 
 * Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 六  1月 10 11:09:52 2009 (CST)
 * Version: 
 * Last-Updated: 
 *           By: 
 *     Update #: 0
 * URL: 
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change log:
 * 
 * 
 */

#ifndef SSH_FILE_INFO_H
#define SSH_FILE_INFO_H

#include <QtCore>

#include "libssh2.h"
#include "libssh2_sftp.h"

class SSHFileInfo
{
public:
    SSHFileInfo(LIBSSH2_SFTP_ATTRIBUTES attr);
    SSHFileInfo(LIBSSH2_SFTP_ATTRIBUTES *pattr);
    ~SSHFileInfo();

    uint groupId () const;
    bool isDir () const;
    bool isFile () const;
    bool isHidden () const;
    bool isSymLink () const;
    QDateTime lastModified () const;
    QDateTime lastRead () const;
    uint ownerId () const;
    qint64 size () const;

private:
    LIBSSH2_SFTP_ATTRIBUTES mAttr;
};

#endif

