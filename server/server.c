#include "header.h"

// Global variables
extern int SERVER_STATE;

room_info GAME_ROOMS[NUM_GAMES];

int main(int argc, char** argv)
{
    char status[256];

    int epoll_fd;
    int server_fd;
    int server_port = 5555;
    
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    struct epoll_event *events;

    socklen_t client_len = sizeof(client_addr);

    if (argc < 3) {
        err_msg("", "not enought argumens");
        printf("usage: ./server [port] [state]\n");
        return 1;
    }
    
    // Initialize server state
    SERVER_STATE = atoi(argv[2]);
    
    // Inintialize server port
    server_port = atoi(argv[1]);

    // Initialize game rooms
    initialize_game_rooms(GAME_ROOMS, NUM_GAMES);

    print_game_rooms(GAME_ROOMS, NUM_GAMES);

    if (SERVER_STATE == 0) {
        log_msg("[LOG] CREATE MAIN SERVER       - OK", 1);
        create_main_server(server_port, argv[1]);
    } else if (SERVER_STATE == 1) {
        log_msg("[LOG] CREATE RESERVE SERVER    - OK", 1);
        create_reserve_server(server_port, argv[1]);
    } else {
        err_msg(" Failed to associate server state.", "");
        return -1;
    }

    return 0;
}