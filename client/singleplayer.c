#include "header.h"

// библиотеки для рисования интерфейса считывания клавиш
#include "../lib/graphics/myTerm.h"
#include "../lib/graphics/myBigChars.h"
#include "../lib/graphics/myReadKey.h"

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <pthread.h>

#include <signal.h> // для обработки сигнала прерывания

#include <sys/poll.h>

#include "../lib/message.h"

int gameField[3][3] = { { EMPTY, EMPTY, EMPTY }, 
                        { EMPTY, EMPTY, EMPTY },
                        { EMPTY, EMPTY, EMPTY } };

int previous_player_move = EMPTY;

pthread_mutex_t game_mutex;


void lock_gameField()
{
    pthread_mutex_lock(&game_mutex);
}

void unlock_gameField()
{
    pthread_mutex_unlock(&game_mutex);
}

void* ctlAiInteraction(void *arg)
{
    int chPlayer = CROSS;

    while(1) {
        sleep(1);
        try_to_make_move_ai(chPlayer);
        sleep(1);
    }

    return 0;
}

int gamePlay(int gameState[3][3])
{

    /*             case7               case8
                     \                   /
                      \   c4   c5  c6   /
                       \  ||   ||  ||  /
                        \ \/   \/  \/ /
        case 1 ----->    | x | o | x |
        case 2 ----->    | o | x | o |
        case 3 ----->    | o | x | x |
    */

    /*******************************/
    /* проверка 8 случаев для нуля */
    /*******************************/

    /* case  1 - 3 */
    if (gameState[0][0] == ZERO && gameState[0][1] == ZERO && gameState[0][2] == ZERO) {
        return ZERO;
    }
    if (gameState[1][0] == ZERO && gameState[1][1] == ZERO && gameState[1][2] == ZERO) {
        return ZERO;
    }
    if (gameState[2][0] == ZERO && gameState[2][1] == ZERO && gameState[2][2] == ZERO) {
        return ZERO;
    }

    /* case 4 - 6 */
    if (gameState[0][0] == ZERO && gameState[1][0] == ZERO && gameState[2][0] == ZERO) {
        return ZERO;
    }
    if (gameState[0][1] == ZERO && gameState[1][1] == ZERO && gameState[2][1] == ZERO) {
        return ZERO;
    }
    if (gameState[0][2] == ZERO && gameState[1][2] == ZERO && gameState[2][2] == ZERO) {
        return ZERO;
    }

    /* case 7 - 8 */
    if (gameState[0][0] == ZERO && gameState[1][1] == ZERO && gameState[2][2] == ZERO) {
        return ZERO;
    }
    if (gameState[0][2] == ZERO && gameState[1][1] == ZERO && gameState[2][0] == ZERO) {
        return ZERO;
    }


    /***********************************/
    /* проверка 8 случаев для крестика */
    /***********************************/

    /* case  1 - 3 */
    if (gameState[0][0] == CROSS && gameState[0][1] == CROSS && gameState[0][2] == CROSS) {
        return CROSS;
    }
    if (gameState[1][0] == CROSS && gameState[1][1] == CROSS && gameState[1][2] == CROSS) {
        return CROSS;
    }
    if (gameState[2][0] == CROSS && gameState[2][1] == CROSS && gameState[2][2] == CROSS) {
        return CROSS;
    }

    /* case 4 - 6 */
    if (gameState[0][0] == CROSS && gameState[1][0] == CROSS && gameState[2][0] == CROSS) {
        return CROSS;
    }
    if (gameState[0][1] == CROSS && gameState[1][1] == CROSS && gameState[2][1] == CROSS) {
        return CROSS;
    }
    if (gameState[0][2] == CROSS && gameState[1][2] == CROSS && gameState[2][2] == CROSS) {
        return CROSS;
    }

    /* case 7 - 8 */
    if (gameState[0][0] == CROSS && gameState[1][1] == CROSS && gameState[2][2] == CROSS) {
        return CROSS;
    }
    if (gameState[0][2] == CROSS && gameState[1][1] == CROSS && gameState[2][0] == CROSS) {
        return CROSS;
    }
    // проверка на ничью
    if (gameState[0][0] != EMPTY && gameState[0][1] != EMPTY && gameState[0][2] != EMPTY &&
        gameState[1][0] != EMPTY && gameState[1][1] != EMPTY && gameState[1][2] != EMPTY &&
        gameState[2][0] != EMPTY && gameState[2][1] != EMPTY && gameState[2][2] != EMPTY) {
        return DRAW;
    }

    return EMPTY;
}

