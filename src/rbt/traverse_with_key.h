/*!
  @file
  @author Xyne
  @copyright GPL2 only

  # Usage

  These functions are kept outside of node.h to enable optimizations for key
  types of different lengths without redefinition of entries in node.h. For
  example, node.h may be used to declare key types with 8-bit pins. These may
  then be used by multiple key types such as integers or strings. The former
  use a fixed key size and thus can be used with static memory in the following
  functions.

  If `RBT_KEY_SIZE_FIXED` is defined, it will be used to allocate a static
  array for tracking the key across nodes during traversal.

  If it is not defined then the key will be resized dynamically, thus incurring
  additional overhead for memory allocation.

  In the latter case no further definitions are required before including this
  file and it can be included immediately after each inclusion of node.h.

  # Required Headers

  - node.h

  # Optional macro definitions

  - RBT_TRAVERSE_H_PREFIX_

    This can be defined to enable variations of the traverse function with the
    same node and internal key type. If not defined it defaults to
    `RBT_NODE_H_PREFIX_`.

  - RBT_KEY_SIZE_FIXED

    The size of the key, in bytes, if it is fixed or if a maximum key size
    can be anticipated.
*/

#ifdef ONLY_FOR_DOXYGEN
#include "node.h"
#endif //ONLY_FOR_DOXYGEN

/*!
  @cond INTERNAL
*/

#ifndef RBT_TRAVERSE_H_PREFIX_
#define RBT_TRAVERSE_H_PREFIX_ RBT_NODE_H_PREFIX_
#endif

#undef RBT_NODE_TRAVERSE_WITH_KEY
#define RBT_NODE_TRAVERSE_WITH_KEY RBT_TOKEN_2_W(RBT_TRAVERSE_H_PREFIX_, node_traverse_with_key)

#undef RBT_NODE_FILTER_WITH_KEY
#define RBT_NODE_FILTER_WITH_KEY RBT_TOKEN_2_W(RBT_TRAVERSE_H_PREFIX_, node_filter_with_key)

/*!
  @endcond
*/


/*!
  As the tree is traversed, the key associated with each node is determined by
  tracking the key fragments associated with each preceding node. If the key
  does not have a fixed length then the memory must be allocated dynamically.

  This macro accepts the current size of the memory block along with the
  required size and a fail statement. The current size should be set to a value
  that is at least equal to the required size.

  The required size of continued traversal should be anticipated to avoid the
  overhead of multiple calls to realloc.

  If the required size cannot be set due to overflow errors, the provided fail
  statement should be run.

  @param[in,out]
  current_size The current size.

  @param[in]
  required_size The required size to hold the key of the current node.

  @param[in]
  fail A statement or series of statements that should be run if the current size
  cannot be expanded to the required size due to integer overflow.
*/

#ifndef RBT_RESIZE_TO_FIT_KEY
#define RBT_RESIZE_TO_FIT_KEY(current_size, required_size, fail) \
do \
{ \
  current_size = required_size * 2; \
  if (current_size < required_size) \
  { \
    current_size = -1; \
    if (current_size < required_size) \
    { \
      fail; \
    } \
  } \
} \
while(0)
#endif // RBT_RESIZE_TO_FIT_KEY






