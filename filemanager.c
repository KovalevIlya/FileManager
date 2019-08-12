#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <panel.h>
#include <signal.h>

#define N 1
#define MYKEY_TAB 9
#define MYKEY_ESCAPE 27
#define MYKEY_ENTER 10

struct dirent ** dir_left, ** dir_right;
char path_left[PATH_MAX], path_right[PATH_MAX];
int n_left, n_right;
struct stat st;
int win;
#if N
  int curl, curr;
#endif
WINDOW * leftwin, * rightwin, * errorwin;
WINDOW * subleftwin, * subrightwin, * suberrorwin;
PANEL * leftpan, * rightpan, * errorpan;


void fin(int ex)
{
  while (n_right--) 
  {
    free(dir_right[n_right]);
  }
  free(dir_right);
  while (n_left--) 
  {
    free(dir_left[n_left]);
  }
  free(dir_left);
  #if N
    del_panel(leftpan);
    del_panel(rightpan);
    del_panel(errorpan);
    delwin(leftwin);
    delwin(rightwin);
    delwin(errorwin);
    delwin(subleftwin);
    delwin(subrightwin);
    delwin(suberrorwin);
    endwin();
  #endif    
  exit(ex);
}

void errort(int err)
{
  switch(err)
  {
    case 1:
    {
      printf("Error: directory access error\n");
      fin(1);
    }
    case 2:
    {
      printf("Error: invalid command entered\n");
      break;
    }
    case 3:
    {
      printf("Error: file is not a directory\n");
      break;
    }
    case 4:
    {
      printf("Error: file access denied\n");
      break;
    }
  }
}

void errorw(int err)
{
  int i;
  top_panel(errorpan);
  update_panels();
  mvwprintw(suberrorwin, 0, getmaxx(suberrorwin) / 2 - 3, "ERROR!");
  wmove(suberrorwin, 1, 0);
  for (i = 0; i < getmaxx(suberrorwin); i++)
  {
    wprintw(suberrorwin, "-");
  }
  wmove(suberrorwin, 2, 0);
  switch(err)
  {
    case 1:
    {
      wprintw(suberrorwin, "Directory access error\n");
    }
    case 2:
    {
      wprintw(suberrorwin, "Invalid command entered\n");
      break;
    }
    case 3:
    {
      wprintw(suberrorwin, "File is not a directory\n");
      break;
    }
    case 4:
    {
      wprintw(suberrorwin, "File access denied\n");
      break;
    }
  }
  wmove(suberrorwin, getmaxy(suberrorwin) - 2, 0);
  for (i = 0; i < getmaxx(suberrorwin); i++)
  {
    wprintw(suberrorwin, "-");
  }
  wmove(suberrorwin, getmaxy(suberrorwin) - 1, 0);
  wprintw(suberrorwin, "Press any key to continue..");
  doupdate();
  wgetch(suberrorwin);
  bottom_panel(errorpan);
  update_panels();
  doupdate();
  if (err == 1) fin(1);
}

void initw()
{
  del_panel(leftpan);
  del_panel(rightpan);
  del_panel(errorpan);
  delwin(leftwin);
  delwin(rightwin);
  delwin(errorwin);
  delwin(subleftwin);
  delwin(subrightwin);
  delwin(suberrorwin);

  bkgd(COLOR_PAIR(1));

  leftwin = newwin(getmaxy(stdscr), getmaxx(stdscr) / 2, 0, 0);
  wbkgd(leftwin, COLOR_PAIR(1));
  wclear(leftwin);
  box(leftwin, 0, 0);
  subleftwin = derwin(leftwin, getmaxy(leftwin) - 2, getmaxx(leftwin) - 2, 1, 1);
  wrefresh(leftwin);
  
  rightwin = newwin(getmaxy(stdscr), getmaxx(stdscr) / 2, 0, getmaxx(stdscr) / 2);
  wbkgd(rightwin, COLOR_PAIR(1));
  wclear(rightwin);
  box(rightwin, 0, 0);
  subrightwin = derwin(rightwin, getmaxy(rightwin) - 2, getmaxx(rightwin) - 2, 1, 1);
  wrefresh(rightwin);

  errorwin = newwin(getmaxy(stdscr) / 3, getmaxx(stdscr) / 2, getmaxy(stdscr) / 3, getmaxx(stdscr) / 4);
  wbkgd(errorwin, COLOR_PAIR(2));
  wclear(errorwin);
  box(errorwin, 0, 0);
  suberrorwin = derwin(errorwin, getmaxy(errorwin) - 2, getmaxx(errorwin) - 2, 1, 1); 
  wrefresh(errorwin);

  leftpan = new_panel(leftwin);
  rightpan = new_panel(rightwin);
  errorpan = new_panel(errorwin);
  bottom_panel(errorpan);
  update_panels();
  doupdate();
}

void sig_winch(int signo)
{
  struct winsize size;
  ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);
  resizeterm(size.ws_row, size.ws_col);
  initw();
}

void print_w(WINDOW ** wnd, struct dirent ** dir, char * path, int n, int cur)
{
  int i, j;
  wclear(* wnd);
  wattron(* wnd, COLOR_PAIR(1));
  mvwprintw(* wnd, 0, 0, "%s", path);
  wmove(* wnd, 2, 0);
  for (j = 0; j < getmaxx(* wnd); j++)
  {
    wprintw(* wnd, "-");
  } 
  wmove(* wnd, 3, 0);
  for (i = 1; i < n; i++)
  {
    wattron(* wnd, cur == i ? COLOR_PAIR(3) : COLOR_PAIR(1));
    wprintw(* wnd, "%s\n", dir[i]->d_name);
  }
  wrefresh(* wnd);
}

