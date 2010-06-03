// dirnavbar.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-06-02 09:54:24 +0800
// Version: $Id$
// 

#include "utils.h"
#include "dirnavbar.h"

#include <QtCore>
#include <QtGui>

// TODO now not support CJKs paths, why?
// TODO drop duplicate item in history vector
DirNavBar::DirNavBar(QWidget *parent)
    : QWidget(parent)
{
    this->uiw.setupUi(this);

    this->blockCompleteRequest = false;
    this->dirHistoryCurrentPos = -1;
    this->comer = this->uiw.comboBox->completer();
    this->comer->setCompletionMode(QCompleter::PopupCompletion);
    this->comer->setMaxVisibleItems(32);
    this->comModel = new QStringListModel();
    // this->comModel = this->comer->completionModel(); // it is static , can not add more item
    this->comer->setModel(this->comModel);

    QObject::connect(this->uiw.comboBox, SIGNAL(currentIndexChanged(const QString &)),
                     this, SLOT(onComboboxIndexChanged(const QString &)));
    QObject::connect(this->uiw.comboBox, SIGNAL(editTextChanged(const QString &)),
                     this, SLOT(onComboBoxEditTextChanged(const QString &)));
    QObject::connect(this->uiw.toolButton_5, SIGNAL(clicked()),
                     this, SLOT(onGoHome()));
    QObject::connect(this->uiw.toolButton_4, SIGNAL(clicked()),
                     this, SLOT(onGoPrevious()));
    QObject::connect(this->uiw.toolButton_3, SIGNAL(clicked()),
                     this, SLOT(onGoNext()));
    QObject::connect(this->uiw.toolButton_2, SIGNAL(clicked()),
                     this, SLOT(onGoUp()));
    QObject::connect(this->uiw.toolButton, SIGNAL(clicked()),
                     this, SLOT(onReload()));

    this->uiw.comboBox->installEventFilter(this);

    this->maxHistoryCount = 100;
}

DirNavBar::~DirNavBar()
{
}


void DirNavBar::onSetHome(QString path)
{
    this->blockCompleteRequest = true;
    Q_ASSERT(!path.isEmpty());
    this->homePath = path;
    this->uiw.comboBox->setEditText(path);
    this->dirConfirmHistory.append(path);
    if (this->dirConfirmHistory.count() > 100) {
        this->dirConfirmHistory.remove(0);
    }
    this->dirHistoryCurrentPos = this->dirConfirmHistory.count() - 1;
    this->blockCompleteRequest = false;
}
void DirNavBar::onNavToPath(QString path)
{
    Q_ASSERT(!path.isEmpty());
    this->blockCompleteRequest = true;
    this->uiw.comboBox->setEditText(path);
    this->dirConfirmHistory.append(path);
    if (this->dirConfirmHistory.count() > 100) {
        this->dirConfirmHistory.remove(0);
    }
    this->dirHistoryCurrentPos = this->dirConfirmHistory.count() - 1;
    this->blockCompleteRequest = false;
}

void DirNavBar::onSetCompleteList(QString dirPrefix, QStringList paths)
{
    this->blockCompleteRequest = true;
    // q_debug()<<paths;
    QModelIndex index;
    int nrc = paths.count();
    int orc = this->comModel->rowCount(QModelIndex());

    for (int i = nrc - 1 ; i >= 0 ; --i) {
        this->comModel->insertRows(0,1, QModelIndex());
        index = this->comModel->index(0, 0, QModelIndex());
        this->comModel->setData(index, paths.at(i));
    }

    // -1, the last item, - 1, side
    for (int i = orc + nrc - 2; i >= nrc ; --i) {
        this->comModel->removeRows(i, 1, QModelIndex());
    }

    this->blockCompleteRequest = false;
}
    
