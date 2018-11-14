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
#define FALSE 0
#define BUFMAX 512

int getinput(char *buffer, int *size);
void printinput(char *buffer, int row, int col, WINDOW* win, short color = 0);
void sig_handler(int signo);
void p_exit(void);
WINDOW * create_newwin(int height, int width, int starty, int startx);
void scrollwin(WINDOW *win, int n);
