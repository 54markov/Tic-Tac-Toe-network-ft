#include "header.h"

int CONF_LIST[] = { 2345,    // Main server port
                    2346,    // First reserve server port
                    2347 };  // Second reserve server port

extern room_info GAME_ROOMS[NUM_GAMES];

// Global variable (0-main/1-reserve)
int SERVER_STATE;

int reserve_list[100];
int counter = 0;

// display error message
void err_msg(char *msg, char *err)
{
    fprintf(stderr, "[%sERROR%s]", COLOR_RED, COLOR_OFF);
    fprintf(stderr, "%s", msg);
    fprintf(stderr, " %s\n", err);
}

// display log message
void log_msg(char *msg, int color)
{
    switch(color)
    {
        case 1: // green
            printf("%s", COLOR_GRN);
            break;

        case 2: // yellow
            printf("%s", COLOR_YEL);
            break;

        case 3: // blue
            printf("%s", COLOR_BLU);
            break;

        case 4: //magnetta
            printf("%s", COLOR_MAG);
            break;

        case 5: // cyan
            printf("%s", COLOR_CYN);
            break;

        default: //white
            printf("%s", COLOR_WHT);
            break;
    }

    printf("%s", msg);
    printf("%s\n",COLOR_OFF);
}

int create_socket(int fd)
{
    return socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);    
}

int close_socket(int fd)
{
    return close(fd);
}

int bind_socket(int fd, struct sockaddr_in server_addr, int port)
{
    int rs; // return status
    
    int option = 1;

    // Clean up structure
    bzero(&server_addr, sizeof(server_addr));

    // Fill the structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);                 // specify port
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);    // 127.0.0.1

    //Bind the server socket to the required ip-address and port.
    rs = bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    if (setsockopt(fd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0)
    {
        fprintf(stderr, "error: setsockopt failed!\n");
        close(fd);
        exit(1);
    }
    
    return rs;    
}

int listen_socket(int fd)
{   
    return listen(fd, MAXCONN);   
}

int create_epoll(int epoll_fd)
{
    return epoll_create(MAXCONN);
}

int modify_epoll_context(int epoll_fd, int operation, int server_fd, uint32_t events, void* data)
{
    int rs;
    struct epoll_event server_listen_event;

    server_listen_event.events = events;
    server_listen_event.data.ptr = data;

    rs = epoll_ctl(epoll_fd, operation, server_fd, &server_listen_event);

    return rs;    
}

int make_socket_non_blocking(int fd)
{
    int flag;
    
    flag = fcntl(fd, F_GETFL, NULL);

    if (flag == -1) {
        err_msg("[FCNTL] F_GETFL failed.", strerror(errno));
        return -1;
    }

    flag |= O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flag) == -1) {
        err_msg("[FCNTL] F_SETFL failed.%s", strerror(errno));
        return -1;
    }

    return 0;        
}

void broadcast_to_reserve(common_msg data, int lenght, void *ptr)
{
    printf("\nbroadcast\n");
    int i;
    for (i = 0; i < counter; i++) {
        printf("=>%d ",reserve_list[i]);
        write(reserve_list[i], &data, lenght);
    }
    printf("\n");
}

// clear up one room
void free_room(int index)
{
    int j, k;

    GAME_ROOMS[index].room_state = EMPTY;
    
    for (j = 0; j < 2; j++) {
        GAME_ROOMS[index].room_players[j] = -1;
    }

    for (j = 0; j < 3; j++) {
        for (k = 0; k < 3; k++) {
            GAME_ROOMS[index].room_field[j][k] = -1;
        }
    }

    print_game_rooms(GAME_ROOMS, NUM_GAMES);
}

int get_another_client(int fd, int *index)
{
    int i, j;

    for (i = 0; i < NUM_GAMES; i++) {

        if (GAME_ROOMS[i].room_state == EMPTY) {
            return 0;
        }

        for (j = 0; j < 2; j++) {
            if (GAME_ROOMS[i].room_players[j] == fd) {
                *index = i;
                if (j == 0) {
                    return GAME_ROOMS[i].room_players[1];
                } else {
                    return GAME_ROOMS[i].room_players[0];
                }
            }   
        }
    }
    return -1;
}

