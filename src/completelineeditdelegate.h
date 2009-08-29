// completelineeditdelegate.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-07-19 15:20:00 +0800
// Version: $Id$
// 

#ifndef COMPLETELINEEDITDELEGATE_H
#define COMPLETELINEEDITDELEGATE_H

#include <QtGlobal>
#include <QtCore>
#include <QtGui>

class CompleteLineEditDelegate: public QItemDelegate
{
    Q_OBJECT;
public:
    CompleteLineEditDelegate(QObject *parent = 0);
    ~CompleteLineEditDelegate();

    //impl
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel * model, const QModelIndex &index) const;

};

#endif
