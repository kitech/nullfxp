// synchronizewindow.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-08-08 13:44:42 +0800
// Version: $Id$
// 

#include "utils.h"
#include "globaloption.h"
#include "sshconnection.h"
#include "basestorage.h"
#include "sshfileinfo.h"

#include "syncdiffermodel.h"
#include "synctransferthread.h"
#include "synchronizewindow.h"
#include "ui_synchronizewindow.h"

SyncWalker::SyncWalker(QObject *parent)
    :QThread(0)
{
    this->parent = static_cast<SynchronizeWindow*>(parent);
    ssh2_sess = NULL;
    ssh2_sftp = NULL;
    hsftp = NULL;
}
SyncWalker::~SyncWalker()
{
    q_debug()<<"destructured";
}

bool SyncWalker::checkLocalInfo()
{
    this->localBasePath = this->parent->local_dir;
    return true;
    return false;
}

bool SyncWalker::connectToRemoteHost()
{
    this->remoteBasePath = this->parent->remote_dir;

    QMap<QString, QString> host;
    BaseStorage *storage = BaseStorage::instance();

    this->remoteHost = host = storage->getHost(this->parent->sess_name);
    q_debug()<<host;
    
    // RemoteHostConnectThread *conn 
    //     = new RemoteHostConnectThread(host["user_name"], host["password"], host["host_name"],
    //                                   host["port"].toShort(), host["pubkey"]);
    // conn->run();
    // ssh2_sess = (LIBSSH2_SESSION*)conn->get_ssh2_sess();
    // delete conn; conn = NULL;
    SSHConnection *conn = new SSHConnection();
    conn->setHostInfo(host);
    conn->connect();
    ssh2_sess = conn->sess;
    Q_ASSERT(ssh2_sess != NULL);

    ssh2_sftp = libssh2_sftp_init(ssh2_sess);
    
    if (ssh2_sftp != NULL) {
        Q_ASSERT(ssh2_sftp != NULL);
        q_debug()<<"connect ok";
        return true;
    }
    
    return false;
}
bool SyncWalker::checkRemoteInfo()
{
    Q_ASSERT(ssh2_sess != NULL);
    Q_ASSERT(ssh2_sftp != NULL);

    LIBSSH2_SFTP_ATTRIBUTES attr;
    int rc = 0;
    unsigned long sftp_error = 8888;
    char *sess_errmsg = NULL;
    int  errmsg_len = 0;

    rc = libssh2_sftp_stat(ssh2_sftp, this->remoteBasePath.toAscii().data(), &attr);
    if (rc != 0) {
        sftp_error = libssh2_sftp_last_error(ssh2_sftp);
        rc = libssh2_session_last_error(ssh2_sess, &sess_errmsg, &errmsg_len, 0);
        q_debug()<<"STAT ERROR: "<<rc<<QString(sess_errmsg);
        return false;
    }
    // is dir
    // what link which link to a dir?
    if (! S_ISDIR(attr.permissions)) {
        q_debug()<<"not directory";
        return false;
    }

    // TODO
    // is readable    

    // is writable
    
    q_debug()<<"check info ok";
    return true;
}
bool SyncWalker::disconnectFromRemoteHost()
{
    if (ssh2_sftp != NULL) {
        libssh2_sftp_shutdown(ssh2_sftp);
    }
    if (ssh2_sess != NULL) {
        libssh2_session_free(ssh2_sess);
    }

    return false;
}

QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > 
SyncWalker::getRemoteFiles()
{
    q_debug()<<"";

    // files is relative path just after this->remoteBasePath
    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > files;
    LIBSSH2_SFTP_ATTRIBUTES *attr ;
    LIBSSH2_SFTP_HANDLE *hsftp = NULL;

    int rc = 0;
    char fnbuf[5120] = {0};
    QString currDir;
    QString currFile;
    QString nopFile;
    QStack<QString> dirStack;
    
    dirStack.push(this->remoteBasePath);

    while (!dirStack.empty()) {
        currDir = dirStack.pop(); // currDir is absolute dir path
        hsftp = libssh2_sftp_opendir(ssh2_sftp, gOpt->remote_codec->fromUnicode(currDir).data());
        if (hsftp == NULL) {
            q_debug()<<"opendir error:"<<currDir;
            continue;
        }
        while (true) {
            memset(fnbuf, 0, sizeof(fnbuf));
            rc = libssh2_sftp_readdir(hsftp, fnbuf, sizeof(fnbuf)-1, &ssh2_attr);
            if (rc == -1) {
                q_debug()<<"readdir error:"<<libssh2_sftp_last_error(ssh2_sftp)
                         <<", "<<currDir;
                break;
            }
            // no entry left
            if (rc == 0) {
                q_debug()<<"read 0 len file, readdir end.";
                break;
            }
            if ((rc == 1 && fnbuf[0] == '.') || (rc == 2 && fnbuf[0] == '.' && fnbuf[1] == '.')) {
                continue;
            }
            fnbuf[rc] = '\0';
            currFile = gOpt->remote_codec->toUnicode(QByteArray(fnbuf));
            if (S_ISDIR(ssh2_attr.permissions)) {
                dirStack.push(currDir + "/" + currFile);
            }
            attr = (LIBSSH2_SFTP_ATTRIBUTES*)malloc(sizeof(LIBSSH2_SFTP_ATTRIBUTES));
            attr->flags |= FLAG_REMOTE_FILE;
            memcpy(attr, &ssh2_attr, sizeof(LIBSSH2_SFTP_ATTRIBUTES));
            nopFile = currDir + "/" + currFile;
            nopFile = nopFile.right(nopFile.length() - this->remoteBasePath.length() - 1);
            files.append(QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*>(nopFile, attr));
        }
        libssh2_sftp_closedir(hsftp);
        hsftp = NULL;
    }
    q_debug()<<"return ";
    return files;
}

LIBSSH2_SFTP_ATTRIBUTES *SyncWalker::QFileInfoToLIBSSH2Attribute(QFileInfo &fi)
{
    LIBSSH2_SFTP_ATTRIBUTES *attr = NULL ;

    attr = (LIBSSH2_SFTP_ATTRIBUTES*)malloc(sizeof(LIBSSH2_SFTP_ATTRIBUTES));
    memset(attr, 0, sizeof(LIBSSH2_SFTP_ATTRIBUTES));

    attr->filesize = fi.size();
    attr->atime = fi.lastRead().toTime_t();
    attr->mtime = fi.lastModified().toTime_t();
    attr->permissions = fi.permissions();
    attr->uid = fi.ownerId();
    attr->gid = fi.groupId();

    attr->flags = LIBSSH2_SFTP_ATTR_SIZE | LIBSSH2_SFTP_ATTR_UIDGID
        | LIBSSH2_SFTP_ATTR_PERMISSIONS | LIBSSH2_SFTP_ATTR_ACMODTIME ;

    return attr ;
}

QFileInfo SyncWalker::LIBSSH2AttributeToQFileInfo(LIBSSH2_SFTP_ATTRIBUTES *attr)
{
    QFileInfo fi;
    Q_UNUSED(attr);
    Q_ASSERT(1 == 2);
    return fi;
}

QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > 
SyncWalker::getLocalFiles()
{
    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > files;
    LIBSSH2_SFTP_ATTRIBUTES *attr ;
    QString currDir;
    QString currFile;
    QString nopFile;
    QStack<QString> dirStack;
    
    dirStack.push(this->localBasePath);

    while (!dirStack.empty()) {
        currDir = dirStack.pop();
        QDir dh(currDir);
        QStringList fileList = dh.entryList();
        for(int i = 0 ; i < fileList.count(); i++) {
            if(fileList.at(i) == "." || fileList.at(i) == "..") {
                continue;
            }
            currFile = currDir + "/" + fileList.at(i);
            QFileInfo fi(currFile);
            if (fi.isDir()) {
                dirStack.push(currFile);
            }
            attr = this->QFileInfoToLIBSSH2Attribute(fi);
            Q_ASSERT(attr != NULL);
            attr->flags |= FLAG_LOCAL_FILE;
            nopFile = currFile.right(currFile.length() - this->localBasePath.length() - 1);
            files.append(QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*>(nopFile, attr));
        }
    }
    q_debug()<<"done";
    return files;
}
bool SyncWalker::sameFileTime(unsigned long a, unsigned long b)
{
    if (a < b)
        return b - a <= FILE_TIME_PRECISION;
    else
        return a - b <= FILE_TIME_PRECISION;    
}

QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > 
SyncWalker::sortMerge(
          QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > &rfiles,
          QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > &lfiles)
{
    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > files;
    QString rfile;
    QString lfile;
    LIBSSH2_SFTP_ATTRIBUTES *rattr;
    LIBSSH2_SFTP_ATTRIBUTES *lattr;
    int rcnt = rfiles.count();
    int lcnt = lfiles.count();
    int  found;
    int upCount = 0;
    int downloadCount = 0;
    int newerCount = 0;
    int olderCount = 0;
    
    for (int i = 0 ; i < rcnt; ++i) {
        rfile = rfiles.at(i).first;
        rattr = rfiles.at(i).second;
        found = -1;
        for (int j = 0; j < lfiles.count(); ++j) {
            lfile = lfiles.at(j).first;
            lattr = lfiles.at(j).second;

            // compare name
            if (lfile == rfile) {
                found = j;
                break;
            }
        }
        if (found != -1) {
            // compare time
            if (this->sameFileTime(rattr->mtime, lattr->mtime)) {
                // compare size
                if (rattr->filesize == lattr->filesize) {
                    rattr->flags |= FLAG_FILE_EQUAL;
                    lattr->flags |= FLAG_FILE_EQUAL;
                } else {
                    rattr->flags |= FLAG_FILE_DIFFERENT;
                    lattr->flags |= FLAG_FILE_DIFFERENT;
                }                
            } else {
                if (rattr->mtime > lattr->mtime) {
                    rattr->flags |= FLAG_REMOTE_NEWER;
                    lattr->flags |= FLAG_REMOTE_NEWER;
                } else {
                    rattr->flags |= FLAG_LOCAL_NEWER;
                    lattr->flags |= FLAG_LOCAL_NEWER;
                }
            }
            files.append(QPair<QString,LIBSSH2_SFTP_ATTRIBUTES*>(rfile,rattr));
            free(lattr); lattr = NULL;
            lfiles.remove(found);
        } else {
            rattr->flags |= FLAG_REMOTE_ONLY;
            files.append(QPair<QString,LIBSSH2_SFTP_ATTRIBUTES*>(rfile,rattr));
        }
    }

    lcnt = lfiles.count();
    if (lcnt > 0) {
        for (int j = 0; j < lcnt; ++j) {
            lfile = lfiles.at(j).first;
            lattr = lfiles.at(j).second;
            
            lattr->flags |= FLAG_LOCAL_ONLY;
            files.append(QPair<QString,LIBSSH2_SFTP_ATTRIBUTES*>(lfile,lattr));
        }
    }

    rfiles.clear();
    lfiles.clear();

    // 应该首先显示需要更新的文件，而不是先显示相同的文件。
    // make statusMessage, show in status bar
    QString statusMessage = QString("LC: %1, RC: %2, UC: %3, DC: %4, NC: %5, OC: %6")
        .arg(lcnt).arg(rcnt).arg(upCount).arg(downloadCount).arg(newerCount).arg(olderCount);

    return files;
}

bool SyncWalker::dumpMergeResult(QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > &files)
{
    QString file;
    LIBSSH2_SFTP_ATTRIBUTES *attr;
    QString fline;

    for(int i = 0 ; i < files.count(); ++i) {
        file = files.at(i).first;
        attr = files.at(i).second;

        fline = QString("%1").arg(i) + ": " + file + " [ ";
        if (attr->flags & FLAG_LOCAL_ONLY) {
            fline += QString("local only");
        } 
        if (attr->flags & FLAG_REMOTE_ONLY) {
            fline += QString("remote only");
        } 
        if (attr->flags & FLAG_LOCAL_NEWER) {
            fline += QString("local newer");            
        } 
        if (attr->flags & FLAG_REMOTE_NEWER) {
            fline += QString("remote newer");
        } 
        if (attr->flags & FLAG_FILE_EQUAL) {
            fline += QString("the same");
        } 
        if (attr->flags & FLAG_FILE_DIFFERENT) {
            fline += QString("???");
        }
        fline += " ] ";

        qDebug()<<fline;
    }
    return true;
}
QString SyncWalker::diffDesciption(unsigned long flags)
{
    QString fline;

    if (flags & FLAG_LOCAL_ONLY) {
        fline += QString("local only");
    } 
    if (flags & FLAG_REMOTE_ONLY) {
        fline += QString("remote only");
    } 
    if (flags & FLAG_LOCAL_NEWER) {
        fline += QString("local newer");            
    } 
    if (flags & FLAG_REMOTE_NEWER) {
        fline += QString("remote newer");
    } 
    if (flags & FLAG_FILE_EQUAL) {
        fline += QString("the same");
    } 
    if (flags & FLAG_FILE_DIFFERENT) {
        fline += QString("???");
    }

    return fline;
}
/*
 */