void game_handler(int fd, int event)
{
    int index; // game_room[index]
    common_msg msg;
    msg.type = EVN;
    msg.evnt.event = TECH_WIN;

    if (event == 1) {
        int client = get_another_client(fd, &index);
        if (client == -1) {
            fprintf(stderr, "Error: can't get player\n");
            return;
        } else if (client == 0) {
            fprintf(stderr, "NOTE: player left\n");
        } else {
            free_room(index);
            write(client, &msg, sizeof(common_msg));           
        }   
    }
}

// main network epoll handler
void *handle(void *ptr, int epoll_fd)
{
    struct EchoEvent *echoEvent = ptr;
    common_msg broadcast_msg;

    if (EPOLLIN == echoEvent->event) {
        int n = read(echoEvent->fd, &echoEvent->msg, sizeof(common_msg));
        if (n == 0) {
            // Client closed connection
            if (echoEvent->type == 0) {
                printf("\nPlayer on fd = %d closed connection!\n", echoEvent->fd);
                game_handler(echoEvent->fd, 1);
                printf("broadcast player left\n");
                broadcast_msg = send_update(broadcast_msg, 0);
                broadcast_to_reserve(broadcast_msg, sizeof(common_msg), NULL);
            } else {
                printf("\nServer on fd = %d closed connection!\n", echoEvent->fd);
            }
            close(echoEvent->fd);
            free(echoEvent);
        } else if(n == -1) {
            printf("\nClient error - closed connection.\n");
            close(echoEvent->fd);
            free(echoEvent);
        } else {
            echoEvent->length = n;
            printf("\nRead data form fd: %d\n", echoEvent->fd);
            switch (echoEvent->msg.type)
            {
                case GAME:
                    game_routine(echoEvent->msg);
                    broadcast_to_reserve(echoEvent->msg, sizeof(common_msg), ptr);
                    break;

                case CHAT:
                    chat_routine(echoEvent->msg);
                    break;
                default:
                    break;
            }
            modify_epoll_context(epoll_fd, EPOLL_CTL_ADD, echoEvent->fd, EPOLLOUT, echoEvent);
        }        
    
    } else if (EPOLLOUT == echoEvent->event) {
        modify_epoll_context(epoll_fd, EPOLL_CTL_ADD, echoEvent->fd, EPOLLIN, echoEvent);
    }
}

struct sockaddr_in configure_socket(struct sockaddr_in server, int port)
{
    bzero(&server, sizeof(server));

    server.sin_family = AF_INET;
    // переводим в сетевой порядок
    server.sin_port = htons(port);
    // преобразует строку с адресом в двоичную форму с сетевым порядком
    inet_aton(SERVER_IP, &(server.sin_addr));

    return server;   
}

