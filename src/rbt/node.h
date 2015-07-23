/*!
  @file
  @author Xyne
  @copyright GPL2 only

  # Required Headers

  - key.h


  # Required Macro Definitions:

  - RBT_NODE_H_PREFIX_

    The prefix for visible variable and function declarations in this header.

  - RBT_VALUE_T

    The type of the value held by each node in the tree.

  - RBT_VALUE_NULL

    The "null" value. A node containing this value is considered empty. Empty
    leaf nodes are removed from the tree.

  - RBT_VALUE_IS_EQUAL(a, b)

    A macro function that should evaluate to "true" if and only if the arguments
    `a` and `b` of type `RBT_VALUE_T` are considered equal. This must also
    handle `RBT_VALUE_NULL` arguments. It will be used to define an additional
    macro: `RBT_VALUE_IS_NULL(x)`.

  - RBT_VALUE_COPY(a, b, fail)

    Copy the value "b" to the variable "a". If the copy operation fails, run
    "fail". For statically allocated values, `a = b` will suffice. For
    dynamically allocated values, 4 cases must be handled, with proper
    allocation and freeing:

      - both a and b are RBT_VALUE_NULL
      - a is RBT_VALUE_NULL, b is not
      - b is RBT_VALUE_NULL, a is not
      - neither a nor b is RBT_VALUE_NULL


  - RBT_VALUE_FREE(val)

    Free any dynamically allocated memory associated with the given value.

  - RBT_VALUE_FPRINT(fd, val)

    Print a representation of the value to the given file descriptor. This
    should not print anything for the null value.



  # Optional Macro Definitions

  - RBT_NODE_CACHE_SIZE

    An unsigned integer value. If set, a cache of previously allocated nodes
    will be kept for re-use to avoid redundant memory allocation requests.
    `RBT_NODE_CREATE()` will check the cache before allocating new memory and
    `RBT_NODE_FREE()` will return nodes to the cache instead of freeing them.

    If the value is a positive then the number of nodes in the cache will be
    limited to this number. If it is zero, they will not.

    Setting this value will also define the following:

    - `RBT_NODE_CACHE_T`

      A typedef'd struct type to track the nodes in the cache.

    - `RBT_NODE_CACHE`

      The global cache of type `RBT_NODE_CACHE_T`.

    - `RBT_NODE_CACHE_FREE`

      A function to free all of the nodes in the cache. This should be called
      when no more nodes will be required and before the program exits.

    This should be used when working with dynamic tree structures.


  - RBT_CONCURRENCY_PTHREAD

    Define this macro to include concurrency/node_pthread.h



  # Examples
  ## Statically allocated string values.
  Note that `RBT_VALUE_COPY` does not use the "fail" statement because it is a
  simple assignment and cannot fail. RBT_VALUE_FREE does nothing because there
  is no dynamically allocated memory to free.

      #define RBT_VALUE_T char *
      #define RBT_VALUE_NULL NULL
      #define RBT_VALUE_IS_EQUAL(a, b) \
      ( \
        ((a == NULL) == (b == NULL)) && \
        (a == NULL || ! strcmp(a,b)) \
      )
      #define RBT_VALUE_COPY(a, b, fail) a = b
      #define RBT_VALUE_FREE(val)
      #define RBT_VALUE_FPRINT(fd, val) \
      do \
      { \
        if (val != NULL) \
        { \
          fprintf(fd, "%s", val); \
        } \
      } while(0)


  ## Dynamically allocated string values.
  Each node will hold its own copy of the string in allocated memory. The
  memory will be managed automatically when working with the tree.

      #define RBT_VALUE_T char *
      #define RBT_VALUE_NULL NULL
      #define RBT_VALUE_IS_EQUAL(a, b) \
      ( \
        ((a == NULL) == (b == NULL)) && \
        (a == NULL || ! strcmp(a,b)) \
      )
      #define RBT_VALUE_COPY(a, b, fail) \
      do \
      { \
        free(a); \
        a = malloc(strlen(b) + 1); \
        if (a == NULL || strcpy(a, b) == NULL) \
        { \
          fail; \
        } \
      }
      while (0)
      #define RBT_VALUE_FREE(val) free(val)
      #define RBT_VALUE_FPRINT(fd, val) \
      do \
      { \
        if (val != NULL) \
        { \
          fprintf(fd, "%s", val); \
        } \
      } while(0)
*/


/*
  # TODO

  * Consider creating a central traversal function that can track keys and
    handle insertions/deletions. Such a function should be implemented if it can
    be done without the introduction of unreasonable complexity and overhead.
*/

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "debug.h"

#ifdef ONLY_FOR_DOXYGEN
#include "key.h"
#endif //ONLY_FOR_DOXYGEN

/*!
  @cond INTERNAL
*/

#ifndef RBT_NODE_H_PREFIX_
#error RBT_NODE_H_PREFIX_ is not defined.
#endif // RBT_NODE_H_PREFIX_

#ifndef RBT_VALUE_T
#error RBT_VALUE_T is not defined.
#endif // RBT_VALUE_T

#ifndef RBT_VALUE_NULL
#error RBT_VALUE_NULL is not defined.
#endif // RBT_VALUE_NULL

#ifndef RBT_VALUE_IS_EQUAL
#error RBT_VALUE_IS_EQUAL is not defined.
#endif // RBT_VALUE_IS_EQUAL

#ifndef RBT_VALUE_COPY
#error RBT_VALUE_COPY is not defined.
#endif // RBT_VALUE_COPY

#ifndef RBT_VALUE_FREE
#error RBT_VALUE_FREE is not defined.
#endif // RBT_VALUE_FREE

#ifndef RBT_VALUE_FPRINT
#error RBT_VALUE_FPRINT is not defined.
#endif // RBT_VALUE_FPRINT

/*
  This is mostly for doxygen output when macro exansion is enabled, but it may
  be convenient in some cases.
*/
// #ifndef RBT_NODE_H_PREFIX_
// #define RBT_NODE_H_PREFIX_ rbt_
// #endif


typedef RBT_VALUE_T                             RBT_TOKEN_2_W(RBT_KEY_H_PREFIX_, value_t);


#undef RBT_NODE_COPY
#define RBT_NODE_COPY                         RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_copy)

#undef RBT_NODE_COUNT
#define RBT_NODE_COUNT                        RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_count)

#undef RBT_NODE_CREATE
#define RBT_NODE_CREATE                       RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_create)

#undef RBT_NODE_FILTER
#define RBT_NODE_FILTER                       RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_filter)

#undef RBT_NODE_FPRINT
#define RBT_NODE_FPRINT                       RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_fprint)

#undef RBT_NODE_FPRINT_INTERNAL
#define RBT_NODE_FPRINT_INTERNAL              RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_fprint_internal)

#undef RBT_NODE_FREE
#define RBT_NODE_FREE                         RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_free)

#undef RBT_KEY_DATA_T
#define RBT_KEY_DATA_T                        RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, key_data_t)

#undef RBT_NODE_INSERT_CHILD
#define RBT_NODE_INSERT_CHILD                 RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_insert_child)

#undef RBT_NODE_INSERT_PARENT
#define RBT_NODE_INSERT_PARENT                RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_insert_parent)

#undef RBT_NODE_INSERT_SIBLING
#define RBT_NODE_INSERT_SIBLING               RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_insert_sibling)

#undef RBT_NODE_IS_COPY
#define RBT_NODE_IS_COPY                      RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_is_copy)

#undef RBT_NODE_MERGE_CHILD
#define RBT_NODE_MERGE_CHILD                  RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_merge_child)

#undef RBT_NODE_NEW
#define RBT_NODE_NEW                          RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_new)

#undef RBT_NODE_REMOVE
#define RBT_NODE_REMOVE                       RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_remove)

#undef RBT_NODE_RETRIEVE
#define RBT_NODE_RETRIEVE                     RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_retrieve)

#undef RBT_NODE_QUERY
#define RBT_NODE_QUERY                        RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_query)

#undef RBT_NODE_TRAVERSE
#define RBT_NODE_TRAVERSE                        RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_traverse)

#undef RBT_NODE_STACK_T
#define RBT_NODE_STACK_T                      RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_stack_t)

#undef RBT_NODE_T
#define RBT_NODE_T                            RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_t)

#undef RBT_VALUE_T
#define RBT_VALUE_T                           RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, value_t)

#undef RBT_NODE_WITH_PREFIX_SUBTREE_DO
#define RBT_NODE_WITH_PREFIX_SUBTREE_DO       RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, with_prefix_subtree_do)

#undef RBT_NODE_FILTER_FUNCTION_T
#define RBT_NODE_FILTER_FUNCTION_T            RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_filter_function_t)

#undef RBT_NODE_FILTER_WITH_KEY_FUNCTION_T
#define RBT_NODE_FILTER_WITH_KEY_FUNCTION_T   RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_filter_with_key_function_t)

#undef RBT_NODE_TRAVERSE_FUNCTION_T
#define RBT_NODE_TRAVERSE_FUNCTION_T          RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_traverse_function_t)

#undef RBT_NODE_TRAVERSE_WITH_KEY_FUNCTION_T
#define RBT_NODE_TRAVERSE_WITH_KEY_FUNCTION_T RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_traverse_with_key_function_t)


#undef _RBT_NODE_PRINT_STACK_T
#define _RBT_NODE_PRINT_STACK_T             _RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_print_stack_t)

#undef _RBT_NODE_COPY_STACK_T
#define _RBT_NODE_COPY_STACK_T              _RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_copy_stack_t)

#undef _RBT_NODE_IS_COPY_STACK_T
#define _RBT_NODE_IS_COPY_STACK_T           _RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_is_copy_stack_t)

#undef _RBT_NODE_TRAVERSE_STACK_T
#define _RBT_NODE_TRAVERSE_STACK_T          _RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_traverse_stack_t)

#undef _RBT_NODE_TRAVERSE_WITH_KEY_STACK_T
#define _RBT_NODE_TRAVERSE_WITH_KEY_STACK_T _RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_traverse_with_key_stack_t)

#undef _RBT_NODE_FILTER_STACK_T
#define _RBT_NODE_FILTER_STACK_T            _RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_filter_stack_t)

#undef _RBT_NODE_FILTER_WITH_KEY_STACK_T
#define _RBT_NODE_FILTER_WITH_KEY_STACK_T   _RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_filter_with_key_stack_t)



