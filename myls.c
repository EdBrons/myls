#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <pwd.h>


// base buffer sized used in program
#define BUFFERSIZE 80

#define DATESTRBUFSIZE 13

// globals used for formatting
int list_all = false;
bool long_format = false;
bool multiple = false;
bool printed_prev = false;

// describes usage of program for user
void usage()
{
  fprintf(stderr, "Usage: ./myls [-l] [-a] [files...]\n");
}

// prints the dirent with long format
void print_long_format(const char *fname, struct stat *s)
{
    if (printed_prev)
      printf("\n");

    char ftype;
    off_t size;
   
    switch (s -> st_mode & S_IFMT)
    {
      case S_IFBLK: ftype = 'b'; break;
      case S_IFCHR: ftype = 'c'; break;
      case S_IFDIR: ftype = 'd'; break;
      case S_IFIFO: ftype = 'f'; break;
      case S_IFLNK: ftype = 'l'; break;
      case S_IFREG: ftype = '-'; break;
      case S_IFSOCK: ftype = 's'; break;
    }
    
    uid_t uid = s -> st_uid;  /* User Id of owner */ 
    gid_t gid = s -> st_gid;  /* Group ID of owner */
    size = s -> st_size; /* size of file in bytes */
    intmax_t file_size = (intmax_t) size; 
    uintmax_t hard_link = (uintmax_t) s -> st_nlink; 

    char last_mod [DATESTRBUFSIZE]; /* holds date and time */
    char *fmt = "%b %R";  /* format of date and time  */
    struct tm *local_time = localtime(&s ->st_ctim.tv_sec); /*convert to right format */
    strftime ( last_mod, DATESTRBUFSIZE, fmt, local_time);
     
    struct passwd* user;
    if ((user = getpwuid(uid)) == NULL)
    {
      perror("getpwuid");
      return;
    }

    struct passwd* group;
    if ((group = getpwuid(gid)) == NULL)
    {
      perror("getpwuid");
      return ;
    }

    char *name = user -> pw_name;
    char *group_n =  group -> pw_name;

    char p[11]; /*  hold all permissions */
    for(int i = 0; i < 10; i++){
    	p[i] = '-';
    }
    p[0] = ftype;
    if (S_IRUSR & s-> st_mode) {
    	p[1] = 'r';
    }
    if (S_IWUSR & s-> st_mode) {
    	p[2] = 'w';
    }
    if (S_IXUSR & s-> st_mode) {
    	p[3] = 'x';
    }
    if (S_IRGRP & s-> st_mode) {
    	p[4] = 'r';
    }
    if (S_IWGRP & s-> st_mode) {
    	p[5] = 'w';
    }
    if (S_IXGRP & s-> st_mode) {
    	p[6] = 'x';
    }
    if (S_IROTH & s-> st_mode) {
    	p[7] = 'r';
    }
    if (S_IWOTH & s-> st_mode) {
    	p[8] = 'w';
    }
    if (S_IXOTH & s-> st_mode) {
    	p[9] = 'x';
    }
    p[10] = '\0';
    
    
   printf("%s %ju %s %s %jd %s %s", p, hard_link, name, group_n,
    file_size, last_mod,fname);
   printed_prev = true;
}

// prints dirent depending on the value of list_all and long_format
// if long_format is true reads the stat struct for that dirent
// then calls long_format
// returns true if it printed
bool print_dirent(struct dirent *de, const char *dirname)
{
  if (!list_all && de->d_name[0] == '.')
    return false;
  if (!long_format)
  {
    printf("%s ", de->d_name);
    return false;
  }
  else
  {
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
      perror(path);
      return false;
    }
    print_long_format(de->d_name, &s);
    return true;
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
  printed_prev = false;

  // open dirent and check if error occured
  if ((dir = opendir(dirname)) == NULL)
  {
    perror(dirname);
    return;
  }

  // reset errno
  errno = 0;
  // read dirent and check if error occured
  while ((de = readdir(dir)) != NULL)
  {
    if (errno != 0)
    {
      perror(dirname);
      return;
    }
    else
    {
      // if no error occured print dir entry
      print_dirent(de, dirname);
    }
    errno = 0;
  }
  
  closedir(dir);
}

void print_arg(const char *filename)
{
  if (multiple)
    printf("%s: \n", filename);
  struct stat s;
  if (stat(filename, &s) == -1)
  {
    perror(filename);
    return;
  }

  switch (s.st_mode & S_IFMT)
  {
    case S_IFDIR: print_dir(filename); break;
    default: print_long_format(filename, &s); break;
  }
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
  {
    if (argc - optind > 0) multiple = true;
    for (int i = optind; i < argc; i++)
    {
      print_arg(argv[i]);
      printf("\n");
    }
  }
  // else print curent dir
  else
  {
    print_arg(".");
    printf("\n");
  }
}
