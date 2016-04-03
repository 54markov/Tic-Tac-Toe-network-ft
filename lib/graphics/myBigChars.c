#include "myBigChars.h"

/* вывод строки символов с использованием дополнительной таблицы кодировок */
int bc_printA(char *str)
{
    int tty;
    tty = open("/dev/tty", O_RDWR);
    if (tty == -1) {
        fprintf(stderr, "\nbc_printA(char *)\n: Can not open tty\n");
        close(tty);
        return -1;
    } else {
        write(tty, "\033(0", 3);
        write(tty, str, strlen(str));
        write(tty, "\033(B", 3);
    }
    close(tty);
    return 0;
}

/* рамка x1-y1 - левый верхний угол, x2-y2 - высота, длина */
int bc_box(int x1, int y1, int x2, int y2)
{
    int tty, i, j;
    tty = open("/dev/tty", O_RDWR);
    if (tty == -1) {
        fprintf(stderr, "\nbc_box(int, int, int, int)\n: Can not open tty\n");
        close(tty);
        return -1;
    } else {
        for (i = 0; i < x2; i++) {
            mt_gotoXY(x1 + i, y1);
            if (i == 0) {
                bc_printA("l"); /* левый верхний угол */
                for (j = 2; j < y2; j++) {
                    bc_printA("q"); /* горизонталь */
                }
                bc_printA("k"); /* правый верхний угол */
            }
            if (i != 0 && i != x2 - 1) { /* если не начало и не конец, то пробелы */
                bc_printA("x"); /* левая вертикаль */
                for (j = 2; j < y2; j++) {
                    bc_printA(" ");
                }
                bc_printA("x"); /* правая вертикаль */
            }
            if (i == x2 - 1) {
                bc_printA("m");/* левый нижний угол */
                for (j = 2; j < y2; j++) {
                    bc_printA("q"); /* горизонталь */
                }
                bc_printA("j"); /* правый нижний угол */
            }
        }
        close(tty);
        return 0;
    }
}

/* вывод большого символа на экран */
int bc_printbigchar(int mas[], int x, int y, int color_fg, int color_bg)
{
    int tty, i, j;
    tty = open("/dev/tty", O_RDWR);
    if (tty == -1) {
        fprintf(stderr, "\nbc_printbigchar(int *, int, int, int, int)\n: Can not open tty\n");
        close(tty);
        return -1;
    } else {
        mt_setfgcolor(color_fg);
        mt_setbgcolor(color_bg);
        mt_gotoXY(x, y);
        for (i = 0; i < 2; i++) {

            for(j = 0; j < 32; j++) {
                 if (j % 8 == 0) {
                    mt_gotoXY(x++, y);
                }
                if ((mas[i] >> j) & 0x1) {
                    bc_printA("a");
                } else {
                    bc_printA(" ");

                }
            }
        }
    }
    close(tty);
    return 0;
}

/* вывод определеннго символа в нужном месте */
int bc_setbigcharpos(int *big, int x, int y, char value, int color_fg, int color_bg)
{
    int tty;
    tty = open("/dev/tty", O_RDWR);
    if (tty == -1) {
        fprintf(stderr, "\nbc_setbigcharpos(int *, int, int, int *)\n: Can not open tty\n");
        close(tty);
        return -1;
    } else {
        if (mt_gotoXY(x, y) != -1) {
            switch(value) {

                case 'x':
                    big[0] = 1013367747;//1013367681;
                    big[1] = 3284362812;//2177066556;
                    break;
                case 'o':
                    big[0] = 3284386620;//3284362812;
                    big[1] = 1019462595;//1013367747;
                    break;
                case '*':
                    big[0] = 0;
                    big[1] = 0;
                break;
            }
        }
        /*if (big[0] == 0 && big[1] == 0) {
            fprintf(stderr, "\nbc_setbigcharpos(int *, int, int, int)\n: Incorrect value\n");
            close(tty);
            return -1;
        }*/
        bc_printbigchar(big, x, y, color_fg, color_bg);
    }
    close(tty);
  return 0;
}

int bc_getbigcharpos(int *big, int x, int y, int *value)
{
    int tty;
    int pos = 0;
    tty = open("/dev/tty", O_RDWR);
    if (tty == -1) {
        fprintf(stderr, "\nbc_setbigcharpos(int *, int, int, int *)\n: Can not open tty\n");
        close(tty);
        return -1;
    } else {
        if (x > 4) {
            pos = 1;
        }
    }
    x--;
    y--;
    *value = (big[pos] >> (x * 8 + y)) & 0x1;
    close(tty);
    return 0;
}

int bc_bigcharwrite (int fd, int * big, int count)
{
    int write_bytes;
    if ( fd < 0) {
        fprintf(stderr, "\nbc_bigcharwrite(int, int*, int)\n: Incorrect fd value\n");
        return -1;
    } else if (count < 1) {
        fprintf(stderr, "\nbc_bigcharwrite(int, int*, int)\n: Incorrect count value\n");
        return -1;
    } else {
       write_bytes = write(fd, (void *)big, sizeof(int) * 2 * count);
    }
    if (write_bytes != sizeof(int) * 2 * count) {
        fprintf(stderr, "\nbc_bigcharwrite(int, int*, int)\n: Incorrect write\n");
        return -1;
    } else {
        return 0;
    }
}

int bc_bigcharread (int fd, int * big, int need_count, int * count)
{
   int read_bytes;
    if ( fd < 0) {
        fprintf(stderr, "\nbc_bigcharread(int, int*, int, int*)\n: Incorrect fd value\n");
        return -1;
    } else if (need_count < 1) {
        fprintf(stderr, "\nbc_bigcharread(int, int*, int, int*)\n: Incorrect count value\n");
        return -1;
    } else {
        *count = 0;
        read_bytes = read(fd, (void *)big, sizeof(int) * 2 * need_count);
        if (read_bytes != need_count) {
            fprintf(stderr, "\nbc_bigcharread(int, int*, int, int*)\n: Incorrect read\n");
            return -1;
        }
        *count = read_bytes / sizeof(int) / 2;
    }
    return 0;
}
