#ifndef MAOWN_RBT_H
#define MAOWN_RBT_H
/*
  Set up rabbit trees to map watchlist descriptors to watchlist data structs.
*/

// #define RBT_DEBUG 1
// #define RBT_USE_COLOR 1

#include <rbt/common.h>

#define RBT_KEY_H_PREFIX_ wd_
#define RBT_PIN_T unsigned int
#define RBT_KEY_SIZE_T uint_fast8_t
#define RBT_KEY_SIZE_T_FORMAT "%u"
#include <rbt/key.h>

watchlist_data_t wd_empty_value = {.target = NULL, .path = NULL};

#define RBT_NODE_H_PREFIX_ RBT_KEY_H_PREFIX_
#define RBT_VALUE_T watchlist_data_t
#define RBT_VALUE_NULL wd_empty_value

#define RBT_VALUE_IS_EQUAL(a, b)  \
( \
  (a.target == b.target) && \
  ( \
    (a.path == NULL) ? \
    (b.path == NULL) : \
    (strcmp(a.path, b.path) == 0) \
  ) \
)

#define RBT_VALUE_COPY(var,val,fail) \
do \
{ \
  var.target = val.target; \
  if (var.path != NULL) \
  { \
    free(var.path); \
    var.path = NULL; \
  } \
  if (val.path != NULL) \
  { \
    var.path = strdup(val.path); \
    if (var.path == NULL) \
    { \
      fail; \
    } \
  } \
} \
while (0)

#define RBT_VALUE_FREE(val) \
do \
{ \
  if (val.path != NULL) \
  { \
    free(val.path); \
  } \
} \
while (0)

#define RBT_VALUE_FPRINT(fd, val) fprintf(fd, val.path)
#include <rbt/node.h>

#include <rbt/traverse_with_key.h>


#define RBT_WRAPPER_H_PREFIX_ RBT_KEY_H_PREFIX_
#define RBT_KEY_T int
#define RBT_KEY_SIZE_FIXED sizeof(RBT_KEY_T)
#define RBT_KEY_COUNT_BITS(key) (sizeof(RBT_KEY_T) * BITS_PER_BYTE)
#define RBT_KEY_PTR(key) (&key)
#define RBT_KEY_FPRINT(fd, key) fprintf(fd, "%d", key);
#include <rbt/wrapper.h>

#endif //MAOWN_RBT_H
