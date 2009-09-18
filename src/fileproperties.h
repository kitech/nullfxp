/* fileproperties.h --- 
 * 
 * Author: liuguangzhao
 * Copyright (C) 2007-2010 liuguangzhao@users.sf.net
 * URL: http://www.qtchina.net http://nullget.sourceforge.net
 * Created: 2008-07-19 14:43:21 +0800
 * Version: $Id$
 */

#ifndef FILEPROPERTIES_H
#define FILEPROPERTIES_H

#include <QtCore>
#include <QtGui>
#include <QDialog>

#include "libssh2.h"
#include "libssh2_sftp.h"

#include "ui_fileproperties.h"

class Connection;

class FilePropertiesRetriveThread : public QThread
{
    Q_OBJECT;
public:
    FilePropertiesRetriveThread(Connection *conn, LIBSSH2_SFTP *ssh2_sftp, QString file_path, QObject *parent = 0);
    virtual ~FilePropertiesRetriveThread();
    virtual void run();
    virtual void run_sftp();
    virtual void run_ftp();

signals:
    void file_attr_abtained(QString file_name, void *attr);
private:
    Connection *conn;
    LIBSSH2_SFTP *ssh2_sftp;
    QString file_path;
};

/**
 * @author liuguangzhao <gzl@localhost>
 */
class FileProperties : public  QDialog
{
    Q_OBJECT;
public:
    FileProperties(QWidget *parent = 0);
    virtual ~FileProperties();
    void set_ssh2_sftp(void *ssh2_sftp);
    void setConnection(Connection *conn);
    
    void set_file_info_model_list(QModelIndexList &mil);

public slots:
    void slot_prop_thread_finished();
    void slot_file_attr_abtained(QString file_name, void *attr);
        
private:
    void update_perm_table( QString file_perm);
    QString type(QString file_name);
    QIcon fileIcon(QString file_name);

    Ui::FileProperties ui_file_prop_dialog;
    LIBSSH2_SFTP *ssh2_sftp;
    Connection *conn;
};

class LocalFileProperties: public QDialog
{
    Q_OBJECT;
public:
    LocalFileProperties(QWidget *parent = 0);
    ~LocalFileProperties();
    void set_file_info_model_list(QString file_name);

public slots:

private:
    void update_perm_table( QString file_name);
    QString type(QString file_name);
    QString digit_mode(int mode);
    QIcon fileIcon(QString file_name);

    Ui::FileProperties ui_file_prop_dialog;
    QString file_name;
};

#endif
