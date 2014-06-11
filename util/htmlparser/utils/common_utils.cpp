/*
 * common_utils.cpp
 *
 *  Created on: 2012-12-20
 *      Author: sue
 */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <errno.h>

#include "string_util.h"
#include "url.h"
#include "common_utils.h"

int check_path(const char * dirpath) {
  struct stat buf;
  int result = stat(dirpath, &buf);
  if (result != 0) {
    if (errno == ENOENT) {
      return 1;
    } else {
      return -1;
    }
  } else {
    if (buf.st_mode & S_IFDIR) {
      return 0;
    } else if (buf.st_mode & S_IFREG) {
      return 2;
    } else {
      return -1;
    }
  }
}
