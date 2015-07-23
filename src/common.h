#ifndef MAOWN_COMMON_H
#define MAOWN_COMMON_H

#include <errno.h>
#include <stdio.h>
#include <time.h>

#ifndef LOG_FD
#  define LOG_FD stderr
#endif // LOG_FD


/*!
  The mask to interpret as a signal to remove the given filetype.
*/
#define KILLMASK 0700

#define MAX_PW_NAME 0xff
#define MAX_GR_NAME 0xff


/*!
  @brief
  Log messages.

  @param
  fmt The printf format string.

  @param
  ... printf parameters.
*/
void
msg_log(char * fmt, ...);


/*!
  @brief
  Print an error message and exit.

  @param
  fmt The printf format string.

  @param
  ... printf parameters.
*/
void
die(char * fmt, ...);

#endif //MAOWN_COMMON_H
