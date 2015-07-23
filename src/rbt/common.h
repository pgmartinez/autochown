/*!
  @file
  @author Xyne
  @copyright GPL2 only
*/

/*!
  @mainpage

  @tableofcontents

  # Introduction

  ## Overview
  Rabbit trees (radix bit tries) are binary [radix tries](https://secure.wikimedia.org/wikipedia/en/wiki/Radix_trie) that use bit arrays as keys.

  A visual representation should help to clarify the concept before proceeding.
  The following is a rabbit tree that associates string constants with 32-bit
  keys:

      00000
          ├0
          │├0
          ││├0000000000000000000000000 zero
          ││└1000000000000000000000000 one
          │└1
          │ ├0000000000000000000000000 two
          │ └1000000000000000000000000 three
          └1
           ├0
           │├0000000000000000000000000 four
           │└1000000000000000000000000 five
           └1
            ├0000000000000000000000000 six
            └1000000000000000000000000 seven

  Each line represents a node. The leading "0"s and "1"s represent the bits in
  each node's key fragment. Box-drawing characters show the tree hierarchy, with
  the first line representing the root node. Strings at the ends of lines
  represent values held by nodes. The key associated with a node is the sequence
  of key fragments that start at the root node and end at the target node.

  For example, the key associated with the node containing the string "four" is
  `00000` + `1` + `0` + `0000000000000000000000000` =
  `00000100000000000000000000000000`. Incidentally, this corresponds to a 32-bit
  `int` with the value 4, because the tree maps integer numbers to their English
  string representations.

  Lookups proceed by matching fragments of a query key against the key fragment
  of a node. When all of the bits match, the next bit of the query key determines
  which child will be used to continue the query.

  For example, given the key `00000110000000000000000000000000` and the previous
  tree, the following will occur, starting at the root node:

  - The root node's key fragment matches the first 5 bits of the query key:
  `00000` + `110000000000000000000000000`. The next bit in the query key is `1`
  so the right child is chosen (the lower child in the above representation).

  - The node's single bit matches the next bit of the query key: `1` +
  `10000000000000000000000000`. The bit after that is also `1`, so go right
  again.

  - The node's single bit matches the next bit of the query key: `1` +
  `0000000000000000000000000`. The bit after that is `0`, so go left.

  - The node's bits match the remaining query key bits:
  `0000000000000000000000000` This is the associated node.


  # Motivation
  If a pointer to the underlying contiguous bits can be acquired for a given
  data type then objects of that type can be used as keys. This means that
  rabbit trees can be used with almost any data type, such as `int`s, `float`s,
  `struct`s and even strings. The key length does not need to be fixed either.

  A rabbit tree requires at most n-1 placeholder nodes to hold n values and the
  depth of a tree of key size k cannot exceed k. Lookups, insertions and
  deletions are done O(k) time in the worst case scenario, which only occurs
  when the tree is full.


  # Implementation

  ## Concepts
  Rabbit tree keys are implemented as arrays of unsigned integer types. The type
  of integer used to implement the array is referred to as the "pin" type due to
  the analogy with a [pin tumbler
  lock](https://secure.wikimedia.org/wikipedia/en/wiki/Pin_tumbler_lock).

  Wrapper functions are provided to enable transparent conversion between user
  key types (e.g. ints, structs, strings) and the internal key type represented
  as an array of pins.

  ## Code
  To provide the greatest flexibility, rabbit trees rely on macros and use
  several header files to enable code re-use. It is therefore possible to use
  nodes holding a given value type with different key types without redundant
  definitions of the node functions.

  Each header will require certain macros to be defined before it is included.
  These are detailed in the documentation for each header.
*/

