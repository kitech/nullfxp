// gccfeatures.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2009-10-18 09:07:27 +0800
// Version: $Id$
// 

#include <stdio.h>


#include "gccfeatures.h"

void gcc_feature_statements_and_declarations_in_expressions()
{
    int ia;
    ia = ({
        int y = 5;
        int z;
        if (y > 0) z = y;
        else z= -y;
        z;
    });
    printf("a=%d\n", ia);

    // for classes
    // 还可以在一个函数内定义一个类。
    class A {
    public:
        void  foo() {
            printf("it's A.foo()\n");
        }
    };
    
    A ca;
    ({ca;}).foo();

    int la,lb;
#define maxint_sd_express   ({ \
        int __return_value;   \
        if (la > lb) __return_value = la;  \
        else __return_value = lb;   \
        __return_value;  \
        });

    la = 6, lb = 3;
    int maxv = maxint_sd_express(la, lb);
    printf("maxint of %d,%d is %d\n", la, lb, maxv);
    
    // 这代码就够复杂了，如果不知道这个扩展，看着也头晕。
}

