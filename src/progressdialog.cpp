// progressdialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-05-06 22:14:59 +0800
// Version: $Id$
// 

#include <cassert>
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <sys/types.h>

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#ifdef WIN32
#define HAVE_SYS_UIO_H 0
#else
#include <sys/uio.h>
#endif

#include "utils.h"
#include "globaloption.h"
#include "progressdialog.h"
#include "fileexistaskdialog.h"
// #include "sshtransportor.h"
// #include "ftptransportor.h"
#include "transportor.h"
#include "taskqueuemodel.h"

ProgressDialog::ProgressDialog(QWidget *parent)
    : QWidget(parent)
{
    this->uiw.setupUi(this);
    this->setObjectName("pv");
    //////////
    this->transportor = new Transportor();
    
    QObject::connect(this->transportor, SIGNAL(finished()),
                     this, SLOT(slot_transfer_thread_finished()));
    QObject::connect(this->transportor, SIGNAL(transfer_percent_changed(int, quint64, int)),
                     this, SLOT(slot_set_transfer_percent(int, quint64, int)));
    QObject::connect(this->transportor, SIGNAL(transfer_new_file_started(QString)),
                     this, SLOT(slot_new_file_transfer_started(QString)));
    QObject::connect(this->uiw.pushButton, SIGNAL(clicked()),
                     this, SLOT(slot_cancel_button_clicked()));
    QObject::connect( this->transportor, SIGNAL(transfer_got_file_size(quint64)),
                      this, SLOT(slot_transfer_got_file_size(quint64)));
    QObject::connect(this->transportor, SIGNAL(transfer_log(QString)),
                     this, SLOT(slot_transfer_log(QString)));

    QObject::connect(this->transportor, 
                     SIGNAL(dest_file_exists(QString, QString, QString, QString, QString, QString)),
                     this, SLOT(slot_dest_file_exists(QString, QString, QString, QString, QString, QString)));
    
    this->first_show = true;
    this->uiw.progressBar->setValue(0);
    this->total_files_size = 0;
    this->total_files_count = 0;
    this->abtained_files_size = 0;
    this->abtained_files_count = 0;

    this->time_cacl_timer.setInterval(1000*1);
    QObject::connect(&this->time_cacl_timer, SIGNAL(timeout()), this, SLOT(slot_speed_timer_timeout()));
}

ProgressDialog::~ProgressDialog()
{
    delete this->transportor;
}

void ProgressDialog::set_transfer_info(TaskPackage local_pkg, TaskPackage remote_pkg)
{
    QString local_file_name;
    QString remote_file_name;
    QString tmp_str;
    
    this->local_pkg = local_pkg;
    this->remote_pkg = remote_pkg;

    // transportor 对象应该是哪个呢？ 只应该有一个Transportor, 
    // 在这个唯一的Transportor中根据不同的协议调用不同的传输
    /*
      FILE -> SFTP  SSHTransportor
      FILE -> FTP   FTPTransportor
      FTP -> FILE   FTPTransportor
      SFTP -> FILE  SSHTransportor
      FTP -> SFTP   ???
      SFTP -> FTP   ???
      FTP -> FTP    FTPTransportor
      SFTP -> SFTP  SSHTransportor
     */

    this->transportor->set_transport_info(local_pkg, remote_pkg);
    
    QString local_full_path;
    
    this->uiw.comboBox->clear();
    this->uiw.comboBox_2->clear();

    for (int i = 0; i < remote_pkg.files.count(); i ++) {
        remote_file_name = remote_pkg.files.at(i);
        local_full_path = local_file_name + "/"
            + remote_file_name.split("/").at(remote_file_name.split("/").count()-1);
        this->uiw.lineEdit->setText(local_pkg.getProtocolNameById(local_pkg.scheme)
                                                   + "-->" 
                                                   + remote_pkg.getProtocolNameById(remote_pkg.scheme));
        this->uiw.comboBox_2->addItem(remote_file_name);
    }
}

void ProgressDialog::slot_set_transfer_percent(int percent, quint64 total_transfered, int transfer_delta)
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    this->uiw.progressBar->setValue(percent);
    this->uiw.lineEdit_5->setText(QString("%1").arg(total_transfered));
    this->abtained_files_size += transfer_delta; //TODO no right logic, when resume file
    if (percent == 100) {
        this->abtained_files_count += 1;
    }
    if (!start_time.isValid()) {
        start_time = QDateTime::currentDateTime();
        transfer_speed = 0;
        this->time_cacl_timer.start();
    } else {
        end_time = QDateTime::currentDateTime();
        if ((end_time.toTime_t()-start_time.toTime_t()) != 0) {
            transfer_speed = this->abtained_files_size/(end_time.toTime_t() - start_time.toTime_t());
        } else {
            transfer_speed = this->abtained_files_size/1;
        }
        transfer_speed /= 1024;
    }
    this->update_transfer_state();

    this->taskQueueModel->slot_set_transfer_percent(this->modelId, percent, this->abtained_files_size, this->transfer_speed);
}