void SyncWalker::run()
{
    if (!this->checkLocalInfo()) {
        q_debug()<<"";
        return ;
    }
    if (!this->connectToRemoteHost() || !this->checkRemoteInfo()) {
        q_debug()<<"error";
        return ;
    }

    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > remoteFiles;
    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > localFiles;
    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > mergedFiles;

    remoteFiles = this->getRemoteFiles();
    localFiles = this->getLocalFiles();

    q_debug()<<"Remote File count: "<<remoteFiles.count();
    // q_debug()<<remoteFiles;
    q_debug()<<"File count: "<<localFiles.count();
    // q_debug()<<localFiles;
    q_debug()<<"merging...";
    mergedFiles = this->sortMerge(remoteFiles, localFiles);
    this->dumpMergeResult(mergedFiles);
    this->mMergedFiles = mergedFiles;
   
    q_debug()<<"thread end";
}

//////////////////////
/////////////////////
SynchronizeWindow::SynchronizeWindow(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , ui_win(new Ui::SynchronizeWindow())
    , ctxMenu(0), transfer(0), busy(false)
{
    this->ui_win->setupUi(this);

    walker = new SyncWalker(this);
    QObject::connect(walker, SIGNAL(status_msg(QString)), this, SLOT(slot_status_msg(QString)));
    QObject::connect(walker, SIGNAL(finished()), this, SLOT(slot_finished()));
    QObject::connect(this->ui_win->toolButton_4, SIGNAL(clicked()), this, SLOT(start()));
    this->running = false;

    // model = new SyncDifferModel(this);
    // this->ui_win->treeView->setModel(model);
    // QObject::connect(walker, SIGNAL(found_row()), model, SLOT(maybe_has_data()));
    this->ui_win->treeView->setModel(0);    
    QObject::connect(&this->progress_timer, SIGNAL(timeout()), this, SLOT(progress_timer_timeout()));
    QObject::connect(this->ui_win->treeView, SIGNAL(customContextMenuRequested ( const QPoint & )),
                     this, SLOT(showCtxMenu(const QPoint &)));
    QObject::connect(this->ui_win->toolButton, SIGNAL(clicked()),
                     this, SLOT(syncAllFiles()));
    QObject::connect(this->ui_win->toolButton_2, SIGNAL(clicked()),
                     this, SLOT(uploadAllFiles()));
    QObject::connect(this->ui_win->toolButton_3, SIGNAL(clicked()),
                     this, SLOT(downloadAllFiles()));
}

SynchronizeWindow::~SynchronizeWindow()
{
    q_debug()<<"destructured";    
    if(this->walker->isRunning()) {
        q_debug()<<"walker thread is running";
    }
    delete this->walker;
}

void SynchronizeWindow::set_sync_param(QString local_dir, QString sess_name, 
                                       QString remote_dir, bool recursive, int way)
{
    this->local_dir = local_dir;
    this->sess_name = sess_name;
    this->remote_dir = remote_dir;
    this->recursive = recursive;
    this->way = way;

    this->ui_win->lineEdit->setText(local_dir);
    this->ui_win->lineEdit_2->setText(sess_name + ":" + remote_dir);
}

void SynchronizeWindow::start()
{
    if(!this->running) {
        this->running = true;
        this->walker->start();
        this->progress_timer.start(100);
    }
    this->initCtxMenu();
}
void SynchronizeWindow::stop()
{
    
}
void SynchronizeWindow::slot_finished()
{
    SyncWalker *sender_walker = (SyncWalker*)(sender());
    Q_ASSERT(sender_walker == this->walker);
    Q_UNUSED(sender_walker);

    this->running = false;
    model = new SyncDifferModel(this);
    model->setDiffFiles(this->walker->mMergedFiles);
    this->ui_win->treeView->setModel(model);

    this->ui_win->treeView->expandAll();
    this->ui_win->treeView->setColumnWidth(0, 360);
    this->ui_win->treeView->setColumnWidth(2, 100);

    this->progress_timer.stop();
    this->ui_win->progressBar->setValue(100);

    ////////
    if (this->transfer == 0) {
        this->transfer = new SyncTransferThread(this);
        this->transfer->setRemoteSession(this->sess_name);
        this->transfer->setBasePath(this->local_dir, this->remote_dir);
        QObject::connect(this, SIGNAL(syncDownload(QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*>)),
                         this->transfer, SLOT(slot_syncDownload(QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*>)));
        QObject::connect(this, SIGNAL(syncUpload(QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*>)),
                         this->transfer, SLOT(slot_syncUpload(QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*>)));

        // update status
        QObject::connect(this->transfer, SIGNAL(syncFileStarted(QString, quint64)),
                         this->model, SLOT(startSyncFile(QString, quint64)));
        QObject::connect(this->transfer, SIGNAL(syncFileStopped(QString, int)),
                         this->model, SLOT(stopSyncFile(QString, int)));
        QObject::connect(this->transfer, SIGNAL(transfer_percent_changed(QString, int, quint64, quint64)),
                         this->model, SLOT(changeTransferedPercent(QString, int, quint64, quint64)));

    }
}

void SynchronizeWindow::slot_status_msg(QString msg)
{
    this->ui_win->label->setText(msg);
}

void SynchronizeWindow::closeEvent(QCloseEvent *evt)
{
    //q_debug()<<"";
    this->deleteLater();
    QWidget::closeEvent(evt);
}

QString SynchronizeWindow::diffDesciption(unsigned long flags)
{
    return this->walker->diffDesciption(flags);
}

void SynchronizeWindow::progress_timer_timeout()
{
    int val = this->ui_win->progressBar->value();
    if (val == 100) {
        val = 0;
    }
    val = val == 100 ? 0 : val + 5;
    this->ui_win->progressBar->setValue(val);
}

void SynchronizeWindow::initCtxMenu()
{
    if (this->ctxMenu == NULL) {
        this->ctxMenu = new QMenu(this);
        QAction *action;
        action = new QAction(tr("File &info..."), this->ctxMenu);
        this->ctxMenu->addAction(action);
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(showDiffFileInfo()));

        dlAction = new QAction(tr("&Download selected files"), this->ctxMenu);
        this->ctxMenu->addAction(dlAction);
        QObject::connect(dlAction, SIGNAL(triggered()), this, SLOT(dlSelectedDiffFiles()));
        
        upAction = new QAction(tr("&Upload selected files"), this->ctxMenu);
        this->ctxMenu->addAction(upAction);
        QObject::connect(upAction, SIGNAL(triggered()), this, SLOT(upSelectedDiffFiles()));

        upAction = new QAction(tr("&Sync download all files"), this->ctxMenu);
        this->ctxMenu->addAction(upAction);
        QObject::connect(upAction, SIGNAL(triggered()), this, SLOT(downloadAllFiles()));

        upAction = new QAction(tr("&Sync upload all files"), this->ctxMenu);
        this->ctxMenu->addAction(upAction);
        QObject::connect(upAction, SIGNAL(triggered()), this, SLOT(uploadAllFiles()));

        upAction = new QAction(tr("&Download remote only files"), this->ctxMenu);
        this->ctxMenu->addAction(upAction);
        QObject::connect(upAction, SIGNAL(triggered()), this, SLOT(downloadRemoteOnlyFiles()));

        upAction = new QAction(tr("&Upload local only files"), this->ctxMenu);
        this->ctxMenu->addAction(upAction);
        QObject::connect(upAction, SIGNAL(triggered()), this, SLOT(uploadLocalOnlyFiles()));

        upAction = new QAction(tr("&Sync all files"), this->ctxMenu);
        this->ctxMenu->addAction(upAction);
        QObject::connect(upAction, SIGNAL(triggered()), this, SLOT(syncAllFiles()));
    }
}