/*
  Use a linked list to track right nodes while descending into left nodes. The
  linked list is constructed in such a way that the bit order (as determined by
  the endianess of RBT_PIN_T) is respected when traversing the tree. The order
  may vary across systems and should not be relied on for e.g. sorting.

  stack_unused is used to track allocated list elements to avoid unnecessary
  reallocation of memory (i.e. free -> malloc).

*/
/*!
  Traverse a tree and pass each node and its full associated key to a function,
  along with the height of the node.

  @param[in]
  node The root node.

  @param[in]
  func_v The function to call with each node.

  @param[in]
  ... Additional argument to pass to the called function along with each node.
  These will be passed as a `va_list`.
*/
void
RBT_NODE_TRAVERSE_WITH_KEY(
  RBT_NODE_T * node,
  RBT_NODE_TRAVERSE_WITH_KEY_FUNCTION_T func_v,
  ...
)
{
  int rc;
  _RBT_NODE_TRAVERSE_WITH_KEY_STACK_T * stack, * stack_tmp, * stack_unused;
  RBT_KEY_SIZE_T size, pins, bytes, tmp, height;
  RBT_KEY_DATA_T key_data;

  va_list func_args;

  errno = 0;

  if (node == NULL)
  {
    return;
  }

  key_data.bytes = 0;
  height = 0;
  stack = NULL;
  stack_unused = NULL;
  rc = 0;

///////////////////////// BEGINNING OF COMMON SECTION //////////////////////////
  /*
    If the key size is fixed then we can use an array and avoid dynamic
    allocation.
  */
#ifdef RBT_KEY_SIZE_FIXED
    RBT_PIN_T key[RBT_KEY_SIZE_FIXED/RBT_PIN_SIZE];
    key_data.key = (RBT_PIN_T *) key;
    size = RBT_KEY_SIZE_FIXED;
#else
    key_data.key = NULL;
    size = 0;
#endif //RBT_KEY_SIZE_FIXED

  while (1)
  {
    key_data.node = node;
    RBT_DIVMOD(node->bits, RBT_PIN_SIZE_BITS, pins, key_data.bits);
    bytes = pins * RBT_PIN_SIZE;
    if (key_data.bits)
    {
      bytes += RBT_PIN_SIZE;
    }
    tmp = key_data.bytes + bytes;
#ifndef RBT_KEY_SIZE_FIXED
    if (tmp > size)
    {
      RBT_RESIZE_TO_FIT_KEY(size, tmp, errno = EOVERFLOW; break);
      key_data.key = realloc(key_data.key, size);
      if (key_data.key == NULL)
      {
        rc = errno;
        break;
      }
    }
#endif //RBT_KEY_SIZE_FIXED
    memcpy(((uint8_t *) key_data.key) + key_data.bytes, node->key, bytes);
    key_data.bytes += bytes;
    if (key_data.bits)
    {
      key_data.bytes -= RBT_PIN_SIZE;
    }
    key_data.bits += (key_data.bytes * RBT_PIN_SIZE_BITS);
//////////////////////////// END OF COMMON SECTION /////////////////////////////

    va_start(func_args, func_v);
    if (func_v(&key_data, height, func_args) || errno)
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
          key_data.bytes = stack->bytes_in_key_prefix;
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
      _RBT_NODE_STACK_ALLOCATE(stack, stack_tmp, stack_unused, _RBT_NODE_TRAVERSE_WITH_KEY_STACK_T);
      height ++;
      stack->node = node->right;
      stack->height = height;
      stack->bytes_in_key_prefix = key_data.bytes;
      node = node->left;
    }
  }
#ifndef RBT_KEY_SIZE_FIXED
//   if (! (RBT_KEY_SIZE_FIXED || key == NULL))
  if (key_data.key != NULL)
  {
    free(key_data.key);
  }
#endif
  _RBT_NODE_STACK_FREE(stack, stack_tmp, stack_unused);
  errno = rc;
}

















/*!
  @brief
  Filter tree nodes.

  Remove nodes for which the supplied function returns a non-zero int. The
  function is passed a pointer to the value instead of the value itself so that
  it can also modify it if necessary.

  This function differs from RBT_NODE_FILTER() by passing the reconstructed key
  for each node and its size as the first arguments to the filter function.

  @attention
  The value of `errno` should be checked for errors when this function returns.

  @attention
  Keys for empty nodes should not be cast back to key types unless you know
  exactly what you are doing. If the reason for this is not obvious then you do
  not know exactly what you are doing and should not do it. ;)

  @warning
  See the warning for `RBT_NODE_RETRIEVE()` about directly inserting
  `RBT_VALUE_NULL` values.

  @param[in]
  node The root node.

  @param[in]
  func_v A function of type `RBT_NODE_FILTER__WITH_KEY_FUNCTION_T` for filtering
  nodes by value.

  @param[in]
  include_empty Pass empty nodes to the function.

  @param[in]
  ... Additional arguments to pass to `func_v`. They will be passed in a
  `va_list`, which will be re-initialized before each call.

  @see
  - `RBT_NODE_FILTER()
  - `RBT_NODE_FILTER_WITH_KEY_FUNCTION_T`
*/

