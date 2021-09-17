#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFERSIZE 80

int list_all = false;
bool long_format = false;

void usage()
{
  fprintf(stderr, "Usage: myls [-l] [-a] [files...]\n");
}

void print_long_format(struct dirent *de, struct stat *s)
{
    // file type | permissions | number | owner | group | size | last modified | file name
    nlink_t links = s->st_nlink;
    uid_t uid = s->st_uid;
    gid_t gid = s->st_gid;
    off_t size = s->st_size;
    struct timespec last_access = s->st_ctim;
    struct timespec last_modified = s->st_atim;
    struct timespec last_status_change = s->st_mtim;
    char ftype;
    switch (s->st_mode & S_IFMT)
    {
      case S_IFBLK: ftype = 'a'; break;
      case S_IFCHR: ftype = 'b'; break;
      case S_IFDIR: ftype = 'd'; break;
      case S_IFIFO: ftype = 'c'; break;
      case S_IFLNK: ftype = 'e'; break;
      case S_IFREG: ftype = '-'; break;
      case S_IFSOCK: ftype = 'g'; break;
    }
    printf("%c %s\n", ftype, de->d_name);
}

void print_dirent(struct dirent *de, char *dirname)
{
  if (!list_all && de->d_name[0] == '.')
    return;
  if (!long_format)
    printf("%s ", de->d_name);
  else
  {
    // make path
    char path[BUFFERSIZE];
    int dnl = strlen(dirname);
    int a = 0;
    strncpy(path, dirname, dnl);
    if (dirname[dnl-1] != '/')
    {
      path[dnl] = '/';
      a = 1;
    }
    strncpy(path+dnl+1, de->d_name, strlen(de->d_name));
    path[dnl + a + strlen(de->d_name)] = '\0';
    printf("path: %s\n", path);

    struct stat s;
    if (stat(path, &s) == -1)
    {
      int errsv = errno;
      printf("myls: error getting stats for %s: ", de->d_name);
      switch (errsv)
      {
        case EACCES: printf("access error"); break;
        case EBADF: printf("badf"); break;
        case EFAULT: printf("bad address"); break;
        default: printf("%d", errsv);
      }
      printf("\n");
      return;
    }
    print_long_format(de, &s);
  }
}

void print_dir(const char *dirname)
{
  DIR *dir;
  struct dirent *de;

  if ((dir = opendir(dirname)) == NULL)
  {
    int errsv = errno;
    printf("myls: cannot open '%s': ", dirname);
    switch (errsv)
    {
      case EACCES: printf("cannot acccess file or directory"); break;
      case ENOENT: printf("no such file or directory"); break;
      default: printf("errno: %d", errsv); break;
    }
    printf("\n");
    return;
  }

  errno = 0;
  while ((de = readdir(dir)) != NULL)
  {
    if (errno != 0)
    {
      int errsv = errno;
      switch (errsv)
      {
        case EBADF: printf("myls: cannot access dir entry: invalid directory stream"); break;
        default: printf("errno: %d", errsv);
      }
      printf("\n");
      return;
    }
    else
      print_dirent(de, dirname);
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
      case 'l': long_format = true; break;
      case 'a': list_all = true; break;
      default: usage(); exit(1);
    }
  }
  struct stat s;
  if (optind != argc)
    for (int i = optind; i < argc; i++)
      print_dir(argv[i]);
  else
    print_dir(".");
}
