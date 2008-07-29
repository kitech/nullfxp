#ifndef SYNCHRONIZEOPTIONDIALOG_H
#define SYNCHRONIZEOPTIONDIALOG_H


#include <QtGlobal>
#include <QtCore>
#include <QtGui>


#include "ui_synchronizeoptiondialog.h"

class SynchronizeOptionDialog : public QDialog
{
    Q_OBJECT;
public:
    SynchronizeOptionDialog(QWidget *parent = 0,Qt::WindowFlags flags = 0);
    ~SynchronizeOptionDialog();

private:
    Ui::SynchronizeOptionDialog ui_dlg;

private slots:
    void slot_select_local_base_directory();

};

#endif
