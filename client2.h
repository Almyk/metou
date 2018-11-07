#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include <ncurses.h>

#include <signal.h>

#define PORT 8888
#define TRUE 1
#define BUFMAX 1024
