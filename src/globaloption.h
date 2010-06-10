/* globaloption.h --- 
 * 
 * Author: liuguangzhao
 * Copyright (C) 2007-2010 liuguangzhao@users.sf.net
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


class QTextCodec;

class GlobalOption 
{
public:
    static GlobalOption *instance();
    ~GlobalOption();

    enum {FLV_DETAIL, FLV_LIST, FLV_SMALL_ICON, FLV_LARGE_ICON, FLV_BOTH_VIEW};
    
    //options
    QTextCodec *remote_codec ;
    QTextCodec *locale_codec ;
	QTextCodec *test_codec ;
    bool        keep_alive ;
    bool        kepp_alive_internal ;
    unsigned char file_list_view_mode;

    void set_remote_codec(const char *codes);
    
private:
    GlobalOption();
    static GlobalOption *mInstance;
    
};

///////////////
extern GlobalOption * gOpt;

#endif
