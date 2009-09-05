// remotehostquickconnectinfodialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-08-08 11:28:59 +0800
// Version: $Id$
// 

#include  <QtGui>

#include "utils.h"

#include "remotehostquickconnectinfodialog.h"

RemoteHostQuickConnectInfoDialog::RemoteHostQuickConnectInfoDialog(QWidget* parent, Qt::WindowFlags f): QDialog(parent, f)
{
    this->quick_connect_info_dialog.setupUi(this);
    QObject::connect(this->quick_connect_info_dialog.checkBox_3, SIGNAL(stateChanged(int)),
                     this, SLOT(slot_pubkey_checked(int)));
    QObject::connect(this->quick_connect_info_dialog.toolButton, SIGNAL(clicked()),
                     this, SLOT(slot_select_pubkey()));
    QObject::connect(this->quick_connect_info_dialog.comboBox, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(slot_protocol_changed(int)));

    this->pubkey_path = QString::null;
}


RemoteHostQuickConnectInfoDialog::~RemoteHostQuickConnectInfoDialog()
{
}

QString RemoteHostQuickConnectInfoDialog::get_protocol()
{
    return this->quick_connect_info_dialog.comboBox->currentText().trimmed();
}
QString RemoteHostQuickConnectInfoDialog::get_user_name ()
{
    return this->quick_connect_info_dialog.lineEdit_3->text().trimmed();
}

QString RemoteHostQuickConnectInfoDialog::get_host_name ()
{
    return this->quick_connect_info_dialog.lineEdit->text().trimmed();
}

QString RemoteHostQuickConnectInfoDialog::get_password()
{
    QString passwd = this->quick_connect_info_dialog.lineEdit_4->text();
    passwd = QUrl::toPercentEncoding(passwd);
    return passwd;
}
short  RemoteHostQuickConnectInfoDialog::get_port()
{
    QString str_port = this->quick_connect_info_dialog.lineEdit_2->text().trimmed();
    int port = str_port.toShort();
    
    return port;
}
QString RemoteHostQuickConnectInfoDialog::get_pubkey()
{
    return this->pubkey_path;
}

void RemoteHostQuickConnectInfoDialog::set_active_host(QMap<QString,QString> host)
{
    QString show_name = host["show_name"];
    QString protocol = host["protocol"];
    QString host_name = host["host_name"];
    QString user_name = host["user_name"];
    QString password = host["password"];
    QString port = host["port"];
    QString pubkey_path ;//
    if (host.contains("pubkey")) {
        pubkey_path = host["pubkey"];
    }

    if (protocol == QString("FTPS")) {
        this->quick_connect_info_dialog.comboBox->setCurrentIndex(2);
    } else if (protocol == QString("FTP")) {
        this->quick_connect_info_dialog.comboBox->setCurrentIndex(1);
    } else {
        this->quick_connect_info_dialog.comboBox->setCurrentIndex(0);
    }

    this->quick_connect_info_dialog.lineEdit->setText(host_name);
    this->quick_connect_info_dialog.lineEdit_2->setText(port);
    this->quick_connect_info_dialog.lineEdit_3->setText(user_name);
    
    this->quick_connect_info_dialog.lineEdit_4->setFocus();
    this->quick_connect_info_dialog.lineEdit_4->setText(QUrl::fromPercentEncoding(password.toAscii()));
    
    this->quick_connect_info_dialog.groupBox->setTitle(QString(tr("Host Infomation: %1")).arg(show_name));
    this->show_name = show_name;
    this->pubkey_path = pubkey_path;

    if (host.contains("pubkey")) {
        this->quick_connect_info_dialog.toolButton->setToolTip(QString(tr("Current key: ")) 
                                                               + this->pubkey_path);
    } else {
        this->quick_connect_info_dialog.toolButton->setToolTip(tr("No key"));
    }
}

QMap<QString,QString> RemoteHostQuickConnectInfoDialog::get_host_map()
{
    QMap<QString,QString> host;

    host["show_name"] = this->show_name;
    host["protocol"] = this->quick_connect_info_dialog.comboBox->currentText().trimmed();
    host["host_name"] = this->quick_connect_info_dialog.lineEdit->text().trimmed();
    host["user_name"] = this->quick_connect_info_dialog.lineEdit_3->text().trimmed();
    host["password"] = this->quick_connect_info_dialog.lineEdit_4->text();
    host["password"] = QUrl::toPercentEncoding(host["password"]);
    host["port"] = this->quick_connect_info_dialog.lineEdit_2->text().trimmed();
    if (this->pubkey_path != QString::null && this->pubkey_path.length() > 0) {
        host["pubkey"] = this->pubkey_path;
    }

  return host;
}

void RemoteHostQuickConnectInfoDialog::slot_pubkey_checked(int state)
{
    if (state == Qt::Checked || state == Qt::PartiallyChecked) {
        this->quick_connect_info_dialog.toolButton->setEnabled(true);
    } else {
        this->quick_connect_info_dialog.toolButton->setEnabled(false);
    }
}

void RemoteHostQuickConnectInfoDialog::slot_select_pubkey()
{
    QString path = QString::null;
    QString init_dir;

#ifdef WIN32
    init_dir = ".";
#else
    init_dir = QDir::homePath();
#endif

    path = QFileDialog::getOpenFileName(this, tr("Publickey Select Dialog"),init_dir, tr("Public Key Files (*.pub);;All Files (*)"));
    qDebug()<<path;    
    if( path == QString::null) {
        //cancel
    } else if(path.length() == 0) {
        qDebug()<<"select null";
    } else {
        //TODO 检查文件格式, 
        //TODO 简单检查是否是一个有效的public key 
        //TODO 检查key中的用户名及主机是否与上面输入的一致
        //检查对应的私钥文件;
        QString privkey = path.left(path.length() - 4);
        if (!QFile::exists(privkey)) {
            QMessageBox::warning(this, tr("Warning"), tr("Can not find related private key file."));
        }
        this->pubkey_path = path;
        this->quick_connect_info_dialog.toolButton->setToolTip(QString(tr("Current key: ")) 
                                                               + this->pubkey_path);
    }
}

void RemoteHostQuickConnectInfoDialog::slot_protocol_changed(int index)
{
    switch (index) {
    case 0:
        this->quick_connect_info_dialog.lineEdit_2->setText("22");
        break;
    case 1:
        this->quick_connect_info_dialog.lineEdit_2->setText("21");
        break;
    case 2:
        // ctrl 990, data 989
        this->quick_connect_info_dialog.lineEdit_2->setText("990");
        break;
    default:
        qDebug()<<"Unkown protocl:"<<index;
        break;
    }
}


