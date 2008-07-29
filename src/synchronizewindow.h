#ifndef SYNCHRONIZEWINDOW_H
#define SYNCHRONIZEWINDOW_H

#include <QtGlobal>
#include <QtCore>
#include <QtGui>

#include "ui_synchronizewindow.h"

class SynchronizeWindow : public QWidget
{
    Q_OBJECT;
public:
    SynchronizeWindow(QWidget *parent = 0);
    ~SynchronizeWindow();

private:
    Ui::SynchronizeWindow  ui_win;
};

#endif
