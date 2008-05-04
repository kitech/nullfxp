
#ifndef ENCRYPTIONDETAILFOCUSLABEL_H
#define ENCRYPTIONDETAILFOCUSLABEL_H


#include <QtCore>
#include <QtGui>

class EncryptionDetailFocusLabel : public QLabel
{
Q_OBJECT
  public:
  EncryptionDetailFocusLabel ( const QString & text, QWidget * parent = 0, Qt::WindowFlags f = 0 );
  ~EncryptionDetailFocusLabel();

  signals:
  void mouseDoubleClick();
  protected:
  virtual void mouseDoubleClickEvent ( QMouseEvent * event );
};

#endif