/*
  The following are internal convencience macros for managing a stack using a
  linked list. To prevent unnecessary cycles of allocation and freeing as the
  stack changes, previously allocated elements are reused.
*/
#undef _RBT_NODE_STACK_ALLOCATE
#define _RBT_NODE_STACK_ALLOCATE(stack, stack_tmp, stack_unused, stack_t) \
do \
{ \
  if (stack_unused != NULL) \
  { \
    stack_tmp = stack_unused->next; \
    stack_unused->next = stack; \
    stack = stack_unused; \
    stack_unused = stack_tmp; \
  } \
  else \
  { \
    stack_tmp = malloc(sizeof(stack_t)); \
    if (stack_tmp == NULL) \
    { \
      rc = errno; \
      break; \
    } \
    stack_tmp->next = stack; \
    stack = stack_tmp; \
  } \
} \
while(0)


#undef _RBT_NODE_STACK_POP
#define _RBT_NODE_STACK_POP(stack, stack_tmp, stack_unused) \
do \
{ \
  stack_tmp = stack->next; \
  stack->next = stack_unused; \
  stack_unused = stack; \
  stack = stack_tmp; \
} \
while(0)

#undef _RBT_NODE_STACK_FREE
#define _RBT_NODE_STACK_FREE(stack, stack_tmp, stack_unused) \
do \
{ \
  while (stack != NULL) \
  { \
    stack_tmp = stack->next; \
    free(stack); \
    stack = stack_tmp; \
  } \
  while (stack_unused != NULL) \
  { \
    stack_tmp = stack_unused->next; \
    free(stack_unused); \
    stack_unused = stack_tmp; \
  } \
} \
while(0)

/*!
  @endcond
*/




/*!
  An expression that should evaluate to a non-zero `int` if the given value
  is equivalent to `RBT_VALUE_NULL`.

  @param[in]
  value The value to evaluate.
*/
#undef RBT_VALUE_IS_NULL
#define RBT_VALUE_IS_NULL(value) RBT_VALUE_IS_EQUAL(value, RBT_VALUE_NULL)








/*!
  @brief
  The node's value type.
*/
typedef
RBT_VALUE_T RBT_VALUE_T;



/*!
  @brief
  Rabbit Tree node type
*/

typedef
struct RBT_NODE_T
{
  /*!
    @brief
    Key segment associated with this node.

    The key is a pointer to an array of unsigned integers. The type of the
    integers is determined by the RBT_PIN_T macro.
  */
  RBT_PIN_T * key;

  /*!
    @brief
    Number of significant bits in the key.

    If a node has no children then all of the bits in its key are significant,
    otherwise all bits until the first bit that differs between the child node
    keys.
  */
  RBT_KEY_SIZE_T bits;

  /*!
    @brief
    Value associated with the node.

    The value is considered empty if RBT_VALUE_IS_NULL returns non-zero.
  */
  RBT_VALUE_T value;

  /*!
    @brief
    Left child node.

    The first additional significant bit of this child is 0.
  */
  struct RBT_NODE_T * left;

  /*!
    @brief
    Right child node.

    The first additional significant bit of this child is 1.
  */
  struct RBT_NODE_T * right;
}
RBT_NODE_T;



/*!
  @brief
  Rabbit Tree key data type with associated node.

  This is intended for use in tree traversal functions that require the full
  associated keys.
*/

typedef
struct RBT_KEY_DATA_T
{
  /*!
    @brief
    The key.

    The key is a pointer to an array of unsigned integers. The type of the
    integers is determined by the RBT_PIN_T macro.
  */
  RBT_PIN_T * key;

  /*!
    @brief
    Number of significant bits in the key.
  */
  RBT_KEY_SIZE_T bits;

  /*!
    @brief
    Number of significant bits in the key.

    This value will always be equal to `bits/8`. It is included because the
    value is often calculated simultaneously with the bits in various traversal
    functions.
  */
  RBT_KEY_SIZE_T bytes;

  /*!
    @brief
    The associated node.
  */
  RBT_NODE_T * node;
}
RBT_KEY_DATA_T;





#ifdef RBT_NODE_CACHE_SIZE
#undef RBT_NODE_CACHE
#define RBT_NODE_CACHE      RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_cache)
#undef RBT_NODE_CACHE_T
#define RBT_NODE_CACHE_T    RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_cache_t)
#undef RBT_NODE_CACHE_FREE
#define RBT_NODE_CACHE_FREE RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_cache_free)

/*!
  @brief
  The cache of unused nodes.
*/
typedef
struct RBT_NODE_CACHE_T
{

  /*!
    @brief
    A pointer to the first cached node.

    A pointer to the first cached node. The nodes are used as linked lists with
    the left child pointer pointing to the next node in the cache.
  */
  RBT_NODE_T * node;

  /*!
    @brief
    The number of nodes in the cache.

    The number of nodes in the cache. This is compared to `RBT_NODE_CACHE_SIZE`
    by `RBT_NODE_CACHE_OR_FREE()` to determine if another node should be added
    to the cache. It will be incremented and decremented by
    `RBT_NODE_CACHE_OR_FREE()` and `RBT_NODE_CREATE()`, respectively.
  */
  unsigned int n;
}
RBT_NODE_CACHE_T;

/*!
  @brief
  The global node cache.
*/
RBT_NODE_CACHE_T RBT_NODE_CACHE = {.node=NULL, .n=0};

#endif //RBT_NODE_CACHE_SIZE



//////////////////////////// Concurrency Inclusions ////////////////////////////
#ifdef RBT_CONCURRENCY_PTHREAD
#  include "concurrency/pthread.h"
#endif //RBT_CONCURRENCY_PTHREAD



#ifdef RBT_NODE_CACHE_SIZE
#  ifndef RBT_NODE_CACHE_LOCK
#    define RBT_NODE_CACHE_LOCK
#  endif //RBT_NODE_CACHE_LOCK

#  ifndef RBT_NODE_CACHE_UNLOCK
#    define RBT_NODE_CACHE_UNLOCK
#  endif //RBT_NODE_CACHE_UNLOCK


/*!
  Free the nodes in the cache. This should be called when no more nodes will be
  needed and before the program exits.
*/

void
RBT_NODE_CACHE_FREE()
{
  RBT_NODE_T * node_tmp;
  RBT_NODE_CACHE_LOCK
  while (RBT_NODE_CACHE.node != NULL)
  {
    node_tmp = (RBT_NODE_CACHE.node)->left;
    free(RBT_NODE_CACHE.node);
    RBT_NODE_CACHE.node = node_tmp;
  }
  RBT_NODE_CACHE.n = 0;
  RBT_NODE_CACHE_UNLOCK
}
#endif //RBT_NODE_CACHE_SIZE






/*!
  @brief
  Linked list of nodes.

  A unidirectional linked list used for implementing dynamic stacks of nodes for
  e.g. tracking parent nodes when traversing a tree.
*/

typedef
struct RBT_NODE_STACK_T
{
  /*!
    The node.
  */
  RBT_NODE_T * node;

  /*!
    The next item.
  */
  struct RBT_NODE_STACK_T * next;
}
RBT_NODE_STACK_T;



/*
  Print a node and its descendents to a file descriptor in a simple format.
  This is mostly for debugging but may be expanded later.
void
RBT_NODE_FPRINT(FILE * fd, RBT_NODE_T * node, int indent, int skip)
{
  int i = indent;
  while(i--)
  {
//     fprintf(fd, "  ");
    fputc('.', fd);
  }
  RBT_FPRINT_BITS(fd, node->key, node->bits - skip, skip);
  fprintf(fd, " ");
  fprintf(fd, " (%p) ", node);
  RBT_VALUE_FPRINT(fd, node->value);
  fprintf(fd, "\n");
  indent += node->bits - skip;
  skip = node->bits % RBT_PIN_SIZE_BITS;
  if (node->left != NULL)
  {
    RBT_NODE_FPRINT(fd, node->left, indent, skip);
  }
  if (node->right != NULL)
  {
    RBT_NODE_FPRINT(fd, node->right, indent, skip);
  }
}
*/


/*
  The original recursive version that I wrote for initial testing.

void
RBT_NODE_FPRINT_INTERNAL(FILE * fd, RBT_NODE_T * node, int print_bits, RBT_KEY_SIZE_T indent, unsigned long vert, int end, int skip)
{
  RBT_KEY_SIZE_T i;
  unsigned long shifted_vert;

#ifdef RBT_USE_COLOR
  fprintf(fd, "\033[32m");
#endif

  fprintf(fd, "%10p ", node);

#ifdef RBT_USE_COLOR
  fprintf(fd, "\033[34m");
#endif

  if (indent)
  {
    for (i = indent - 1; i; i--)
    {
      shifted_vert = (vert >> i);
      if (shifted_vert & 1)
      {
        fprintf(fd, "│");
      }
      else
      {
        fprintf(fd, " ");
      }
    }
    if (indent)
    {
      if (end)
      {
        fprintf(fd, "└");
      }
      else
      {
        fprintf(fd, "├");
      }
    }
  }

#ifdef RBT_USE_COLOR
  fprintf(fd, "\033[35m");
#endif


  if (print_bits)
  {
    RBT_FPRINT_BITS(fd, node->key, node->bits - skip, skip);
    fprintf(fd, " ");
    i = node->bits - skip;
  }
  else
  {
    fprintf(fd, "● ");
    i = 1;
  }

  indent += i;
  vert <<= i;
  vert ++;

#ifdef RBT_USE_COLOR
  fprintf(fd, "\033[36m");
#endif

  RBT_VALUE_FPRINT(fd, node->value);
  fprintf(fd, "\n");

#ifdef RBT_USE_COLOR
  fprintf(fd, "\033[0m");
#endif


  skip = node->bits % RBT_PIN_SIZE_BITS;
  if (node->left != NULL)
  {
    RBT_NODE_FPRINT_INTERNAL(fd, node->left, print_bits, indent, vert, (node->right == NULL), skip);
  }
  if (node->right != NULL)
  {
    vert --;
    RBT_NODE_FPRINT_INTERNAL(fd, node->right, print_bits, indent, vert, 1, skip);
  }
}
*/

/*!
  @cond INTERNAL
*/

/*!
  @brief
  Linked list for implementing RBT_NODE_FPRINT_INTERNAL's dynamic stack.

  \warning
  This is an internal structure.
*/

typedef
struct _RBT_NODE_PRINT_STACK_T
{
  /*!
    @brief
    The node.
  */
  RBT_NODE_T * node;

  /*!
    @brief
    The indentation level.
  */
  RBT_KEY_SIZE_T indent;

  /*!
    @brief
    A number tracking unprinted siblings.

    This number tracks siblings for drawing branches with vertical box-drawing
    characters. Each bit in the number is used as a switch to determine whether
    or not a vertical bar should be drawn to the left of the node
    representation.
  */
  RBT_KEY_SIZE_T vert;

  /*!
    @brief
    A boolean indicating if the node is the last child of its parent.
  */
  int is_last_child;

  /*!
    @brief
    The number of bits to skip in the nodes key.
  */
  int skip;

  /*!
    @brief
    The next item in the stack.
  */
  struct _RBT_NODE_PRINT_STACK_T * next;
}
_RBT_NODE_PRINT_STACK_T;