void
RBT_NODE_FILTER_WITH_KEY(
  RBT_NODE_T * node,
  RBT_NODE_FILTER_WITH_KEY_FUNCTION_T func_v,
  int include_empty,
  ...
)
{
  int rc, unwanted, is_left, placeholder;
  RBT_NODE_T * tmp_node, * sibling_node;
  RBT_NODE_STACK_T * parent_stack, * parent_stack_tmp, * parent_stack_unused;
  RBT_KEY_SIZE_T size, pins, bytes, tmp;
  _RBT_NODE_FILTER_WITH_KEY_STACK_T * stack, * stack_tmp, * stack_unused;
  RBT_KEY_DATA_T key_data;
  va_list func_args;

  errno = 0;

  if (node == NULL)
  {
    return;
  }

  key_data.bytes = 0;
  stack = NULL;
  stack_unused = NULL;
  parent_stack = NULL;
  parent_stack_unused = NULL;
  rc = 0;

///////////////////////// BEGINNING OF COMMON SECTION //////////////////////////
  /*
    If the key size is fixed then we can use an array and avoid dynamic
    allocation.
  */
#ifdef RBT_KEY_SIZE_FIXED
    RBT_PIN_T key[RBT_KEY_SIZE_FIXED/RBT_PIN_SIZE];
    key_data.key = (RBT_PIN_T *) key;
    size = RBT_KEY_SIZE_FIXED;
#else
    key_data.key = NULL;
    size = 0;
#endif //RBT_KEY_SIZE_FIXED

  while (1)
  {
    key_data.node = node;
    RBT_DIVMOD(node->bits, RBT_PIN_SIZE_BITS, pins, key_data.bits);
    bytes = pins * RBT_PIN_SIZE;
    if (key_data.bits)
    {
      bytes += RBT_PIN_SIZE;
    }
    tmp = key_data.bytes + bytes;
#ifndef RBT_KEY_SIZE_FIXED
    if (tmp > size)
    {
      RBT_RESIZE_TO_FIT_KEY(size, tmp, errno = EOVERFLOW; break);
      key_data.key = realloc(key_data.key, size);
      if (key_data.key == NULL)
      {
        rc = errno;
        break;
      }
    }
#endif //RBT_KEY_SIZE_FIXED
    memcpy(((uint8_t *) key_data.key) + key_data.bytes, node->key, bytes);
    key_data.bytes += bytes;
    if (key_data.bits)
    {
      key_data.bytes -= RBT_PIN_SIZE;
    }
    key_data.bits += (key_data.bytes * RBT_PIN_SIZE_BITS);
//////////////////////////// END OF COMMON SECTION /////////////////////////////

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
        unwanted = func_v(&key_data, func_args);
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
                  key_data.bytes -= bytes;
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
                  if (stack != NULL)
                  {
                    key_data.bytes = stack->bytes_in_key_prefix;
                  }
                  else
                  {
                    // TODO: break instead?
                    key_data.bytes = 0;
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
                  key_data.bytes = stack->bytes_in_key_prefix;
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
          _RBT_NODE_STACK_ALLOCATE(stack, stack_tmp, stack_unused, _RBT_NODE_FILTER_WITH_KEY_STACK_T);
          stack->node = node->right;
          stack->bytes_in_key_prefix = key_data.bytes;
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
      key_data.bytes = stack->bytes_in_key_prefix;
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
#ifndef RBT_KEY_SIZE_FIXED
//   if (! (RBT_KEY_SIZE_FIXED || key == NULL))
  if (key_data.key != NULL)
  {
    free(key_data.key);
  }
#endif
  _RBT_NODE_STACK_FREE(parent_stack, parent_stack_tmp, parent_stack_unused);
  _RBT_NODE_STACK_FREE(stack, stack_tmp, stack_unused);
  errno = rc;
}
