#!/bin/bash

# tiny_mime.sh --- 
# 
# Author: liuguangzhao
# Copyright (C) 2007-2010 liuguangzhao@users.sf.net
# URL: http://www.qtchina.net http://nullget.sourceforge.net
# Created: 2009-08-30 09:02:54 +0800
# Version: $Id$
# 

# 根据标准的freedesktop.org.xml生成一个简单的mimetypes对照表。
# usage: tiny_mime.sh > xxx.cpp
# freedesktop.org.xml 在linux系统的/usr/share/mime/packages/目录下,用这个即可.

##########
function out_line()
{
    echo $1;
}
function out_header()
{
    # echo "header now";
    dd=`date`;
    out_line "// $dd";
    out_line "#include <QHash>";
    out_line ;
    out_line "QHash<QString, QString> gMimeHash;";
    out_line ;
    out_line "static int mimeHashInit() {";
}
function out_extra_pairs()
{
    echo "    gMimeHash.insertMulti(\"c\", \"text-x-csrc\");";
    echo "    gMimeHash.insertMulti(\"cc\", \"text-x-c++src\");";
    echo "    gMimeHash.insertMulti(\"cpp\", \"text-x-c++src\");";
    echo "    gMimeHash.insertMulti(\"c++\", \"text-x-c++src\");";
    echo "    gMimeHash.insertMulti(\"cxx\", \"text-x-c++src\");";
    echo "    gMimeHash.insertMulti(\"h\", \"text-x-chdr\");";
    echo "    gMimeHash.insertMulti(\"hpp\", \"text-x-c++hdr\");";
    echo "    gMimeHash.insertMulti(\"hxx\", \"text-x-c++hdr\");";

    echo "    gMimeHash.insertMulti(\"jpg\", \"image-x-generic\");";
    echo "    gMimeHash.insertMulti(\"jpeg\", \"image-x-generic\");";
    echo "    gMimeHash.insertMulti(\"ico\", \"image-x-generic\");";
    echo "    gMimeHash.insertMulti(\"png\", \"image-x-generic\");";
    echo "    gMimeHash.insertMulti(\"gif\", \"image-x-generic\");";
    echo "    gMimeHash.insertMulti(\"bmp\", \"image-x-generic\");";

    echo "    gMimeHash.insertMulti(\"htm\", \"text-html\");";
    echo "    gMimeHash.insertMulti(\"html\", \"text-html\");";

    echo "    gMimeHash.insertMulti(\"wmv\", \"video-x-generic\");";
    echo "    gMimeHash.insertMulti(\"avi\", \"video-x-generic\");";
    echo "    gMimeHash.insertMulti(\"asf\", \"video-x-generic\");";
    echo "    gMimeHash.insertMulti(\"mpg\", \"video-x-generic\");";
    echo "    gMimeHash.insertMulti(\"mpeg\", \"video-x-generic\");";
    echo "    gMimeHash.insertMulti(\"m2v\", \"video-x-generic\");";
    echo "    gMimeHash.insertMulti(\"mp3\", \"video-x-generic\");";

    echo "    gMimeHash.insertMulti(\"iso\", \"application-x-cd-image\");";
    echo "    gMimeHash.insertMulti(\"ppt\", \"application-vnd.ms-powerpoint\");";
    echo "    gMimeHash.insertMulti(\"dmg\", \"application-x-dmg\");";
}

function out_footer()
{
    # echo "footer now";
    out_extra_pairs;

    echo "    return 0;";
    out_line "}";
    out_line "static int mimeHashInited = mimeHashInit();";
    echo ;
}


#############
out_header;

### parse it
MIME_XML=src/data/freedesktop.org.xml
while read mline
do
    # echo $mline;
    sline=`echo $mline | grep \<mime-type`
    # echo $sline;
    eline=`echo $mline | grep \</mime-type`
    gline=`echo $mline | grep \<glob`

    # echo $eline;
    if [ x"$sline" != x"" ] ; then
        # echo "mime-type begin";
        save_sline=$sline;
    fi

    if [ x"$gline" != x"" ] ; then
        # echo "mime-type begin";
        save_gline=$gline;
    fi

    if [ x"$eline" != x"" ] ; then
        # echo "mime-type end of $save_sline $save_gline";

        ###### got it
        suffix=`echo $save_gline | awk -F\" '{print $2}' | awk -F\. '{print $2}'`
        # echo $suffix;
        mtype=`echo $save_sline | awk -F\" '{print $2}' | awk -F/ '{print $1 "-" $2}'`
        # echo $mtype;
        echo "    if (!gMimeHash.contains(\"$suffix\")) gMimeHash.insert(\"$suffix\", \"$mtype\");";
    fi   

done < $MIME_XML

#######
out_footer;