/*!
  @endcond
*/

/*!
  The internal function used by RBT_NODE_FPRINT() for printing trees using
  box-drawing characters.

  @attention
  The value of `errno` should be checked for errors when this function returns.

  @param[in]
  fd The output file descriptor.

  @param[in]
  node A pointer to the target root node.

  @param[in]
  print_bits A boolean value to determine if the key bits of each node should
  be printed. If false, the character '●' will be printed instead.

  @param[in]
  print_pointer A value indicating if the pointer should be printed. If
  non-zero, the value will be used as the width of the pointer in the call to
  fprintf.

  @param[in]
  indent The indentation level.

  @param[in]
  vert A numerical value used to track vertical branches that should be printed
  to the left of the current node.

  @param[in]
  is_last_child A boolean value indicating if the current node is the last child of
  its parent.

  @param[in]
  skip The number of bits held by this node that should be skipped. These are
  the staggered bits of the parent node.

  @see
  - `RBT_NODE_FPRINT()`
*/

void
RBT_NODE_FPRINT_INTERNAL(
  FILE * fd,
  RBT_NODE_T * node,
  int print_bits,
  int print_pointer,
  RBT_KEY_SIZE_T indent,
  RBT_KEY_SIZE_T vert,
  int is_last_child,
  int skip
)
{
  RBT_KEY_SIZE_T i;
  _RBT_NODE_PRINT_STACK_T * stack, * stack_tmp, * stack_unused;
  int rc;
  unsigned long shifted_vert;

  errno = 0;

  if (node == NULL)
  {
    return;
  }

  stack = NULL;
  stack_unused = NULL;
  rc = 0;

  while (1)
  {
    if (print_pointer)
    {
#ifdef RBT_USE_COLOR
      fprintf(fd, "\033[32m");
#endif
      fprintf(fd, "%*p ", print_pointer, node);
    }

#ifdef RBT_USE_COLOR
    fprintf(fd, "\033[34m");
#endif

    if (indent)
    {
      for (i = indent - 1; i; i--)
      {
        shifted_vert = (vert >> i);
        if (shifted_vert & 1)
        {
          fprintf(fd, "│");
        }
        else
        {
          fprintf(fd, " ");
        }
      }
      if (indent)
      {
        if (is_last_child)
        {
          fprintf(fd, "└");
        }
        else
        {
          fprintf(fd, "├");
        }
      }
    }

#ifdef RBT_USE_COLOR
    fprintf(fd, "\033[35m");
#endif


    if (print_bits)
    {
      RBT_FPRINT_BITS(fd, node->key, node->bits - skip, skip);
      fprintf(fd, " ");
      i = node->bits - skip;
    }
    else
    {
      fprintf(fd, "● ");
      i = 1;
    }

    indent += i;
    vert <<= i;
    vert ++;

#ifdef RBT_USE_COLOR
    fprintf(fd, "\033[36m");
#endif

    RBT_VALUE_FPRINT(fd, node->value);
    fprintf(fd, "\n");


    skip = node->bits % RBT_PIN_SIZE_BITS;
    if (node->left != NULL)
    {
      is_last_child = (node->right == NULL);
      if (!is_last_child)
      {
        /*
          errno and rc may be set to non-zero values here, in which case the
          loop will also be broken.
        */
        _RBT_NODE_STACK_ALLOCATE(stack, stack_tmp, stack_unused, _RBT_NODE_PRINT_STACK_T);
        stack->node = node->right;
        stack->indent = indent;
        stack->vert = vert - 1;
        stack->is_last_child = 1;
        stack->skip = skip;
      }
      node = node->left;
    }
    else if (node->right != NULL)
    {
      vert --;
      is_last_child = 1;
      node = node->right;
    }
    else if (stack != NULL)
    {
      node = stack->node;
      indent = stack->indent;
      vert = stack->vert;
      is_last_child = stack->is_last_child;
      skip = stack->skip;
      _RBT_NODE_STACK_POP(stack, stack_tmp, stack_unused);
    }
    else
    {
      break;
    }
  }

#ifdef RBT_USE_COLOR
  fprintf(fd, "\033[0m");
#endif

  _RBT_NODE_STACK_FREE(stack, stack_tmp, stack_unused);
  errno = rc;
}




/*!
  @brief
  Print trees using box-drawing characters.

  A function for printing representations of trees using box-drawing characters.
  This function is mostly for debugging purposes. It will visualize the tree
  hierarchy and optionally show the pointers of each node along with the key
  bits that they contain.

  @attention
  The value of `errno` should be checked for errors when this function returns.

  @param[in]
  fd The output file descriptor.

  @param[in]
  node A pointer to the target root node.

  @param[in]
  print_bits A boolean value to determine if the key bits of each node should
  be printed. If false, the character '●' will be printed instead.

  @param[in]
  print_pointer A value indicating if the pointer should be printed. If
  non-zero, the value will be used as the width of the pointer in the call to
  fprintf.
*/

void
RBT_NODE_FPRINT(
  FILE * fd,
  RBT_NODE_T * node,
  int print_bits,
  int print_pointer
)
{
  RBT_NODE_FPRINT_INTERNAL(fd, node, print_bits, print_pointer, 0, 0, 0, 0);
}



/*!
  @brief
  Create a node.

  Create a node to hold the given key fragment and insert the given value.
  The memory is dynamically allocated and should be freed with `RBT_NODE_FREE()`
  when no longer needed.

  If `RBT_NODE_CACHE_SIZE` is defined, a previously allocated node will be
  taken from the cache if available instead of allocating a new block of
  memory.

  @attention
  The value of `errno` should be checked for errors when this function returns.

  @param[in]
  key A pointer to the key fragment.

  @param[in]
  bits The number of bits in the key fragment.

  @param[in]
  value The value to insert.

  @param[in]
  left The left child. The first new bit in this child will be 0.

  @param[in]
  right The right child. The first new bit in this child will be 1.

  @return
  A pointer to the new node, or `NULL` if an error occured.
*/
RBT_NODE_T *
RBT_NODE_CREATE(
  RBT_PIN_T * key,
  RBT_KEY_SIZE_T bits,
  RBT_VALUE_T value,
  RBT_NODE_T * left,
  RBT_NODE_T * right
)
{
  RBT_NODE_T * node;
  RBT_KEY_SIZE_T bytes;

#ifdef RBT_NODE_CACHE_SIZE
  RBT_NODE_CACHE_LOCK
  node = RBT_NODE_CACHE.node;
  if (node != NULL)
  {
    RBT_NODE_CACHE.node = node->left;
    RBT_NODE_CACHE.n --;
    RBT_NODE_CACHE_UNLOCK
  }
  else
  {
    RBT_NODE_CACHE_UNLOCK
#endif

    debug_print("creating node\n");
    node = malloc(sizeof(RBT_NODE_T));
    if (node == NULL)
    {
      debug_print("failed to allocate memory for node\n");
      return NULL;
    }

#ifdef RBT_NODE_CACHE_SIZE
  }
#endif

  bytes = BITS_TO_PINS_TO_BYTES(bits);
  node->key = malloc(bytes);
  if (node->key == NULL)
  {
    debug_printf("failed to allocate %" RBT_KEY_SIZE_T_FORMAT " bytes for key\n", bits);
    free(node);
    return NULL;
  }
  memcpy(node->key, key, bytes);
  node->bits = bits;
  node->value = RBT_VALUE_NULL;
  RBT_VALUE_COPY(node->value, value, free(node->key);free(node);return NULL);
  node->left = left;
  node->right = right;
  debug_printf("created node %p\n", node);
  debug_print_func(RBT_FPRINT_BITS, 1, node->key, node->bits, 0);
  return node;
}



/*!
  A conditional macro that either caches nodes or frees them depending on the
  value of `RBT_NODE_CACHE_SIZE`. It is used internally by `RBT_NODE_FREE()`.
*/
#ifdef RBT_NODE_CACHE_SIZE
  /*!
    The displayed definition is for the caching variant.
  */
  #undef RBT_NODE_CACHE_OR_FREE
  #define RBT_NODE_CACHE_OR_FREE(node) \
  do \
  { \
    RBT_NODE_CACHE_LOCK \
    if ( \
      ! RBT_NODE_CACHE_SIZE || \
      RBT_NODE_CACHE.n <= RBT_NODE_CACHE_SIZE \
    ) \
    { \
      node->left = RBT_NODE_CACHE.node; \
      RBT_NODE_CACHE.node = node; \
      RBT_NODE_CACHE.n ++; \
      RBT_NODE_CACHE_UNLOCK \
    } \
    else \
    { \
      RBT_NODE_CACHE_UNLOCK \
      free(node); \
    } \
  } \
  while (0)
#else
  /*!
    The displayed definition is for the non-caching variant.
  */
  #undef RBT_NODE_CACHE_OR_FREE
  #define RBT_NODE_CACHE_OR_FREE(node) free(node)
#endif

/*!
  Free the target node and all its descendents. If `RBT_NODE_CACHE_SIZE` is set
  then the nodes will be stored in the cache instead of being freed, if the
  cache is not full.

  @param[in]
  node The target node.

  @see
  - `RBT_NODE_CACHE_OR_FREE()`
*/
void
RBT_NODE_FREE(
  RBT_NODE_T * node
)
{
//   debug_printf("freeing node %p\n", node);
  RBT_NODE_T * orphan, * heir, * descendent;

  errno = 0;

  if (node == NULL)
  {
    return;
  }

  descendent = NULL;

  /*
    Do it this way to avoid recursion, which would require two calls per
    invocation to handle both child nodes.

    The idea here is to free the top node then append the right tree to the
    bottom of the left tree. The left tree's top node then replaces the current
    node. The path to the bottom of the left tree will grow for every node
    with two children. To avoid descending the tree from the top each time,
    the last leftmost descendent is tracked.

    Tree traversal is thus reduces to traversing a linked list.
  */
  while (1)
  {
//     debug_print_func(RBT_FPRINT_BITS, 1, node->key, node->bits, 0);
    debug_printf("freeing node: %p\n", node);
//     debug_printf("node key pointer: %p\n", node->key);
    free(node->key);
    RBT_VALUE_FREE(node->value);
    if (node->right == NULL)
    {
      if (node->left == NULL)
      {
        RBT_NODE_CACHE_OR_FREE(node);
//         debug_print("freed\n");
        return;
      }
      else
      {
        heir = node->left;
        RBT_NODE_CACHE_OR_FREE(node);
        node = heir;
      }
    }
    else if (node->left == NULL)
    {
      heir = node->right;
      RBT_NODE_CACHE_OR_FREE(node);
      node = heir;
    }
    else
    {
      orphan = node->right;
      heir = node->left;
      RBT_NODE_CACHE_OR_FREE(node);
      node = heir;
      if (descendent == NULL)
      {
        descendent = heir;
        while (descendent->left !=NULL)
        {
          descendent = descendent->left;
        }
      }
      descendent->left = orphan;
      descendent = orphan;
      while (descendent->left !=NULL)
      {
        descendent = descendent->left;
      }
    }
  }
  debug_print("freed\n");
}