QModelIndexList SynchronizeWindow::getSelectedModelIndexes()
{
    QItemSelectionModel *ism = NULL;
    QModelIndex cidx, idx, pidx;
    QModelIndexList mil;

    ism = this->ui_win->treeView->selectionModel();
    if (ism != NULL) {
        // mil = ism->selectedIndexes();
        if (ism->hasSelection()) {
            cidx = ism->currentIndex();
            pidx = cidx.parent();

            for (int i = 0 ; i < ism->model()->rowCount(pidx); i++) {
                if (ism->isRowSelected(i, pidx)) {
                    idx = ism->model()->index(i, 0, pidx);
                    mil << idx;
                }
            }
        }
    }

    return mil;
}
void SynchronizeWindow::manualShowWhatsThis(QString what)
{
    QPoint pos = QCursor::pos();
    //QWhatsThis::showText(pos, tr("sdfsdfsdfdsf\nsdfijsdfiowej"));
    QWhatsThis::showText(pos, what);    
}
void SynchronizeWindow::showCtxMenu(const QPoint & pos)
{
    if (this->ctxMenu == NULL) {
        return ;
    }
    //if (this->model == NULL) {
    //  return ;
    //}
    QPoint adjPos = this->ui_win->treeView->mapToGlobal(pos);
    adjPos.setX(adjPos.x() + 3);
    adjPos.setY(adjPos.y() + 25);
    this->ctxMenu->popup(adjPos);
}