void init()
{
  #if N
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_RED, COLOR_WHITE);
    init_pair(3, COLOR_BLACK, COLOR_YELLOW);
    signal(SIGWINCH, sig_winch);
    initw();
    keypad(subleftwin, 1);
    keypad(subrightwin, 1);
    curl = 1;
    curr = 1;
  #endif
  win = 0;
  n_left = scandir(".", &dir_left, NULL, alphasort);
  if (n_left == -1) 
  {
    #if N
      errorw(1);
    #else 
      errort(1);
    #endif
  }
  n_right = scandir(".", &dir_right, NULL, alphasort);
  if (n_right == -1) 
  {
    #if N
      errorw(1);
    #else 
      errort(1);
    #endif
  }
  getcwd(path_left, PATH_MAX);
  getcwd(path_right, PATH_MAX);
  #if N
    print_w(&subleftwin, dir_left, path_left, n_left, 0);
    print_w(&subrightwin, dir_right, path_right, n_right, 0);
  #endif
}

void print_t(struct dirent ** dir, char * path, int n)
{
  int i;
  printf("Current directory: %s\n", path);
  for(i = 1; i < n; i++)
  {
    printf("  %d) %s\n", i, dir[i]->d_name);
  }
}

int term()
{
  int temp;
  printf("\n");
  if (win == 0)
  {
    printf("LEFT win\n");
    print_t(dir_left, path_left, n_left);
  }
  else
  {
    printf("RIGHT win\n");
    print_t(dir_right, path_right, n_right);
  }
  printf("Enter file number\n");
  printf("Enter '-1' to exit\n");
  printf("Enter '0' to go to another win\n> ");
  scanf("%d", &temp);
  return temp;
}

int wind()
{
  int temp;
  if (win == 0)
  {
    print_w(&subleftwin, dir_left, path_left, n_left, curl);
    temp = wgetch(subleftwin);
    switch(temp)
    {
      case KEY_UP:
      {
        curl = curl == 1 ? curl : curl - 1;
        return -2;
      } 
      case KEY_DOWN:
      {
        curl = curl == (n_left - 1) ? curl : curl + 1;
        return -2;
      } 
      case MYKEY_TAB: 
      {
        return 0;
      } 
      case MYKEY_ESCAPE:
      {
        return -1;
      }
      case MYKEY_ENTER:
      {
        return curl;
      }
      default:
        return temp; 
    }
  }
  else
  {
    print_w(&subrightwin, dir_right, path_right, n_right, curr);
    temp = wgetch(subleftwin);
    switch(temp)
    {
      case KEY_UP:
      {
        curr = curr == 1 ? curr : curr - 1;
        return -2;
      } 
      case KEY_DOWN:
      {
        curr = curr == (n_right - 1) ? curr : curr + 1;
        return -2;
      } 
      case MYKEY_TAB:
      {
        return 0;
      } 
      case MYKEY_ESCAPE:
      {
        return -1;
      }
      case MYKEY_ENTER:
      {
        return curr;
      }
      default:
        return temp;
    }
  }
}

void run()
{
  int temp;
  char * path;
  path = malloc(3);
  while(1)
  {
    #if N
      temp = wind();
    #else 
      temp = term();
    #endif
    switch(temp)
    {
      #if N
        case -2:
          break;
      #endif
      case -1:
      {
        fin(0);
      }
      case 0:
      {
        if (win == 0)
        {
          win = 1;
          chdir(path_right);
          #if N
            print_w(&subleftwin, dir_left, path_left, n_left, 0);
          #endif
        }
        else
        {
          win = 0;
          chdir(path_left);
          #if N
            print_w(&subrightwin, dir_right, path_right, n_right, 0);
          #endif
        }
        break;
      }
      case 1:
      {
        if (win == 0)
        {
          n_left = scandir(dir_left[temp]->d_name, &dir_left, NULL, alphasort);
          chdir(dir_left[temp]->d_name);
          getcwd(path_left, PATH_MAX);
        }
        else
        {
          n_right = scandir(dir_right[temp]->d_name, &dir_right, NULL, alphasort);
          chdir(dir_right[temp]->d_name);
          getcwd(path_right, PATH_MAX);
        }
        break;
      }
      default:
      {
        if ((temp >= (win == 0 ? n_left : n_right)) || (temp < -1)) 
        {
          #if N
            errorw(2);
          #else 
            errort(2);
          #endif
          break;
        }
        if (win == 0) dir_left[temp]->d_name;
        else dir_right[temp]->d_name;
        free(path);
        path = malloc(2);
        path[0] = '.';
        path[1] = '/';
        path = strcat(path, win == 0 ? dir_left[temp]->d_name : dir_right[temp]->d_name);
        stat(path, &st);
        if (S_ISDIR(st.st_mode))
        {
          if ((st.st_mode & S_IRUSR) && (st.st_mode & S_IRGRP))
          {
            if (win == 0) 
            {
              n_left = scandir(path, &dir_left, NULL, alphasort);
              chdir(path);
              getcwd(path_left, PATH_MAX);
              #if N
                curl = 1;
              #endif
            }
            else
            {
              n_right = scandir(path, &dir_right, NULL, alphasort);
              chdir(path);
              getcwd(path_right, PATH_MAX);
              #if N
                curr = 1;
              #endif
            }
          }
          else
          {
            #if N
              errorw(4);
            #else
              errort(4);
            #endif
          }
        }
        else
        {
          #if N
            errorw(3);
          #else
            errort(3);
          #endif
        }
      }
    }
  }
}

int main(void)
{
  init();
  run();
  fin(0);
}

