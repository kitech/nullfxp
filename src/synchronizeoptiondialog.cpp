
#include "synchronizeoptiondialog.h"

SynchronizeOptionDialog::SynchronizeOptionDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    this->ui_dlg.setupUi(this);

    QObject::connect(this->ui_dlg.toolButton, SIGNAL(clicked()),
                     this, SLOT(slot_select_local_base_directory()));
}

SynchronizeOptionDialog::~SynchronizeOptionDialog()
{

}

void SynchronizeOptionDialog::slot_select_local_base_directory()
{
    QString dir;

    dir = QFileDialog::getExistingDirectory(this, tr("Select directory"), ".");
    if(!dir.isEmpty()) {
        this->ui_dlg.lineEdit->setText(dir);
    }
}


