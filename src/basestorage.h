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
    static BaseStorage * instance();
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
    QMap<QString,QString> & getHost(QString show_name);
    int hostCount();
  
signals:
    void hostListChanged();
    void hostLIstChanged(QString show_name);
private:
    BaseStorage();

    static BaseStorage * mInstance;
    QMap<QString,QMap<QString,QString> >  hosts;
    QVector<QMap<QString, QString> > vec_hosts;
    bool opened;
    bool changed;
    QDataStream  ioStream;

    enum {PROTO_NONE, PROTO_SFTP, PROTO_FTP};
    enum {LTYPE_ALL = 0x01|0x02|0x04|0x08, LTYPE_PASSWD=0x01, LTYPE_PKEY = 0x02, 
          LTYPE_KBI = 0x04, LTYPE_GASSAPI = 0x08};

    //host 结点结构
    class hnode{
    public:
        int hid;
        int ptype; //协议类型
        int ltype; //登陆类型
        short port;
        short passive;
        char *nname; //node name, or show name
        char *hname; //host name
        char *uname; // user name , login name
        char *passwd; // 
        char *pkeypath; //登陆密钥路径
    };
    QVector< hnode*>  hv;

    QMap<QString, QString> node_to_map( hnode *node);
    hnode * map_to_node(QMap<QString, QString> map);
    int generate_hid();
};


/* basestorage.h ends here */
