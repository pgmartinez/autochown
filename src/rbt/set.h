/*!
  @file
  @author Xyne
  @copyright GPL2 only

  # Overview
  Sets are implemented with associative arrays that map keys to integer values.
  The keys act as set items and the integer values determine set membership.

  All node functions will work on sets.

  # Required Headers

  - key.h

  # Included Headers

  - node.h
  - wrapper.h

  # Required macro definitions:
  - RBT_SET_H_PREFIX_

    The prefix for visible variable and function declarations in this header. This
    will also be used as the prefix for the included files.

  - RBT_KEY_T

    The type of the keys that will be used. The only requirement for a key is
    that a pointer can be extracted from it and used to read the underlying
    contiguous bits.

  - RBT_KEY_COUNT_BITS(key)

    Return the number of bits in the key.

  - RBT_KEY_PTR(key)

    Return a pointer to the underlying bytes.

  - RBT_KEY_FPRINT(fd, key)

    Print a representation of the key to the given file descriptor.

  # Optional macro definitions

  - RBT_KEY_SIZE_FIXED

    The size of the key, in bytes, if it is fixed or if a maximum key size
    can be anticipated.

*/

#include <stdint.h>

#include "common.h"
#include "debug.h"

#ifdef ONLY_FOR_DOXYGEN
#include "key.h"
#endif //ONLY_FOR_DOXYGEN

/*!
  @cond INTERNAL
*/

#ifndef RBT_SET_H_PREFIX_
#error RBT_SET_H_PREFIX_ is not defined.
#endif // RBT_SET_H_PREFIX_

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


#undef RBT_SET_ADD
#define RBT_SET_ADD                          RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_add)

#undef RBT_SET_DIFFERENCE
#define RBT_SET_DIFFERENCE                   RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_difference)

#undef RBT_SET_EXCLUSIVE_DISJUNCTION
#define RBT_SET_EXCLUSIVE_DISJUNCTION        RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_exclusive_disjunction)

#undef RBT_SET_INCLUDES
#define RBT_SET_INCLUDES                     RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_includes)

#undef RBT_SET_INTERSECTION
#define RBT_SET_INTERSECTION                 RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_intersection)

#undef RBT_SET_IS_SUBSET
#define RBT_SET_IS_SUBSET                    RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_is_subset)

#undef RBT_SET_REMOVE
#define RBT_SET_REMOVE                       RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_remove)

#undef RBT_SET_UNION
#define RBT_SET_UNION                        RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_union)

#undef RBT_SET_MODIFY_DIFFERENCE
#define RBT_SET_MODIFY_DIFFERENCE            RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_modify_difference)

#undef RBT_SET_MODIFY_EXCLUSIVE_DISJUNCTION
#define RBT_SET_MODIFY_EXCLUSIVE_DISJUNCTION RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_modify_exclusive_disjunction)

#undef RBT_SET_MODIFY_INTERSECTION
#define RBT_SET_MODIFY_INTERSECTION          RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_modify_intersection)

#undef RBT_SET_MODIFY_UNION
#define RBT_SET_MODIFY_UNION                 RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_modify_union)



/*
  Internal definitions.
*/
#undef _RBT_SET_DIFFERENCE_TRAVERSE
#define _RBT_SET_DIFFERENCE_TRAVERSE                   _RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_difference_traverse)

#undef _RBT_SET_MODIFY_EXCLUSIVE_DISJUNCTION_TRAVERSE
#define _RBT_SET_MODIFY_EXCLUSIVE_DISJUNCTION_TRAVERSE _RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_modify_exclusive_disjunction_traverse)

#undef _RBT_SET_MODIFY_INTERSECTION_FILTER
#define _RBT_SET_MODIFY_INTERSECTION_FILTER            _RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_modify_intersection_filter)

#undef _RBT_SET_MODIFY_INTERSECTION_TRAVERSE
#define _RBT_SET_MODIFY_INTERSECTION_TRAVERSE          _RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_modify_intersection_traverse)

#undef _RBT_SET_MODIFY_IS_SUBSET_TRAVERSE
#define _RBT_SET_MODIFY_IS_SUBSET_TRAVERSE             _RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_modify_subset_traverse)

#undef _RBT_SET_MODIFY_UNION_TRAVERSE
#define _RBT_SET_MODIFY_UNION_TRAVERSE                 _RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_modify_union_traverse)

