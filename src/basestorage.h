/* basestorage.h --- 
 * 
 * Filename: basestorage.h
 * Description: 
 * Author: liuguangzhao
 * Maintainer: 
 * Created: 五  4月  4 14:46:49 2008 (CST)
 * Version: 
 * Last-Updated: 五  4月  4 14:47:37 2008 (CST)
 *           By: liuguangzhao
 *     Update #: 1
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

/* Code: */




#include <QtCore>
#include <QDataStream>

class BaseStorage
{
public:
    BaseStorage();
    ~BaseStorage();
    bool open();
    bool close();
    bool save();

    bool addHost(QMap<QString,QString> host);
    bool removeHost(QString show_name);
    bool updateHost(QMap<QString,QString> host);
    bool clearHost();

    bool containsHost(QString show_name);

    QMap<QString, QMap<QString,QString> > & getAllHost();
    QMap<QString,QString> getHost(QString show_name);
    int hostCount();
  
signals:
    void hostListChanged();
    void hostLIstChanged(QString show_name);
private:
    QMap<QString,QMap<QString,QString> >  hosts;
    bool opened;
    bool changed;
    QDataStream  ioStream;

};

/* basestorage.h ends here */
