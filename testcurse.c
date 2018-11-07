#include <ncurses.h>

int main(void)
{
  initscr();
  printw("hello, world!\n");
  refresh();
  getch();
  endwin();
  return 0;
}