// additional main server function
int create_main_new_server(int server_port, char *argc)
{
    char status[256];
    
    int option = 1;

    int epoll_fd;
    int server_fd;
    
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    struct epoll_event *events;

    socklen_t client_len = sizeof(client_addr);

    // Create the server socket
    server_fd = create_socket(server_fd);
    if (server_fd == -1) {
        err_msg("[ERROR] Failed to create socket.", strerror(errno));
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0) {
        fprintf(stderr, "error: setsockopt failed!\n");
        close(server_fd);
        exit(1);
    }

    log_msg("[LOG] CREATE SOCKET       - OK", 1);

    //Bind the server socket
    if (bind_socket(server_fd, server_addr, server_port) == -1) {
    //if (bind_socket(server_fd, server_addr, SERVERPORT) == -1) {
        err_msg("[ERROR] Failed to bind.", strerror(errno));
        return -1;
    }
    log_msg("[LOG] BIND SOCKET         - OK", 1);

    // Listen the server socket
    if (listen_socket(server_fd) == -1) {
        err_msg("[ERROR] Failed to listen.", strerror(errno));
        return -1;
    }
    log_msg("[LOG] LISTEN SOCKET       - OK", 1);

    //Create the epoll context    
    epoll_fd = create_epoll(epoll_fd);
    if (epoll_fd == -1) {
        err_msg("[ERROR] Failed to create epoll context", strerror(errno));
        return -1;
    }

    // Create the read event for server socket
    if (modify_epoll_context(epoll_fd, EPOLL_CTL_ADD, server_fd,\
                                                 EPOLLIN, &server_fd) == -1) 
    {
        err_msg("[ERROR]Failed to add an event for socket", strerror(errno));
        return -1;        
    }
    log_msg("[LOG] EPOLL CREATE SOCKET - OK", 1);

    events = calloc(MAXEVENTS, sizeof(struct epoll_event));

    strcpy(status, "netstat -a |grep ");
    strcat(status, argc);
    printf("==============================================================\n");
    system(status);
    printf("==============================================================\n");
    
    log_msg("[LOG] WAITING CONNECTION  - ...", 1);

    bool first_cycle = true;
    // Main loop
    while(1) {
        int events_count;
        
        if (first_cycle) {
            events_count = epoll_wait(epoll_fd, events, MAXEVENTS, 6000);
            if (events_count == 0) {
                printf("No connection accepted, change state to reserve\n");
                sleep(1);
                create_reserve_server(server_port, argc);
            }
            first_cycle = false;
        } else {
            events_count = epoll_wait(epoll_fd, events, MAXEVENTS, -1);
        }   

        if (events_count ==-1) {
            err_msg("[ERROR] Failed to wait.", strerror(errno));
            return -1;
        }
        int i;

        for (i = 0; i < events_count; i++) {

            if (events[i].data.ptr == &server_fd) {
                if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR) {
                    close(server_fd);
                    return 1;
                }

                int conn_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                if (conn_fd == -1) {
                    printf("Accept failed. %s", strerror(errno));
                    return 1;
                } else {
                    printf("[NET] Accepted connection...");
                    int type;
                    int conn_type;
                    common_msg msg;
                    read(conn_fd, &msg, sizeof(common_msg));
                    switch (msg.type)
                    {
                        case AUTH:
                            printf("client %d\n", conn_fd);
                            accept_new_client(msg, conn_fd);
                            conn_type = 0;
                            break;
                        case RESR:
                            printf("server %d\n",conn_fd);
                            accept_new_rserver(msg, conn_fd);
                            conn_type = 1;
                            break;

                        default:
                            fprintf(stderr, "Error: unknow connection %d - closing connection!\n", conn_fd);
                            close(conn_fd);
                            break;
                    }

                    struct EchoEvent *echoEvent = calloc(1, sizeof(struct EchoEvent));
                    echoEvent->fd = conn_fd;
                    echoEvent->type = conn_type;                     
                    //Add a read event
                    modify_epoll_context(epoll_fd, EPOLL_CTL_ADD, echoEvent->fd, EPOLLIN, echoEvent);

                }

            } else {
                if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR) {
                    struct EchoEvent* echoEvent = (struct EchoEvent*) events[i].data.ptr;
                    printf("\nClosing connection socket\n");
                    close(echoEvent->fd);
                    free(echoEvent);
                } else if (EPOLLIN == events[i].events) {
                    struct EchoEvent* echoEvent = (struct EchoEvent*) events[i].data.ptr;
                    echoEvent->event = EPOLLIN;
                    // Delete the read event

                    printf("epoll 1 \n");

                    modify_epoll_context(epoll_fd, EPOLL_CTL_DEL, echoEvent->fd, 0, 0);
                    handle(echoEvent, epoll_fd);
                } else if (EPOLLOUT == events[i].events) {
                    struct EchoEvent* echoEvent = (struct EchoEvent*) events[i].data.ptr;
                    echoEvent->event = EPOLLOUT;
                    // Delete the write event.

                    printf("epoll 2 \n");

                    modify_epoll_context(epoll_fd, EPOLL_CTL_DEL, echoEvent->fd, 0, 0);
                    handle(echoEvent, epoll_fd);
                }
            }
        }
    }

    free(events);
    
    if (close_socket(server_fd) == 1) {
        err_msg("[ERROR] Failed to close.", strerror(errno));
        return -1;
    }

    return 0;
}

