
#include "encryptiondetailfocuslabel.h"




















EncryptionDetailFocusLabel::EncryptionDetailFocusLabel ( const QString & text, QWidget * parent , Qt::WindowFlags f  )
  :QLabel(text, parent, f)
{

}
EncryptionDetailFocusLabel::~EncryptionDetailFocusLabel()
{

}

void EncryptionDetailFocusLabel::mouseDoubleClickEvent ( QMouseEvent * event )
{
  emit this->mouseDoubleClick();
  QLabel::mouseDoubleClickEvent(event);
}

