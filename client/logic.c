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

#include <signal.h> // для обработки сигнала прерывания

#include <sys/poll.h>

#include "../lib/message.h"

extern int GAME_ROOM;
extern int PLAYER;
extern int SERVER_FD;
int gameState[3][3]= {{0}};

int rowField;
int colField;

// хранит актуальное значение конца массива "архив"
int scrollRow;

// для хранения истории чата и вывода
char archive[1000][100];
int rangeBegin;
int rangeEnd;

// кто ходил последний
int previousMove = 1;

int isFinishedGame = 0;

int chatCurrentRow = 2; // текущая строка в чате

// отрисовка основного интерфейса
int mi_drawMainInterface()
{
    int tty; /* хранит номер дескриптора */
    tty = open("/dev/tty", O_RDWR); /* открытие файла терминала в режиме чтение/запись */
    if (tty == -1) { /* проверка на открытие */
        fprintf(stderr, "\nmi_drawMainInterface()\n: Can not open tty\n");
        close(tty);
        return -1;
    }
    mt_clrscr();
    mt_setbgcolor(clBlack);
    mt_setfgcolor(clGreen);

    /* рамки */
    /* 1 параметр - начало по y, 2 параметр - начало по x, 3 параметр - ширина по у, 4 параметр - ширина по х */
    bc_box(1, 1, 32, 46); /* рамка для игрового поля */

    /* рамки для знакомест первый ряд */
    bc_box(2, 3, 10, 12);
    bc_box(12, 3, 10, 12);
    bc_box(22, 3, 10, 12);

    /* рамки для знакомест второй ряд */
    bc_box(2, 18, 10, 12);
    bc_box(12, 18, 10, 12);
    bc_box(22, 18, 10, 12);

    /* рамки для знакомест третий ряд */
    bc_box(2, 33, 10, 12);
    bc_box(12, 33, 10, 12);
    bc_box(22, 33, 10, 12);

    /* рамка для сообщений игры */
    bc_box(33, 1, 4, 39);

    /* рамка для чата */
    bc_box(1, 47, 32, 55);
    /* рамка помощи */
    bc_box(33, 40, 4, 62);

    /* подпись игрового поля */
    mt_gotoXY(1, 17);
    write(tty, " PLAYING FIELD ", 15);

    /* подпись чата */
    mt_gotoXY(1, 69);
    write(tty, " CHAT ", 6);

    /* подпись области сообщений игры */
    mt_gotoXY(33, 11);
    write(tty, " MESSAGE FROM GAME ", 19);

    /* подпись области помощи */
    mt_gotoXY(33, 67);
    write(tty, " HELP ", 6);

    /* в рамке помощи */
    mt_gotoXY(34, 45);
    write(tty, "F5 - put zero", 13);
    mt_gotoXY(35, 45);
    write(tty, "Enter - write in chat", 21);

    mt_gotoXY(34, 78);
    write(tty, "F6 - put cross", 14);
    mt_gotoXY(35, 78);
    write(tty, "ESC - quit the game", 19);

    return 0;
}