// main server main function
int create_main_server(int server_port, char *argc)
{
    char status[256];
    
    int option = 1;

    int epoll_fd;
    int server_fd;
    
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    struct epoll_event *events;

    socklen_t client_len = sizeof(client_addr);

    // Create the server socket
    server_fd = create_socket(server_fd);
    if (server_fd == -1) {
        err_msg("[ERROR] Failed to create socket.", strerror(errno));
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0) {
        fprintf(stderr, "error: setsockopt failed!\n");
        close(server_fd);
        exit(1);
    }

    log_msg("[LOG] CREATE SOCKET       - OK", 1);

    //Bind the server socket
    if (bind_socket(server_fd, server_addr, server_port) == -1) {
    //if (bind_socket(server_fd, server_addr, SERVERPORT) == -1) {
        err_msg("[ERROR] Failed to bind.", strerror(errno));
        return -1;
    }
    log_msg("[LOG] BIND SOCKET         - OK", 1);

    // Listen the server socket
    if (listen_socket(server_fd) == -1) {
        err_msg("[ERROR] Failed to listen.", strerror(errno));
        return -1;
    }
    log_msg("[LOG] LISTEN SOCKET       - OK", 1);

    //Create the epoll context    
    epoll_fd = create_epoll(epoll_fd);
    if (epoll_fd == -1) {
        err_msg("[ERROR] Failed to create epoll context", strerror(errno));
        return -1;
    }

    // Create the read event for server socket
    if (modify_epoll_context(epoll_fd, EPOLL_CTL_ADD, server_fd,\
                                                 EPOLLIN, &server_fd) == -1) 
    {
        err_msg("[ERROR]Failed to add an event for socket", strerror(errno));
        return -1;        
    }
    log_msg("[LOG] EPOLL CREATE SOCKET - OK", 1);

    events = calloc(MAXEVENTS, sizeof(struct epoll_event));

    strcpy(status, "netstat -a |grep ");
    strcat(status, argc);
    printf("==============================================================\n");
    system(status);
    printf("==============================================================\n");
    
    log_msg("[LOG] WAITING CONNECTION  - ...", 1);

    // Main loop
    while(1) {
        int events_count = epoll_wait(epoll_fd, events, MAXEVENTS, -1);
        if (events_count ==-1) {
            err_msg("[ERROR] Failed to wait.", strerror(errno));
            return -1;
        }
        int i;
        for (i = 0; i < events_count; i++) {

            if (events[i].data.ptr == &server_fd) {
                if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR) {
                    close(server_fd);
                    return 1;
                }

                int conn_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                if (conn_fd == -1) {
                    printf("Accept failed. %s", strerror(errno));
                    return 1;
                } else {
                    printf("[NET] Accepted connection...");
                    int type;
                    int conn_type;
                    common_msg msg;
                    read(conn_fd, &msg, sizeof(common_msg));
                    switch (msg.type)
                    {
                        case AUTH:
                            printf("client %d\n", conn_fd);
                            accept_new_client(msg, conn_fd);
                            conn_type = 0;
                            break;
                        case RESR:
                            printf("server %d\n",conn_fd);
                            accept_new_rserver(msg, conn_fd);
                            conn_type = 1;
                            break;

                        default:
                            fprintf(stderr, "Error: unknow connection %d - closing connection!\n", conn_fd);
                            close(conn_fd);
                            break;
                    }

                    struct EchoEvent *echoEvent = calloc(1, sizeof(struct EchoEvent));
                    echoEvent->fd = conn_fd;
                    echoEvent->type = conn_type;                     
                    //Add a read event
                    modify_epoll_context(epoll_fd, EPOLL_CTL_ADD, echoEvent->fd, EPOLLIN, echoEvent);

                }

            } else {
                if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR) {
                    struct EchoEvent* echoEvent = (struct EchoEvent*) events[i].data.ptr;
                    printf("\nClosing connection socket\n");
                    close(echoEvent->fd);
                    free(echoEvent);
                } else if (EPOLLIN == events[i].events) {
                    struct EchoEvent* echoEvent = (struct EchoEvent*) events[i].data.ptr;
                    echoEvent->event = EPOLLIN;
                    // Delete the read event

                    printf("epoll 1 \n");

                    modify_epoll_context(epoll_fd, EPOLL_CTL_DEL, echoEvent->fd, 0, 0);
                    handle(echoEvent, epoll_fd);
                } else if (EPOLLOUT == events[i].events) {
                    struct EchoEvent* echoEvent = (struct EchoEvent*) events[i].data.ptr;
                    echoEvent->event = EPOLLOUT;
                    // Delete the write event.

                    printf("epoll 2 \n");

                    modify_epoll_context(epoll_fd, EPOLL_CTL_DEL, echoEvent->fd, 0, 0);
                    handle(echoEvent, epoll_fd);
                }
            }
        }
    }

    free(events);
    
    if (close_socket(server_fd) == 1) {
        err_msg("[ERROR] Failed to close.", strerror(errno));
        return -1;
    }

    return 0;
}

