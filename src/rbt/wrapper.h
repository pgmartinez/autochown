/*!
  @file
  @author Xyne
  @copyright GPL2 only

  # Required Headers
  - query.h

  # Required Macro Definitions:
  - RBT_WRAPPER_H_PREFIX_

    The prefix for visible variable and function declarations in this header.

  - RBT_KEY_T

    The type of the keys that will be used. The only requirement for a key is
    that a pointer can be extracted from it and used to read the underlying
    contiguous bits.

  - RBT_KEY_COUNT_BITS(key)

    Return the number of bits in the key.

  - RBT_KEY_PTR(key)

    Return a pointer to the underlying bytes.

  - RBT_KEY_FPRINT(fd, key, len)

    Print a representation of the key to the given file descriptor. `len` is the
    length of the key in bytes.

  # Optional Macro Definitions

  - RBT_KEY_SIZE_FIXED

    The size of the key, in bytes, if it is fixed or if a maximum key size
    can be anticipated.

*/
#include "common.h"
#include "debug.h"

#ifdef ONLY_FOR_DOXYGEN
#include "node.h"
#endif //ONLY_FOR_DOXYGEN


/*!
  @cond INTERNAL
*/

#ifndef RBT_WRAPPER_H_PREFIX_
#error RBT_WRAPPER_H_PREFIX_ is not defined.
#endif // RBT_WRAPPER_H_PREFIX_

#ifndef RBT_KEY_T
#error RBT_KEY_T is not defined.
#endif // RBT_KEY_T

#ifndef RBT_KEY_COUNT_BITS
#error RBT_KEY_COUNT_BITS is not defined.
#endif // RBT_KEY_COUNT_BITS

#ifndef RBT_KEY_PTR
#error RBT_KEY_PTR is not defined.
#endif // RBT_KEY_PTR

#ifndef RBT_KEY_FPRINT
#error RBT_KEY_FPRINT is not defined.
#endif // RBT_KEY_FPRINT

/*!
  The key type.
*/
typedef RBT_KEY_T RBT_TOKEN_2_W(RBT_WRAPPER_H_PREFIX_, key_t);

#undef RBT_NODE_QUERY_WRAPPER
#define RBT_NODE_QUERY_WRAPPER RBT_TOKEN_2_W(RBT_WRAPPER_H_PREFIX_, node_query_wrapper)

#undef RBT_DELETE
#define RBT_DELETE   RBT_TOKEN_2_W(RBT_WRAPPER_H_PREFIX_, delete)

#undef RBT_HAS_KEY
#define RBT_HAS_KEY  RBT_TOKEN_2_W(RBT_WRAPPER_H_PREFIX_, has_key)

#undef RBT_INSERT
#define RBT_INSERT   RBT_TOKEN_2_W(RBT_WRAPPER_H_PREFIX_, insert)

#undef RBT_RETRIEVE
#define RBT_RETRIEVE RBT_TOKEN_2_W(RBT_WRAPPER_H_PREFIX_, retrieve)

#undef RBT_SWAP
#define RBT_SWAP     RBT_TOKEN_2_W(RBT_WRAPPER_H_PREFIX_, swap)

/*
  When a given key type is cast into the internal key type, care must be taken
  to prevent reads past the boundary of allocated memory. This is best
  illustrated with an example, however contrived.

  If the user chooses an internal key type that uses arrays of `unsigned int`s
  (either by explicit choice or by using e.g. uint_fast8_t on some platform),
  but then chooses a key type of "char", then the following situation would
  arise, where "x" represents an allocated byte and "." an unallocated byte, and
  `int`s are 32 bits:

      key:          x...
      internal key: xxxx

  Even if the various functions will only ever compare the first 8 bits, casting
  the key to an internal key will dereference 3 unallocated bytes. The behavior
  is undefined. If there were a guarantee that it would never segfault then it
  would not be a problem, because the value of those bits will never be directly
  accessed.

  Unfortunately, segfaults may occur, so the best solution is to allocate memory
  and copy over the allocated bytes from the key. The rest of the bytes can be
  left uninitialized as they have no effect.

  Of course, if the internal key is an array of single-byte elements then this
  is never a concern, nor is it a concern if the two types share a common factor
  greater than 1, as a cast will not run past the bountary in that case.
*/

#undef RBT_PINS_FIXED
/*!
  The fixed number of pins given a fixed key size.
*/
#undef RBT_PINS_FIXED
#define RBT_PINS_FIXED ((RBT_KEY_SIZE_FIXED + (RBT_PIN_SIZE - 1)) / RBT_PIN_SIZE)

/*!
  Invoke `RBT_NODE_QUERY()` with a key cast to an array of pins.

  @param[in]
  node The root node.

  @param[in]
  The uncast key.

  @param[in]
  The number of bits in the key.

  @param[in]
  The action to perform.

  @param[in]
  The value with which to perform the action.
*/
#undef _RBT_NODE_QUERY_WITH_CAST_KEY
#ifdef RBT_KEY_SIZE_FIXED
  #define _RBT_NODE_QUERY_WITH_CAST_KEY(node, key, bits, action, value) \
    do \
    { \
      if (RBT_PIN_SIZE != 1) \
      { \
        if (RBT_KEY_SIZE_FIXED % RBT_PIN_SIZE) \
        { \
          RBT_PIN_T cast_key[RBT_PINS_FIXED]; \
          memset(cast_key, 0, sizeof(cast_key)); \
          memcpy(cast_key, RBT_KEY_PTR(key), RBT_KEY_SIZE_FIXED); \
          bits = sizeof(cast_key) * BITS_PER_BYTE; \
          return RBT_NODE_QUERY(node, cast_key, bits, action, value); \
        } \
        else \
        { \
          return RBT_NODE_QUERY(node, (RBT_PIN_T *) RBT_KEY_PTR(key), bits, action, value); \
        } \
      } \
      else \
      { \
        return RBT_NODE_QUERY(node, (RBT_PIN_T *) RBT_KEY_PTR(key), bits, action, value); \
      } \
    } \
    while (0)