int winGame(int winner)
{
    if (winner == EMPTY) {
        return 0;
    }

    mt_setstdcolor();
    mt_clrscr();
    mt_setbgcolor(clBlack);
    mt_setfgcolor(clGreen);
    mt_gotoXY(15, 17);

    switch(winner)
    {
        case ZERO:
            write(1, "WON ZEROS", 9);
            return 1;

        case CROSS:
            write(1, "WON CROSSES", 11);
            return 1;

        case DRAW:
            write(1, "PLAYED IN DRAW!", 15);
            return 1;
        default:
            return 0;
    }
    return 0;
}

// отрисовывет значение клеток и выделяет нужную
void drawGameField(int ROW, int COL)
{
    int big[2];

    /* позиция символа по Х */
    unsigned int posX;
    unsigned int i;

    /* 1 - нолик, 2 - крестик */
    for (i = 0, posX = 5; i < 3; i++, posX = posX + 15) {
        /* выделим нужную ячейку */
        if (ROW == 0 && COL == i) {
            if (gameField[0][i] == ZERO) {
                bc_setbigcharpos(big, 3, posX, 'o', clBlack, clGreen);
            }
            if (gameField[0][i] == CROSS) {
                bc_setbigcharpos(big, 3, posX, 'x', clBlack, clGreen);
            }
            if (gameField[0][i] == EMPTY) {
                bc_setbigcharpos(big, 3, posX, '*', clBlack, clGreen);
            }
        } else {

            if (gameField[0][i] == ZERO) {
                bc_setbigcharpos(big, 3, posX, 'o', clGreen, clBlack);
            }
            if (gameField[0][i] == CROSS) {
                bc_setbigcharpos(big, 3, posX, 'x', clGreen, clBlack);
            }
            /*****/
            if (gameField[0][i] == EMPTY) {
                bc_setbigcharpos(big, 3, posX, '*', clGreen, clBlack);
            }
        }
    }

    for (i = 0, posX = 5; i < 3; i++, posX = posX + 15) {
        /* выделим нужную ячейку */
        if (ROW == 1 && COL == i) {
            if (gameField[1][i] == ZERO) {
                bc_setbigcharpos(big, 13, posX, 'o', clBlack, clGreen);
            }
            if (gameField[1][i] == CROSS) {
                bc_setbigcharpos(big, 13, posX, 'x', clBlack, clGreen);
            }
            if (gameField[1][i] == EMPTY) {
                bc_setbigcharpos(big, 13, posX, '*', clBlack, clGreen);
            }
        } else {

            if (gameField[1][i] == ZERO) {
                bc_setbigcharpos(big, 13, posX, 'o', clGreen, clBlack);
            }
            if (gameField[1][i] == CROSS) {
                bc_setbigcharpos(big, 13, posX, 'x', clGreen, clBlack);
            }
            /*****/
            if (gameField[1][i] == EMPTY) {
                bc_setbigcharpos(big, 13, posX, '*', clGreen, clBlack);
            }
        }
    }

    for (i = 0, posX = 5; i < 3; i++, posX = posX + 15) {
        /* выделим нужную ячейку */
        if (ROW == 2 && COL == i) {
            if (gameField[2][i] == ZERO) {
                bc_setbigcharpos(big, 23, posX, 'o', clBlack, clGreen);
            }
            if (gameField[2][i] == CROSS) {
                bc_setbigcharpos(big, 23, posX, 'x', clBlack, clGreen);
            }
            if (gameField[2][i] == EMPTY) {
                bc_setbigcharpos(big, 23, posX, '*', clBlack, clGreen);
            }
        } else {
            if (gameField[2][i] == ZERO) {
                bc_setbigcharpos(big, 23, posX, 'o', clGreen, clBlack);
            }
            if (gameField[2][i] == CROSS) {
                bc_setbigcharpos(big, 23, posX, 'x', clGreen, clBlack);
            }
            /*****/
            if (gameField[2][i] == EMPTY) {
                bc_setbigcharpos(big, 23, posX, '*', clGreen, clBlack);
            }
        }
    }
}

