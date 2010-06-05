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
    void dirPrefixChanged(const QString &dirPrefix);
    void dirInputConfirmed(const QString &path);
    void goHome();

    void iconSizeChanged(int value);

private slots:
    void onGoPrevious();
    void onGoNext();
    void onGoUp();
    void onGoHome();
    void onReload();

    void onComboBoxEditTextChanged(const QString &text);
    void onComboboxIndexChanged(const QString &text);

    // 
    void onDropDownZoonSlider();
    void onSliderChanged(int value);

protected:
    // event filter for dir combobox edit
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::DirNavBar uiw;
    QString homePath;
    QVector<QString> dirConfirmHistory;
    unsigned char dirHistoryCurrentPos;
    unsigned char maxHistoryCount;
    // bool blockCompleteRequest;
    QCompleter *comer;
    QStringListModel *comModel;

    QSlider *zoonSlider;
};

#endif /* _DIRNAVBAR_H_ */
