#include "myReadKey.h"

char *key_sequences[SIZE_KEY_SEQUENCES];

int is_probably_key(char *sequence, int *key, int len)
{
    int i;

    if (sequence == NULL || key == NULL) {
        return -1;
    }
    key_sequences[K_UNKNOWN] = "";
    key_sequences[K_UP] = "\033[A";
    key_sequences[K_DOWN] = "\033[B";
    key_sequences[K_LEFT] = "\033[D";
    key_sequences[K_RIGHT] = "\033[C";
    key_sequences[K_F5] = "\033[15~";
    key_sequences[K_F6] = "\033[17~";
    key_sequences[K_ESC] = "\033\033";
    key_sequences[K_ENTER] = "\n";
    key_sequences[K_NUMUP] = "\033E0A";
    key_sequences[K_NUMDOWN] = "\033E0B";
    key_sequences[K_R] = "r";
    key_sequences[K_T] = "t";
    key_sequences[K_I] = "i";
    key_sequences[K_A] = "a";

    for (i = K_UNKNOWN; i < SIZE_KEY_SEQUENCES; i++) {
        if (strlen(key_sequences[i]) < len) {
            continue;
        }
        if (strncmp(sequence, key_sequences[i], len) == 0) {
            if (len < strlen (key_sequences[i])) {
                return 1;
            }
            *key = i;
            return 0;
        }
    }
    *key = K_UNKNOWN;
    return 0;
}

int rk_readkey(int *key)
{
    if (key == NULL) {
        return -1;
    }
    int len = 1;
    int res;
    char tmp_key[KEY_MAX_LEN];

    while (read(0, &(tmp_key[len - 1]), 1) > 0 && len <= KEY_MAX_LEN) {
        res = is_probably_key(tmp_key, key, len);
        if (res <= 0) {
            return res;
        }
        len ++;
    }
    return 0;
}

/* сохраняет текущие параметры терминала */
/*
tcflag_t c_iflag;      // режимы ввода с клавы
tcflag_t c_oflag;      // режимы вывода на экран
tcflag_t c_cflag;      // режимы передачи инфы в эвм
tcflag_t c_lflag;      // режимы дополнительные
cc_t c_cc[NCCS];       // управляющие символы
*/
int rk_mytermsave()
{
    if (tcgetattr(0, &setsave) == 0) { /* записываем в структуру значения атрибутов стандартного потока ввода */
        return 0;
    } else
        return -1;
}

/* восстанавливает сохраненные параметры терминала */
int rk_mytermrestore()
{
    if (tcsetattr(0, TCSANOW, &setsave) == 0) {
        return 0;
    } else
        return -1;
}

int rk_mytermregime (int canon, int vtime, int vmin, int echo, int sigint)
{
    struct termios setting;
    int result;
    if ((canon > 1) || (canon < 0)) {
        return -1;
    }
    if ((echo > 1) || (echo < 0) || (sigint > 1) || (sigint < 0) || (vtime < 0) || (vmin < 0)) {
        return -1;
    }
    result = tcgetattr (0, &setting); /* получаем значения атрибутов */
    if (result == -1) {
        return -1;
    }
    if (canon == 1) {
      setting.c_lflag |= ICANON;
    } else {
        setting.c_lflag &= ~ICANON;
        setting.c_lflag &= ~ECHO;
        setting.c_lflag &= ~ISIG;
        if(echo == 1) {
            setting.c_lflag |= ECHO;
        }
        if(sigint == 1) {
            setting.c_lflag |= ISIG;
        }
        setting.c_cc[VTIME] = vtime;
        setting.c_cc[VMIN] = vmin;
    }
    result = tcsetattr (0,TCSANOW,&setting); /* применить немедленно */
    if (result == -1) {
        return -1;
    }
    return 0;
}
