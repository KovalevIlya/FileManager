#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define N 0

struct dirent ** dir_left, ** dir_right;
char path_left[PATH_MAX], path_right[PATH_MAX];
int n_left, n_right;
struct stat st;
int window;

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
  exit(ex);
}

int errort(int err)
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
      printf("Error: the file is not a directory\n");
      break;
    }
    case 4:
    {
      printf("Error: file access denied\n");
      break;
    }
  }
}

void init()
{
  window = 0;
  n_left = scandir(".", &dir_left, NULL, alphasort);
  if (n_left == -1) 
  {
    #if N
    #else 
      errort(1);
    #endif
  }
  n_right = scandir(".", &dir_right, NULL, alphasort);
  if (n_right == -1) 
  {
    #if N
    #else 
      errort(1);
    #endif
  }
  getcwd(path_left, PATH_MAX);
  getcwd(path_right, PATH_MAX);
}

void printt(struct dirent ** dir, char * path, int n)
{
  int i;
  printf("Current directory: %s\n", path);
  for(i = 1; i < n; i++)
  {
    printf("  %d) %s\n", i, dir[i]->d_name);
  }
}

void term(int * temp)
{
  printf("\n");
  if (window == 0)
  {
    printf("LEFT WINDOW\n");
    printt(dir_left, path_left, n_left);
  }
  else
  {
    printf("RIGHT WINDOW\n");
    printt(dir_right, path_right, n_right);
  }
  printf("Enter file number\n");
  printf("Enter '-1' to exit\n");
  printf("Enter '0' to go to another window\n> ");
  scanf("%d", temp);
}

void run()
{
  int temp;
  char * path;
  path = malloc(3);
  while(1)
  {
    #if N
    #else 
      term(&temp);
    #endif
    switch(temp)
    {
      case -1:
      {
        fin(0);
      }
      case 0:
      {
        if (window == 0)
        {
          window = 1;
          chdir(path_right);
        }
        else
        {
          window = 0;
          chdir(path_left);
        }
        break;
      }
      case 1:
      {
        if (window == 0)
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
        if ((temp >= (window == 0 ? n_left : n_right)) || (temp < -1)) 
        {
          #if N
          #else 
            errort(2);
          #endif
          break;
        }
        if (window == 0) dir_left[temp]->d_name;
        else dir_right[temp]->d_name;
        free(path);
        path = malloc(2);
        path[0] = '.';
        path[1] = '/';
        path = strcat(path, window == 0 ? dir_left[temp]->d_name : dir_right[temp]->d_name);
        stat(path, &st);
        if (S_ISDIR(st.st_mode))
        {
          if ((st.st_mode & S_IRUSR) && (st.st_mode & S_IRGRP))
          {
            if (window == 0) 
            {
              n_left = scandir(path, &dir_left, NULL, alphasort);
              chdir(path);
              getcwd(path_left, PATH_MAX);
            }
            else
            {
              n_right = scandir(path, &dir_right, NULL, alphasort);
              chdir(path);
              getcwd(path_right, PATH_MAX);
            }
          }
          else
          {
            #if N
            #else
              errort(4);
            #endif
          }
        }
        else
        {
          #if N
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
}

