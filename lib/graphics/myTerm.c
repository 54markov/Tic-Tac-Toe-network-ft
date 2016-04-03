#include "myTerm.h"

/* очистка экрана и курсор вверх, влево */
int mt_clrscr()
{
    int tty; /* хранит номер дескриптора */
    tty = open("/dev/tty", O_RDWR); /* открытие файла терминала в режиме чтение/запись */
    if (tty == -1) { /* проверка на открытие */
        fprintf(stderr, "\nmt_clrscr()\n: Can not open tty\n");
        close(tty);
        return -1;
    } else {
        /* отправляем в терминал esc-последовательность */
        write(tty, "\033[H\033[J", 6);
        close(tty);
        return 0;
    }
}

/* координаты курсора */
int mt_gotoXY (int row, int column)
{    int len;
    int tty; /* хранит номер дескриптора */
    tty = open("/dev/tty", O_RDWR);
    if (tty == -1) { /* проверка на открытие */
        fprintf(stderr, "\nmt_gotoXY()\n: Can not open tty\n");
           close(tty);
           return -1; /* ошибка открытия файла терминала */
      } else {
          if ((row > -1 && row <= TERM_ROW) && (column > -1 && column <= TERM_COL)) {
            char str[20];
            /* собираем последовательность в масс вместе с аргументами */
            len = sprintf(str, "\033[%d;%dH", row, column);
            /* запись в файл терминала */
            write(tty, str, len);
            close(tty);
            return 0; /* успешное завершение */
        } else {
              fprintf(stderr, "\nmt_gotoXY(int, int)\n: Incorrect row or column value\n");
            close (tty);
            return 1; /* некорректные входные данные */
        }
    }
}

int mt_getscreensize(int *row, int *column)
{
    struct winsize ws;
    if (ioctl(1, TIOCGWINSZ, &ws) == 0) {
        *row = ws.ws_row;
        *column = ws.ws_col;
        return 0;
    } else {
        fprintf(stderr, "\nmt_getscreensize(int, int)\n: Can not get tty size\n");
        return -1;
    }
}

/* устанавливает цвет текста */
int mt_setfgcolor(int color)
{
      int tty, len;
      if (color < 0 && color > 7) {
      fprintf(stderr, "\nmt_setfgcolor(int)\n: Incorrect text color\n");
      return -1;
      }

      tty = open("/dev/tty", O_RDWR);
      if (tty == -1) {
        fprintf(stderr, "\nmt_setfgcolor(int)\n: Can not open TTY\n");
        close(tty);
        return -1;
      } else {
    /* составление esc-пос. "\E[34m" */
    char str[20];
    /* собираем последовательность в масс вместе с аргументами */
    len = sprintf(str, "\033[3%dm", color);
    /* запись в файл терминала */
    write(tty, str, len);
    write(tty, "\033[1m", 4);
    close(tty);
    return 0;
  }
}

int mt_setbgcolor(int color)
{
      int tty, len;
      if (color < 0 && color > 7) {
      fprintf(stderr, "\nmt_setfgcolor(int)\n: Incorrect text color\n");
      return -1;
      }

      tty = open("/dev/tty", O_RDWR);
      if (tty == -1) {
        fprintf(stderr, "\nmt_setfgcolor(int)\n: Can not open TTY\n");
        close(tty);
        return -1;
      } else {
    /* составление esc-пос.  */
    char str[20];
    /* собираем последовательность в масс вместе с аргументами */
    len = sprintf(str, "\033[4%dm", color);
    /* запись в файл терминала */
    write(tty, str, len);
    close(tty);
    return 0;
  }
}

int mt_setstdcolor()
{
    int tty;
       tty = open("/dev/tty", O_RDWR);
      if (tty == -1) {
        fprintf(stderr, "\nmt_setfgcolor(int)\n: Can not open TTY\n");
        close(tty);
        return -1;
      } else {
        write(tty,"\033[0m",5);
        return 0;
    }
}

int mt_setcursor (int visible)
{
    int tty;
    tty = open("/dev/tty", O_RDWR);
    if (tty == -1) {
        fprintf(stderr, "\nmt_setcursor(int)\n: Can not open TTY\n");
        close(tty);
        return -1;
    }
    char * buf;
    if (visible == 1) {
        buf = "\033[?12;25h";
    } else {
        buf = "\033[?25l";
    }
    if (write (tty, buf, strlen(buf)) != strlen (buf)) {
        return -1;
    }
    return 0;
}
