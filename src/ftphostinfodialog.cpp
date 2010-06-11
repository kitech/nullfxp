// ftphostinfodialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-06-10 23:07:15 +0800
// Version: $Id$
// 

#include "ftphostinfodialog.h"

FTPHostInfoDialog::FTPHostInfoDialog(QWidget *parent)
    : QDialog(parent)
{
    this->uiw.setupUi(this);

    this->uiw.graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    this->uiw.graphicsView->setSceneRect(0, 0, 200, 300);
    this->mainScene = new QGraphicsScene();
}

FTPHostInfoDialog::~FTPHostInfoDialog()
{
}

void FTPHostInfoDialog::setHostType(QString type)
{
    this->uiw.lineEdit->setText(type);

    QPushButton *pb = new QPushButton("sdfffffffff");
    this->mainScene->addWidget(pb);
    this->uiw.graphicsView->setScene(this->mainScene);
}

void FTPHostInfoDialog::setWelcome(QString welcome)
{
    this->uiw.plainTextEdit->setPlainText(welcome);

    // QPlainTextEdit *pte = new QPlainTextEdit();
    // pte->setPlainText(welcome);
    // this->mainScene->addWidget(pte);
    // this->uiw.graphicsView->setScene(this->mainScene);
    this->uiw.graphicsView->rotate(60);
}