// отрисовывет значение клеток и выделяет нужную
void mi_drawGameField(int ROW, int COL)
{
    int big[2];

    /* позиция символа по Х */
    unsigned int posX;
    unsigned int i;

    /* 1 - нолик, 2 - крестик */
    for (i = 0, posX = 5; i < 3; i++, posX = posX + 15) {
        /* выделим нужную ячейку */
        if (ROW == 0 && COL == i) {
            if (gameState[0][i] == 1) {
                bc_setbigcharpos(big, 3, posX, 'o', clBlack, clGreen);
            }
            if (gameState[0][i] == 2) {
                bc_setbigcharpos(big, 3, posX, 'x', clBlack, clGreen);
            }
            if (gameState[0][i] == 0) {
                bc_setbigcharpos(big, 3, posX, '*', clBlack, clGreen);
            }
        } else {

            if (gameState[0][i] == 1) {
                bc_setbigcharpos(big, 3, posX, 'o', clGreen, clBlack);
            }
            if (gameState[0][i] == 2) {
                bc_setbigcharpos(big, 3, posX, 'x', clGreen, clBlack);
            }
            /*****/
            if (gameState[0][i] == 0) {
                bc_setbigcharpos(big, 3, posX, '*', clGreen, clBlack);
            }
        }
    }

    for (i = 0, posX = 5; i < 3; i++, posX = posX + 15) {
        /* выделим нужную ячейку */
        if (ROW == 1 && COL == i) {
            if (gameState[1][i] == 1) {
                bc_setbigcharpos(big, 13, posX, 'o', clBlack, clGreen);
            }
            if (gameState[1][i] == 2) {
                bc_setbigcharpos(big, 13, posX, 'x', clBlack, clGreen);
            }
            if (gameState[1][i] == 0) {
                bc_setbigcharpos(big, 13, posX, '*', clBlack, clGreen);
            }
        } else {

            if (gameState[1][i] == 1) {
                bc_setbigcharpos(big, 13, posX, 'o', clGreen, clBlack);
            }
            if (gameState[1][i] == 2) {
                bc_setbigcharpos(big, 13, posX, 'x', clGreen, clBlack);
            }
            /*****/
            if (gameState[1][i] == 0) {
                bc_setbigcharpos(big, 13, posX, '*', clGreen, clBlack);
            }
        }
    }

    for (i = 0, posX = 5; i < 3; i++, posX = posX + 15) {
        /* выделим нужную ячейку */
        if (ROW == 2 && COL == i) {
            if (gameState[2][i] == 1) {
                bc_setbigcharpos(big, 23, posX, 'o', clBlack, clGreen);
            }
            if (gameState[2][i] == 2) {
                bc_setbigcharpos(big, 23, posX, 'x', clBlack, clGreen);
            }
            if (gameState[2][i] == 0) {
                bc_setbigcharpos(big, 23, posX, '*', clBlack, clGreen);
            }
        } else {
            if (gameState[2][i] == 1) {
                bc_setbigcharpos(big, 23, posX, 'o', clGreen, clBlack);
            }
            if (gameState[2][i] == 2) {
                bc_setbigcharpos(big, 23, posX, 'x', clGreen, clBlack);
            }
            /*****/
            if (gameState[2][i] == 0) {
                bc_setbigcharpos(big, 23, posX, '*', clGreen, clBlack);
            }
        }
    }
}

// вывод сообщения в область сообщений от игры
int mi_writeMessage(char *msg)
{
    int len = strlen(msg);

    mt_setbgcolor(clBlack);
    mt_setfgcolor(clGreen);

    /* рамка для сообщений игры */
    bc_box(33, 1, 4, 39);
    /* подпись области сообщений игры */
    mt_gotoXY(33, 11);
    write(1, " MESSAGE FROM GAME ", 19);

    if (len != 0) {
        mt_gotoXY(35, 3);
        write(1, msg, len);
    }
    mt_gotoXY(36, 3);

    return 0;
}