/*!
  Merge a child node into the parent. The value and children of the target child
  will be inserted into the parent and the key bits will be merged.

  The merged child should be the only child of the parent. If it is not, the sibling will
  be removed and freed along with the target child.

  Logically this removes the parent node from the tree.

  @attention
  The value of `errno` should be checked for errors when this function returns.

  @param[in]
  parent_node The parent node.

  @param[in]
  child_node The child node.
*/
void
RBT_NODE_MERGE_CHILD(
  RBT_NODE_T * parent_node,
  RBT_NODE_T * child_node
)
{
  debug_printf("merging child node %p (parent: %p)\n", child_node, parent_node);
  RBT_PIN_T * tmp_key;
  RBT_KEY_SIZE_T parent_bytes, child_bytes;
  RBT_VALUE_T tmp_value;
  RBT_NODE_T * tmp_node;

  errno = 0;

  /*
    Flooring division is required for parent to truncate overlap.
  */
  parent_bytes = PINS_TO_BYTES(parent_node->bits / RBT_PIN_SIZE_BITS);
  child_bytes = BITS_TO_PINS_TO_BYTES(child_node->bits);
  tmp_key = realloc(parent_node->key, parent_bytes + child_bytes);
  if (tmp_key == NULL)
  {
    debug_printf("failed to realloc key (%s)\n", strerror(errno));
    return;
  }

  parent_node->key = tmp_key;
  memcpy(((BYTE_T *) parent_node->key) + parent_bytes, child_node->key, child_bytes);
  parent_node->bits = (parent_bytes * BITS_PER_BYTE) + child_node->bits;

  /*
    Swap the values around so the value can be freed if necessary when freeing
    the child node. Do *not* use RBT_VALUE_COPY here.
  */
  tmp_value = parent_node->value;
  parent_node->value = child_node->value;
  child_node->value = tmp_value;

  /*
    Give the unexpected sibling to the child so that it can be freed along with
    the sibling.
  */
  if (parent_node->left == child_node)
  {
    tmp_node = parent_node->right;
  }
  else
  {
    tmp_node = parent_node->left;
  }
  parent_node->left = child_node->left;
  parent_node->right = child_node->right;
  // Prefer the left node because of the left bias in RBT_NODE_FREE
  child_node->left = tmp_node;
  child_node->right = NULL;

  RBT_NODE_FREE(child_node);
  debug_print("merged\n");
}




/*!
  @brief
  Remove a node from a tree.

  Logically remove a node from a tree. If the node can be completely removed, it
  will be freed and the parent node will be restructured as necessary.

  @attention
  The value of `errno` should be checked for errors when this function returns.


  - `RBT_NODE_MERGE_CHILD()`
  - `RBT_NODE_FREE()`

  @param[in]
  node A pointer to the target node.

  @param[in]
  parent_node The parent node of the target node.

  @return
  The passed node if it remains in the tree, otherwise the parent.
*/

RBT_NODE_T *
RBT_NODE_REMOVE(
  RBT_NODE_T * node,
  RBT_NODE_T * parent_node
)
{
  debug_printf("removing node %p (parent: %p)\n", node, parent_node);

  if (node->left == NULL)
  {
    if (node->right == NULL)
    {
      debug_print("no children\n");
      /*
        If the parent is null then this is a childless root node. It cannot be
        removed, but it can be emptied.
      */
      if (parent_node == NULL)
      {
        debug_print("root node\n");
        RBT_VALUE_COPY(node->value, RBT_VALUE_NULL, );
        free(node->key);
        node->key = NULL;
        node->bits = 0;
      }
      /*
        This node can be removed from the parent if it has no children. If the
        parent value is empty (i.e. the parent is a placeholder node) then the
        parent needs to be merged with the sibling node.
      */
      else
      {
        if (RBT_VALUE_IS_NULL(parent_node->value))
        {
          if (parent_node->left == node)
          {
            RBT_NODE_MERGE_CHILD(parent_node, parent_node->right);
          }
          else
          {
            RBT_NODE_MERGE_CHILD(parent_node, parent_node->left);
          }
        }
        else
        {
          if (parent_node->left == node)
          {
            parent_node->left = NULL;
          }
          else
          {
            parent_node->right = NULL;
          }
          RBT_NODE_FREE(node);
        }
        node = parent_node;
      }
    }
    /*
      A single child should be merged into this node.
    */
    else
    {
      debug_print("right child\n");
      RBT_NODE_MERGE_CHILD(node, node->right);
    }
  }
  /*
    A single child should be merged into this node.
  */
  else if (node->right == NULL)
  {
    debug_print("left child\n");
    RBT_NODE_MERGE_CHILD(node, node->left);
  }
  /*
    If the node has two children then the value needs to be emptied and key
    information must be kept.
  */
  else
  {
    debug_print("two children\n");
    RBT_VALUE_COPY(node->value, RBT_VALUE_NULL, );
  }
  debug_print("removed\n");
  return node;
}


/*!
  @brief
  Insert a parent node.

  Insert a parent node above the given node to hold the given value. The key
  will be extracted from the beginning of the current key.

  @attention
  The value of `errno` should be checked for errors when this function returns.

  @param[in]
  node The current node.

  @param[in]
  bits The number of key bits that will be made part of the parent node.

  @param[in]
  value The value to insert.

  @see
  - `RBT_NODE_INSERT_SIBLING()`
  - `RBT_NODE_INSERT_CHILD()`
*/
void
RBT_NODE_INSERT_PARENT(
  RBT_NODE_T * node,
  RBT_KEY_SIZE_T bits,
  RBT_VALUE_T value
)
{
  debug_print("inserting parent\n");
  RBT_KEY_SIZE_T pins, staggered_bits, parent_pins;
  RBT_NODE_T * child;
  RBT_PIN_T * tmp_key;

  RBT_DIVMOD(bits, RBT_PIN_SIZE_BITS, pins, staggered_bits);

  /*
    RBT_NODE_CREATE will copy the value passed to it, so pass it the new value
    and then swap the values of the parent and child below, instead of using
    superfluous RBT_VALUE_COPY statements.
  */
  child = RBT_NODE_CREATE(
    node->key + pins,
    node->bits - bits + staggered_bits,
    value,
    node->left,
    node->right
  );
  if (child == NULL)
  {
    debug_printf("failed to create child node (%s)\n", strerror(errno));
    return;
  }

  parent_pins = pins;
  if (staggered_bits > 0)
  {
    parent_pins ++;
  }
  tmp_key = realloc(node->key, parent_pins * RBT_PIN_SIZE);
  if (tmp_key == NULL && parent_pins)
  {
    debug_printf("failed to realloc key (%s)\n", strerror(errno));
    return;
  }
  node->key = tmp_key;
  node->bits = bits;
  /*
    Swap the values. Do *not* use RBT_VALUE_COPY.
  */
  value = child->value;
  child->value = node->value;
  node->value = value;

  if (N_BIT_IS_1(child->key[0], staggered_bits))
  {
    node->right = child;
    node->left = NULL;
  }
  else
  {
    node->left = NULL;
    node->right = child;
  }
  debug_print("inserted\n");
}

/*!
  @brief
  Insert a child node.

  Insert a child node to hold the given value.

  @attention
  The value of `errno` should be checked for errors when this function returns.

  @param[in]
  child_node_ptr The child node pointer of the parent node that should be set to
  point to this node.

  @param[in]
  key The key fragment that should be held by this node.

  @param[in]
  bits The number of bits in the key fragment.

  @param[in]
  value The value to insert.

  @see
  - `RBT_NODE_INSERT_PARENT()`
  - `RBT_NODE_INSERT_SIBLING()`
*/
void
RBT_NODE_INSERT_CHILD(
  RBT_NODE_T * * child_node_ptr,
  RBT_PIN_T * key,
  RBT_KEY_SIZE_T bits,
  RBT_VALUE_T value
)
{
  debug_print("inserting child\n");
  RBT_NODE_T * node;
  node = RBT_NODE_CREATE(key, bits, value, NULL, NULL);
  * child_node_ptr = node;
  debug_print("inserted\n");
}


/*!
  @brief
  Insert a sibling node.

  Insert a sibling node to hold the given value. The given node is made a
  placeholder parent and the current data in the node is moved to a new node
  that is inserted as a child of the parent. The new data is inserted into
  a second new node and made the second child of the parent.

  @attention
  The value of `errno` should be checked for errors when this function returns.

  @param[in]
  node The current node.

  @param[in]
  key The key fragment that spans the current node and the target child node.

  @param[in]
  bits The number of bits in the key fragment.

  @param[in]
  common_bits The number of common bits between the current node's key fragment
  and the given key. These will form the key fragment of the new parent node.

  @param[in]
  common_pins The number of common pins between the current node's key fragment
  and the given key.

  @param[in]
  common_staggered_bits The number of uncommon bits in the current node's key
  fragment. These will form the key fragment of the sibling node.

  @param[in]
  value The value to insert.

  @return
  A pointer to the new node holding the value, or NULL if an error occured.

  This function is called internally by `RBT_NODE_RETRIEVE()`, which calculates
  `common_bits`, `common_pins` and `common_staggered_bits` for tree traversal.
  The value are accepted as parameters here to avoid redundant calculations.

  In contrast to the other insertion functions, this function returns a pointer
  to the inserted child. This is to avoid redundant calculations for determining
  whether the new child is the left or right child of the node.

  @see
  - `RBT_NODE_INSERT_PARENT()`
  - `RBT_NODE_INSERT_CHILD()`

*/
RBT_NODE_T *
RBT_NODE_INSERT_SIBLING(
  RBT_NODE_T * node,
  RBT_PIN_T * key,
  RBT_KEY_SIZE_T bits,
  RBT_KEY_SIZE_T common_bits,
  RBT_KEY_SIZE_T common_pins,
  RBT_KEY_SIZE_T common_staggered_bits,
  RBT_VALUE_T value
)
{
  RBT_KEY_SIZE_T parent_pins;
  RBT_NODE_T * sibling, * baby;
  RBT_PIN_T * tmp_key;

  /*
    `sibling` is the node to which the existing data is moved.
    `baby` is the new node used to hold the new value.
  */

  debug_printf(
    "inserting sibling (%"
    RBT_KEY_SIZE_T_FORMAT ", %"
    RBT_KEY_SIZE_T_FORMAT ", %"
    RBT_KEY_SIZE_T_FORMAT ", %"
    RBT_KEY_SIZE_T_FORMAT ")\n",
    bits, common_bits, common_pins, common_staggered_bits
  );

  baby = RBT_NODE_CREATE(
    key + common_pins,
    bits - common_bits + common_staggered_bits,
    value,
    NULL,
    NULL
  );

  if (baby == NULL)
  {
    debug_printf("failed to create baby node (%s)\n", strerror(errno));
    return NULL;
  }

  /*
    Insert a null value here and then swap it with the current node's value to
    avoid redundant copying. The `value` variable above can be used as a
    placeholder.
  */
  sibling = RBT_NODE_CREATE(
    node->key + common_pins,
    node->bits - common_bits + common_staggered_bits,
    RBT_VALUE_NULL,
    node->left,
    node->right
  );

  if (sibling == NULL)
  {
    debug_printf("failed to create sibling node (%s)\n", strerror(errno));
    RBT_NODE_FREE(baby);
    return NULL;
  }


  parent_pins = common_pins;
  if (common_staggered_bits > 0)
  {
    parent_pins ++;
  }
  tmp_key = realloc(node->key, parent_pins * RBT_PIN_SIZE);
  // parent_pins may be 0, in which case realloc should return NULL.
  if (tmp_key == NULL && parent_pins)
  {
    debug_printf("failed to realloc key (%s)\n", strerror(errno));
    RBT_NODE_FREE(sibling);
    RBT_NODE_FREE(baby);
    return NULL;
  }

  node->key = tmp_key;
  node->bits = common_bits;

  value = node->value;
  node->value = sibling->value;
  sibling->value = value;

  if (N_BIT_IS_1(baby->key[0], common_staggered_bits))
  {
    node->right = baby;
    node->left = sibling;
  }
  else
  {
    node->left = baby;
    node->right = sibling;
  }
  debug_print("inserted\n");
  return baby;
}



