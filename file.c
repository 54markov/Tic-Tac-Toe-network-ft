typedef struct
{
    int previousMove;
    int rowField;
    int colField;
} signleplayer_game_t;

signleplayer_game_t singleplayer_game;

void* ctlAiInteraction(void *arg)
{
    int chPlayer = 2;



    return 0;
}

void singleplayer_set(int who, int row, int col)
{
    mutex.lock();

    if (singleplayer_game.previousMove == who) {
        mutex.unlock();
        if (who != AI)
            mi_writeMessage("Another player move!");
        
        return;
    }

    singleplayer_game.previousMove        = who;
    singleplayer_game.gameField[row][col] = who;

    mutex.unlock();
}

int singleplayer_get(int who, int row, int col)
{
    lock();

    singleplayer_game.previousMove = who;
    singleplayer_game.rowField     = row;
    singleplayer_game.colField     = col;

    unlock();
}

// main single player game routine
int single_game_session()
{
    int chPlayer = 1;
    int previousMove = 0;

    pthread_t ai_player;
    int key;
    
    rowField = colField = 0;

    char msgChat[250] = "";
    int len;

    // Draw main part of interface
    mi_drawMainInterface();
    mi_drawGameField(0, 0);

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
        previousMove = singleplayer_get(&rowField, &colField);
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
                if (chPlayer == 1) { // if ZERO player
                    if (gameState[rowField][colField] == 0 && previousMove != 1) {
                        previousMove = 1;
                        
                        gameState[rowField][colField] = 1;    // ставим в массиве нолик
                        mi_drawGameField(rowField, colField); // отрисовываем интерфейс заново

                        singleplayer_set(ZERO, rowField, colField);
                        
                        // проверяем на выигрышную ситуацию (другой клиент проерит в своей потоке)
                        whoWin = mi_gamePlay();
                        mi_winGame(whoWin);

                    } else if (gameState[rowField][colField] != 0){
                        mi_writeMessage("This place is taken!");
                    } else if (previousMove == 1) {
                        mi_writeMessage("CROSS move!");
                    }
                }
                break;
                
            case K_F6: // place CROSS
                if (chPlayer == 2) { // if CROSS player
                    if (gameState[rowField][colField] == 0 && previousMove != 2) {
                        previousMove = 2;

                        gameState[rowField][colField] = 2;    // ставим в массиве нолик
                        mi_drawGameField(rowField, colField); // отрисовываем интерфейс заново

                        singleplayer_gameplay(CROSS, rowField, colField);
                        
                        // проверяем на выигрышную ситуацию (другой клиент проерит в своей потоке)
                        whoWin = mi_gamePlay();
                        mi_winGame(whoWin);
                        
                    } else if (gameState[rowField][colField] != 0){
                        mi_writeMessage("This place is taken!");
                    } else if (previousMove == 2) {
                        mi_writeMessage("ZEOR move!");
                    }
                }
                break;
        }
    }

    pthread_cancel(ai_player);
    return 0;
}