void ProgressDialog::slot_transfer_thread_finished()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    
    //this->done(QDialog::Accepted);
    int error_code = this->transportor->get_error_code();
    QString error_string = this->transportor->get_error_message(error_code);
    emit this->transfer_finished(error_code, error_string);

    this->taskQueueModel->slot_transfer_thread_finished(this->modelId);
}

void ProgressDialog::exec()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    if (this->first_show) {
        this->first_show = false;
        this->transportor->start(); 
    }
    //QDialog::exec();
}
void ProgressDialog::show() 
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    if (this->first_show) {
        this->first_show = false;
        this->taskQueueModel = TaskQueueModel::instance();
        this->transportor->start(); 
    }
    QWidget::show(); 
}

void ProgressDialog::slot_new_file_transfer_started(QString new_file_name)
{
    QString u_new_file_name =  new_file_name;

    bool found = 0;
    for (int i = 0; i < this->uiw.comboBox->count(); i ++) {
        if (this->uiw.comboBox->itemText(i) == u_new_file_name) {
            this->uiw.comboBox->setCurrentIndex(i);
            found = 1;
            break;
        }
    }
    if (found == 0) {
        this->uiw.comboBox->addItem(u_new_file_name);
        this->uiw.comboBox->setCurrentIndex(this->uiw.comboBox->count()-1);
    }
    u_new_file_name = QString(tr("processing: %1")).arg(u_new_file_name);
    this->uiw.progressBar->setStatusTip(u_new_file_name);
    ////
    this->total_files_count ++;
    this->update_transfer_state();
    //this->setToolTip(u_new_file_name);
    this->uiw.lineEdit_4->setText(this->type(u_new_file_name));

    QString dest_path = this->uiw.comboBox_2->currentText();

    this->modelId = this->taskQueueModel->slot_new_file_transfer_started(new_file_name, dest_path);
}

void ProgressDialog::closeEvent(QCloseEvent * event) 
{
    int rv = 0;
    
    //this->setVisible(false);
    rv = QMessageBox::information(this, tr("Attemp to close this window?"),
                                   tr("Are you sure to stop the transfomition ?"),
                                   QMessageBox::Ok | QMessageBox::Cancel );
    if (rv == QMessageBox::Ok) {
        this->transportor->set_user_cancel(true);
        //event->ignore();
        this->setVisible(false);
        //emit this->transfer_finished(this->transportor->get_error_code());
        qDebug()<<"Transfer error: "<<this->transportor->get_error_code();
    } else {
        event->ignore();
    }
}

void ProgressDialog::slot_cancel_button_clicked()
{
    this->close();
}
void ProgressDialog::update_transfer_state()
{
    this->uiw.lineEdit_8->setText(QString("%1").arg(this->transfer_speed));
    this->uiw.lineEdit_9->setText(QString("%1").arg(this->abtained_files_count));
    this->uiw.lineEdit_10->setText(QString("%1").arg(this->total_files_count));
    this->uiw.lineEdit_11->setText(QString("%1").arg(this->abtained_files_size));
    this->uiw.lineEdit_12->setText(QString("%1").arg(this->total_files_size));
    if (this->total_files_size == 0) {
        this->uiw.progressBar_2->setValue(100);
    } else {
        this->uiw.progressBar_2->setValue(100.0*((double)this->abtained_files_size/(double)this->total_files_size));
    }
}
void ProgressDialog::slot_transfer_got_file_size(quint64 size)
{
    this->total_files_size += size;
    this->uiw.lineEdit_13->setText(QString("%1").arg(size));
    this->update_transfer_state();

    this->taskQueueModel->slot_transfer_got_file_size(this->modelId, size);
}
void ProgressDialog::slot_transfer_log(QString log)
{
    this->uiw.lineEdit_14->setText(log);
}

