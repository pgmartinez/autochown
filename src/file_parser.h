#ifndef MAOWN_FILE_PARSER_H
#define MAOWN_FILE_PARSER_H


#include <errno.h>
#include <fnmatch.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#define BUFSIZE 0x1000

/*!
  @brief
  Actions to take when parsing file paths.
*/
/*
  Leave this for extensibility.
*/
typedef enum
{
  INCLUDE,
  EXCLUDE
}
action_t;


/*!
  @brief
  A unidirectional linked list of patterns.
*/
typedef
struct pattern
{
  /*!
    @brief
    The pattern in the input file.
  */
  char * pattern;

  /*!
    @brief
    The action to perform.
  */
  action_t action;

  /*!
    @brief
    The next pattern in the list.
  */
  struct pattern * next;

}
pattern_t;


/*!
  @brief
  Patterns and associated actions along with necessary data.
*/
typedef
struct
{
  /*!
    @brief
    The target directory.
  */
  char * target;
  /*!
    @brief
    The list of patterns to check.
  */
  pattern_t * pattern;

  /*!
    @brief
    The user ID.
  */
  uid_t uid;

  /*!
    @brief
    Change owner to user ID.
  */
  int chown_uid;

  /*!
    @brief
    The group ID.
  */
  gid_t gid;

  /*!
    @brief
    Change group to group ID.
  */
  int chown_gid;

  /*!
    @brief
    The mode mask.

    The mask is not used the same way as chmod. The owners permissions are
    applied to the group and others and the mask acts on that. Any bits in the
    mask that apply to the owner will be applied first.

    This is the default mask applied to all filetypes. To specify separate masks
    for different filetypes, use the other mask fields.
  */
  mode_t mask;

  /*!
    @brief
    Change the file mode.
  */
  int chmod;

  /*!
    @brief
    Mask for directories.

    See documentation for "mask" field.
  */
  mode_t mask_d;

  /*!
    @brief
    Change directory file mode.
  */
  int chmod_d;

  /*!
    @brief
    Mask for character special files.

    See documentation for "mask" field.
  */
  mode_t mask_c;

  /*!
    @brief
    Change character special file mode.
  */
  int chmod_c;

  /*!
    @brief
    Mask for block special files.

    See documentation for "mask" field.
  */
  mode_t mask_b;

  /*!
    @brief
    Change block special file mode.
  */
  int chmod_b;

  /*!
    @brief
    Mask for regular files.

    See documentation for "mask" field.
  */
  mode_t mask_r;

  /*!
    @brief
    Change regular file mode.
  */
  int chmod_r;

  /*!
    @brief
    Mask for FIFO special files.

    See documentation for "mask" field.
  */
  mode_t mask_f;

  /*!
    @brief
    Change FIFO special file mode.
  */
  int chmod_f;

  /*!
    @brief
    Mask for symbolic links.

    See documentation for "mask" field.
  */
  mode_t mask_l;

  /*!
    @brief
    Change symbolic link mode.
  */
  int chmod_l;

  /*!
    @brief
    Mask for sockets.

    See documentation for "mask" field.
  */
  mode_t mask_s;

  /*!
    @brief
    Change socket mode.
  */
  int chmod_s;
}
target_t;





/*!
  @brief
  Data associated with each watchlist descriptor.
*/
typedef
struct
{
  /*!
    @brief
    The original target that let to the addition of this descriptor.
  */
  target_t * target;

  /*!
    @brief
    The full path of the current target.
  */
  char * path;
}
watchlist_data_t;



/*!
  @brief
  Parse an input file.

  @param
  fpath The file path, or "-" to read from STDIN.

  @return
  An array of targets.
*/
target_t *
parse_targets(char * fpath);


/*!
  @brief
  Free the pattern queue.

  @param
  targets The array of targets to free, as returned by `parse_targets()`.
*/
void
free_targets(target_t * targets);


/*!
  @brief
  Match path against pattern queue and return the resulting action.

  @param
  patterns The linked list of sequential patterns.

  @param
  path The path to match against the patterns.

  @return
  The resulting action.
*/
action_t
match_pattern_queue(pattern_t * pattern, char * path);


#endif //MAOWN_FILE_PARSER_H
