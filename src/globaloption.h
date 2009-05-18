/* globaloption.h --- 
 * 
 * Author: liuguangzhao
 * Copyright (C) 2007-2010 liuguangzhao@users.sf.net
 * URL: http://www.qtchina.net http://nullget.sourceforge.net
 * Created: 2007-05-17 11:04:33 +0800
 * Last-Updated: 2009-05-17 11:05:18 +0800
 * Version: $Id$
 */

#ifndef GLOBALOPTION_H
#define GLOBALOPTION_H

#include <cstdlib>
#include <cstring>
#include <vector>
#include <map>
#include <string>


class QTextCodec ;

/**
	@author liuguangzhao <gzl@localhost>
*/
class GlobalOption{
public:
    static GlobalOption * instance();

    ~GlobalOption();
    
    //options
    //std::string  remote_codec ;
    //std::string  locale_codec ;
    QTextCodec * remote_codec ;
    QTextCodec * locale_codec ;
	QTextCodec * test_codec ;
    bool         keep_alive ;
    bool         kepp_alive_internal ;
    
    void set_remote_codec( const char * codes );
    
    private:
        GlobalOption();
        static GlobalOption * mInstance ;
        
};

///////////////
extern GlobalOption * gOpt;

#endif
