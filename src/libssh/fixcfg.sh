#!/bin/sh


# ls *.c
for f in `ls libssh/*.c`
do
    echo $f
    HAS_CONFIG_H_LINE=`cat $f|grep \"config.h\"`
    #echo $HAS_CONFIG_H_LINE
    if [ x"$HAS_CONFIG_H_LINE" = x"" ] ; then
        echo "no config.h line in $f";
    else
        echo "has config.h line in $f";
        cat $f | sed -e 's/\"config.h\"/\"libssh_config.h\"/g' > /tmp/ab.c
        cp -v /tmp/ab.c $f
        echo "Replace done $f";
        # exit;
    fi
done

exit;


for f in `ls include/libssh/*.h`
do
    #echo $f
    HAS_CONFIG_H_LINE=`cat $f|grep \"config.h\"`
    #echo $HAS_CONFIG_H_LINE
    if [ x"$HAS_CONFIG_H_LINE" = x"" ] ; then
        echo "no config.h line in $f";
    else
        echo "has config.h line in $f";
        cat $f | sed -e 's/\"config.h\"/\"libssh_config.h\"/g' > /tmp/ab.c
        cp -v /tmp/ab.c $f
        echo "Replace done $f";
    fi
done

rm -vf /tmp/ab.c