#undef _RBT_SET_MODIFY_DIFFERENCE_TRAVERSE
#define _RBT_SET_MODIFY_DIFFERENCE_TRAVERSE            _RBT_TOKEN_2_W(RBT_SET_H_PREFIX_, set_modify_difference_traverse)




/*!
  @brief
  Required definition for node.h.
*/
#undef RBT_NODE_H_PREFIX_
#define RBT_NODE_H_PREFIX_ RBT_SET_H_PREFIX_

/*!
  @brief
  Required definition for node.h.
*/
#undef RBT_VALUE_T
#define RBT_VALUE_T int

/*!
  @brief
  Required definition for node.h.
*/
#undef RBT_VALUE_NULL
#define RBT_VALUE_NULL 0

/*!
  @brief
  Required definition for node.h.
*/
#undef RBT_VALUE_IS_EQUAL
#define RBT_VALUE_IS_EQUAL(a, b) (a == b)

/*!
  @brief
  Required definition for node.h.
*/
#undef RBT_VALUE_COPY
#define RBT_VALUE_COPY(var,val,fail) var = val

/*!
  @brief
  Required definition for node.h.
*/
#undef RBT_VALUE_FREE
#define RBT_VALUE_FREE(val)

/*!
  @brief
  Required definition for node.h.
*/
#undef RBT_VALUE_FPRINT
#define RBT_VALUE_FPRINT(fd, val) \
do \
{ \
  if (val != RBT_VALUE_NULL) \
  { \
    fprintf(fd, "%d", val); \
  } \
} while(0)

#include "node.h"
#include "traverse_with_key.h"

/*!
  @brief
  Required definition for wrapper.h.
*/
#undef RBT_WRAPPER_H_PREFIX_
#define RBT_WRAPPER_H_PREFIX_ RBT_SET_H_PREFIX_
#include "wrapper.h"


/*!
  @endcond
*/



//////////////////////////////////// Union /////////////////////////////////////

/*!
  @cond INTERNAL
*/

/*!
  The internal traversal function of `RBT_SET_MODIFY_UNION()`.
*/

int
_RBT_SET_MODIFY_UNION_TRAVERSE(
  RBT_KEY_DATA_T * key_data,
  RBT_KEY_SIZE_T height,
  va_list args
)
{
  RBT_NODE_T * target;

  if (! RBT_VALUE_IS_NULL(key_data->node->value))
  {
    target = va_arg(args, RBT_NODE_T *);

    RBT_NODE_RETRIEVE(
      target, key_data->key, key_data->bits, RBT_RETRIEVE_ACTION_INSERT, 1, NULL
    );
  }
  return 0;
}

/*!
  @endcond
*/

/*!
  Add all elements of set `b` to set `a`.

  `b` is not changed.

  @param[in]
  a The set to which to add the elements of the other set.

  @param[in]
  b The other set.
*/

void
RBT_SET_MODIFY_UNION(RBT_NODE_T * a, RBT_NODE_T * b)
{
  RBT_NODE_TRAVERSE_WITH_KEY(b, _RBT_SET_MODIFY_UNION_TRAVERSE, a);
}


/*!
  Create a set that is the union of sets `a` and `b`.

  Neither `a` nor `b` is not changed.

  @param[in]
  a The first set.

  @param[in]
  b The second set.

  @return
  The set containing the union.
*/

RBT_NODE_T *
RBT_SET_UNION(RBT_NODE_T * a, RBT_NODE_T * b)
{
  RBT_NODE_T * c;

  c = RBT_NODE_COPY(a);
  RBT_NODE_TRAVERSE_WITH_KEY(b, _RBT_SET_MODIFY_UNION_TRAVERSE, c);
  return c;
}





////////////////////////////////// Difference //////////////////////////////////

/*!
  @cond INTERNAL
*/

/*!
  The internal traversal function of `RBT_SET_MODIFY_DIFFERENCE()`.
*/

int
_RBT_SET_MODIFY_DIFFERENCE_TRAVERSE(
  RBT_KEY_DATA_T * key_data,
  RBT_KEY_SIZE_T height,
  va_list args
)
{
  RBT_NODE_T * target, * parent;

  if (! RBT_VALUE_IS_NULL(key_data->node->value))
  {
    target = va_arg(args, RBT_NODE_T *);

    target = RBT_NODE_RETRIEVE(
      target, key_data->key, key_data->bits, RBT_RETRIEVE_ACTION_NOTHING, RBT_VALUE_NULL, &parent
    );
    if (target != NULL)
    {
      RBT_NODE_REMOVE(target, parent);
    }
  }
  return 0;
}