/*!
  @brief
  Create a new tree.

  Create a new, empty node. This should be used to create root nodes for new
  trees.
*/
RBT_NODE_T *
RBT_NODE_NEW()
{
  return RBT_NODE_CREATE(NULL, 0, RBT_VALUE_NULL, NULL, NULL);
}








///////////////////////////////////// Copy /////////////////////////////////////

/*!
  @cond INTERNAL
*/

/*!
  @brief
  Linked list for implementing RBT_NODE_COPY's dynamic stack.

  \warning
  This is an internal structure.
*/

typedef
struct _RBT_NODE_COPY_STACK_T
{
  /*!
    @brief
    The node.
  */
  RBT_NODE_T * node;

  /*!
    @brief
    The child pointer of the target parent node into which the node should be
    inserted.
  */

  RBT_NODE_T * * ptr;

  /*!
    @brief
    The next item in the stack.
  */
  struct _RBT_NODE_COPY_STACK_T * next;
}
_RBT_NODE_COPY_STACK_T;

/*!
  @endcond
*/




/*!
  @brief
  Copy a tree.

  Create a node-by-node copy of a tree given the root node.

  @warning
  If the values held by the tree are pointers then the data referenced by the
  copy will obviously be the same as the data referenced by the original and the
  two will not be independent. If, however, the data itself is held by the node
  then the copy will be completely independent.

  @param[in]
  node The root node of the original tree to copy.

  @return
  A pointer to the root node of the copy.
*/
RBT_NODE_T *
RBT_NODE_COPY(
  RBT_NODE_T * node
)
{
  RBT_NODE_T * new_root, * new_node;
  RBT_NODE_T * * child_ptr;
  _RBT_NODE_COPY_STACK_T * stack, * stack_tmp, * stack_unused;
  int rc;

  errno = 0;

  if (node == NULL)
  {
    return NULL;
  }


  child_ptr = &new_root;
  stack = NULL;
  stack_unused = NULL;
  rc = 0;

  while (node != NULL)
  {
    new_node = RBT_NODE_CREATE(node->key, node->bits, node->value, NULL, NULL);
    * child_ptr = new_node;

    if (node->left != NULL)
    {
      if (node->right != NULL)
      {
        _RBT_NODE_STACK_ALLOCATE(stack, stack_tmp, stack_unused, _RBT_NODE_COPY_STACK_T);
        stack->node = node->right;
        stack->ptr = &(new_node->right);
      }
      child_ptr = &(new_node->left);
      node = node->left;
    }
    else if (node->right != NULL)
    {
      child_ptr = &(new_node->right);
      node = node->right;
    }
    else if (stack != NULL)
    {
      node = stack->node;
      child_ptr = stack->ptr;
      _RBT_NODE_STACK_POP(stack, stack_tmp, stack_unused);
    }
    else
    {
      break;
    }
  }
  if (rc)
  {
    RBT_NODE_FREE(new_root);
  }
  _RBT_NODE_STACK_FREE(stack, stack_tmp, stack_unused);
  errno = rc;
  return new_root;
}





//////////////////////////////////// Filter ////////////////////////////////////

/*!
  @cond INTERNAL
*/

/*!
  @brief
  Linked list for implementing RBT_NODE_FILTER's dynamic stack.

  \warning
  This is an internal structure.
*/

typedef
struct _RBT_NODE_FILTER_STACK_T
{
  /*!
    @brief
    The node.
  */
  RBT_NODE_T * node;

  /*!
    @brief
    The stack of parent nodes for this node.
  */
  RBT_NODE_STACK_T * parent_stack;

  /*!
    @brief
    The next item in the stack.
  */
  struct _RBT_NODE_FILTER_STACK_T * next;
}
_RBT_NODE_FILTER_STACK_T;

/*!
  @endcond
*/



/*!
  @brief
  The function type accepted by `RBT_NODE_FILTER()`.

  @param[in]
  value A pointer to the current node's value.

  @param[in]
  args A `va_list` containing the additional arguments passed to
  `RBT_NODE_FILTER()`.

  @return
  An `int`. If it is non-zero, the current node will be *removed*.
*/
typedef
int
(* RBT_NODE_FILTER_FUNCTION_T)(
  RBT_VALUE_T * value,
  va_list args
);

/*!
  @brief
  Filter tree nodes.

  Remove nodes for which the supplied function returns a non-zero int. The
  function is passed a pointer to the value instead of the value itself so that
  it can also modify it if necessary.

  @attention
  The value of `errno` should be checked for errors when this function returns.

  @warning
  See the warning for `RBT_NODE_RETRIEVE()` about directly inserting
  `RBT_VALUE_NULL` values.

  @param[in]
  node The root node.

  @param[in]
  func_v A function of type `RBT_NODE_FILTER_FUNCTION_T` for filtering nodes by
  value.

  @param[in]
  include_empty Pass empty nodes to the function.

  @param[in]
  ... Additional arguments to pass to `func_v`. They will be passed in a
  `va_list`, which will be re-initialized before each call.

  @see
  - `RBT_NODE_FILTER_FUNCTION_T`
*/

void
RBT_NODE_FILTER(
  RBT_NODE_T * node,
  RBT_NODE_FILTER_FUNCTION_T func_v,
  int include_empty,
  ...
)
{
  int rc, unwanted, is_left, placeholder;
  RBT_NODE_T * tmp_node, * sibling_node;
  RBT_NODE_STACK_T * parent_stack, * parent_stack_tmp, * parent_stack_unused;
  _RBT_NODE_FILTER_STACK_T * stack, * stack_tmp, * stack_unused;
  va_list func_args;

  errno = 0;

  if (node == NULL)
  {
    return;
  }

  stack = NULL;
  stack_unused = NULL;
  parent_stack = NULL;
  parent_stack_unused = NULL;
  rc = 0;

//   RBT_NODE_T * root
//   root = node;

  while (1)
  {
//     if (RBT_DEBUG)
//     {
//       RBT_NODE_FPRINT(RBT_DEBUG_FD, root, 0, 1);
//     }
    if (node != NULL)
    {
      if (parent_stack != NULL)
      {
        debug_printf("filtering node: %p (parent: %p)\n", node, parent_stack->node);
      }
      else
      {
        debug_printf("filtering node: %p (no parent)\n", node);
      }
      if (include_empty || !RBT_VALUE_IS_NULL(node->value))
      {
        va_start(func_args, include_empty);
        /*
          This would purge superfluous nodes:
        */
//         unwanted = func_v(node->value, func_args) || RBT_VALUE_IS_NULL(node->value);
        unwanted = func_v(&(node->value), func_args);
        va_end(func_args);
        if (errno)
        {
          rc = errno;
          break;
        }
        if (unwanted)
        {
          if (parent_stack != NULL)
          {
            is_left = parent_stack->node->left == node;
            /*
              The node will not be removed if it has any children. If it has
              two then it will be left as a placeholder node. If it has only
              one then it will be merged and we will need to continue the loop.
            */
            placeholder = (node->left != NULL) && (node->right != NULL);
            if (is_left)
            {
              sibling_node = parent_stack->node->right;
            }
            else
            {
              sibling_node = parent_stack->node->left;
            }
            tmp_node = RBT_NODE_REMOVE(node, parent_stack->node);
            if (errno)
            {
              rc = errno;
              break;
            }
            /*
              If the node has been completely removed then the sibling may have
              been merged into the parent. Go back to the parent in that case.
            */
            if (tmp_node != node)
            {
              if (is_left)
              {
                /*
                  Jump directly to the sibling node if it has not changed.
                */
                if (parent_stack->node->right == sibling_node)
                {
                  node = sibling_node;
                }
                /*
                  Otherwise jump to the parent.
                */
                else
                {
                  node = parent_stack->node;
                  _RBT_NODE_STACK_POP(parent_stack, parent_stack_tmp, parent_stack_unused);
                  if (stack != NULL && stack->parent_stack->node == node)
                  {
                    _RBT_NODE_STACK_POP(stack, stack_tmp, stack_unused);
                  }
                }
              }
              /*
                If the node was the right child, then jump back to the last
                right child.
              */
              else
              {
                if (stack != NULL)
                {
                  node = stack->node;
                  while (parent_stack != NULL && parent_stack != stack->parent_stack)
                  {
                    _RBT_NODE_STACK_POP(parent_stack, parent_stack_tmp, parent_stack_unused);
                  }
                  _RBT_NODE_STACK_POP(stack, stack_tmp, stack_unused);
                }
                else
                {
                  break;
                }
              }
              continue;
            }
            /*
              If the node remains, it is either a placeholder or it has been
              merged with a child. In the latter case, continue to filter the
              new value of the node.
            */
            if (! placeholder)
            {
              continue;
            }
          }
          else
          {
            RBT_NODE_REMOVE(node, NULL);
            if (errno)
            {
              rc = errno;
              break;
            }
          }
        }
      }
      if (node->left != NULL)
      {
        _RBT_NODE_STACK_ALLOCATE(parent_stack, parent_stack_tmp, parent_stack_unused, RBT_NODE_STACK_T);
        parent_stack->node = node;
        if (node->right != NULL)
        {
          _RBT_NODE_STACK_ALLOCATE(stack, stack_tmp, stack_unused, _RBT_NODE_FILTER_STACK_T);
          stack->node = node->right;
          stack->parent_stack = parent_stack;
        }
        node = node->left;
        continue;
      }
      else if (node->right != NULL)
      {
        _RBT_NODE_STACK_ALLOCATE(parent_stack, parent_stack_tmp, parent_stack_unused, RBT_NODE_STACK_T);
        parent_stack->node = node;
        node = node->right;
        continue;
      }
    }
    if (stack != NULL)
    {
      node = stack->node;
      while (parent_stack != NULL && parent_stack != stack->parent_stack)
      {
        _RBT_NODE_STACK_POP(parent_stack, parent_stack_tmp, parent_stack_unused);
      }
      _RBT_NODE_STACK_POP(stack, stack_tmp, stack_unused);
    }
    else
    {
      break;
    }
  }
  _RBT_NODE_STACK_FREE(parent_stack, parent_stack_tmp, parent_stack_unused);
  _RBT_NODE_STACK_FREE(stack, stack_tmp, stack_unused);
  errno = rc;
}