#else
  #define _RBT_NODE_QUERY_WITH_CAST_KEY(node, key, bits, action, value) \
    do \
    { \
      if (RBT_PIN_SIZE != 1) \
      { \
        int q; \
        int r; \
        RBT_DIVMOD(bits, RBT_PIN_SIZE_BITS, q, r); \
        if (r) \
        { \
          q ++; \
          RBT_PIN_T cast_key[q]; \
          memset(cast_key, 0, sizeof(cast_key)); \
          memcpy(cast_key, RBT_KEY_PTR(key), (bits + (BITS_PER_BYTE - 1)) / BITS_PER_BYTE); \
          bits = q * RBT_PIN_SIZE_BITS; \
          return RBT_NODE_QUERY(node, cast_key, bits, action, value); \
        } \
        else \
        { \
          return RBT_NODE_QUERY(node, (RBT_PIN_T *) RBT_KEY_PTR(key), bits, action, value); \
        } \
      } \
      else \
      { \
        return RBT_NODE_QUERY(node, (RBT_PIN_T *) RBT_KEY_PTR(key), bits, action, value); \
      } \
    } \
    while (0)
#endif //RBT_KEY_SIZE_FIXED

/*!
  @endcond
*/


/*!
  A wrapper function around `RBT_NODE_QUERY()` that automatically casts the key
  to the required pointer type.

  @param[in]
  node The root node.

  @param[in]
  key The query key.

  @param[in]
  action The action to perform.

  @param[in]
  value The value with which to perform the action.

  @return
  The value returned by `RBT_NODE_QUERY()` for this action.
*/
RBT_VALUE_T
RBT_NODE_QUERY_WRAPPER(
  RBT_NODE_T * node,
  RBT_KEY_T key,
  rbt_query_action_t action,
  RBT_VALUE_T value
)
{
  RBT_KEY_SIZE_T bits;
  bits = RBT_KEY_COUNT_BITS(key);
//   _RBT_NODE_QUERY_WITH_CAST_KEY(cast_key, key, bits);
//   return RBT_NODE_QUERY(node, cast_key, bits, action, value);
  _RBT_NODE_QUERY_WITH_CAST_KEY(node, key, bits, action, value);
}



/*!
  A convenient wrapper for `RBT_NODE_QUERY_WRAPPER()` for inserting values.

  @param[in]
  node The root node.

  @param[in]
  key The query key.

  @param[in]
  value The value to insert.

  @return
  The value returned by `RBT_NODE_QUERY()` for this action.
*/
RBT_VALUE_T
RBT_INSERT(
  RBT_NODE_T * node,
  RBT_KEY_T key,
  RBT_VALUE_T value
)
{
  return RBT_NODE_QUERY_WRAPPER(node, key, RBT_QUERY_ACTION_INSERT, value);
}



/*!
  A convenient wrapper for `RBT_NODE_QUERY_WRAPPER()` for retrieving values.

  @param[in]
  node The root node.

  @param[in]
  key The query key.

  @return
  The value returned by `RBT_NODE_QUERY()` for this action.
*/
RBT_VALUE_T
RBT_RETRIEVE(
  RBT_NODE_T * node,
  RBT_KEY_T key
)
{
  return RBT_NODE_QUERY_WRAPPER(node, key, RBT_QUERY_ACTION_RETRIEVE, RBT_VALUE_NULL);
}



/*!
  A convenient wrapper for `RBT_NODE_QUERY_WRAPPER()` for deleting values.

  @param[in]
  node The root node.

  @param[in]
  key The query key.

  @return
  The value returned by `RBT_NODE_QUERY()` for this action.
*/
RBT_VALUE_T
RBT_DELETE(
  RBT_NODE_T * node,
  RBT_KEY_T key
)
{
  return RBT_NODE_QUERY_WRAPPER(node, key, RBT_QUERY_ACTION_DELETE, RBT_VALUE_NULL);
}



/*!
  A convenient wrapper for `RBT_NODE_QUERY_WRAPPER()` for swapping values.

  @param[in]
  node The root node.

  @param[in]
  key The query key.

  @param[in]
  value The value to insert.

  @return
  The value returned by `RBT_NODE_QUERY()` for this action.
*/
RBT_VALUE_T
RBT_SWAP(
  RBT_NODE_T * node,
  RBT_KEY_T key,
  RBT_VALUE_T value
)
{
  return RBT_NODE_QUERY_WRAPPER(node, key, RBT_QUERY_ACTION_SWAP, value);
}




/////////////////////////////////// Has Key ////////////////////////////////////

/*!
  Check if a key is present in the tree.
*/

int
RBT_HAS_KEY(RBT_NODE_T * node, RBT_KEY_T key)
{
  return !RBT_VALUE_IS_NULL(RBT_RETRIEVE(node, key));
}