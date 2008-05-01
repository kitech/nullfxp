#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <utime.h>
#include <errno.h>
#include <assert.h>
#include <glob.h>
#ifndef HAVE_NO_SYS_SELECT_H
#include <sys/select.h>
#endif

#include "putty.h"
#include "psftp.h"
#include "int64.h"

extern Config cfg;
extern char *cmdline_password;
int main(int argc, char **argv)
{
  uxsel_init();

  sk_init();
  int ret;

  do_defaults(NULL, &cfg);

    flags = FLAG_STDERR | FLAG_INTERACTIVE
#ifdef FLAG_SYNCAGENT
      | FLAG_SYNCAGENT
#endif
      ;
  flags |= FLAG_VERBOSE;
  cmdline_password = malloc(100);
  strcpy(cmdline_password,"2113");

  ret = psftp_connect("localhost","root",22);
  printf("psftp_connect: %d\n", ret);
  free(cmdline_password);cmdline_password = NULL;
  ret = do_sftp_init();
  printf("do_sftp_init: %d\n", ret);
  //do_sftp(0, 0, 0);

  printf("aaaaaaaaa\n");

  //connection 2
  cmdline_password = malloc(100);
  strcpy(cmdline_password,"2113");

  ret = psftp_connect("localhost","root",22);
  printf("psftp_connect: %d\n", ret);
  free(cmdline_password);cmdline_password = NULL;
  ret = do_sftp_init();
  printf("do_sftp_init: %d\n", ret);


  //  return psftp_main(argc, argv);
  pause();
  return 0;
}
