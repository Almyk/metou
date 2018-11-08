#include "client2.h"

void getinput(char *buffer, int *size);
void sig_handler(int signo);

int main(int argc, char const *argv[])
{
  initscr(); // start curses mode

  int sock = 0;
  int valread;
  struct sockaddr_in serv_addr;
  char buf_send[BUFMAX] = {0};
  char buf_rcv[BUFMAX] = {0};
  int size;
  int activity;
  int row, col;
  int cur_r, cur_c;

  if(signal(SIGINT, sig_handler) == SIG_ERR)
    printw("Cant't catch SIGINT\n");

  // setup the rows
  getmaxyx(stdscr, row, col);
  cur_r = 1; cur_c = 0;

  // set of socket descriptors.
  fd_set readfds;

  if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    printw("\n Socket creation error\n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // convert ipv4 and ipv6 addresses from text to binary form
  if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0){
    printw("\nInvalid address/ Address not supported\n");
    return -1;
  }

  if(connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    printw("\nConnection failed\n");
    return -1;
  }

  // get and print greeting message in bold and underlined
  valread = read(sock, buf_rcv, BUFMAX);
  attron(A_BOLD | A_UNDERLINE);
  addstr(buf_rcv);
  attroff(A_BOLD | A_UNDERLINE);

  memset(buf_rcv, 0, BUFMAX); // clear the buffer

  while(TRUE){
    getmaxyx(stdscr, row, col); // macro, not a function
    move(row-1,0);
    refresh();

    FD_ZERO(&readfds); // clear readfds
    FD_SET(0, &readfds); // add stdin to readfds
    FD_SET(sock, &readfds); // add socket to server

    // listen for activity on readfds
    activity = select(sock + 1, &readfds, NULL, NULL, NULL);
    if(activity < 0){
      //printw("Error: select error");
      continue;
    }

    // input from stdin
    if(FD_ISSET(0, &readfds)){
      getinput(buf_send, &size);
      if(size > 0) send(sock, buf_send, size, 0);
    }

    // else it is input from server
    else{
      if((valread = read(sock, buf_rcv, BUFMAX)) > 0){
        mvaddstr(cur_r++, cur_c, buf_rcv);
        if(cur_r == row - 1) cur_r--;
        memset(buf_rcv, 0, BUFMAX);
      }
    }
  }

  endwin();
  return 0;
}

void getinput(char *buffer, int *size)
{
  memset(buffer, 0, BUFMAX);
  getstr(buffer);
  *size = strlen(buffer);
}

void sig_handler(int signo)
{
  if(signo == SIGINT)
  {
    endwin();
    exit(0);
  }
}