/*!
  @page remarks Remarks

  # Coding Style
  I am relatively new to C, which I am sure is the first thing that you want to
  hear from the author. If you see something in the code that could have been
  done much more elegantly or efficiently, it is probably because I did not
  know how. My goal is to improve the code along with my understanding of C, so
  please share any insights on how I can do so.

  ## Macro Abuse
  I use macros extensively to provide the greatest versatility. It may be
  possible to replace some of them with inline functions but for many that would
  be too restrictive. I have attempted to limit their use to what I consider
  necessary, but some will nevertheless find even this too much.

  I use what works. If you have a better suggestion, I will gladly hear it.

  Incidentally, the first draft implementation used macros for everything. I
  soon realized just how difficult that was to maintain and rewrote everything
  from scratch.

  # Stability
  I will try to keep the interface stable but I guarantee nothing. The code is
  still fresh and largely untested in real-world contexts. If I suddenly realize
  that I did something the wrong way (see previous section) then I will change
  it. Initially I do not expect many if anyone else to use this, so interface
  changes will not be a problem. If others do begin to use the code then I will
  try to stabilize the interface while taking into account feedback.

  # Utility
  Some may ask why I even bothered writing this. Dictionaries have been
  implemented every which way possible and surely far more competent people have
  come up with better solutions than this.

  I started to write this one day when I needed a dictionary type for some small
  C script that I was writing. After a quick search for implementations I found
  that most pulled in a load of dependencies or were not readily packaged.

  I admit that I did not look very long or hard though, but I had already begun
  thinking about ways that I could implement a dictionary myself. The concept of
  radix bit tries occurred to me quite quickly, even if I did not know their
  technical name at the time. A binary tree that forks based on bit values
  strikes me as very natural.

  Once I had that idea in my head, I just started to code. Most of the code that
  I write begins with a bit of curiosity and some free time (well, usually
  procrastination, but it's almost the same thing, at least until the deadline).

  In addition to ending up with something that I consider useful, it has also
  served as a pet project with which I have familiarized myself with C to a
  point of feeling comfortable. I obviously haven't delved too deep, but deep
  enough to have come across several different things. Finally, coming from
  high-level languages, I actually find it fun to fiddle bits (<insert joke
  here>) and implement things that those languages provide out of the box.

  Whether or not this proves to be comparably efficient to more widely-used
  alternatives, I expect it to suffice for my own applications. It frees me from
  dealing with others' dependencies and it lets me easily tweak it when needed.

  In short, it suites me and I provide it in case it suites someone else. It is
  up to you to determine if it is useful for you.

  # Source Code Management
  I manage the source code internally with Git, but I rewrite history so often
  that I could work for Minitrue. At some point I will stop this and upload the
  repository.
*/


#ifndef RBT_HEADER_COMMON
#define RBT_HEADER_COMMON

#include <limits.h>

/*!
  @brief
  A byte type.
*/
#define BYTE_T                  char

/*!
  @brief
  The number of bits in a byte.
*/
#define BITS_PER_BYTE           CHAR_BIT


/*!
  @brief
  Return the minimum of `a` or `b`.

  @param[in]
  a The first number.

  @param[in]
  b The second number.
*/
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

/*!
  @brief
  Return the maximum of `a` or `b`.

  @param[in]
  a The first number.

  @param[in]
  b The second number.
*/
#define MAX(a,b) (((a) > (b)) ? (a) : (b))


/*!
  @brief
  Combine `a` and `b` into a single token.

  @param[in]
  a The first part.

  @param[in]
  b The second part.
*/
#define RBT_TOKEN_2(a,b) a ## b

/*!
  @brief
  A wrapper for `RBT_TOKEN_2()` to ensure macro expansion.

  @param[in]
  a The first part. This may be a macro.

  @param[in]
  b The second part. This may be a macro.
*/
#define RBT_TOKEN_2_W(a,b) RBT_TOKEN_2(a,b)

/*!
  @brief
  Identical to `RBT_TOKEN_2()` except that the resulting token is prefixed with
  an underscore.

  @param[in]
  a The first part.

  @param[in]
  b The second part.
*/
#define _RBT_TOKEN_2(a,b) _ ## a ## b

/*!
  @brief
  A wrapper for `_RBT_TOKEN_2()` to ensure macro expansion.

  @param[in]
  a The first part. This may be a macro.

  @param[in]
  b The second part. This may be a macro.
*/
#define _RBT_TOKEN_2_W(a,b) _RBT_TOKEN_2(a,b)

/*!
  @brief
  The most significant bit of the given integer's type.

  @param[in]
  x The number to check.
*/
#define MOST_SIGNIFICANT_BIT(x) (((x) ~ 0) ^ (((x) ~ 0) >> 1))

/*!
  @brief
  A wrapper for `MOST_SIGNIFICANT_BIT()` to ensure macro expansion.

  @param[in]
  x The number to check.
*/
#define MOST_SIGNIFICANT_BIT_W(x) MOST_SIGNIFICANT_BIT(x)

/*!
  @brief
  Determine if the most significant bit is 1.

  @param[in]
  x The number containing the bit to check.
*/
#define FIRST_BIT_IS_1(x) (MOST_SIGNIFICANT_BIT(typeof(x)) & (x))