QString ProgressDialog::type(QString file_name)
{
    QFileInfo info(file_name);
    
    if (file_name == "/")
        return QApplication::translate("QFileDialog", "Drive");
//     if (info.isFile()) {
    if (1) {
        if (!info.suffix().isEmpty())
            return info.suffix() + QLatin1Char(' ') + QApplication::translate("QFileDialog", "File");
        return QApplication::translate("QFileDialog", "File");
    }

    if (info.isDir())
        return QApplication::translate("QFileDialog",
#ifdef Q_WS_WIN
                                       "File Folder", "Match Windows Explorer"
#else
                                       "Folder", "All other platforms"
#endif
            );
    // Windows   - "File Folder"
    // OS X      - "Folder"
    // Konqueror - "Folder"
    // Nautilus  - "folder"

    if (info.isSymLink())
        return QApplication::translate("QFileDialog",
#ifdef Q_OS_MAC
                                       "Alias", "Mac OS X Finder"
#else
                                       "Shortcut", "All other platforms"
#endif
            );
    // OS X      - "Alias"
    // Windows   - "Shortcut"
    // Konqueror - "Folder" or "TXT File" i.e. what it is pointing to
    // Nautilus  - "link to folder" or "link to object file", same as Konqueror

    return QApplication::translate("QFileDialog", "Unknown");
}

void ProgressDialog::slot_dest_file_exists(QString src_path, QString src_file_size, 
                                           QString src_file_date, QString dest_path, 
                                           QString dest_file_size, QString dest_file_date)
{
    qDebug()<<"Dest file exists: "<<dest_path<<". src path:"<<src_path;
    FileExistAskDialog *ask_dlg;
    int rv = 0;

    ask_dlg = new FileExistAskDialog(this);
    ask_dlg->set_files(src_path, src_file_size, src_file_date, dest_path, dest_file_size, dest_file_date);

    QObject::connect(ask_dlg, SIGNAL(acceptedOne(int)), this, SLOT(slot_ask_accepted(int)));

    rv = ask_dlg->exec();
    delete ask_dlg;
}

void ProgressDialog::slot_ask_accepted(int which)
{
    if (which >= Transportor::OW_CANCEL && which <= Transportor::OW_NO_ALL) {
        this->transportor->user_response_result(which);
    } else {
        qDebug()<<"No care response";
    }
}

void ProgressDialog::slot_speed_timer_timeout()
{
    QDateTime now_time = QDateTime::currentDateTime();
    int days = 0;
    int hours = 0;
    int minites = 0;
    int seconds = 0;
    int total_seconds = 0;
    quint64 left_size = 0;
    float speed_value = 0.0;
    QString time_str;
    QString eclapsed_time_str, left_time_str;
    char time_str_buf[16];

    total_seconds = this->start_time.secsTo(now_time);

    days = total_seconds/(24*3600);
    hours = total_seconds%(24*3600)/3600;
    minites = total_seconds%3600/60;
    seconds = total_seconds%60;    
    if (days > 0) {
        snprintf(time_str_buf, sizeof(time_str_buf) - 1, "%dd:%.2d:%.2d:%.2d",
                 days, hours, minites, seconds);
        // time_str = QString("%1d:%2:%3:%4").arg(days).arg( hours).arg(minites).arg(seconds);
    } else {
        snprintf(time_str_buf, sizeof(time_str_buf) - 1, "%.2d:%.2d:%.2d",
                 hours, minites, seconds);
        // time_str = QString("%1:%2:%3").arg( hours).arg(minites).arg(seconds);
    }
    time_str = QString(time_str_buf);

    this->uiw.lineEdit_6->setText(time_str);
    eclapsed_time_str = time_str;

    ////////////
    speed_value = this->transfer_speed * 1024;
    left_size = total_files_size - abtained_files_size;

    total_seconds = left_size/speed_value;

    days = total_seconds/(24*3600);
    hours = total_seconds%(24*3600)/3600;
    minites = total_seconds%3600/60;
    seconds = total_seconds%60;    

    memset(time_str_buf, 0, sizeof(time_str_buf));
    if (days > 0) {
        snprintf(time_str_buf, sizeof(time_str_buf) - 1, "%dd:%.2d:%.2d:%.2d",
                 days, hours, minites, seconds);
        // time_str = QString("%1d:%2:%3:%4").arg(days).arg(hours).arg(minites).arg(seconds);
    } else {
        snprintf(time_str_buf, sizeof(time_str_buf) - 1, "%.2d:%.2d:%.2d",
                 hours, minites, seconds);
        // time_str = QString("%1:%2:%3").arg( hours).arg(minites).arg(seconds);
    }
    time_str = QString(time_str_buf);

    this->uiw.lineEdit_7->setText(time_str);
    left_time_str = time_str;
    
    this->taskQueueModel->slot_transfer_time_update(this->modelId, eclapsed_time_str, left_time_str);
}
