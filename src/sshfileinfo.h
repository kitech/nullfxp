/* sshfileinfo.h --- 
 * 
 * Author: liuguangzhao
 * Copyright (C) 2007-2010 liuguangzhao@users.sf.net
 * URL: http://www.qtchina.net http://nullget.sourceforge.net
 * Created: 2009-01-10 11:09:52 +0800
 * Version: $Id$
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
    virtual ~SSHFileInfo();

    uint groupId () const;
    bool isDir () const;
    bool isFile () const;
    bool isHidden () const;
    bool isSymLink () const;
    QDateTime lastModified () const;
    QDateTime lastRead () const;
    uint ownerId () const;
    qint64 size () const;
    QString stringMode() const;
    uint mode() const;
    QFile::Permissions qMode() const;

    static SSHFileInfo fromQFileInfo(QFileInfo &fi);
    
private:
    LIBSSH2_SFTP_ATTRIBUTES mAttr;
};

#endif

