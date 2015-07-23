/*!
  @file
  @author Xyne
  @copyright GPL2 only

  This file is included automatically by node.h when certain macros are defined.
  It provides macros and functions for using rabbit trees in a threadsafe way
  when using pthreads.

  @warning
  This is currently untested.
*/

#include <pthread.h>

/*!
  @cond INTERNAL
*/
#undef RBT_NODE_ROOT_T
#define RBT_NODE_ROOT_T RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_root_t)
/*!
  @endcond
*/

/*!
  A convenient wrapper around root nodes to provide thread safety.
*/
typedef
struct RBT_NODE_ROOT_T
{
  /*!
    @brief
    The root node.
  */
  RBT_NODE_T * node;

  /*!
    @brief
    The number of concurrent read operations.
  */
  unsigned int readers;

  /*!
    @brief
    The number of queued write operations.
  */
  unsigned int writers;

  /*!
    @brief
    The mutex that protects this root node.
  */
  pthread_mutex_t mutex;

  /*!
    @brief
    The condition variable used to signal waiting threads.
  */
  pthread_cond_t cond;
}
RBT_NODE_ROOT_T;


/*!
  @brief
  Create a new root node.

  Create a new, empty root node. The root node type is a wrapper struct around the
  node type. It includes mutexes and other variables that are used to protect
  the data in the tree from corruption caused by concurrent writes.

  @see
  RBT_NODE_ROOT_READ RBT_NODE_ROOT_WRITE
*/
RBT_NODE_ROOT_T
RBT_NODE_NEW()
{
  RBT_NODE_ROOT_T root;
  root.node = RBT_NODE_CREATE(NULL, 0, RBT_VALUE_NULL, NULL, NULL);
  root.mutex = PTHREAD_MUTEX_INITIALIZER;
  root.cond = PTHREAD_COND_INITIALIZER;
  root.readers = 0;
  root.writers = 0;
}

/*!
  Execute a read-only operation on the root node. This will prevent any write
  operation from interfering with the tree while the read operation occurs.

  The steps:

  1. Lock the mutex.
  2. Wait for writers to finish.
  3. Increment readers to prevent writes from occuring while reading.
  4. Unlock the mutex to enable concurrent reads and the queuing of writes.
  5. Proceed with read.
  6. Lock the mutex.
  7. Decrement readers.
  8. Signal other threads on last reader.
  9. Unlock the mutex.

  @param[in]
  root The root node structure.

  @param[in]
  func The function to call. The first argument will be the node followed by any
  additional arguments.

  @param[in]
  ... Additional arguments to pass to the function.
*/
#undef RBT_NODE_ROOT_READ
#define RBT_NODE_ROOT_READ(root, func, ...) \
pthread_mutex_lock(&(root.mutex)); \
while (root.writers) \
{ \
  pthread_cond_wait(&(root.cond), &(root.mutex)); \
} \
root.readers ++; \
pthread_mutex_unlock(&(root.mutex)); \
func(root.node, ##__VA_ARGS__); \
pthread_mutex_lock(&(root.mutex)); \
root.readers --; \
if (! root.readers) \
{ \
  pthread_cond_signal(&(root.cond)); \
} \
pthread_mutex_unlock(&(root.mutex))


/*!
  Execute a write operation on the root node. This will prevent any other write
  operation from interfering with the tree while the operation occurs.

  The operation will wait for all read operations to finish.

  The steps:

  1. Lock the mutex and increment writers to indicate that a write is pending.
  2. Wait for all readers to finish.
  3. Proceed with write. Other writes cannot proceed until the mutex is unlocked.
  4. Decrement writers to indicate that write is complete.
  5. Signal other threads.
  6. Unlock mutex.

  @param[in]
  root The root node structure.

  @param[in]
  func The function to call. The first argument will be the node followed by any
  additional arguments.

  @param[in]
  ... Additional arguments to pass to the function.
*/
#undef RBT_NODE_ROOT_WRITE
#define RBT_NODE_ROOT_WRITE(root, func, ...) \
pthread_mutex_lock(&(root.mutex)); \
root.writers ++; \
while (root.readers) \
{ \
  pthread_cond_wait(&(root.cond), &(root.mutex)); \
} \
func(root.node, ##__VA_ARGS__); \
root.writers --; \
pthread_cond_signal(&(root.cond)); \
pthread_mutex_unlock(&(root.mutex))



#ifdef RBT_NODE_CACHE_SIZE
/*!
  @cond INTERNAL
*/
#undef RBT_NODE_CACHE_MUTEX
#define RBT_NODE_CACHE_MUTEX RBT_TOKEN_2_W(RBT_NODE_H_PREFIX_, node_cache_mutex)
/*!
  @endcond
*/

/*!
  The mutex used to lock the node cache.
*/
pthread_mutex_t RBT_NODE_CACHE_MUTEX = PTHREAD_MUTEX_INITIALIZER;

/*!
  Lock the node cache.
*/
#undef RBT_NODE_CACHE_LOCK
#define RBT_NODE_CACHE_LOCK pthread_mutex_lock(&RBT_NODE_CACHE_MUTEX)

/*!
  Unlock the node cache.
*/
#undef RBT_NODE_CACHE_UNLOCK
#define RBT_NODE_CACHE_UNLOCK pthread_mutex_unlock(&RBT_NODE_CACHE_MUTEX)

#endif //RBT_NODE_CACHE_SIZE