int mi_gamePlay()
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
    if (gameState[0][0] == 1 && gameState[0][1] == 1 && gameState[0][2] == 1) {
        return 1;
    }
    if (gameState[1][0] == 1 && gameState[1][1] == 1 && gameState[1][2] == 1) {
        return 1;
    }
    if (gameState[2][0] == 1 && gameState[2][1] == 1 && gameState[2][2] == 1) {
        return 1;
    }

    /* case 4 - 6 */
    if (gameState[0][0] == 1 && gameState[1][0] == 1 && gameState[2][0] == 1) {
        return 1;
    }
    if (gameState[0][1] == 1 && gameState[1][1] == 1 && gameState[2][1] == 1) {
        return 1;
    }
    if (gameState[0][2] == 1 && gameState[1][2] == 1 && gameState[2][2] == 1) {
        return 1;
    }

    /* case 7 - 8 */
    if (gameState[0][0] == 1 && gameState[1][1] == 1 && gameState[2][2] == 1) {
        return 1;
    }
    if (gameState[0][2] == 1 && gameState[1][1] == 1 && gameState[2][0] == 1) {
        return 1;
    }


    /***********************************/
    /* проверка 8 случаев для крестика */
    /***********************************/

    /* case  1 - 3 */
    if (gameState[0][0] == 2 && gameState[0][1] == 2 && gameState[0][2] == 2) {
        return 2;
    }
    if (gameState[1][0] == 2 && gameState[1][1] == 2 && gameState[1][2] == 2) {
        return 2;
    }
    if (gameState[2][0] == 2 && gameState[2][1] == 2 && gameState[2][2] == 2) {
        return 2;
    }

    /* case 4 - 6 */
    if (gameState[0][0] == 2 && gameState[1][0] == 2 && gameState[2][0] == 2) {
        return 2;
    }
    if (gameState[0][1] == 2 && gameState[1][1] == 2 && gameState[2][1] == 2) {
        return 2;
    }
    if (gameState[0][2] == 2 && gameState[1][2] == 2 && gameState[2][2] == 2) {
        return 2;
    }

    /* case 7 - 8 */
    if (gameState[0][0] == 2 && gameState[1][1] == 2 && gameState[2][2] == 2) {
        return 2;
    }
    if (gameState[0][2] == 2 && gameState[1][1] == 2 && gameState[2][0] == 2) {
        return 2;
    }
    // проверка на ничью
    if (gameState[0][0] != 0 && gameState[0][1] != 0 && gameState[0][2] != 0 &&
        gameState[1][0] != 0 && gameState[1][1] != 0 && gameState[1][2] != 0 &&
        gameState[2][0] != 0 && gameState[2][1] != 0 && gameState[2][2] != 0) {
        return 3;
    }

    return 0;
}

// выводит сообщение о победителе если он есть и ставит флаг конца игры в 1
void mi_winGame(int winner)
{
    if (winner == 0) {
        return;
    }

    mt_setstdcolor();
    mt_clrscr();
    mt_setbgcolor(clBlack);
    mt_setfgcolor(clGreen);
    mt_gotoXY(15, 17);

    switch(winner)
    {
        case 1:
            write(1, "WON ZEROS", 9);
            break;

        case 2:
            write(1, "WON CROSSES", 11);
            break;

        case 3:
            write(1, "PLAYED IN DRAW!", 15);
            break;

        case -1:
            write(1, "ANOTHER PLAYER LEFT GAME!", 25);
            break;

        default:
            break;
    }

    isFinishedGame = 1;

    rk_mytermrestore();
    mt_setcursor(1);
    mt_setstdcolor();
    mt_gotoXY(39, 1);
}

// вывести сообщение в чат
void displayMsgChat(char msg[])
{
    // запоминаем ообщение в архив
    strcpy(archive[rangeEnd], msg);

    // первые 30 сообщений просто выводим
    if (rangeEnd < 30) {
        mt_gotoXY(chatCurrentRow, 49);
        write(1, msg, strlen(msg));
        chatCurrentRow++;
    // остальные выводим из архива
    } else {
            // обновляем рамку для чата чтобы стереть предыдущий вывод
            bc_box(1, 47, 32, 55);
            // подпись чата
            mt_gotoXY(1, 69);
            write(1, " CHAT ", 6);
        for (rangeBegin = rangeEnd - 29, chatCurrentRow = 2; chatCurrentRow < 32; chatCurrentRow++, rangeBegin++) {
            mt_gotoXY(chatCurrentRow, 49);
            write(1, archive[rangeBegin], strlen(archive[rangeBegin]));
        }
    }
    // запоминаем актуальное значение
    scrollRow = rangeEnd;
    rangeEnd++;
}

// выводит 30 сообщений в чат начиная с индекса rBegin
void scrollChat(int rBegin)
{
    // обновляем рамку для чата чтобы стереть предыдущий вывод
    bc_box(1, 47, 32, 55);
    // подпись чата
    mt_gotoXY(1, 69);
    write(1, " CHAT ", 6);
    for (chatCurrentRow = 2; chatCurrentRow < 32; chatCurrentRow++, rBegin++) {
        mt_gotoXY(chatCurrentRow, 49);
        write(1, archive[rBegin], strlen(archive[rBegin]));
    }
}