void SynchronizeWindow::showDiffFileInfo()
{
    QString info;
    QItemSelectionModel *ism = NULL;
    QModelIndexList mil;
    QModelIndex cidx, idx ;
    QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> file;

    ism = this->ui_win->treeView->selectionModel();
    if (ism != NULL) {
        if (ism->hasSelection()) {
            // mil = ism->selectedIndexes();
            cidx = ism->currentIndex();
            idx = ism->model()->index(cidx.row(), 0, cidx.parent());
            // idx = mil.at(0);
            file = this->model->getFile(idx);
            Q_ASSERT(file.second != NULL);

            QString ftype;
            QString fname=this->local_dir + "/" + file.first;
            ftype = (QFile::exists(fname) ? QFileInfo(fname).isDir() : SSHFileInfo(file.second).isDir()) 
        		? tr("Direcotry") : tr("File");
            info = QString(tr("%1\nType: %2\nSize: %3\nLast modified: %4\nSync Status: %5"))
                .arg(file.first, ftype,
                     QString("%1").arg(file.second->filesize),
                     QDateTime::fromTime_t(file.second->mtime).toString(),
                     this->walker->diffDesciption(file.second->flags)
                     );
        }
    }

    this->manualShowWhatsThis(info);
    // QPoint pos = QCursor::pos();
    // //QWhatsThis::showText(pos, tr("sdfsdfsdfdsf\nsdfijsdfiowej"));
    // QWhatsThis::showText(pos, info);
}

void SynchronizeWindow::dlSelectedDiffFiles()
{
    QString lfname, rfname;
    QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> file;
    QModelIndex idx ;
    QModelIndexList mil = this->getSelectedModelIndexes();
    
    if (mil.count() == 0) {
        q_debug()<<"No item selected";
        return;
    }

    for (int i = 0 ; i < mil.count(); i ++) {
        idx = mil.at(i);
        file = this->model->getFile(idx);
        q_debug()<<file;

        unsigned long flags = file.second->flags;
        if (flags & SyncWalker::FLAG_LOCAL_ONLY) {
            //fline += QString("local only");
            q_debug()<<QString("local only")<<", can not download";
        } else if (flags & SyncWalker::FLAG_REMOTE_ONLY) {
            //fline += QString("remote only");        
        } else if (flags & SyncWalker::FLAG_LOCAL_NEWER) {
            //fline += QString("local newer");            
        } else if (flags & SyncWalker::FLAG_REMOTE_NEWER) {
            //fline += QString("remote newer");
        } else if (flags & SyncWalker::FLAG_FILE_EQUAL) {
            //fline += QString("the same");
            q_debug()<<QString("the same")<<", download not needed";
        } else if (flags & SyncWalker::FLAG_FILE_DIFFERENT) {
            //fline += QString("???");
            q_debug()<<QString("???")<<", not possible";
        } else {

        }

        emit syncDownload(file);
    }
}
void SynchronizeWindow::upSelectedDiffFiles()
{
    QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> file;
    QModelIndex idx ;
    QModelIndexList mil = this->getSelectedModelIndexes();
    
    if (mil.count() == 0) {
        q_debug()<<"No item selected";
        return;
    }
    for (int i = 0 ; i < mil.count() ; i ++) {
        idx = mil.at(i);
        file = this->model->getFile(idx);
        q_debug()<<file;

        unsigned long flags = file.second->flags;
        if (flags & SyncWalker::FLAG_LOCAL_ONLY) {
            //fline += QString("local only");
        } 
        if (flags & SyncWalker::FLAG_REMOTE_ONLY) {
            //fline += QString("remote only");
            q_debug()<<QString("remote only")<<", can not upload";
        } 
        if (flags & SyncWalker::FLAG_LOCAL_NEWER) {
            //fline += QString("local newer");            
        } 
        if (flags & SyncWalker::FLAG_REMOTE_NEWER) {
            //fline += QString("remote newer");
        } 
        if (flags & SyncWalker::FLAG_FILE_EQUAL) {
            //fline += QString("the same");
            q_debug()<<QString("the same")<<", download not needed";
        } 
        if (flags & SyncWalker::FLAG_FILE_DIFFERENT) {
            //fline += QString("???");
            q_debug()<<QString("???")<<", not possible";
        }

        emit syncUpload(file);
    }
}

