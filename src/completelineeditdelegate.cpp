// CompleteLineEditDelegate.cpp --- 
// 
// Filename: CompleteLineEditDelegate.cpp
// Description: 
// Author: 刘光照<liuguangzhao@users.sf.net>
// Maintainer: 
// Copyright (C) 2007-2010 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 六  7月 19 15:24:13 2008 (CST)
// Version: 
// Last-Updated: 
//           By: 
//     Update #: 0
// URL: 
// Keywords: 
// Compatibility: 
// 
// 

// Commentary: 
// 
// 
// 
// 

// Change log:
// 
// 
// 

#include "utils.h"
#include "completelineeditdelegate.h"

CompleteLineEditDelegate::CompleteLineEditDelegate(QObject *parent)
    :QItemDelegate(parent)
{

}
CompleteLineEditDelegate::~CompleteLineEditDelegate()
{
}

QWidget *CompleteLineEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                      const QModelIndex &index) const
{
    QWidget *editor = 0;
    editor = new QLineEdit(parent);
    return editor;
}

void CompleteLineEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString data = index.data(Qt::DisplayRole).toString();
    QLineEdit *lineEditor = static_cast<QLineEdit*>(editor);
    lineEditor->setText(data);
}

void CompleteLineEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *lineEditor = static_cast<QLineEdit*>(editor);
    QString value = lineEditor->text();

    model->setData(index, value, Qt::EditRole);
}
