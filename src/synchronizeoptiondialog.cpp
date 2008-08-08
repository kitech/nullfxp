
#include "basestorage.h"
#include "synchronizeoptiondialog.h"

SynchronizeOptionDialog::SynchronizeOptionDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    this->ui_dlg.setupUi(this);

    QObject::connect(this->ui_dlg.toolButton, SIGNAL(clicked()),
                     this, SLOT(slot_select_local_base_directory()));

    QObject::connect(this->ui_dlg.toolButton_2, SIGNAL(clicked()),
                     this, SLOT(slot_show_session_list()));
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

void SynchronizeOptionDialog::slot_show_session_list()
{
    QMenu * popmenu = new QMenu(this);
    QAction *action;

    BaseStorage * storage = BaseStorage::instance();
    storage->open();
    
    QStringList nlist = storage->getNameList();

    for(int i = 0; i< nlist.count(); i++) {
        action = new QAction(nlist.at(i), this);
        QObject::connect(action, SIGNAL(triggered()), 
                         this, SLOT(slot_session_item_selected()));
        popmenu->addAction(action);        
    }

    QPoint pos = this->ui_dlg.toolButton_2->pos();
    pos = this->mapToGlobal(pos);
    pos.setX(pos.x() + 35);
    popmenu->popup(pos);
}

void SynchronizeOptionDialog::slot_session_item_selected()
{
    QAction *a = static_cast<QAction *>(sender());
    this->ui_dlg.lineEdit_2->setText(a->text());
}