/*!
  @brief
  Determine if the nth bit is 1.

  @param[in]
  x The number containing the bit to check.

  @param[in]
  n The number of the bit to check.
*/
#define N_BIT_IS_1(x,n) (MOST_SIGNIFICANT_BIT(typeof(x)) & (x << n))

/*!
  @brief
  Ceiling division of a positive integer.

  Divide dividend by divisor, rounding any remainder up. This catches overflows.

  @param[in]
  x The dividend.

  @param[in]
  y The divisor.
*/

#define DIV_UP(x,y) ((x) ? (1 + (x-1)/y) : 0)





/*!
  @brief
  Retrieval action for `RBT_NODE_RETRIEVE()`.
*/
typedef enum
{
  /*!
    No additional action is performed.
  */
  RBT_RETRIEVE_ACTION_NOTHING,

  /*!
    An insertion is made if the target node does not exist. Existing values are
    not modified.
  */
  RBT_RETRIEVE_ACTION_INSERT,

  /*!
    An insertion is made if the target node does not exist. Existing values are
    modified.
  */
  RBT_RETRIEVE_ACTION_INSERT_OR_REPLACE,

  /*!
    There are three possibilities when matching a prefix.

    1) No node contains the prefix.
    2) One node's associated key is equal to the prefix. All children of this
       node also contain the prefix in their associated keys.
    3) One node's associated key is the first to contain the prefix. All
       children of this node also contain the prefix in their associated keys.

    Case 1 returns NULL without ambiguity. For cases 2 and 3, information about
    the eventual key difference is required. This can be retrieved by updating
    the bits field of the passed parent_node_ptr in the retrieve function.

    @todo
    Update with reference to example wrapper.
  */
  RBT_RETRIEVE_ACTION_PREFIX_SUBTREE
}
rbt_retrieve_action_t;


/*!
  @brief
  Return a string representing a retrieval action.

  @param[in]
  action The action for which to return a string.
*/
const char *
rbt_retrieve_action_string(rbt_retrieve_action_t action)
{
  switch (action)
  {
    case RBT_RETRIEVE_ACTION_NOTHING:
      return "NOTHING";
      break;

    case RBT_RETRIEVE_ACTION_INSERT:
      return "INSERT";
      break;

    case RBT_RETRIEVE_ACTION_INSERT_OR_REPLACE:
      return "INSERT OR REPLACE";
      break;

    default:
      return "UNKNOWN ACTION";
      break;
  }
}





/*!
  @brief
  Query action for `RBT_NODE_QUERY()`.
*/
typedef enum
{
  /*!
    Delete a key-value pair.
  */
  RBT_QUERY_ACTION_DELETE,

  /*!
    Insert a key-value pair. Existing values will be overwritten.
  */
  RBT_QUERY_ACTION_INSERT,

  /*!
    Retrieve a value associated with a key.
  */
  RBT_QUERY_ACTION_RETRIEVE,

  /*!
    Retrieve a value associated with a key and replace it with a new value.
  */
  RBT_QUERY_ACTION_RETRIEVE_AND_INSERT,

  /*!
    Directly insert a key-value pair and return an existing value if present.
    For statically allocated values this will be identical to
    `RBT_QUERY_ACTION_RETRIEVE_AND_INSERT`. For dynamically allocated values,
    this will directly swap the pointers without any freeing or allocation.
  */
  RBT_QUERY_ACTION_SWAP
}
rbt_query_action_t;


/*!
  @brief
  Return a string representing a query action.

  @param[in]
  action The action for which to return a string.
*/
const char *
rbt_query_action_string(rbt_query_action_t action)
{
  switch (action)
  {
    case RBT_QUERY_ACTION_DELETE:
      return "DELETE";
      break;

    case RBT_QUERY_ACTION_INSERT:
      return "INSERT";
      break;

    case RBT_QUERY_ACTION_RETRIEVE:
      return "RETRIEVE";
      break;

    case RBT_QUERY_ACTION_RETRIEVE_AND_INSERT:
      return "RBT_QUERY_ACTION_RETRIEVE_AND_INSERT";
      break;

    case RBT_QUERY_ACTION_SWAP:
      return "SWAP";
      break;

    default:
      return "UNKNOWN ACTION";
      break;
  }
}

#endif // RBT_HEADER_COMMON