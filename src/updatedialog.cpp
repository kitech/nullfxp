// updatedialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sf.net
// Created: 2009-10-12 19:56:06 +0800
// Version: $Id$
// 

#include "utils.h"

#include "updatedialog.h"

UpdateDialog::UpdateDialog(QWidget *parent)
    : QDialog(parent)
{
    this->ui_win.setupUi(this);

    this->ui_win.label_2->setText(NULLFXP_VERSION_STR);
    this->ui_win.label_5->setVisible(false);
    this->ui_win.progressBar->setVisible(false);
    
    this->inChecking = false;
    this->http = NULL;

    QObject::connect(this->ui_win.pushButton, SIGNAL(clicked()),
                     this, SLOT(slotStartCheck()));
    QObject::connect(this->ui_win.pushButton_2, SIGNAL(clicked()),
                     this, SLOT(slotCancelCheck()));
}

UpdateDialog::~UpdateDialog()
{
    if (this->http != NULL) {
        delete this->http;
        this->http = NULL;
    }
}

void UpdateDialog::slotStartCheck()
{
    QString cvUrl = "http://localhost/nullfxp_update/version.txt";
    QUrl uu(cvUrl);
    if (!this->inChecking) {
        this->inChecking = true;
        this->ui_win.label_5->setVisible(true);
        this->ui_win.progressBar->setVisible(true);
        
        this->http = new QHttp(uu.host(), uu.port()<0?80:uu.port());

        QObject::connect(this->http, SIGNAL(dataReadProgress(int, int)),
                         this, SLOT(slotDataReadProgress(int, int)));
        QObject::connect(this->http, SIGNAL(done(bool)),
                         this, SLOT(slotDone(bool)));
        QObject::connect(this->http, SIGNAL(readyRead(const QHttpResponseHeader&)),
                         this, SLOT(slotReadyRead(const QHttpResponseHeader&)));

        this->http->get(uu.path());
    } else {
        qDebug()<<"Is checking, waiting for a while please.";
    }
}

void UpdateDialog::slotCancelCheck()
{
    
}

void UpdateDialog::slotDataReadProgress(int done, int total)
{
    q_debug()<<"Got:"<<done<<" Total:"<<total;
    if (done == total) {
        this->ui_win.progressBar->setValue(100);
    } else {
        this->ui_win.progressBar->setValue(100*done/total);
    }
}

void UpdateDialog::slotDone(bool error)
{
    q_debug()<<"Get Done"<<error;
    if (error != QHttp::NoError) {
        q_debug()<<"Http error:"<<this->http->errorString();
    }
}

void UpdateDialog::slotReadyRead(const QHttpResponseHeader &resp)
{
    // q_debug()<<resp;
    QByteArray ba = this->http->readAll();
    q_debug()<<ba;
    if (!ba.isEmpty()) {
        QStringList sl = QString(ba).split('\n');
        this->ui_win.label_4->setText(sl.at(0).trimmed());
        if (this->hasUpdate(sl.at(0).trimmed())) {
            this->ui_win.label_6->setText(tr("Has update."));
        } else {
            this->ui_win.label_6->setText(tr("No update."));
        }
    } else {

    }
}

bool UpdateDialog::hasUpdate(QString lastestVersion)
{
    return lastestVersion > NULLFXP_VERSION_STR;
    return false;
}