/////////////////////////////////// Is Copy ////////////////////////////////////

/*!
  @cond INTERNAL
*/

/*!
  @brief
  Linked list for implementing RBT_NODE_IS_COPY's dynamic stack.

  \warning
  This is an internal structure.
*/

typedef
struct _RBT_NODE_IS_COPY_STACK_T
{
  /*!
    @brief
    One of the nodes to compare.
  */
  RBT_NODE_T * a;

  /*!
    @brief
    The other node to compare.
  */
  RBT_NODE_T * b;

  /*!
    @brief
    The next item in the stack.
  */
  struct _RBT_NODE_IS_COPY_STACK_T * next;
}
_RBT_NODE_IS_COPY_STACK_T;

/*!
  @endcond
*/

/*!
  @brief
  Check if a tree is a copy.

  Determine if two trees are copies of each other. A node-by-node comparison is
  used.

  @attention
  The value of `errno` should be checked for errors when this function returns.

  @param[in]
  a The first root node.

  @param[in]
  b The second root node.

  @return
  An `int` that will be non-zero if the trees are copies.
*/

int
RBT_NODE_IS_COPY(
  RBT_NODE_T * a,
  RBT_NODE_T * b
)
{
  _RBT_NODE_IS_COPY_STACK_T * stack, * stack_tmp, * stack_unused;
  int rc;

  errno = 0;

  if (a == NULL)
  {
    if (b == NULL)
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }
  else if (b == NULL)
  {
    return 0;
  }

  stack = NULL;
  stack_unused = NULL;

  rc = 1;
  while (rc)
  {
    if (
      a->bits != b->bits ||
      (a->left == NULL) != (b->left == NULL) ||
      (a->right == NULL) != (b->right == NULL) ||
      ! RBT_VALUE_IS_EQUAL(a->value, b->value) ||
      RBT_COMMON_BIT_PREFIX_LEN(a->key, b->key, a->bits) != a->bits
    )
    {
      rc = 0;
      break;
    }
    if (a->left != NULL)
    {
      if (a->right != NULL)
      {
        _RBT_NODE_STACK_ALLOCATE(stack, stack_tmp, stack_unused, _RBT_NODE_IS_COPY_STACK_T);
        stack->a = a->right;
        stack->b = b->right;
      }
      a = a->left;
      b = b->left;
    }
    else if (a->right != NULL)
    {
      a = a->right;
      b = b->right;
    }
    else if (stack != NULL)
    {
      a = stack->a;
      b = stack->b;
      _RBT_NODE_STACK_POP(stack, stack_tmp, stack_unused);
    }
    else
    {
      break;
    }
  }
  _RBT_NODE_STACK_FREE(stack, stack_tmp, stack_unused);
  return rc;
}








/////////////////////////////////// Retrieve ///////////////////////////////////
/*
  @cond INTERNAL
  Use a macro to avoid repetition in the function. This is strictly internal
  and independent of user-defined macros.
*/

#ifndef _RBT_RETRIEVE_RETURN_WITH_PARENT
  #define _RBT_RETRIEVE_RETURN_WITH_PARENT(node, parent_node_ptr) \
  do \
  { \
    if (parent_node_ptr != NULL) \
    { \
      * parent_node_ptr = parent_node; \
    } \
    return node; \
  } \
  while (0)
#endif

/*!
  @endcond
*/

/*!
  @brief
  Retrieve a node matching the given key.

  A function to retrieve a node from a tree given a key. If a matching node is
  found, it is returned. Depending on the given action, missing nodes may also
  be inserted along with a given value, or the value of an existing node may be
  updated. The latter action is supported as a matter of algorithmic
  convenience.

  This function does not remove nodes. Instead it will update a given pointer to
  point to the parent node of the returned node. A wrapper function  such as `RBT_NODE_ may then
  call `RBT_NODE_REMOVE()`, perhaps conditionally given the current value of the
  node. This provides versatility without the complexity of handling multiple
  cases in this function along with the concomitant overhead of additional
  checks.

  @attention
  The value of `errno` should be checked for errors when this function returns.

  @warning
  The value of a node that is not `RBT_VALUE_NULL` should not be directly set to
  `RBT_VALUE_NULL`. Nodes with a value of `RBT_VALUE_NULL` are considered
  placeholder nodes and are only used as parent nodes when required for the tree
  structure. Manually inserting `RBT_VALUE_NULL` will cause some assumptions
  about the tree structure to fail. To remove a node, pass it to
  `RBT_NODE_REMOVE()` along with the parent pointer, which will restructure the
  tree if necessary.

  @warning
  There are some exceptional cases in which this may nevertheless be useful. In
  those it should be possible to pass `RBT_VALUE_NULL` to `RBT_NODE_FILTER()` to
  remove the unexpected nodes. However, all functions are written under certain
  assumptions about the tree structure. It is up to you to determine if any
  would be broken by the presence of such nodes.

  @warning
  In summary, don't do this unless you really know what you're doing.

  @param[in]
  node A pointer to the root node.

  @param[in]
  key The key.

  @param[in]
  bits The number of significant bits in the key. Bits beyond these will
  be ignored.

  @param[in]
  action The action that should be performed.

  @param[in]
  value The value with which the action will be performed.

  @param[out]
  parent_node_ptr An optional pointer to a parent node pointer. This
  pointer will be updated with the target node's parent for some actions if it
  is not NULL. This is required for deleting nodes in a wrapper function. For
  other actions it has a special use (mostly for wrapper functions).

  RBT_RETRIEVE_ACTION_PREFIX_SUBTREE
  :   The point must point to an allocated node. The bits field of the node
      will be updated to indicate the bit different between the retrieved node
      and the target prefix. This is used to construct the matching subtree in
      a separate function.

  @return
  The target node pointer, or `NULL` if no matching node was found or an
  error occured.

  @see
  - `RBT_NODE_QUERY()`
*/

