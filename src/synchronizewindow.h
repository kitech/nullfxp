/* synchronizewindow.h --- 
 * 
 * Filename: synchronizewindow.h
 * Description: 
 * Author: 刘光照<liuguangzhao@users.sf.net>
 * Maintainer: 
 * Copyright (C) 2007-2010 liuguangzhao <liuguangzhao@users.sf.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 五  8月  8 13:44:51 2008 (CST)
 * Version: 
 * Last-Updated: 日  8月 10 11:57:51 2008 (CST)
 *           By: 刘光照<liuguangzhao@users.sf.net>
 *     Update #: 1
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
#ifndef SYNCHRONIZEWINDOW_H
#define SYNCHRONIZEWINDOW_H

#include <QtGlobal>
#include <QtCore>
#include <QtGui>

#include "libssh2.h"
#include "libssh2_sftp.h"

#include "ui_synchronizewindow.h"

class SyncDifferModel;
class SynchronizeWindow;
class SyncTransferThread;

//////////////
//////////////
class SyncWalker : public QThread
{
    Q_OBJECT;
public:
    SyncWalker(QObject *parent = 0);
    ~SyncWalker();
    void run();

    QString diffDesciption(unsigned long flags);
    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > mMergedFiles;

    enum {FLAG_LOCAL_FILE=0x00100000,FLAG_REMOTE_FILE=0x00200000,
          FLAG_LOCAL_ONLY=0x00400000,FLAG_REMOTE_ONLY=0x00800000,
          FLAG_LOCAL_NEWER=0x01000000,FLAG_REMOTE_NEWER=0x02000000,
          FLAG_FILE_EQUAL=0x04000000,FLAG_FILE_DIFFERENT=0x08000000,
          FLAG_WANT_DOWNLOAD=0x10000000,FLAG_WANT_UPLOAD=0x20000000,
    };

signals:
    void found_row();

private:
    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > getRemoteFiles();
    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > getLocalFiles();
    bool checkLocalInfo();
    bool connectToRemoteHost();
    bool checkRemoteInfo();
    bool disconnectFromRemoteHost();
    LIBSSH2_SFTP_ATTRIBUTES * QFileInfoToLIBSSH2Attribute(QFileInfo &fi);
    QFileInfo LIBSSH2AttributeToQFileInfo(LIBSSH2_SFTP_ATTRIBUTES *attr);
    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > sortMerge(
        QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > &rfiles,
        QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > &lfiles);
    bool sameFileTime(unsigned long a, unsigned long b);
    bool dumpMergeResult(QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > &files);

private:
    SynchronizeWindow *parent;
    // default is CMP_BY_TIME_SIZE
    enum {CMP_BY_TIME_SIZE=0x0001, CMP_BY_CONTENT=0x0002};
    // file times have precision of 2 seconds due to FAT/FAT32 file systems
    static const unsigned int FILE_TIME_PRECISION = 2;

    LIBSSH2_SESSION *ssh2_sess ;
    LIBSSH2_SFTP *ssh2_sftp ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_attr;
    LIBSSH2_SFTP_HANDLE *hsftp ;

    QMap<QString, QString> remoteHost;
    QString remoteBasePath;
    QString localBasePath;

signals:
    void file_got(QString file_name, LIBSSH2_SFTP_ATTRIBUTES *attr);
    void status_msg(QString msg);
};

class SynchronizeWindow : public QWidget
{
    Q_OBJECT;
public:
    SynchronizeWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~SynchronizeWindow();

    void set_sync_param(QString local_dir, QString sess_name, QString remote_dir, bool recursive, int way);
    QString diffDesciption(unsigned long flags);

public slots:
    void slot_status_msg(QString msg);
    void slot_finished();

private slots:
    void start();
    void stop();
    void progress_timer_timeout();
    void showCtxMenu(const QPoint & pos);
    void showDiffFileInfo();
    void dlSelectedDiffFiles();
    void upSelectedDiffFiles();

private:
    void initCtxMenu();
    QModelIndexList getSelectedModelIndexes();
    void manualShowWhatsThis(QString what);

private:
    Ui::SynchronizeWindow  ui_win;
    QString local_dir;
    QString sess_name;
    QString remote_dir;
    bool recursive;
    int way;
    QTimer progress_timer;
    bool running;
    QMenu *ctxMenu;
    QAction * dlAction;
    QAction * upAction;

    SyncWalker *walker;
    SyncDifferModel *model;
    SyncTransferThread *transfer;

    enum {ST_LZERO = 0x00, ST_RZERO = 0x01, ST_LRSAME = 0x02, ST_LNEW = 0x04, ST_RNEW = 0x08 };
    enum {FT_DIR = 0x10000, FT_REG = 0x20000};
    QStringList dirs;
    QHash<QString, QHash<QString, int> > syncer;
    QVector<QPair<QString, int> >  synckeys;
    //如果此pair的第二个值为 -1 表示，这个值有子结点，否则就是没有,那么其值就为 syncer中的int项值。
    

    friend class SyncWalker;
    friend class SyncDifferModel;
protected:
    void closeEvent(QCloseEvent *evt);
};

#endif
