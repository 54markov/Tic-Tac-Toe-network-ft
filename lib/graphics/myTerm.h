#ifndef MY_TERM_H
#define MY_TERM_H

#include <stdio.h>
#include <unistd.h>     /* для функций open и write */
#include <fcntl.h>      /* режимы открытия файлов */
#include <sys/ioctl.h>  /* функция управления файлами устройств */
#include <termios.h>    /* функции для работы с терминалом */
#include <string.h>

/* размер консоли */
#define TERM_ROW 44/* строки */
#define TERM_COL 168 /* столбцы */

/* цвета для консоли */
enum colors { clBlack, clRed, clGreen, clYellow, clBlue, clPurple, clAqua, clWhite };

int mt_clrscr();
int mt_gotoXY (int row, int column);
int mt_getscreensize(int *row, int *column);
int mt_setfgcolor(int color);
int mt_setbgcolor(int color);
int mt_setstdcolor();
int mt_setcursor (int visible);

#endif
