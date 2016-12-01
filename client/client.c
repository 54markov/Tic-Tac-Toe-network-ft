#include "header.h"

int CONF_LIST[] = {	2345, 	 // Main server port
					2346, 	 // First reserve server port
					2347 };  // Second reserve server port

int GAME_ROOM; // gome room number
int PLAYER;    // player X or O
int SERVER_FD; // server

// print network message
void print_msg(common_msg msg)
{
    switch(msg.type)
    {
        case AUTH:
            printf("AUTH:\n");
            printf("room number %d\n", msg.auth.number);
            if (msg.auth.player == ZERO) {
                printf("player: ZERO\n");
            } else {
                printf("player: CROSS\n");
            }
            if (msg.auth.start == NO ) {
                printf("state: wait\n");
            } else {
                printf("state: play\n");
            } 
            break;      
    }
}

// initialize socket
static struct sockaddr_in configure_socket(struct sockaddr_in server, int port)
{
    bzero(&server, sizeof(server));

    server.sin_family = AF_INET;
    // переводим в сетевой порядок
    server.sin_port = htons(port);
    // преобразует строку с адресом в двоичную форму с сетевым порядком
    inet_aton(SERVER_IP, &(server.sin_addr));

    return server;   
}

// try to connect to server
int try_to_connect(int server_fd, int mode)
{
    int i, fail_conn;
    struct sockaddr_in server_addr;
    common_msg msg;
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0); // initialize socket 
    if (server_fd < 0 ) {
        fprintf(stderr, "error: can't create socket\n");
        return -1;
    }

    // Loop for the try to connect (main or reserve0 or reserve1)
    for (i = 0, fail_conn = 0; i < NUM_SERVER; i++) {
        // Configure socket by differnt port
        server_addr = configure_socket(server_addr, CONF_LIST[i]);
        // Try to connect
        if (connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            // Fail, reconnect to another
            //fprintf(stderr, "error: can't connect to %s:%d. try to another...\n", SERVER_IP, CONF_LIST[i]);
            fail_conn++;
        } else {
            // Success
            switch(mode)
            {
                case 0: // branch new connection
                    printf("connected to %s:%d\n",SERVER_IP, CONF_LIST[i]);
                    msg.type = AUTH;  
                    msg.auth.new_game = YES;
                    write(server_fd, &msg, sizeof(msg));
                    bzero(&msg, sizeof(msg));
                    read(server_fd, &msg, sizeof(msg));
                    
                    // set field game room and player X or O
                    GAME_ROOM = msg.auth.number; 
                    PLAYER = msg.auth.player;
                    print_msg(msg);
                    break;

                case 1: // branch restore connection
                    displayMsgChat("connection restore...\n");

                    msg.type = AUTH;  
                    msg.auth.new_game = NO;
                    msg.auth.number = GAME_ROOM;
                    msg.auth.player = PLAYER; 
                    write(server_fd, &msg, sizeof(msg));
                    break;
                
                default:
                    break;
            }
            break;
        }
        
        if (fail_conn == NUM_SERVER) {
            // All server are down
            fprintf(stderr, "error: all servers are down, shutting down...\n");
            close(server_fd);
            return -1;
        }
    }
    return server_fd;
}

int main(int argc, char const *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s [single]/[multiplayer]\n", argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "single") == 0) {
        single_game_session();
    } else if (strcmp(argv[1], "multiplayer") == 0) {
        SERVER_FD = try_to_connect(SERVER_FD, 0);
        if (SERVER_FD == -1) {
            return -1;
        }
        game_session(SERVER_FD, 1);
        close(SERVER_FD);
    }

    return 0;
}