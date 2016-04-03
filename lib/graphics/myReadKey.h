#ifndef MY_READKEY_H
#define MY_READKEY_H

#include <termios.h>
#include <string.h>
#include <unistd.h>

#define KEY_MAX_LEN 20
#define SIZE_KEY_SEQUENCES 15

/* поддерживаемые клавиши */
enum keys { K_UNKNOWN, K_UP, K_DOWN, K_LEFT, K_RIGHT, K_F5, K_F6, K_ESC, K_ENTER, K_NUMUP, K_NUMDOWN , K_R , K_T, K_I, K_A };

struct termios setsave;

int rk_readkey(int *key);
int rk_mytermsave();
int rk_mytermrestore();
int rk_mytermregime (int canon, int vtime, int vmin, int echo, int sigint);

#endif
