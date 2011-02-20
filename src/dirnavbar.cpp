// dirnavbar.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-06-02 09:54:24 +0800
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>

#include "utils.h"

#include "ui_dirnavbar.h"

#include "dirnavbar.h"

// TODO now not support CJKs paths, why?
// TODO drop duplicate item in history vector
DirNavBar::DirNavBar(QWidget *parent)
    : QWidget(parent)
    ,uiw(new Ui::DirNavBar())
{
    this->uiw->setupUi(this);

    ////
    this->zoonSlider = NULL;

    this->dirHistoryCurrentPos = -1;
    this->comer = this->uiw->comboBox->completer();
    this->comer->setCompletionMode(QCompleter::PopupCompletion);
    this->comer->setMaxVisibleItems(32);
    this->comModel = new QStringListModel();
    this->comer->setModel(this->comModel);
    
    // on emit signal by key, not program setting
    QObject::connect(this->uiw->comboBox, SIGNAL(currentIndexChanged(const QString &)),
                     this, SLOT(onComboboxIndexChanged(const QString &)));
    QObject::connect(this->uiw->comboBox, SIGNAL(editTextChanged(const QString &)),
                     this, SLOT(onComboBoxEditTextChanged(const QString &)));
    QObject::connect(this->uiw->toolButton_5, SIGNAL(clicked()),
                     this, SLOT(onGoHome()));
    QObject::connect(this->uiw->toolButton_4, SIGNAL(clicked()),
                     this, SLOT(onGoPrevious()));
    QObject::connect(this->uiw->toolButton_3, SIGNAL(clicked()),
                     this, SLOT(onGoNext()));
    QObject::connect(this->uiw->toolButton_2, SIGNAL(clicked()),
                     this, SLOT(onGoUp()));
    QObject::connect(this->uiw->toolButton, SIGNAL(clicked()),
                     this, SLOT(onReload()));
    QObject::connect(this->uiw->toolButton_7, SIGNAL(clicked()),
                     this, SLOT(onDropDownZoonSlider()));

    this->uiw->comboBox->installEventFilter(this);

    this->maxHistoryCount = 100;
}

DirNavBar::~DirNavBar()
{
}

void DirNavBar::onSetHome(QString path)
{
    Q_ASSERT(!path.isEmpty());
    this->homePath = path;
    this->uiw->comboBox->setEditText(path);

    int insertOffset = this->uiw->comboBox->count() - 1;
    Q_ASSERT(insertOffset >= 0);
    this->uiw->comboBox->insertItem(insertOffset, path);
    if (this->uiw->comboBox->count() > this->maxHistoryCount) {
        this->uiw->comboBox->removeItem(0);
    }
    this->dirHistoryCurrentPos = this->uiw->comboBox->count() - 2;
    // why -2? because has a Clear... item at last
}
void DirNavBar::onNavToPath(QString path)
{
    if (path.isEmpty()) {
        // is this must the root node?
        Q_ASSERT(!path.isEmpty());
    }

    this->uiw->comboBox->setEditText(path);

    int insertOffset = this->uiw->comboBox->count() - 1;
    Q_ASSERT(insertOffset >= 0);
    this->uiw->comboBox->insertItem(insertOffset, path);
    if (this->uiw->comboBox->count() > this->maxHistoryCount) {
        this->uiw->comboBox->removeItem(0);
    }
    this->dirHistoryCurrentPos = this->uiw->comboBox->count() - 2;
}

void DirNavBar::onSetCompleteList(QString dirPrefix, QStringList paths)
{
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

    this->comer->popup();
}
    
void DirNavBar::onGoPrevious()
{
    // pos - 1, hicnt, 0
    int p1 = this->dirHistoryCurrentPos - 1;
    int p2 = this->uiw->comboBox->count() - 2;

    if (p1 < 0) {
        p1 = p2;
    }
    
    if (p1 < 0) {
        q_debug()<<"no previous item";
    } else if (p1 == 0 && p2 == 0) {
        // only 1 item, no previous and next item, no op
    } else {
        Q_ASSERT(p1 >= 0 && p1 < this->uiw->comboBox->count());
        QString path = this->uiw->comboBox->itemText(p1);
        this->dirHistoryCurrentPos = p1;

        this->uiw->comboBox->setEditText(path);        
        emit dirInputConfirmed(path);
    }
}

void DirNavBar::onGoNext()
{
    int p1 = this->dirHistoryCurrentPos + 1;
    int p2 = this->uiw->comboBox->count() - 2;

    if (p1 > p2) {
        p1 = 0;
    }
    
    if (p1 == 0 && p2 == 0) {
        // only 1 item, no previous and next item, no op
    } else {
        Q_ASSERT(p1 >= 0 && p1 < this->uiw->comboBox->count());
        QString path = this->uiw->comboBox->itemText(p1);
        this->dirHistoryCurrentPos = p1;

        this->uiw->comboBox->setEditText(path);
        emit dirInputConfirmed(path);
    }
}

