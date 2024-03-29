#include "client.h"

int main(int argc, char const *argv[])
{
  initscr(); // start curses mode
  start_color();
  cbreak();
  keypad(stdscr, TRUE);

  struct sockaddr_in serv_addr;
  int sock = 0;
  int valread;
  char buf_send[BUFMAX] = {0};
  char buf_rcv[BUFMAX] = {0};
  int size;
  int activity;
  int cur_r, cur_c;

  // setup the rows
  getmaxyx(stdscr, row, col);
  cur_r = 1; cur_c = 1;

  // window creations
  input_win = create_newwin(3, COLS, LINES-2, 0);
  chat_win = create_newwin(LINES-5, COLS, 3, 0);
  info_win = create_newwin(3, COLS, 0, 0);

  // init scroll and colors
  scrollok(chat_win, TRUE);
  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  init_pair(2, COLOR_RED, COLOR_BLACK);
  init_pair(3, COLOR_BLACK, COLOR_WHITE);


  if(signal(SIGINT, sig_handler) == SIG_ERR)
    printw("Cant't catch SIGINT\n");


  // set of socket descriptors.
  fd_set readfds;

  if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printw("\n Socket creation error\n");
    getch();
    p_exit();
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // convert ipv4 and ipv6 addresses from text to binary form
  if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printw("\nInvalid address/ Address not supported\n");
    p_exit();
  }

  if(connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
  {
    printw("\nConnection failed\n");
    p_exit();
  }

  // get and print greeting message in bold and underlined
  valread = read(sock, buf_rcv, BUFMAX);
  wattron(info_win, A_BOLD | A_UNDERLINE);
  printinput_s(buf_rcv, 1, COLS/2 - strlen(buf_rcv)/2, info_win, 2);
  wattroff(info_win, A_BOLD | A_UNDERLINE);
  box(info_win, 0, 0);
  wrefresh(info_win);

  memset(buf_rcv, 0, BUFMAX); // clear the buffer

  move(row-1,1);
  while(TRUE)
  {
    getmaxyx(stdscr, row, col); // macro, not a function
    refresh();

    FD_ZERO(&readfds); // clear readfds
    FD_SET(0, &readfds); // add stdin to readfds
    FD_SET(sock, &readfds); // add socket to server

    // listen for activity on readfds
    activity = select(sock + 1, &readfds, NULL, NULL, NULL);
    if(activity < 0)
    {
      //printw("Error: select error");
      continue;
    }

    // input from stdin
    if(FD_ISSET(0, &readfds))
    {
      rcv_stdin(buf_send, &size, &cur_r, cur_c, sock);
    }

    // else it is input from server
    else if((valread = read(sock, buf_rcv, BUFMAX)) > 0)
    {
      rcv_server(buf_rcv, &cur_r, cur_c);
      memset(buf_rcv, 0, BUFMAX);
    }
  }

  endwin();
  return 0;
}

int getinput(char *buffer, int *size)
{
  static int i = 0;
  int temp;
  int done = 0;

  if(i == 0)
  {
    memset(buffer, 0, BUFMAX);
    buffer[0] = 'M';
  }

  switch(temp = getch())
  {
  case KEY_BACKSPACE:
    // if backspace is entered remove input
    delch();
    buffer[i] = '\0';
    i--;
    break;

  case '\n':
    // on newline we send buffer to server
    if(i == 0) break;
    buffer[i+1] = '\0';
    done = 1;
    i = 0;
    break;

  default:
    i++;
    buffer[i] = temp;
    *size = i+1;
  }

  return done;
}

void printinput_s(char *buffer, int row, int col, WINDOW *win, short color)
{
  // prints a string
  wattron(win, COLOR_PAIR(color));
  mvwaddstr(win, row, col, buffer);
  wrefresh(win);
  wattroff(win, COLOR_PAIR(color));
}

void printinput_ctoi(char c, int row, int col, WINDOW *win, short color)
{
  // accepts a character and casts it to an int
  wattron(win, COLOR_PAIR(color));
  mvwprintw(win, row, col, "%d", (unsigned)c);
  wrefresh(win);
  wattroff(win, COLOR_PAIR(color));
}

void printinput_ntoi(char *buf, int row, int col, WINDOW *win, short color)
{
  // network to integer
  // accepts an int divided into 4 bytes and converts it to an int
  int temp = buf[0]; temp = temp << 24;
  temp += buf[1]; temp = temp << 16;
  temp += buf[2]; temp = temp << 8;
  temp += buf[3];
  wattron(win, COLOR_PAIR(color));
  mvwprintw(win, row, col, "%d", temp);
  wrefresh(win);
  wattroff(win, COLOR_PAIR(color));
}

void sig_handler(int signo)
{
  if(signo == SIGINT)
    p_exit();
}

void p_exit(void)
{
  endwin();
  exit(0);
}

WINDOW * create_newwin(int height, int width, int starty, int startx)
{
  WINDOW *new_win;
  new_win = newwin(height, width, starty, startx);
  refresh();
  box(new_win, 0, 0);
  wrefresh(new_win);
  return new_win;
}

void scrollwin(WINDOW *win, int n)
{
  wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  wscrl(win, 1);
  box(win, 0, 0);
  wrefresh(win);
}

void rcv_stdin(char *buf, int *size, int *cur_r, int cur_c, int sock)
{
  static int done = 0;
  done = getinput(buf, size);
  if(done == 1)
  {
    wclear(input_win);
    box(input_win, 0, 0);
    wrefresh(input_win);

    if(size > 0)
    {
      if(*cur_r == row - 6)
      {
        scrollwin(chat_win, 1);
        (*cur_r)--;
      }
      printinput_s(buf+1, *cur_r, cur_c, chat_win, 1);
      (*cur_r)++;
      send(sock, buf, *size, 0);
    }
    move(row-1,1);
  }
}

void rcv_server(char *buf, int *cur_r, int cur_c)
{
  switch(buf[0])
  {
    case 'M': // incoming message
      if(*cur_r == row - 6)
      {
        scrollwin(chat_win, 1);
        (*cur_r)--;
      }
      printinput_s(buf+1, *cur_r, cur_c, chat_win);
      (*cur_r)++;
      break;
    case 'C': // new connection count
      if(*cur_r == row - 6)
      {
        scrollwin(chat_win, 1);
        (*cur_r)--;
      }
      printinput_s(newusr, (*cur_r)++, cur_c, chat_win, 2);
      printinput_ntoi(buf+1, 1, COLS-5, info_win, 2);
      break;
    case 'D': // user disconnected
      if(*cur_r == row - 6)
      {
        scrollwin(chat_win, 1);
        (*cur_r)--;
      }
      printinput_s(disc, *cur_r, cur_c, chat_win, 2);
      (*cur_r)++;
      printinput_ntoi(buf+1, 1, COLS-5, info_win, 2);
      break;
  }
}
