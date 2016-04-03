#ifndef HEADER_H
#define HEADER_H

#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/poll.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdbool.h> // for bool type`

#include "../lib/message.h"

#define COLOR_RED "\033[31m" // red
#define COLOR_GRN "\033[32m" // green
#define COLOR_YEL "\033[33m" // yellow
#define COLOR_BLU "\033[34m" // blue
#define COLOR_MAG "\033[35m" // magneta
#define COLOR_CYN "\033[36m" // cyan
#define COLOR_WHT "\033[37m" // white
#define COLOR_OFF "\033[0m"  // reset

#define SERVERPORT  2345
#define MAXCONN     200
#define MAXEVENTS   100
#define MAXLEN      255
#define NUM_GAMES 	4
#define MAXBUFF 	200
#define NUM_SERVER 	3          // Amount of server
#define SERVER_IP 	"127.0.0.1"// Local host adrress


struct EchoEvent
{
    int         fd;
    int         type; // client = '0' / server = '1'
    uint32_t    event;
    char        data[MAXLEN];
    int         length;
    int         offset;
    common_msg  msg;
};

void err_msg(char *msg, char *err); // display error message
void log_msg(char *msg, int color); // display log message

// Common sokets functions
int create_socket(int fd);
int close_socket(int fd);
int bind_socket(int fd, struct sockaddr_in server_addr, int port);
int listen_socket(int fd);

// Epoll socket functions
int create_epoll(int epoll_fd);
int make_socket_non_blocking(int fd);
void *handle(void *ptr, int epoll_fd);
int modify_epoll_context(int epoll_fd, int operation, int server_fd, uint32_t events, void* data);

// Server state main function
int create_main_new_server(int server_port, char *argc);
int create_main_server(int server_port, char *argc);
int create_reserve_server(int server_port, char *argc);

// Game room functions
void initialize_game_rooms(room_info games_room[], int size);
void print_game_rooms(room_info *games_room, int size);
void print_game_room(room_info games_room);
void update_rooms(common_msg msg);
void free_room(int index);
int get_free_room();

// Accept player and reserve server
void accept_new_client(common_msg msg, int fd);
void accept_new_rserver(common_msg msg, int fd);

// Get and set player
int set_new_player_in_room(int fd, int *type_of_player, int *action);
int get_another_client(int fd, int *index);

// Game and chat routine-handler
void chat_routine(common_msg msg);
void game_routine(common_msg msg);
void game_handler(int fd, int event);

// Broadcast to reserve server message
void broadcast_to_reserve(common_msg data, int lenght, void *ptr);
// Gather update message for broadcast
common_msg send_update(common_msg msg, int fd);

#endif