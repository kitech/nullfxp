/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   gzl@localhost   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef GLOBALOPTION_H
#define GLOBALOPTION_H

#include <stdlib.h>
#include <string.h>
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
    
    void set_remote_codec( const char * codes );
    
    private:
        GlobalOption();
        static GlobalOption * mInstance ;
        
};

///////////////全局变量
extern GlobalOption * global_option ;

#endif
