#include <sys/inotify.h>

#include "common.h"

/*!
  @brief
  The inotify instance.
*/
int INOTIFY_INSTANCE;

/*!
  @brief
  The size of an inotify event.
*/
#define EVENT_SIZE  sizeof(struct inotify_event)

/*!
  @brief
  Buffer length for reading inofity events.
*/
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

/*!
  @brief
  The inotify events to watch.
*/
#define EVENTS (\
  IN_CREATE | \
  IN_ATTRIB | \
  IN_DELETE | \
  IN_DELETE_SELF | \
  IN_MOVED_TO | \
  IN_MOVE_SELF | \
  IN_Q_OVERFLOW | \
  IN_DONT_FOLLOW | \
  IN_ONLYDIR \
)

/*!
  @brief
  Read an `int` from a file.

  @param
  path The file to read.

  @return
  The integer in the file.
*/
int
read_int(char * path);
