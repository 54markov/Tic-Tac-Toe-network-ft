#ifndef MY_BIGCHARS_H
#define MY_BIGCHARS_H

#include <stdio.h>
#include <string.h>
#include <unistd.h> /* для функций open и write */
#include <fcntl.h> /* режимы открытия файлов */
#include "myTerm.h" /* нужно для устанвоки курсора */


int bc_printA(char *str);
int bc_box(int x1, int y1, int x2, int y2);
int bc_printbigchar(int mas[], int x, int y, int color_fg, int color_bg);
int bc_setbigcharpos(int *big, int x, int y, char value, int color_fg, int color_bg);
int bc_getbigcharpos(int *big, int x, int y, int *value);
int bc_bigcharwrite (int fd, int * big, int count);
int bc_bigcharread (int fd, int * big, int need_count, int * count);

#endif