// очищает область ввода сообщения в чат
void clearAreaEnterMsg(int row, int col, int height, int width)
{
    mt_gotoXY(row, col);
    int i, j;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            write(1, " ", 1);
        }
        write(1, "\n", 1);
    }
}

// network control
void* ctlNetworking(void *arg)
{
    common_msg msgGameState;

    int whoWin;

    struct pollfd fdt[1];      // poll 

    // Configure the poll mechanism
    int server_fd = SERVER_FD;
    fdt[0].fd = SERVER_FD;
    fdt[0].events = POLLIN;

    // Configure the poll mechanism
    fdt[0].fd = server_fd;
    fdt[0].events = POLLIN;    

    while(1) {
        int len = poll(fdt, 1, 10);// ожидание
        len = read(server_fd, &msgGameState, sizeof(common_msg));
        if (len <= 0) {
            displayMsgChat("Lose connection to Main Server: Reconnecting...\n");
            close(server_fd);
            sleep(1);
            server_fd = try_to_connect(server_fd, 1); 
            if (server_fd == -1) {
                //quit(1);
                exit(0);
            }
            continue;
        }

        switch(msgGameState.type)
        {
            case EVN:
                if (msgGameState.evnt.event == TECH_WIN) {
                    mi_winGame(-1);
                    if (isFinishedGame == 1) {
                        exit(0);
                    } 
                }
                break;

            case GAME:

                if (msgGameState.game.player == ZERO) {
                    gameState[msgGameState.game.row][msgGameState.game.col] = 1;
                } else {
                    gameState[msgGameState.game.row][msgGameState.game.col] = 2; 
                }
                
                rowField = msgGameState.game.row;
                colField = msgGameState.game.col;
                
                mi_drawGameField(rowField, colField);
                whoWin = mi_gamePlay();
                mi_winGame(whoWin);
                
                if (isFinishedGame == 1) {
                    exit(0);
                }
                
                // если обработали сообщение значит текущий ход теперь наш
                if (previousMove == 1) {
                    previousMove = 2;
                } else {
                    previousMove = 1;
                }
                break;

            case CHAT:
                displayMsgChat(msgGameState.chat.string);
                break;

            default:
                displayMsgChat("[ERROR] Unknown message recive from server\n");
                break;
        }
    }
    return 0;
}

// send chat message
void send_chat_msg(char *msg)
{
    common_msg chat_msg;
    
    chat_msg.type = CHAT;
    chat_msg.chat.number = GAME_ROOM;
    chat_msg.chat.player = PLAYER;
    strcpy(chat_msg.chat.string, msg);

    write(SERVER_FD, &chat_msg, sizeof(common_msg));
}

