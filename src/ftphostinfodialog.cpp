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

    // this->uiw.graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    this->uiw.graphicsView->setSceneRect(0, 0, 200, 300);
    this->mainScene = new QGraphicsScene();
}

FTPHostInfoDialog::~FTPHostInfoDialog()
{
}

void FTPHostInfoDialog::setHostType(QString type)
{
    this->uiw.lineEdit->setText(type);

    QLineEdit *le = new QLineEdit();
    le->setText(type);

    QPlainTextEdit *pte = new QPlainTextEdit();
    pte->setPlainText("fsdddddddddddddddddddddddxcivsf");

    QPushButton *pb = new QPushButton("sdfffffffff");
    QGraphicsProxyWidget *w = new QGraphicsProxyWidget();
    w->setPos(20, 20);
    w->setWidget(pb);
    w->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QGraphicsProxyWidget *lew = new QGraphicsProxyWidget();
    lew->setPos(50, 50);
    lew->setWidget(le);
    lew->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QGraphicsProxyWidget *pew = new QGraphicsProxyWidget();
    pew->setPos(100, 100);
    pew->setWidget(pte);
    pew->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QGraphicsWidget *gw = new QGraphicsWidget(0, Qt::Window);
    QGraphicsAnchorLayout *l = new QGraphicsAnchorLayout();
    gw->setLayout(l);
    gw->setPreferredSize(300, 200);
    
    QGraphicsAnchor *anchor;
    anchor = l->addAnchor(pew, Qt::AnchorTop, l, Qt::AnchorTop);
    anchor = l->addAnchor(w, Qt::AnchorTop, l, Qt::AnchorTop);
    anchor = l->addAnchor(lew, Qt::AnchorBottom, l, Qt::AnchorTop);
    
    this->mainScene->addItem(gw);
    this->mainScene->setSceneRect(0, 0, 800, 480);
    this->mainScene->setBackgroundBrush(Qt::darkGreen);
    this->uiw.graphicsView->setScene(this->mainScene);
}

void FTPHostInfoDialog::setWelcome(QString welcome)
{
    this->uiw.plainTextEdit->setPlainText(welcome);

    // QPlainTextEdit *pte = new QPlainTextEdit();
    // pte->setPlainText(welcome);
    // this->mainScene->addWidget(pte);
    // this->uiw.graphicsView->setScene(this->mainScene);
    this->uiw.graphicsView->rotate(30);
}