// reserver server main function
int create_reserve_server(int server_port, char *argc)
{
    int i, fail_conn;

    int server_fd;

    struct sockaddr_in server_addr;

    struct pollfd fdt[1];
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (server_fd < 0 ) {
        fprintf(stderr, "error: can't create socket\n");
    }

    // Loop for the try to connect (main or reserve0 or reserve1)
    for (i = 0, fail_conn = 0; i < NUM_SERVER; i++) {
        // Configure socket by differnt port
        server_addr = configure_socket(server_addr, CONF_LIST[i]);
        // Try to connect
        if (connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            // Fail, reconnect to another
            fprintf(stderr, "error: can't connect to %s:%d. try to another\n", SERVER_IP, CONF_LIST[i]);
            fail_conn++;
        } else {
            // Success
            common_msg msg;
            msg.type = RESR;
            msg.resr.request = YES;

            write(server_fd, &msg, sizeof(common_msg));
            printf("connected to %s:%d\n",SERVER_IP,CONF_LIST[i]);
            read(server_fd, &msg, sizeof(common_msg));
            update_rooms(msg);
            break;
        }
        if (fail_conn == NUM_SERVER) {
            // All server are down
            fprintf(stderr, "error: all servers are down, shutting down...\n");
            return -1;
        }
    }

    // Настраиваем что проверять функции poll
    fdt[0].fd = server_fd;
    fdt[0].events = POLLIN;    

    // Main loop    
    while(1) {

        common_msg msg;
        // Ожидаем приход пакета от сервера
        int len = poll(fdt, 1, 5000);
        len = read(server_fd, &msg, sizeof(msg));// ожидание
        if (len == 0 ) {
            printf("\nMain server is down. Change state from reserve to main\n");
            
            close(server_fd);

            create_main_new_server(server_port, argc);

        } else {
            printf("[BROADCAST]\n");
            switch(msg.type)
            {
                case GAME:
                    printf("GAME MSG\n");
                    game_routine(msg);
                    break;

                case EVN:

                    break;

                case RESR:
                    printf("UPDATE MSG\n");
                    update_rooms(msg);
                    break;

                default:
                    break;
            }
        }
    }

    close(server_fd);

    return 0;
}

void initialize_game_rooms(room_info *games_room, int size)
{
    int i;

    for (i = 0; i < size; i++) {
        int j, k;
        games_room[i].room_state = EMPTY;
        games_room[i].room_number = i;
        for (j = 0; j < 2; j++) {
            games_room[i].room_players[j] = -1;
        }
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 3; k++) {
                games_room[i].room_field[j][k] = -1;
            }
        }
    }
}

