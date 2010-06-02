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
    this->uiw.comboBox->installEventFilter(this);
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
    this->blockCompleteRequest = false;
}
void DirNavBar::onNavToPath(QString path)
{
    Q_ASSERT(!path.isEmpty());
    this->dirConfirmHistory.append(path);
}

void DirNavBar::onSetCompleteList(QString dirPrefix, QStringList paths)
{
    this->blockCompleteRequest = true;
    // q_debug()<<paths;
    QModelIndex index;
    int nrc = paths.count();
    int orc = this->comModel->rowCount(QModelIndex());
    // q_debug()<<nrc<<orc<<this->uiw.comboBox->currentIndex()<<this->comModel->rowCount();
    for (int i = nrc - 1 ; i >= 0 ; --i) {
        // this->uiw.comboBox->insertItem(0, paths.at(i));
        this->comModel->insertRows(0,1, QModelIndex());
        index = this->comModel->index(0, 0, QModelIndex());
        this->comModel->setData(index, paths.at(i));
        // q_debug()<<"completer elem count:"<<this->comModel->rowCount();
        if (i == 0) {
            // this->uiw.comboBox->setCurrentIndex(0);
        }
    }
    // q_debug()<<nrc<<orc<<this->uiw.comboBox->count()<<this->uiw.comboBox->currentIndex()<<this->comModel->rowCount();
    // -1, the last item, - 1, side
    for (int i = orc + nrc - 2; i >= nrc ; --i) {
        // q_debug()<<"removing "<<i<<this->uiw.comboBox->itemText(i);
        // this->uiw.comboBox->removeItem(i);
        // index = this->comModel->index(i, 0, QModelIndex());
        this->comModel->removeRows(i, 1, QModelIndex());
    }
    // q_debug()<<nrc<<orc<<this->uiw.comboBox->count()<<this->uiw.comboBox->currentIndex()<<this->comModel->rowCount();
    this->blockCompleteRequest = false;
}
    
void DirNavBar::onGoPrevious()
{
    // pos - 1, hicnt, 0
    int p1 = this->dirHistoryCurrentPos - 1;
    int p3 = this->dirConfirmHistory.count();
    int p4 = p1;

    if (p3 == 0) {
        p3 = -1;
    }
    if (p4 < 0) {
        p4 = p3;
    }
    if (p4 < 0) {
        // no op
    } else {
        Q_ASSERT(p4 >= 0 && p4 < this->dirConfirmHistory.count());
        QString path = this->dirConfirmHistory.at(p4);
        this->dirHistoryCurrentPos = p4;
        this->uiw.comboBox->setEditText(path);
        emit dirInputConfirmed(path);
    }
}

void DirNavBar::onGoNext()
{
}

void DirNavBar::onGoUp()
{
    
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
