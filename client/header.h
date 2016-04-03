#ifndef HEADER_H
#define HEADER_H

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

#include <sys/poll.h>

#include "../lib/message.h"

#define COLOR_RED "\033[31m" // red
#define COLOR_GRN "\033[32m" // green
#define COLOR_YEL "\033[33m" // yellow
#define COLOR_BLU "\033[34m" // blue
#define COLOR_MAG "\033[35m" // magneta
#define COLOR_CYN "\033[36m" // cyan
#define COLOR_WHT "\033[37m" // white
#define COLOR_OFF "\033[0m"  // reset

#define MAXBUFF 200
#define NUM_SERVER 3 		 // Amount of server
#define SERVER_IP "127.0.0.1"// Local host adrress

#endif