RBT_NODE_T *
RBT_NODE_RETRIEVE(
  RBT_NODE_T * node,
  RBT_PIN_T * key,
  RBT_KEY_SIZE_T bits,
  rbt_retrieve_action_t action,
  RBT_VALUE_T value,
  RBT_NODE_T * * parent_node_ptr
)
{
  debug_print_func(RBT_FPRINT_BITS, 1, key, bits, 0);
  debug_printf("bits: %" RBT_KEY_SIZE_T_FORMAT "\n", bits);
  debug_printf("action: %s\n", rbt_retrieve_action_string(action));
  /*
    The "pin" is the unit of the key array in the node. If the array is a byte
    array then the pin is a byte, etc. The name is from the pins in a tumbler
    lock.
  */
  int rc;
  RBT_KEY_SIZE_T common_bits, common_staggered_bits, common_pins;
  RBT_NODE_T * parent_node;
  RBT_NODE_T * * child_node_ptr;
  RBT_PIN_T * tmp_key;

  errno = rc = 0;
  parent_node = NULL;
  child_node_ptr = NULL;

  /*
    The root node is the only node that may have 0 bits in its key and which
    cannot be deleted. Handle it as a special case.
  */
  if (node->bits == 0)
  {
    debug_print("keyless root node\n");
    /*
      The key has no bits. This should be uncommon but there is no reason to
      prevent it.
    */
    if (bits == 0)
    {
      debug_print("received empty key\n");
      switch (action)
      {
        case RBT_RETRIEVE_ACTION_INSERT_OR_REPLACE:
          RBT_VALUE_COPY(node->value, value, return NULL);

        default:
          _RBT_RETRIEVE_RETURN_WITH_PARENT(node, parent_node_ptr);
          break;
      }
    }
    /*
      The requested key is non-zero here whereas the root node is zero. If the
      root node has no children then there are two possibilities: it is empty
      and can be used to hold a new value, or it is not empty and a new child
      would be required to hold the value.
    */
    else if (node->left == NULL && node->right == NULL)
    {
      debug_print("root node has no children\n");
      switch (action)
      {
        case RBT_RETRIEVE_ACTION_INSERT:
        case RBT_RETRIEVE_ACTION_INSERT_OR_REPLACE:
          if (RBT_VALUE_IS_NULL(node->value))
          {
             debug_print("root node is empty\n");
            /*
               common_pins is a misnomer here (variable re-use).
            */
            common_pins = BITS_TO_PINS_TO_BYTES(bits);
            /*
              Use tmp key to preserve node if malloc fails.
            */
            tmp_key = malloc(common_pins);
            if (tmp_key == NULL)
            {
              debug_printf(
                "failed to malloc %" RBT_KEY_SIZE_T_FORMAT " bytes for key (error: %s)\n",
                common_pins,
                strerror(errno)
              );
              return NULL;
            }
            free(node->key);
            node->key = tmp_key;
            memcpy(node->key, key, common_pins);
            node->bits = bits;
            RBT_VALUE_COPY(node->value, value, return NULL);
            debug_print("target is root node\n");
          }
          else
          {
            parent_node = node;
            node = RBT_NODE_CREATE(key, bits, value, NULL, NULL);
            if (node == NULL)
            {
              debug_printf("failed to create node (%s)\n", strerror(errno));
              return NULL;
            }
            if (FIRST_BIT_IS_1(key[0]))
            {
              parent_node->right = node;
            }
            else
            {
              parent_node->left = node;
            }
            debug_print("target is child of root node\n");
          }
          _RBT_RETRIEVE_RETURN_WITH_PARENT(node, parent_node_ptr);
          break;

        default:
          return NULL;
          break;
      }
    }
    /*
      If the root node has children and no key, then a non-zero key must lead
      to one of the children.
    */
    else
    {
      debug_print("root node has at least one child\n");
      parent_node = node;
      if (FIRST_BIT_IS_1(key[0]))
      {
        debug_print("checking right child\n");
        child_node_ptr = &node->right;
      }
      else
      {
        debug_print("checking left child\n");
        child_node_ptr = &node->left;
      }
      parent_node = node;
      /*
        The target child is missing.
      */
      if (* child_node_ptr == NULL)
      {
        debug_print("target child of root node is missing\n");
        switch (action)
        {
          case RBT_RETRIEVE_ACTION_INSERT:
          case RBT_RETRIEVE_ACTION_INSERT_OR_REPLACE:
            node = RBT_NODE_CREATE(key, bits, value, NULL, NULL);
            if (node == NULL)
            {
              debug_printf("failed to create node (%s)\n", strerror(errno));
              return NULL;
            }
            * child_node_ptr = node;
            debug_print("inserted missing child\n");
            _RBT_RETRIEVE_RETURN_WITH_PARENT(node, parent_node_ptr);
            break;

          default:
            return NULL;
            break;
        }
      }
      else
      {
        node = * child_node_ptr;
      }
    }
  }

  /*
    Walk along the nodes until:
    a) we find a matching node (all bits are common)
    b) we run out of nodes (a new child is needed)
    c) we run out of key bits (a new parent is needed)
    d) we find mismatched bits (a new sibling is needed)
  */
//   while (node != NULL)
  while (1)
  {
    common_bits = RBT_COMMON_BIT_PREFIX_LEN(key, node->key, MIN(bits, node->bits));
    debug_print_func(RBT_FPRINT_BITS, 1, key, bits, 0);
    debug_printf("node %p\n", node);
    debug_print_func(RBT_FPRINT_BITS, 1, node->key, node->bits, 0);
    debug_printf("common bits: %" RBT_KEY_SIZE_T_FORMAT "\n", common_bits);
    /*
      If the common bits match the remaining bits then we have matched our
      input key. The matching node is either this one or a missing parent.
    */
    if (common_bits == bits)
    {
      /*
        If all of the node bits are also common then this node matches.
      */
      if (common_bits == node->bits)
      {
        debug_print("matched\n");
        switch (action)
        {
          case RBT_RETRIEVE_ACTION_PREFIX_SUBTREE:
            if (parent_node_ptr != NULL)
            {
              (* parent_node_ptr)->bits = 0;
            }
            return node;
            break;

          case RBT_RETRIEVE_ACTION_INSERT_OR_REPLACE:
            RBT_VALUE_COPY(node->value, value, return NULL);

          default:
            _RBT_RETRIEVE_RETURN_WITH_PARENT(node, parent_node_ptr);
            break;
        }
      }
      /*
        Else the node contains more bits and does not match. The matching node
        is a missing parent node. Inserting the parent node will update the
        current node, so it and the parent node remain valid.
      */
      else
      {
        debug_print("target is parent\n");
        switch (action)
        {
          case RBT_RETRIEVE_ACTION_PREFIX_SUBTREE:
            if (parent_node_ptr != NULL)
            {
              (* parent_node_ptr)->bits = node->bits - common_bits;
            }
            return node;
            break;

          case RBT_RETRIEVE_ACTION_INSERT:
          case RBT_RETRIEVE_ACTION_INSERT_OR_REPLACE:
            RBT_NODE_INSERT_PARENT(node, bits, value);
            if (errno)
            {
              return NULL;
            }
            _RBT_RETRIEVE_RETURN_WITH_PARENT(node, parent_node_ptr);
            break;

          default:
            return NULL;
            break;
        }
      }
    }
    else
    {
      RBT_DIVMOD(common_bits, RBT_PIN_SIZE_BITS, common_pins, common_staggered_bits);
      /*
        If the node bits are common then the key contains more bits than
        the node. The matching node will be a child.
      */
      if (common_bits == node->bits)
      {
        key += common_pins;
        bits +=  common_staggered_bits - common_bits;
        debug_print("found parent\n");
        parent_node = node;
        if (N_BIT_IS_1(key[0], common_staggered_bits))
        {
          debug_print("right\n");
          child_node_ptr = &(node->right);
        }
        else
        {
          debug_print("left\n");
          child_node_ptr = &(node->left);
        }
        /*
          The child node does not exist.
        */
        if (* child_node_ptr == NULL)
        {
          debug_print("missing child\n");
          switch (action)
          {
            case RBT_RETRIEVE_ACTION_INSERT:
            case RBT_RETRIEVE_ACTION_INSERT_OR_REPLACE:
              RBT_NODE_INSERT_CHILD(child_node_ptr, key, bits, value);
              if (errno)
              {
                return NULL;
              }
              parent_node = node;
              node = * child_node_ptr;
              _RBT_RETRIEVE_RETURN_WITH_PARENT(node, parent_node_ptr);
              break;

            default:
              return NULL;
              break;
          }
        }
        else
        {
          debug_print("checking child\n");
          node = * child_node_ptr;
        }
      }
      /*
        If there are common bits followed by divergent bits then the missing
        node is a sibling node and a new parent is required to hold both.
      */
      else
      {
        debug_print("target is sibling\n");
        switch (action)
        {
          case RBT_RETRIEVE_ACTION_INSERT:
          case RBT_RETRIEVE_ACTION_INSERT_OR_REPLACE:
            parent_node = node;
            node = RBT_NODE_INSERT_SIBLING(
              node,
              key,
              bits,
              common_bits,
              common_pins,
              common_staggered_bits,
              value
            );
            _RBT_RETRIEVE_RETURN_WITH_PARENT(node, parent_node_ptr);
            break;

          default:
            return NULL;
            break;
        }
      }
    }
  }
  /*
    This should be unreachable. It is included to prevent compiler warnings.
  */
//   errno = ENOTSUP;
  errno = ENOTRECOVERABLE;
  error_print("control has escaped loop\n");
  return NULL;
}






//////////////////////////////////// Query /////////////////////////////////////

/*!
  @brief
  Query a tree.

  A function to retrieve a node from a tree given a key. If a matching node is
  found, it is returned. Depending on the given action, missing nodes may also
  be inserted along with a given value, or the value of an existing node may be
  updated. The latter action is supported as a matter of algorithmic
  convenience.

  This function does not remove nodes. Instead it will update a given pointer to
  point to the parent node of the returned node. A wrapper function  such as `RBT_NODE_ may then
  call `RBT_NODE_REMOVE()`, perhaps conditionally given the current value of the
  node. This provides versatility without the complexity of handling multiple
  cases in this function along with the concomitant overhead of additional
  checks.

  @attention
  The value of `errno` should be checked for errors when this function returns.

  @param[in]
  root_node A pointer to the root node.

  @param[in]
  key The key.

  @param[in]
  bits The number of significant bits in the key. Bits beyond these will
  be ignored.

  @param[in]
  action The action that should be performed.

  @param[in]
  value The value with which the action will be performed.

  @return
  The value determined by the action.

  @see
  - `RBT_NODE_RETRIEVE()`
*/

RBT_VALUE_T
RBT_NODE_QUERY(
  RBT_NODE_T * root_node,
  RBT_PIN_T * key,
  RBT_KEY_SIZE_T bits,
  rbt_query_action_t action,
  RBT_VALUE_T value
)
{
  RBT_NODE_T * target_node, * parent_node;
  RBT_VALUE_T target_value;
  int effective_deletion;

  /*
    Inserting an empty value is effectively a deletion.

    Store the value here to avoid possible overhead from multiple calls.
  */
  effective_deletion = RBT_VALUE_IS_NULL(value);

  switch (action)
  {
    case RBT_QUERY_ACTION_INSERT:
      if (effective_deletion)
      {
        action = RBT_QUERY_ACTION_DELETE;
      }
      break;

    default:
      break;
  }

  /*
    # TODO
    Consider mergine the following switch statements. They were separated to
    centralize the call to RBT_NODE_RETRIEVE.
  */

  switch (action)
  {
    case RBT_QUERY_ACTION_DELETE:
      target_node = RBT_NODE_RETRIEVE(
        root_node,
        key,
        bits,
        RBT_RETRIEVE_ACTION_NOTHING,
        RBT_VALUE_NULL,
        &parent_node
      );
      if (errno)
      {
        return RBT_VALUE_NULL;
      }
      if (target_node != NULL && ! RBT_VALUE_IS_NULL(target_node->value))
      {
        RBT_NODE_REMOVE(target_node, parent_node);
      }
      return RBT_VALUE_NULL;
      break;



    case RBT_QUERY_ACTION_RETRIEVE:
      target_node = RBT_NODE_RETRIEVE(
        root_node,
        key,
        bits,
        RBT_RETRIEVE_ACTION_NOTHING,
        RBT_VALUE_NULL,
        &parent_node
      );
      if (errno || target_node == NULL)
      {
        return RBT_VALUE_NULL;
      }
      else
      {
        return target_node->value;
      }
      break;



    case RBT_QUERY_ACTION_INSERT:
      target_node = RBT_NODE_RETRIEVE(
        root_node,
        key,
        bits,
        RBT_RETRIEVE_ACTION_INSERT_OR_REPLACE,
        value,
        &parent_node
      );
      if (errno)
      {
        return RBT_VALUE_NULL;
      }
      return value;
      break;



    case RBT_QUERY_ACTION_RETRIEVE_AND_INSERT:
      target_node = RBT_NODE_RETRIEVE(
        root_node,
        key,
        bits,
        RBT_RETRIEVE_ACTION_INSERT,
        RBT_VALUE_NULL,
        &parent_node
      );
      if (errno)
      {
        return RBT_VALUE_NULL;
      }
      /*
        There is no need to copy this value as it would be overwritten when
        the updated with the new value.
      */
      target_value = target_node->value;
      target_node->value = RBT_VALUE_NULL;
      if (effective_deletion)
      {
        RBT_NODE_REMOVE(target_node, parent_node);
        if (errno)
        {
          return RBT_VALUE_NULL;
        }
      }
      else
      {
        /*
          If something goes wrong, restore the value and return the input value.
        */
        RBT_VALUE_COPY(target_node->value, value, target_node->value = target_value; return value);
      }
      return target_value;
      break;



    case RBT_QUERY_ACTION_SWAP:
      target_node = RBT_NODE_RETRIEVE(
        root_node,
        key,
        bits,
        RBT_RETRIEVE_ACTION_INSERT,
        RBT_VALUE_NULL,
        &parent_node
      );
      if (errno)
      {
        return RBT_VALUE_NULL;
      }
      target_value = target_node->value;
      if (effective_deletion)
      {
        /*
          Prevent target_value from being freed.
        */
        target_node->value = RBT_VALUE_NULL;
        RBT_NODE_REMOVE(target_node, parent_node);
        if (errno)
        {
          return RBT_VALUE_NULL;
        }
      }
      else
      {
        target_node->value = value;
      }
      return target_value;
      break;
  }
  /*
    This should be unreachable, but compile complains if there is not return
    statement.
  */
  return RBT_VALUE_NULL;
}







