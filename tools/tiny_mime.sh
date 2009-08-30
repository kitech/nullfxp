#!/bin/sh

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
    echo "    gMimeHash.insert(\"c\", \"text-x-csrc\");";
    echo "    gMimeHash.insert(\"cc\", \"text-x-c++src\");";
    echo "    gMimeHash.insert(\"cpp\", \"text-x-c++src\");";
    echo "    gMimeHash.insert(\"c++\", \"text-x-c++src\");";
    echo "    gMimeHash.insert(\"cxx\", \"text-x-c++src\");";
    echo "    gMimeHash.insert(\"h\", \"text-x-chdr\");";
    echo "    gMimeHash.insert(\"hpp\", \"text-x-c++hdr\");";
    echo "    gMimeHash.insert(\"hxx\", \"text-x-c++hdr\");";

    echo "    gMimeHash.insert(\"jpg\", \"image-x-generic\");";
    echo "    gMimeHash.insert(\"jpeg\", \"image-x-generic\");";
    echo "    gMimeHash.insert(\"ico\", \"image-x-generic\");";
    echo "    gMimeHash.insert(\"png\", \"image-x-generic\");";
    echo "    gMimeHash.insert(\"gif\", \"image-x-generic\");";
    echo "    gMimeHash.insert(\"bmp\", \"image-x-generic\");";

    echo "    gMimeHash.insert(\"htm\", \"text-html\");";
    echo "    gMimeHash.insert(\"html\", \"text-html\");";
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
        echo "    gMimeHash.insert(\"$suffix\", \"$mtype\");";
    fi   

done < $MIME_XML

#######
out_footer;

