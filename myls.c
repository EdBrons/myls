#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// base buffer sized used in program
#define BUFFERSIZE 80

// globals used for formatting
int list_all = false;
bool long_format = false;

// describes usage of program for user
void usage()
{
  fprintf(stderr, "Usage: ./myls [-l] [-a] [files...]\n");
}

// prints the dirent with long format
void print_long_format(struct dirent *de, struct stat *s)
{
    // function not complete yet
    // the function should print the following:
    // file type | permissions | number | owner | group | size | last modified | file name
    nlink_t links = s->st_nlink;
    // check filetype of de
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
    uid_t uid = s->st_uid;
    gid_t gid = s->st_gid;
    // size of file in bytes
    off_t size = s->st_size;
    struct timespec last_access = s->st_ctim;
    struct timespec last_modified = s->st_atim;
    struct timespec last_status_change = s->st_mtim;

    printf("%c %s\n", ftype, de->d_name);
}

// prints dirent depending on the value of list_all and long_format
// if long_format is true reads the stat struct for that dirent
// then calls long_format
void print_dirent(struct dirent *de, const char *dirname)
{
  if (!list_all && de->d_name[0] == '.')
    return;
  if (!long_format)
    printf("%s ", de->d_name);
  else
  {
    // redo how the path is constructed sometime
    // this section of code creates a buffer filled with:
    // "dirname/de->d_name\0" to be passed to stat
    // buffer to hold path string
    char path[BUFFERSIZE];
    // this if statement here handles the case of when dirname is /
    if (strcmp(dirname, "/") == 0)
    {
      path[0] = '/';
      path[1] = '\0';
    }
    // otherwise copy dirname into buffer
    // and make sure it ends with a /
    else
    {
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
    }

    // get stat and check for error
    struct stat s;
    if (stat(path, &s) == -1)
    {
      int errsv = errno;
      printf("myls: error getting stats for %s: ", path);
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

// opens dirent
// checks for errors
// loops through dirents
// check for errors when reading dirents
// calls print_dirent
void print_dir(const char *dirname)
{
  DIR *dir;
  struct dirent *de;

  // open dirent and check if error occured
  if ((dir = opendir(dirname)) == NULL)
  {
    // save error
    int errsv = errno;
    printf("myls: cannot open '%s': ", dirname);
    switch (errsv)
    {
      case EACCES: printf("cannot acccess file or directory"); break;
      case ENOENT: printf("no such file or directory"); break;
      case ENOTDIR: printf("hmm"); break;
      default: printf("errno: %d", errsv); break;
    }
    printf("\n");
    return;
  }

  // reset errno
  errno = 0;
  // read dirent and check if error occured
  while ((de = readdir(dir)) != NULL)
  {
    if (errno != 0)
    {
      // save error
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
      // if no error occured print dir entry
      print_dirent(de, dirname);
    errno = 0;
  }
  
  closedir(dir);

  printf("\n");
}

int main(int argc, char *argv[])
{
  // parse arguments
  int opt;
  while ((opt = getopt(argc, argv, "la")) != -1)
  {
    switch (opt)
    {
      // use long format
      case 'l': long_format = true; break;
      // list all files
      case 'a': list_all = true; break;
      // invalid argument
      default: usage(); exit(1);
    }
  }
  // print dirs given by args
  if (optind != argc)
    for (int i = optind; i < argc; i++)
      print_dir(argv[i]);
  // else print curent dir
  else
    print_dir(".");
}
