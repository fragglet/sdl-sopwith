/*

                        Copyright (C) 1984-2000 David L. Clark.

                        All rights reserved except as specified in the
                        file license.txt.  Distribution of this file
                        without the license.txt file accompanying is
                        prohibited.

*/

#ifndef __STD_H__
#define __STD_H__

#include <stdio.h>
#include <stdlib.h>

#define NULL     ((void *)0)
#define FALSE    0
#define TRUE     1
#define NO       0
#define YES      1
#define FOREVER  for (;;)

int strindex(char *str,int c);
void movblock(unsigned int srcoff,unsigned int srcseg,
              unsigned int destoff,unsigned int destseg,
              unsigned int count);
void movmem(void *src,void *dest,unsigned count);
void setmem(void *dest,unsigned count,int c);

#endif
