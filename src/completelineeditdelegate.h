/* CompleteLineEditDelegate.h --- 
 * 
 * Filename: CompleteLineEditDelegate.h
 * Description: 
 * Author: 刘光照<liuguangzhao@users.sf.net>
 * Maintainer: 
 * Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 六  7月 19 15:20:00 2008 (CST)
 * Version: 
 * Last-Updated: 
 *           By: 
 *     Update #: 0
 * URL: 
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change log:
 * 
 * 
 */

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
