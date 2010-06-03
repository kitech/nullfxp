// dirnavbar.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-06-02 09:51:44 +0800
// Version: $Id$
// 

// it's a component for both local and remote view

#ifndef _DIRNAVBAR_H_
#define _DIRNAVBAR_H_

#include "ui_dirnavbar.h"

#include <QtCore>
#include <QtGui>

class DirNavBar : public QWidget
{
    Q_OBJECT;

public:
    DirNavBar(QWidget *parent = 0);
    ~DirNavBar();

public slots:
    void onSetHome(QString path);
    void onNavToPath(QString path);
    void onSetCompleteList(QString dirPrefix, QStringList paths);
    
signals:
    void dirPrefixChanged(QString dirPrefix);
    void dirInputConfirmed(QString path);
    void goHome();

private slots:
    void onGoPrevious();
    void onGoNext();
    void onGoUp();
    void onGoHome();
    void onReload();

    void onComboBoxEditTextChanged(const QString &text);
    void onComboboxIndexChanged(const QString &text);

protected:
    // event filter for dir combobox edit
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::DirNavBar uiw;
    QString homePath;
    QVector<QString> dirConfirmHistory;
    int dirHistoryCurrentPos;
    bool blockCompleteRequest;
    QCompleter *comer;
    QStringListModel *comModel;
};

#endif /* _DIRNAVBAR_H_ */
