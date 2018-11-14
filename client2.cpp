#include "client2.h"

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
  int row, col;
  int cur_r, cur_c;
  int done = 0;

  // setup the rows
  getmaxyx(stdscr, row, col);
  cur_r = 1; cur_c = 1;

  // window creations
  WINDOW * input_win = create_newwin(3, COLS, LINES-2, 0);
  WINDOW * chat_win = create_newwin(LINES-5, COLS, 3, 0);
  WINDOW * info_win = create_newwin(3, COLS, 0, 0);

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
  printinput(buf_rcv, 1, COLS/2 - strlen(buf_rcv)/2, info_win, 2);
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
      done = getinput(buf_send, &size);
      if(done == 1)
      {
        wclear(input_win);
        box(input_win, 0, 0);
        wrefresh(input_win);
          
        if(size > 0)
        {
          if(cur_r == row - 6)
          {
            scrollwin(chat_win, 1);
            cur_r--;
          }
          printinput(buf_send+1, cur_r++, cur_c, chat_win, 1);
          send(sock, buf_send, size, 0);
        }
        move(row-1,1);
      }
    }

    // else it is input from server
    else
    {
      if((valread = read(sock, buf_rcv, BUFMAX)) > 0)
      {
        // TODO: make this into a function for handling diff IO requests
        switch(buf_rcv[0])
        {
          case 'M': // incoming message
            if(cur_r == row - 6)
            {
              scrollwin(chat_win, 1);
              cur_r--;
            }
            printinput(buf_rcv+1, cur_r++, cur_c, chat_win);
            memset(buf_rcv, 0, BUFMAX);
            break;
          case 'C': // new connection count
            if(cur_r == row - 6)
            {
              scrollwin(chat_win, 1);
              cur_r--;
            }
            char newusr[] = "New user connected";
            printinput(newusr, cur_r++, cur_c, chat_win, 2);
            break;
        }
      }
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

void printinput(char *buffer, int row, int col, WINDOW *win, short color)
{
  wattron(win, COLOR_PAIR(color));
  mvwaddstr(win, row, col, buffer);
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