void DirNavBar::onGoPrevious()
{
    // pos - 1, hicnt, 0
    int p1 = this->dirHistoryCurrentPos - 1;
    int p2 = this->dirConfirmHistory.count() - 1;

    q_debug()<<this->dirConfirmHistory;

    if (this->dirConfirmHistory.isEmpty()) {
        return;
    }

    if (p1 < 0) {
        p1 = p2;
    }
    
    if (p1 < 0) {
        q_debug()<<"no previous item";
    } else if (p1 == 0 && p2 == 0) {
        // only 1 item, no previous and next item, no op
    } else {
        Q_ASSERT(p1 >= 0 && p1 < this->dirConfirmHistory.count());
        QString path = this->dirConfirmHistory.at(p1);
        this->dirHistoryCurrentPos = p1;
        this->uiw.comboBox->setEditText(path);
        emit dirInputConfirmed(path);
    }
}

void DirNavBar::onGoNext()
{
    int p1 = this->dirHistoryCurrentPos + 1;
    int p2 = this->dirConfirmHistory.count() - 1;

    q_debug()<<this->dirConfirmHistory;
    if (this->dirConfirmHistory.isEmpty()) {
        return;
    }

    if (p1 > p2) {
        p1 = 0;
    }
    
    if (p1 == 0 && p2 == 0) {
        // only 1 item, no previous and next item, no op
    } else {
        Q_ASSERT(p1 >= 0 && p1 < this->dirConfirmHistory.count());
        QString path = this->dirConfirmHistory.at(p1);
        this->dirHistoryCurrentPos = p1;
        this->uiw.comboBox->setEditText(path);
        emit dirInputConfirmed(path);
    }
}

void DirNavBar::onGoUp()
{
    QString currPath = this->uiw.comboBox->currentText();
    if (currPath.length() == 0) {
        q_debug()<<"No current path set!!!";
        return;
    }
    if (currPath.length() > 1 
        && (currPath.endsWith('/') || currPath.startsWith('\\'))) {
        currPath = currPath.left(currPath.length() - 2);
    }
    if (currPath == "/") {
        q_debug()<<"already toppest, can not go up";
        return;
    }

    int upos = currPath.lastIndexOf('/');
    if (upos == -1) {
        upos = currPath.lastIndexOf('\\');
    }
    
    if (upos == -1) {
        q_debug()<<"What's happened, not possible???"<<currPath;
        // on windows, if currPath is C: D: and so on, this will happen
        Q_ASSERT(upos != -1);
        return;
    }

    q_debug()<<upos<<currPath;

    currPath = currPath.left(upos + 1);
    
    this->uiw.comboBox->setEditText(currPath);
    this->dirConfirmHistory.append(currPath);
    if (this->dirConfirmHistory.count() > 100) {
        this->dirConfirmHistory.remove(0);
    }
    this->dirHistoryCurrentPos = this->dirConfirmHistory.count() - 1;
    emit this->dirInputConfirmed(currPath);
}

void DirNavBar::onGoHome()
{
    if (this->homePath.isEmpty()) {
        emit goHome();
    } else {
        this->uiw.comboBox->setEditText(this->homePath);
        emit dirInputConfirmed(this->homePath);
    }
}

void DirNavBar::onReload()
{
    QString currPath = this->uiw.comboBox->currentText();
    emit dirInputConfirmed(currPath);
}

void DirNavBar::onComboBoxEditTextChanged(const QString &text)
{
    if (!this->blockCompleteRequest) {
        emit dirPrefixChanged(text);
    }
}

void DirNavBar::onComboboxIndexChanged(const QString &text)
{
    emit dirInputConfirmed(text);
}

bool DirNavBar::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == this->uiw.comboBox) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *kevt = static_cast<QKeyEvent *>(event);
            if (kevt->key() == Qt::Key_Return) {
                q_debug()<<"filter got return key";
                emit dirInputConfirmed(this->uiw.comboBox->currentText());
            }
        }
    }
    return QObject::eventFilter(obj, event);
}
