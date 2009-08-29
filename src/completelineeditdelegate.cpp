// completelineeditdelegate.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-07-19 15:24:13 +0800
// Version: $Id$
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
