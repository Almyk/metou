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
void printinput_s(char *buffer, int row, int col, WINDOW* win, short color = 0);
void printinput_ctoi(char c, int row, int col, WINDOW *win, short color);
void printinput_ntoi(char *buf, int row, int col, WINDOW *win, short color);
void sig_handler(int signo);
void p_exit(void);
WINDOW * create_newwin(int height, int width, int starty, int startx);
void scrollwin(WINDOW *win, int n);
void rcv_stdin(char *buf, int *size, int *cur_r, int cur_c, int sock);

// ncurses windows
WINDOW * input_win;
WINDOW * chat_win;
WINDOW * info_win;

// constant strings
char newusr[] = "New user connected";
char disc[] = "User disconnected";

// max row and col values
int row, col;