void SynchronizeWindow::syncAllFiles()
{
    this->syncAllFiles(SyncTransferThread::TASK_BOTH);
}

void SynchronizeWindow::syncAllFiles(int direct)
{
    int list_count = 0;
    // int download_count = 0;
    // int upload_count = 0;
    QModelIndex idx;
    LIBSSH2_SFTP_ATTRIBUTES *pattr;
    QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> file;
    unsigned long flags;

    Q_ASSERT(this->model != NULL);
    list_count = this->model->rowCount(QModelIndex());
    q_debug()<<"files count: "<<list_count;
    for (int i = 0; i < list_count ; i++) {
        idx = this->model->index(i, 0, QModelIndex());
        file = this->model->getFile(idx);
        pattr = file.second;
        Q_ASSERT(pattr != NULL);
        // q_debug()<<file.first<<file.second;
        flags = pattr->flags;
        if (flags & SyncWalker::FLAG_LOCAL_ONLY) {
            if (direct == SyncTransferThread::TASK_UPLOAD
                || direct == SyncTransferThread::TASK_BOTH) {
                emit syncUpload(file);
            }
            if (direct == SyncTransferThread::TASK_UPLOAD_LOCAL_ONLY) {
                emit syncUpload(file);
            }
        } 
        if (flags & SyncWalker::FLAG_REMOTE_ONLY) {
            // q_debug()<<QString("remote only")<<", can not upload";
            if (direct == SyncTransferThread::TASK_DOWNLOAD
                || direct == SyncTransferThread::TASK_BOTH) {
                emit syncDownload(file);
            }
            if (direct == SyncTransferThread::TASK_DOWNLOAD_REMOTE_ONLY) {
                emit syncDownload(file);
            }
        } 
        if (flags & SyncWalker::FLAG_LOCAL_NEWER) {
            if (direct == SyncTransferThread::TASK_UPLOAD
                || direct == SyncTransferThread::TASK_BOTH) {
                emit syncUpload(file);
            }
        } 
        if (flags & SyncWalker::FLAG_REMOTE_NEWER) {
            if (direct == SyncTransferThread::TASK_DOWNLOAD
                || direct == SyncTransferThread::TASK_BOTH) {
                emit syncDownload(file);
            }
        } 
        if (flags & SyncWalker::FLAG_FILE_EQUAL) {
            // q_debug()<<QString("the same")<<", sync not needed";
        } 
        if (flags & SyncWalker::FLAG_FILE_DIFFERENT) {
            q_debug()<<QString("???")<<", not possible";
        }
    }
}

void SynchronizeWindow::downloadAllFiles()
{
    this->syncAllFiles(SyncTransferThread::TASK_DOWNLOAD);
}

void SynchronizeWindow::uploadAllFiles()
{
    this->syncAllFiles(SyncTransferThread::TASK_UPLOAD);
}

void SynchronizeWindow::downloadRemoteOnlyFiles()
{
    this->syncAllFiles(SyncTransferThread::TASK_DOWNLOAD_REMOTE_ONLY);
}

void SynchronizeWindow::uploadLocalOnlyFiles()
{
    this->syncAllFiles(SyncTransferThread::TASK_UPLOAD_LOCAL_ONLY);
}

void SynchronizeWindow::acquireBusyControl()
{
    bool enable = false;
    this->ui_win->toolButton->setEnabled(enable);
    this->ui_win->toolButton_2->setEnabled(enable);
    this->ui_win->toolButton_3->setEnabled(enable);
    this->ui_win->toolButton_4->setEnabled(enable);
}

void SynchronizeWindow::releaseBusyControl()
{
    bool enable = true;
    this->ui_win->toolButton->setEnabled(enable);
    this->ui_win->toolButton_2->setEnabled(enable);
    this->ui_win->toolButton_3->setEnabled(enable);
    this->ui_win->toolButton_4->setEnabled(enable);
}