// main game routine
int game_session(int server_fd, int isGameBegan)
{
    int chPlayer;
    if (PLAYER == ZERO) {
        chPlayer = 1;
    } else {
        chPlayer = 2;
    }
    // если игра с 2 игроками создана
    if (isGameBegan) {
        // строка и столбец игрового поля
        pthread_t networking; // сетевое взаимодействие
        pthread_t chatInteraction; //взаимодействие чата
        int key;
        rowField = colField = 0;

        char msgChat[250] = "";
        int len;
        // отрисовка основных элементов интерфейса
        mi_drawMainInterface();
        mi_drawGameField(0, 0);

        // сохраняем текущие настройки терминала, переходим в неканонический режим, прячем курсор
        rk_mytermsave();
        rk_mytermregime(0, 0, 1, 0, 1);
        mt_setcursor(0);
        // устанавливаем цветовую схему
        mt_setbgcolor(clBlack);
        mt_setfgcolor(clGreen);

        //struct GameState msgGameState;
        common_msg msgGameState;

        msgGameState.type = GAME;

        int unused;
        pthread_create(&networking, 0, ctlNetworking, (void*)&unused);

        int whoWin;
        char msgSign[250];

        if (chPlayer == 1) {
            mi_writeMessage("You are ZERO");
        } else if (chPlayer == 2) {
            mi_writeMessage("You are CROSS");
        }

        while (isFinishedGame == 0 && rk_readkey(&key) == 0 && key != K_ESC) {
            
            switch (key) 
            {
                case K_R:
                      if (scrollRow > 29) {
                          scrollRow--;
                          scrollChat(scrollRow - 29);
                      }
                      break;
                case K_T:
                      if (scrollRow < rangeEnd - 1) {
                          scrollRow++;
                          scrollChat(scrollRow - 29);
                      }
                      break;
                case K_UP:
                    if (rowField != 0) {
                        rowField--;
                        mi_drawGameField(rowField, colField);
                    }
                    break;
                case K_DOWN:
                    if (rowField < 2) {
                        rowField++;
                        mi_drawGameField(rowField, colField);
                    }
                    break;
                case K_LEFT:
                    if (colField != 0) {
                        colField--;
                        mi_drawGameField(rowField, colField);
                    }
                    break;
                case K_RIGHT:
                    if (colField < 2) {
                        colField++;
                        mi_drawGameField(rowField, colField);
                    }
                    break;
                case K_ENTER:
                    if (chPlayer == 1) {
                        strcpy(msgSign,"ZEROS: ");
                    } else if (chPlayer == 2) {
                        strcpy(msgSign, "CROSSES: ");
                    }
                    int i;
                    for (i = 0; i < 250; i++) {
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
                    send_chat_msg(msgSign);
                    displayMsgChat(msgSign);
                    clearAreaEnterMsg(37, 1, 3, 101);
                    rk_mytermregime(0, 0, 1, 0, 1);
                    mt_setcursor(0);
                    break;

                case K_F5: // ставим нолик
                    // если игрок играет ноликом
                    if (chPlayer == 1) {
                        if (gameState[rowField][colField] == 0 && previousMove != 1) {
                            previousMove = 1;
                            // ставим в массиве нолик
                            gameState[rowField][colField] = 1;
                            // отрисовываем интерфейс заново
                            mi_drawGameField(rowField, colField);
                            //mi_gamePlay();
                            //заполнили структуру с данным для сервера
                            msgGameState.game.number = GAME_ROOM;
                            msgGameState.game.row = rowField;
                            msgGameState.game.col = colField;
                            msgGameState.game.player = ZERO;
                            write(server_fd, &msgGameState, sizeof(common_msg));
                            // проверяем на выигрышную ситуацию (другой клиент проерит в своей потоке)
                            whoWin = mi_gamePlay();
                            mi_winGame(whoWin);
                        } else if (gameState[rowField][colField] != 0){
                            mi_writeMessage("This cell is taken!");
                        } else if (previousMove == 1) {
                            mi_writeMessage("CROSS move!");
                        }
                    }
                    break;
                case K_F6:
                    // если игрок играет крестиком
                    if (chPlayer == 2) {
                        if (gameState[rowField][colField] == 0 && previousMove != 2) {
                            previousMove = 2;
                            // ставим в массиве нолик
                            gameState[rowField][colField] = 2;
                            // отрисовываем интерфейс заново
                            mi_drawGameField(rowField, colField);
                            //mi_gamePlay();
                            //заполнили структуру с данным для сервера
                            msgGameState.game.number = GAME_ROOM;
                            msgGameState.game.row = rowField;
                            msgGameState.game.col = colField;
                            msgGameState.game.player = CROSS;
                            write(server_fd, &msgGameState, sizeof(common_msg));
                            // проверяем на выигрышную ситуацию (другой клиент проерит в своей потоке)
                            whoWin = mi_gamePlay();
                            mi_winGame(whoWin);
                        } else if (gameState[rowField][colField] != 0){
                            mi_writeMessage("This cell is taken!");
                        } else if (previousMove == 2) {
                            mi_writeMessage("ZERO move!");
                        }
                    }
                    break;
            }
        }
        pthread_cancel(networking);
        close(server_fd);
    }
}