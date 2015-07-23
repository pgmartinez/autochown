#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"


/*!
  @brief
  `va_list` variant of `msg_log()`.
*/
void
msg_log_internal_v(int include_errno, char * fmt, va_list args)
{
//   time_t t;
//   t = time(NULL);
//   char timestamp[23];
//   strftime(timestamp, sizeof(timestamp), "[%F %R:%S] ", localtime(&t));
//   fprintf(LOG_FD, timestamp);
  vfprintf(LOG_FD, fmt, args);
  if (include_errno)
  {
    fprintf(LOG_FD, " [%s]", strerror(errno));
  }
  fprintf(LOG_FD, "\n");
}



void
msg_log(char * fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  msg_log_internal_v(0, fmt, args);
  va_end(args);
}



void
die(char * fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  msg_log_internal_v(1, fmt, args);
  va_end(args);
  exit(EXIT_FAILURE);
}