void print_game_rooms(room_info *games_room, int size)
{
    int i;

    for (i = 0; i < size; i++) {
        int j, k;
        if (games_room[i].room_state == FULL){
            printf("Game room %d info: state[FULL]\n", games_room[i].room_number);
        } else {
           printf("Game room %d info: state[EMPTY]\n", games_room[i].room_number); 
        }
        printf("Players:\n");
        printf("ZERO : %d\n", games_room[i].room_players[ZERO]);
        printf("CROSS: %d\n", games_room[i].room_players[CROSS]);
        printf("Game field:\n");
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 3; k++) {
                printf("%d ", games_room[i].room_field[j][k]);
            }
            printf("\n");
            //printf("LAST MOVE: %d\n", games_room[i].room_last);
        }
        printf("#############################################\n");
    }
}

void print_game_room(room_info games_room)
{
    int j, k;
    if (games_room.room_state == FULL){
        printf("Game room %d info: state[FULL]\n", games_room.room_number);
    } else {
       printf("Game room %d info: state[EMPTY]\n", games_room.room_number); 
    }
    printf("Players:\n");
    printf("ZERO : %d\n", games_room.room_players[ZERO]);
    printf("CROSS: %d\n", games_room.room_players[CROSS]);
    printf("Game field:\n");
    for (j = 0; j < 3; j++) {
        for (k = 0; k < 3; k++) {
            if (games_room.room_field[j][k] == 1) {
                printf("X ");
            } else if (games_room.room_field[j][k] == 0) {
                printf("O ");
            } else {
                printf("E ");
            }
        }
        printf("\n");
        //printf("LAST MOVE: %d\n", games_room.room_last);
    }
    printf("#############################################\n");
}

// accept new player
void accept_new_client(common_msg msg, int fd)
{
    int type_of_player;
    int action;

    common_msg broadcast_msg;

    switch(msg.auth.new_game)
    {
        case YES:
            msg.auth.number = set_new_player_in_room(fd, &type_of_player, &action);
            msg.auth.player = type_of_player;
            msg.auth.start = action;
            printf("client %d, assing room %d player %d\n", fd, msg.auth.number, type_of_player);
            
            broadcast_msg = send_update(broadcast_msg, 0);
            broadcast_to_reserve(broadcast_msg, sizeof(common_msg), NULL);

            if (action == YES) {
                write(fd, &msg, sizeof(msg));
                msg.auth.player = ZERO;
                write(GAME_ROOMS[msg.auth.number].room_players[0], &msg, sizeof(msg));
                printf("game room %d start game\n", msg.auth.number);
            }
            break;

        case NO:
            restore_game(msg.auth.number, msg.auth.player, fd);
            printf("client %d, restore to room %d player %d\n", fd, msg.auth.number, msg.auth.player);
        default:
            break;
    }
}

// accept new reserver server
void accept_new_rserver(common_msg msg, int fd)
{
    reserve_list[counter] = fd;
    counter++;
    msg = send_update(msg, fd);
    write(fd, &msg, sizeof(msg));
}

// send upadate message to reserve servers
common_msg send_update(common_msg msg, int fd)
{
    printf("SEND: upadate rooms to reserve server\n");
    msg.type = RESR;
    msg.resr.request = NO;
    int i;
    for (i = 0; i < NUM_GAMES; i++) {
        int j, k;
        msg.resr.game_rooms[i].room_state = GAME_ROOMS[i].room_state;
        msg.resr.game_rooms[i].room_number = GAME_ROOMS[i].room_number;
        msg.resr.game_rooms[i].room_last = GAME_ROOMS[i].room_last;
        for (j = 0; j < 2; j++) {
            msg.resr.game_rooms[i].room_players[j] = GAME_ROOMS[i].room_players[j];
        }
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 3; k++) {
                msg.resr.game_rooms[i].room_field[j][k] = GAME_ROOMS[i].room_field[j][k];
            }
        }
    }
    return msg;
}