/////////////////////////////////// Traverse ///////////////////////////////////


/*!
  @cond INTERNAL
*/

/*!
  @brief
  Linked list for implementing RBT_NODE_TRAVERSE's dynamic stack.

  \warning
  This is an internal structure.
*/

typedef struct _RBT_NODE_TRAVERSE_STACK_T
{
  /*!
    @brief
    The node on the stack.
  */
  RBT_NODE_T * node;

  /*!
    @brief
    The height of the node above the root node (or the depth, depending on
    how you look at it).
  */
  RBT_KEY_SIZE_T height;

  /*!
    @brief
    The next item in the stack.
  */
  struct _RBT_NODE_TRAVERSE_STACK_T * next;
}
_RBT_NODE_TRAVERSE_STACK_T;

/*!
  @endcond
*/


/*!
  @brief
  A tree traversal function.

  The type of function called by `RBT_NODE_TRAVERSE()` for each node as
  the tree is traversed.

  The returned value can be used to stop traversal.

  @param[in]
  node The current node.

  @param[in]
  height The height of the current node above the root node (or the depth if
  you view the root node as being at the top).

  @param[in]
  args A `va_list` containing the additional arguments passed to
  `RBT_NODE_TRAVERSE()`.

  @return
  An `int` that will stop traversal if non-zero.
*/
typedef
int
(* RBT_NODE_TRAVERSE_FUNCTION_T)(
  RBT_NODE_T * node,
  RBT_KEY_SIZE_T height,
  va_list args
);



/*!
  Traverse a tree and pass each node to a function along with its height.

  This function does not reconstruct the key associated with each node.

  @see RBT_NODE_TRAVERSE_WITH_KEY()

  @param[in]
  node The root node.

  @param[in]
  func_v The function to call with each node.

  @param[in]
  ... Additional argument to pass to the called function along with each node.
  These will be passed as a `va_list`.
*/
void
RBT_NODE_TRAVERSE(
  RBT_NODE_T * node,
  RBT_NODE_TRAVERSE_FUNCTION_T func_v,
  ...
)
{
  int rc;
  _RBT_NODE_TRAVERSE_STACK_T * stack, * stack_tmp, * stack_unused;
  RBT_KEY_SIZE_T height;
  va_list func_args;

  errno = 0;

  if (node == NULL)
  {
    return;
  }

  height = 0;
  stack = NULL;
  stack_unused = NULL;
  rc = 0;

  while (1)
  {
    va_start(func_args, func_v);
    if (func_v(node, height, func_args) || errno)
    {
      rc = errno;
      break;
    }
    va_end(func_args);

    if (node->right == NULL)
    {
      if (node->left == NULL)
      {
        if (stack == NULL)
        {
          break;
        }
        else
        {
          node = stack->node;
          height = stack->height;
          _RBT_NODE_STACK_POP(stack, stack_tmp, stack_unused);
          continue;
        }
      }
      else
      {
        node = node->left;
        height ++;
      }
    }
    else if (node->left == NULL)
    {
      node = node->right;
      height ++;
    }
    else
    {
      _RBT_NODE_STACK_ALLOCATE(stack, stack_tmp, stack_unused, _RBT_NODE_TRAVERSE_STACK_T);
      height ++;
      stack->node = node->right;
      stack->height = height;
      node = node->left;
    }
  }
  _RBT_NODE_STACK_FREE(stack, stack_tmp, stack_unused);
  errno = rc;
}








/*
  This is unaffected by fixed length keys, so keep it here.
*/

/*!
  @cond INTERNAL
*/



/*!
  @brief
  Linked list for implementing RBT_NODE_FILTER_WITH_KEY's dynamic stack.

  \warning
  This is an internal structure.
*/
typedef struct _RBT_NODE_FILTER_WITH_KEY_STACK_T
{
  /*!
    @brief
    The node on the stack.
  */
  RBT_NODE_T * node;

  /*!
    @brief
    The number of bytes in the key prefix for this node.
  */
  RBT_KEY_SIZE_T bytes_in_key_prefix;

  /*!
    @brief
    The stack of parent nodes for this node.
  */
  RBT_NODE_STACK_T * parent_stack;

  /*!
    @brief
    The next item in the stack.
  */
  struct _RBT_NODE_FILTER_WITH_KEY_STACK_T * next;
}
_RBT_NODE_FILTER_WITH_KEY_STACK_T;

/*!
  @endcond
*/



/*!
  @brief
  Linked list for implementing RBT_NODE_TRAVERSE_WITH_KEY's dynamic stack.

  \warning
  This is an internal structure.
*/

typedef struct _RBT_NODE_TRAVERSE_WITH_KEY_STACK_T
{
  /*!
    @brief
    The node on the stack.
  */
  RBT_NODE_T * node;

  /*!
    @brief
    The number of bytes in the key prefix for this node.
  */
  RBT_KEY_SIZE_T bytes_in_key_prefix;

  /*!
    @brief
    The height of the node above the root node (or the depth, depending on
    how you look at it).
  */
  RBT_KEY_SIZE_T height;

  /*!
    @brief
    The next item in the stack.
  */
  struct _RBT_NODE_TRAVERSE_WITH_KEY_STACK_T * next;
}
_RBT_NODE_TRAVERSE_WITH_KEY_STACK_T;



/*!
  @brief
  The function type accepted by `RBT_NODE_FILTER_WITH_KEY()`.

  @param[in]
  key_data The full key associated with the node. This must not be modified by
  the function.

  @param[in]
  args A `va_list` containing the additional arguments passed to
  `RBT_NODE_FILTER()`.

  @return
  An `int`. If it is non-zero, the current node will be *removed*.
*/
typedef
int
(* RBT_NODE_FILTER_WITH_KEY_FUNCTION_T)(
  RBT_KEY_DATA_T * key_data,
  va_list args
);



/*!
  @brief
  A tree traversal function.

  The type of function called by `RBT_NODE_TRAVERSE_WITH_KEY()` for each node
  as the tree is traversed.

  The returned value can be used to stop traversal.

  @param[in]
  key_data The full key associated with the node. This must not be modified by
  the function.

  @param[in]
  height The height of the current node above the root node (or the depth if
  you view the root node as being at the top).

  @param[in]
  args A `va_list` containing the additional arguments passed to
  `RBT_NODE_TRAVERSE_WITH_KEY()`.

  @return
  An `int` that will stop traversal if non-zero.
*/
typedef
int
(* RBT_NODE_TRAVERSE_WITH_KEY_FUNCTION_T)(
  RBT_KEY_DATA_T * key_data,
  RBT_KEY_SIZE_T height,
  va_list args
);


//////////////////////////////////// Count /////////////////////////////////////
/*!
  Count the number of values in the tree.

  @param[in]
  node The root node.
*/
RBT_KEY_SIZE_T
RBT_NODE_COUNT(
  RBT_NODE_T * node
)
{
  int rc;
  RBT_NODE_STACK_T * stack, * stack_tmp, * stack_unused;
  RBT_KEY_SIZE_T n;

  errno = 0;

  if (node == NULL)
  {
    return 0;
  }

  n = 0;
  stack = NULL;
  stack_unused = NULL;
  rc = 0;

  while (1)
  {
    if (! RBT_VALUE_IS_NULL(node->value))
    {
      n ++;
    }

    if (node->right == NULL)
    {
      if (node->left == NULL)
      {
        if (stack == NULL)
        {
          break;
        }
        else
        {
          node = stack->node;
          _RBT_NODE_STACK_POP(stack, stack_tmp, stack_unused);
          continue;
        }
      }
      else
      {
        node = node->left;
      }
    }
    else if (node->left == NULL)
    {
      node = node->right;
    }
    else
    {
      _RBT_NODE_STACK_ALLOCATE(stack, stack_tmp, stack_unused, _RBT_NODE_TRAVERSE_WITH_KEY_STACK_T);
      stack->node = node->right;
      node = node->left;
    }
  }
  _RBT_NODE_STACK_FREE(stack, stack_tmp, stack_unused);
  errno = rc;
  return n;
}


/////////////////////////////////// Subtree ////////////////////////////////////

/*!
  @brief
  Perform some action on the subtree defined by a common prefix.

  @attention
  The value of `errno` should be checked for errors when this function returns.

  @param[in]
  node A pointer to the root node.

  @param[in]
  key The key, i.e. the prefix.

  @param[in]
  bits The number of significant bits in the key. Bits beyond these will
  be ignored. This can thus determine the different prefixes for the same key.

  @param[in]
  func_v The function to call with the subtree.

  @param[in]
  ... Additional arguments to func_v.

  @return
  1 if a matching subtree was found,
  0 if no matching subtree was found or if an error occured (check errno).
*/

int
RBT_NODE_WITH_PREFIX_SUBTREE_DO(
  RBT_NODE_T * node,
  RBT_PIN_T * key,
  RBT_KEY_SIZE_T bits,
  void (* func_v)(RBT_NODE_T * subtree, va_list args),
  ...
)
{
  RBT_NODE_T subroot_node, * tmp_node, * node_ptr;
  RBT_KEY_SIZE_T bytes, prefix_bytes, total_bits, extra_bits;
  va_list func_args;

  node_ptr = & subroot_node;

  tmp_node = RBT_NODE_RETRIEVE(
    node, key, bits,
    RBT_RETRIEVE_ACTION_PREFIX_SUBTREE,
    RBT_VALUE_NULL,
    & node_ptr
  );

  if (tmp_node == NULL)
  {
    return 0;
  }

  extra_bits = subroot_node.bits;
  total_bits = bits + extra_bits;
  bytes = BITS_TO_PINS_TO_BYTES(total_bits);
  subroot_node.key = malloc(bytes);
  if (subroot_node.key == NULL)
  {
    debug_printf("failed to allocate memory for key (%s)\n", strerror(errno));
    return 0;
  }
  prefix_bytes = (bits + extra_bits - tmp_node->bits) / BITS_PER_BYTE;
  memcpy(subroot_node.key, key, prefix_bytes);
  memcpy(subroot_node.key + prefix_bytes, tmp_node->key, bytes - prefix_bytes);
  subroot_node.bits = total_bits;
  subroot_node.value = tmp_node->value;
  subroot_node.left = tmp_node->left;
  subroot_node.right = tmp_node->right;


  va_start(func_args, func_v);
  func_v(&subroot_node, func_args);
  va_end(func_args);

  free(subroot_node.key);
  return 1;
}

