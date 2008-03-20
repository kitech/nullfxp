
#ifndef LUNAR_H
#define LUNAR_H

#include <stdio.h>
#include <stdlib.h>

int lunar_cacl(int argc, char ** argv, char ** cacl_res);

/**********
 * file: lunar_test.c
 * ./mylunar -h --utf8 2008 3 20 

Lunar Version 2.2 (October 28, 2001)

我的日历：
阳历：　2008年 3月20日 0时　星期四
阴历：　2008年 2月13日子时　生肖属鼠
干支：　戊子年　乙卯月　己未日　甲子时　
用四柱神算推算之时辰八字：　戊子年　乙卯月　己未日　甲子时　

 *
 *

#include "lunar.h"

int main(int argc, char ** argv)
{
  char * out_res = 0;

  lunar_cacl(argc, argv, &out_res);
  printf("我的日历：\n%s\n", out_res);
  free(out_res);

  return 0;
}

*/
#endif