void DirNavBar::onGoUp()
{
    QString currPath = this->uiw->comboBox->currentText();
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

    currPath = currPath.left(upos);
    if (currPath.length() == 0) {
        currPath = "/";
    }
    
    this->uiw->comboBox->setEditText(currPath);

    int insertOffset = this->uiw->comboBox->count() - 1;
    Q_ASSERT(insertOffset >= 0);
    this->uiw->comboBox->insertItem(insertOffset, currPath);
    if (this->uiw->comboBox->count() > this->maxHistoryCount) {
        this->uiw->comboBox->removeItem(0);
    }
    this->dirHistoryCurrentPos = this->uiw->comboBox->count() - 2;

    emit this->dirInputConfirmed(currPath);
}

void DirNavBar::onGoHome()
{
    if (this->homePath.isEmpty()) {
        emit goHome();
    } else {
        this->uiw->comboBox->setEditText(this->homePath);
        emit dirInputConfirmed(this->homePath);
    }
}

void DirNavBar::onReload()
{
    QString currPath = this->uiw->comboBox->currentText();
    emit dirInputConfirmed(currPath);
}

void DirNavBar::onComboBoxEditTextChanged(const QString &text)
{
    //     emit dirPrefixChanged(text);
    if (text == tr("Clear...")) {
        this->uiw->comboBox->setCurrentIndex(-1);
        this->dirHistoryCurrentPos = -1;

        while (this->uiw->comboBox->count() > 1) {
            this->uiw->comboBox->removeItem(0);
        }
    }
}

void DirNavBar::onComboboxIndexChanged(const QString &text)
{
    if (text == tr("Clear...")) {
        // what can I do???
    } else {
        emit dirInputConfirmed(text);
    }
}

bool DirNavBar::eventFilter(QObject *obj, QEvent *event)
{
    bool hok = QObject::eventFilter(obj, event);

    if (obj == this->uiw->comboBox) {
        if (event->type() == QEvent::KeyRelease) {
            QKeyEvent *kevt = static_cast<QKeyEvent *>(event);
            if (kevt->key() == Qt::Key_Return) {
                q_debug()<<"filter got return key";
                // emit dirInputConfirmed(this->uiw->comboBox->currentText());
                this->onComboboxIndexChanged(this->uiw->comboBox->currentText());
            } else {
                QString currPath = this->uiw->comboBox->currentText();
                q_debug()<<currPath;
                if (currPath.startsWith("/")
                    || (currPath.length() > 1 
                        && (currPath.toUpper().at(0) >= 'A' && currPath.toUpper().at(1) <= 'Z')
                        && currPath.at(1) == ':')) {
                    // q_debug()<<currPath;
                    emit dirPrefixChanged(currPath);
                } else {
                    q_debug()<<"Invalid path prefix.";
                }
            }
        }
    }
    if (obj == this->zoonSlider) {
        if (event->type() == QEvent::FocusOut) {
            q_debug()<<"";
            this->zoonSlider->hide();
        }
    }

    return hok;
}

void DirNavBar::onDropDownZoonSlider()
{
    if (this->zoonSlider == NULL) {
        // this->zoonSlider = new QSlider(this->uiw->toolButton_7);
        this->zoonSlider = new QSlider(this);
        this->zoonSlider->installEventFilter(this);
        // this->zoonSlider->setWindowFlags(Qt::ToolTip);
        this->zoonSlider->setWindowFlags(Qt::Window | Qt::FramelessWindowHint); // ok
        this->zoonSlider->setMinimum(16);
        this->zoonSlider->setMaximum(128);
        this->zoonSlider->setPageStep(16);
        this->zoonSlider->setSingleStep(8);
        this->zoonSlider->setTracking(false);
        this->zoonSlider->setValue(64);
        this->zoonSlider->setOrientation(Qt::Vertical);
        this->zoonSlider->setFixedHeight(130);
        
        QObject::connect(this->zoonSlider, SIGNAL(valueChanged(int)),
                         this, SLOT(onSliderChanged(int)));
    }
    if (this->zoonSlider->isVisible()) {
        this->zoonSlider->hide();
    } else {
        QPoint parentPos = this->uiw->toolButton_7->pos();
        QPoint realPos = this->mapToGlobal(parentPos);
        realPos.setY(realPos.y() + this->uiw->toolButton_7->height());
        q_debug()<<parentPos<<realPos;
        this->zoonSlider->move(realPos);
        this->zoonSlider->show();
        this->zoonSlider->activateWindow();
        this->zoonSlider->setFocus(Qt::PopupFocusReason);
    }
}

void DirNavBar::onSliderChanged(int value)
{
    // q_debug()<<value;
    if (value % 8 == 0) {
        // emit 
    }
    emit this->iconSizeChanged(value);
}