/*!
  @endcond
*/

/*!
  Remove all elements of `b` from `a`.

  `b` is not changed.

  @param[in]
  a The set from which to remove elements of the other set.

  @param[in]
  b The other set.
*/

void
RBT_SET_MODIFY_DIFFERENCE(RBT_NODE_T * a, RBT_NODE_T * b)
{
  RBT_NODE_TRAVERSE_WITH_KEY(b, _RBT_SET_MODIFY_DIFFERENCE_TRAVERSE, a);
}

/*!
  @cond INTERNAL
*/

/*!
  The internal traversal function of `RBT_SET_DIFFERENCE()`.
*/

int
_RBT_SET_DIFFERENCE_TRAVERSE(
  RBT_KEY_DATA_T * key_data,
  RBT_KEY_SIZE_T height,
  va_list args
)
{
  RBT_NODE_T * b, * c;

  if (! RBT_VALUE_IS_NULL(key_data->node->value))
  {
    b = va_arg(args, RBT_NODE_T *);
    c = va_arg(args, RBT_NODE_T *);

    b = RBT_NODE_RETRIEVE(
      b, key_data->key, key_data->bits, RBT_RETRIEVE_ACTION_NOTHING, RBT_VALUE_NULL, NULL
    );
    /*
      If the value does not exist in b, insert it.
    */
    if (b == NULL)
    {
      RBT_NODE_RETRIEVE(
        c, key_data->key, key_data->bits, RBT_RETRIEVE_ACTION_INSERT_OR_REPLACE, b->value, NULL
      );
    }
  }
  return 0;
}

/*!
  @endcond
*/

/*!
  Create a set that contains all items in set `a` that are not in set `b`.

  Neither `a` nor `b` is not changed.

  @param[in]
  a The main set.

  @param[in]
  b The set to remove.

  @return
  The resulting set.
*/

RBT_NODE_T *
RBT_SET_DIFFERENCE(RBT_NODE_T * a, RBT_NODE_T * b)
{
  RBT_NODE_T * c;

  c = RBT_NODE_NEW();
  RBT_NODE_TRAVERSE_WITH_KEY(a, _RBT_SET_DIFFERENCE_TRAVERSE, b, c);
  return c;
}



///////////////////////////////// Intersection /////////////////////////////////

/*!
  @cond INTERNAL
*/

/*!
  The internal traversal function of `RBT_SET_INTERSECTION()`.
*/

int
_RBT_SET_MODIFY_INTERSECTION_FILTER(
  RBT_KEY_DATA_T * key_data,
  va_list args
)
{
  RBT_NODE_T * target;

  if (! RBT_VALUE_IS_NULL(key_data->node->value))
  {
    target = va_arg(args, RBT_NODE_T *);

    target = RBT_NODE_RETRIEVE(
      target, key_data->key, key_data->bits, RBT_RETRIEVE_ACTION_NOTHING, RBT_VALUE_NULL, NULL
    );
    return (target == NULL);
  }
  return 1;
}

/*!
  @endcond
*/


/*!
  Create a set that is the intersection of sets `a` and `b`.

  `b` is not changed.

  @param[in]
  a The set to hold the intersection of itself and another set.

  @param[in]
  b The other set.
*/

void
RBT_SET_MODIFY_INTERSECTION(RBT_NODE_T * a, RBT_NODE_T * b)
{
  RBT_NODE_FILTER_WITH_KEY(a, _RBT_SET_MODIFY_INTERSECTION_FILTER, 0, b);
}

/*!
  Modify `a` in place to be the result of an intersection with `b`.

  Neither `a` nor `b` is not changed.

  @param[in]
  a The first set.

  @param[in]
  b The second set.

  @return
  The set of the intersection.
*/

RBT_NODE_T *
RBT_SET_INTERSECTION(RBT_NODE_T * a, RBT_NODE_T * b)
{
  RBT_NODE_T * c;
  c = RBT_NODE_COPY(a);
  RBT_SET_MODIFY_INTERSECTION(c, b);
  return c;
}


//////////////////////////// Exclusive Disjunction /////////////////////////////

/*!
  @cond INTERNAL
*/

/*!
  The internal traversal function of `RBT_SET_EXCLUSIVE_DISJUNCTION()`.
*/

