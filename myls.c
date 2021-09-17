#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define PATHBUFFER 80

int list_all = false;
bool long_format = false;
char abs_path_buf[PATHBUFFER];

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

void print_dirent(struct dirent *de)
{
  if (!list_all && de->d_name[0] == '.')
    return;
  if (!long_format)
    printf("%s ", de->d_name);
  else
  {
    struct stat s;
    char pathbuffer[PATHBUFFER*2+1];
    int l = strlen(abs_path_buf);
    int j = strlen(de->d_name);
    strncpy(pathbuffer, abs_path_buf, l);
    pathbuffer[l] = '/';
    strncpy(pathbuffer+l+1, de->d_name, j);
    if (stat(pathbuffer, &s) == -1)
    {
      int errsv = errno;
      printf("Error getting stats on %s: ", pathbuffer);
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
  getcwd(abs_path_buf, PATHBUFFER);
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
      case 'l': long_format = true; break;
      case 'a': list_all = true; break;
      default: usage(); exit(1);
    }
  }
  if (optind != argc)
    for (int i = optind; i < argc; i++)
      print_dir(argv[i]);
  else
    print_dir(".");
}