// assing new player in the game room
int set_new_player_in_room(int fd, int *type_of_player, int *action)
{
    int index = get_free_room();
    int i;
    // search empty place for player
    for (i = 0; i < 2; i++) {
        if (GAME_ROOMS[index].room_players[i] == -1) {
            GAME_ROOMS[index].room_players[i] = fd;
            if (i == 0) {
                *action = NO;
                *type_of_player = ZERO;
                return index;
            } else {
                GAME_ROOMS[index].room_state = FULL;
                *action = YES;
                *type_of_player = CROSS;
                return index;
            }
            break;
        }   
    }

    GAME_ROOMS[index];
}

// reconnect client fd with game room structure
int restore_game(int number, int player, int fd)
{
    GAME_ROOMS[number].room_players[player] = fd;
}

// get empty game room
int get_free_room()
{
    int i;
    for (i = 0; i < NUM_GAMES; i++) {
        if (GAME_ROOMS[i].room_state == EMPTY) {
            return i;
        }
    }
}

// chat routine
void chat_routine(common_msg msg)
{
    int index = msg.game.number;
    int player = msg.game.player;

    if (player == ZERO) {
        printf("chat message from: ZERO\n");
        printf("chat msg: from %d to %d \n", GAME_ROOMS[index].room_players[player], GAME_ROOMS[index].room_players[CROSS]);
        write(GAME_ROOMS[index].room_players[CROSS], &msg, sizeof(common_msg));
    } else {
        printf("chat message from: CROSS\n");
        printf("chat msg: from %d to %d \n", GAME_ROOMS[index].room_players[player], GAME_ROOMS[index].room_players[ZERO]);
        write(GAME_ROOMS[index].room_players[ZERO], &msg, sizeof(common_msg));
    }
}

// game routine
void game_routine(common_msg msg)
{
    int index = msg.game.number;
    int row = msg.game.row;
    int col = msg.game.col;
    int player = msg.game.player;

    if (player == ZERO) {
        printf("move: ZERO\n");
    } else {
        printf("move: CROSS\n");
    }

    // make move
    if (GAME_ROOMS[index].room_field[row][col] == -1) {
        GAME_ROOMS[index].room_field[row][col] = player;
        GAME_ROOMS[index].room_last = player;
        if (player == ZERO) {
            printf("from %d to %d \n", GAME_ROOMS[index].room_players[player], GAME_ROOMS[index].room_players[CROSS]);
            write(GAME_ROOMS[index].room_players[CROSS], &msg, sizeof(common_msg));
        } else {
            printf("from %d to %d \n", GAME_ROOMS[index].room_players[player], GAME_ROOMS[index].room_players[ZERO]);
            write(GAME_ROOMS[index].room_players[ZERO], &msg, sizeof(common_msg));
        }
    } else {
        // send ERR MSG to fd
        fprintf(stderr, "Error: cell is taken!\n");
        return;
    }
    print_game_room(GAME_ROOMS[index]);
}

// upade game info in rooms
void update_rooms(common_msg msg)
{
    int i;
    room_info *rooms = msg.resr.game_rooms;

    for (i = 0; i < NUM_GAMES; i++) {
        int j, k;
        GAME_ROOMS[i].room_state = rooms[i].room_state;
        GAME_ROOMS[i].room_number = rooms[i].room_number;
        GAME_ROOMS[i].room_last = rooms[i].room_last;
        for (j = 0; j < 2; j++) {
            GAME_ROOMS[i].room_players[j] = rooms[i].room_players[j];
        }
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 3; k++) {
                GAME_ROOMS[i].room_field[j][k] = rooms[i].room_field[j][k];
            }
        }
    }
    printf("UPDATE GAME ROOMS\n");
    print_game_rooms(GAME_ROOMS, NUM_GAMES);
}