int
_RBT_SET_MODIFY_EXCLUSIVE_DISJUNCTION_TRAVERSE(
  RBT_KEY_DATA_T * key_data,
  RBT_KEY_SIZE_T height,
  va_list args
)
{
  RBT_NODE_T * target, * parent;

  if (! RBT_VALUE_IS_NULL(key_data->node->value))
  {
    target = va_arg(args, RBT_NODE_T *);

    target = RBT_NODE_RETRIEVE(
      target, key_data->key, key_data->bits, RBT_QUERY_ACTION_INSERT, RBT_VALUE_NULL, &parent
    );
    if (target->value)
    {
      RBT_NODE_REMOVE(target, parent);
    }
    else
    {
      target->value = 1;
    }
  }
  return 0;
}

/*!
  @endcond
*/

/*!
  Update `a` in place to be the result of an exclusive disjunction (xor) with
  `b`.

  `b` is not changed.

  @param[in]
  a The set to hold the exclusive disjunction of itself and another set.

  @param[in]
  b The other set.
*/
void
RBT_SET_MODIFY_EXCLUSIVE_DISJUNCTION(RBT_NODE_T * a, RBT_NODE_T * b)
{
  RBT_NODE_TRAVERSE_WITH_KEY(b, _RBT_SET_MODIFY_EXCLUSIVE_DISJUNCTION_TRAVERSE, a);
}

/*!
  Create the exclusive disjunction of sets `a` and `b`.

  Neither `a` nor `b` is not changed.

  @param[in]
  a The first set.

  @param[in]
  b The second set.

  @return
  The set of the exclusive disjunction.
*/
RBT_NODE_T *
RBT_SET_EXCLUSIVE_DISJUNCTION(RBT_NODE_T * a, RBT_NODE_T * b)
{
  RBT_NODE_T * c;
  c = RBT_NODE_COPY(a);
  RBT_NODE_TRAVERSE_WITH_KEY(b, _RBT_SET_MODIFY_EXCLUSIVE_DISJUNCTION_TRAVERSE, c);
  return c;
}


////////////////////////////////// Is Subset ///////////////////////////////////

/*!
  @cond INTERNAL
*/

/*!
  The internal traversal function of `RBT_SET_IS_SUBSET()`.
*/

int
_RBT_SET_IS_SUBSET_TRAVERSE(
  RBT_KEY_DATA_T * key_data,
  RBT_KEY_SIZE_T height,
  va_list args
)
{
  int * is_subset;
  RBT_NODE_T * target;

  if (! RBT_VALUE_IS_NULL(key_data->node->value))
  {
    target = va_arg(args, RBT_NODE_T *);
    is_subset = va_arg(args, int *);

    target = RBT_NODE_RETRIEVE(
      target, key_data->key, key_data->bits, RBT_RETRIEVE_ACTION_NOTHING, RBT_VALUE_NULL, NULL
    );
    if (target == NULL)
    {
      * is_subset = 0;
      return 1;
    }
  }
  return 0;
}

/*!
  @endcond
*/


/*!
  Determine if `a` is a subset of `b`.

  Neither set is changed.

  @param[in]
  a The subset candidate.

  @param[in]
  b The superset candidate.

  @return
  Non-zero if `a` is a subset of `b`.
*/
int
RBT_SET_IS_SUBSET(RBT_NODE_T * a, RBT_NODE_T * b)
{
  int is_subset;
  is_subset = 1;
  /*
    a and b are "switched" here compared to previous functions because each node
    in `a` must be found in `b`, i.e. `a` is traversed.
  */
  RBT_NODE_TRAVERSE_WITH_KEY(a, _RBT_SET_IS_SUBSET_TRAVERSE, b, &is_subset);
  return is_subset;
}



/////////////////////////// Insertions and Deletions ///////////////////////////

/*!
  Add an item to a set.

  @param[in]
  set The root node of the set.

  @param[in]
  item The item to add.
*/
void
RBT_SET_ADD(RBT_NODE_T * set, RBT_KEY_T item)
{
  RBT_INSERT(set, item, 1);
}

/*!
  Remove an item from a set.

  @param[in]
  set The root node of the set.

  @param[in]
  item The item to remove.
*/
void
RBT_SET_REMOVE(RBT_NODE_T * set, RBT_KEY_T item)
{
  RBT_DELETE(set, item);
}

/*!
  Check if a set includes an item.

  @param[in]
  set The root node of the set.

  @param[in]
  item The item to check.
*/
int
RBT_SET_INCLUDES(RBT_NODE_T * set, RBT_KEY_T item)
{
  return RBT_HAS_KEY(set, item);
}