int try_to_make_move(int rowField, int colField, int player)
{
    int isWin = 0;
    
    lock_gameField();

    if (gameField[rowField][colField] == EMPTY && previous_player_move != player) {
        previous_player_move = player;
        gameField[rowField][colField] = player; // ставим в массиве нолик
        drawGameField(rowField, colField);   // отрисовываем интерфейс заново
       
        // проверяем на выигрышную ситуацию (другой клиент проерит в своей потоке)
        isWin = winGame(gamePlay(gameField));

    } else if (gameField[rowField][colField] != EMPTY){
        mi_writeMessage("This place is taken!");
    } else if (previous_player_move == player) {
        mi_writeMessage("CROSS move!");
    }

    unlock_gameField();

    return isWin;
}

void try_to_make_move_ai(int player)
{
    lock_gameField();

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (gameField[i][j] == EMPTY && previous_player_move != player) {
                previous_player_move = player;
                gameField[i][j] = player; // ставим в массиве нолик
                drawGameField(i, j);      // отрисовываем интерфейс заново
                unlock_gameField();
                return;
            }
        }
    }
    unlock_gameField();
}

// main single player game routine
int single_game_session()
{
    int key;
    int isFinishedGame = 0;
    int chPlayer = ZERO;

    pthread_t ai_player;
    
    int rowField = 0;
    int colField = 0;

    char msgChat[250] = "";
    int len;

    // Draw main part of interface
    mi_drawMainInterface();
    drawGameField(0, 0);

    // Save terminal settings, change mode, hide cursor
    rk_mytermsave();
    rk_mytermregime(0, 0, 1, 0, 1);
    mt_setcursor(0);

    // Setup color palete
    mt_setbgcolor(clBlack);
    mt_setfgcolor(clGreen);

    int unused;
    pthread_create(&ai_player, 0, ctlAiInteraction, (void*)&unused);

    int whoWin;
    char msgSign[250];

    if (chPlayer == 1) {
        mi_writeMessage("You are playing ZEROS");
    } else if (chPlayer == 2) {
        mi_writeMessage("You are playing CROSSES");
    }

    while (isFinishedGame == 0 && rk_readkey(&key) == 0 && key != K_ESC) {
        switch (key) 
        {
            case K_R:
                // not supproted
                break;

            case K_T:
                // not supproted
                break;
                
            case K_UP:
                if (rowField != 0) {
                    rowField--;
                    drawGameField(rowField, colField);
                }
                break;
                
            case K_DOWN:
                if (rowField < 2) {
                    rowField++;
                    drawGameField(rowField, colField);
                }
                break;
                
            case K_LEFT:
                if (colField != 0) {
                    colField--;
                    drawGameField(rowField, colField);
                }
                break;
                
            case K_RIGHT:
                if (colField < 2) {
                    colField++;
                    drawGameField(rowField, colField);
                }
                break;
                
            case K_ENTER:
                if (chPlayer == 1) {
                    strcpy(msgSign,"ZEROS: ");
                } else if (chPlayer == 2) {
                    strcpy(msgSign, "CROSSES: ");
                }
                
                for (int i = 0; i < 250; i++) {
                    msgChat[i] = '\0';
                }
                
                bc_box(37, 1, 3, 101);
                mt_gotoXY(37, 2);
                write(1, " WRITE IN CHAT: ", 16);
                mt_gotoXY(38, 2);
                rk_mytermrestore();
                mt_setcursor(1);
                    
                len = read(0, msgChat, sizeof(msgChat));
                msgChat[len - 1] = '\0';
                if (len > 45) {
                    clearAreaEnterMsg(37, 1, 3, 101);
                    break;
                }
                
                strcat(msgSign, msgChat);
                displayMsgChat(msgSign);
                clearAreaEnterMsg(37, 1, 3, 101);
                rk_mytermregime(0, 0, 1, 0, 1);
                mt_setcursor(0);
                break;

            case K_F5: // place ZERO
                isFinishedGame = try_to_make_move(rowField, colField, chPlayer);
                break;
                
            case K_F6: // place CROSS
                isFinishedGame = try_to_make_move(rowField, colField, chPlayer);
                break;
        }
    }

    pthread_cancel(ai_player);

    rk_mytermrestore();
    mt_setcursor(1);
    mt_setstdcolor();
    mt_gotoXY(39, 1);

    return 0;
}