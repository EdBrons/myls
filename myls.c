#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define PATHBUFFER 80

int list_all = false;
bool long_format = false;
char path_buf[PATHBUFFER];

void usage()
{
  fprintf(stderr, "Usage: myls [-l] [-a] [files...]\n");
}

void print_dirent(struct dirent *de)
{
  if (!list_all && de->d_name[0] == '.')
    return;
  if (!long_format)
    printf("%s ", de->d_name);
  else
  {
    struct stat s;
    if (stat(de->d_name, &s) == -1)
    {
      int errsv = errno;
      printf("Error getting stats on file: %d\n", errsv);
      return;
    }
    printf("something something something %s\n", de->d_name);
  }
}

void print_dir(const char *dirname)
{
  DIR *dir;
  struct dirent *de;
  getcwd(path_buf, PATHBUFFER);
  if ((dir = opendir(dirname)) == NULL)
  {
    int errsv = errno;
    printf("myls: cannot open '%s': ", dirname);
    switch (errsv)
    {
      case EACCES:
        printf("cannot acccess file or directory\n");
        break;
      case ENOENT:
        printf("no such file or directory\n");
        break;
      default:
        printf("errno: %d\n", errsv);
        break;
    }
    return;
  }
  errno = 0;
  while ((de = readdir(dir)) != NULL)
  {
    if (errno != 0)
    {
      int errsv = errno;
      if (errsv == EBADF)
      {
        printf("myls: cannot access dir entry: invalid directory stream");
        return;
      }
    }
    else
    {
      print_dirent(de);
    }
    errno = 0;
  }
  printf("\n");
}

int main(int argc, char *argv[])
{
  int opt;
  while ((opt = getopt(argc, argv, "la")) != -1)
  {
    switch (opt)
    {
      case 'l':
        long_format = true;
        break;
      case 'a':
        list_all = true;
        break;
      default:
        usage();
        exit(1);
    }
  }
  if (optind != argc)
  {
    for (int i = optind; i < argc; i++)
    {
      print_dir(argv[i]);
    }
  }
  else
  {
    print_dir(path_buf);
  }
}
