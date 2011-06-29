/* globaloption.h --- 
 * 
 * Author: liuguangzhao
 * Copyright (C) 2007-2012 liuguangzhao@users.sf.net
 * URL: http://www.qtchina.net http://nullget.sourceforge.net
 * Created: 2007-05-17 11:04:33 +0800
 * Version: $Id$
 */

#ifndef GLOBALOPTION_H
#define GLOBALOPTION_H

#include <cstdlib>
#include <cstring>
#include <vector>
#include <map>
#include <string>
#include <QObject>


class QTextCodec;

// add realtime signal option change feature
class GlobalOption : public QObject
{
    Q_OBJECT;
public:
    static GlobalOption *instance();
    virtual ~GlobalOption();

    enum {FLV_DETAIL, FLV_LIST, FLV_SMALL_ICON, FLV_LARGE_ICON, FLV_BOTH_VIEW};
    
    //options
    QTextCodec *remote_codec;
    QTextCodec *locale_codec;
	QTextCodec *test_codec;
    bool        keep_alive;
    int        kepp_alive_internal;
    unsigned char file_list_view_mode;

    void set_remote_codec(const char *codes);
    
private:
    GlobalOption(QObject *parent = 0);
    static GlobalOption *mInstance;
    
};

///////////////
extern GlobalOption * gOpt;

#endif
