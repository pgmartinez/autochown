/*!
  @file
  @author Xyne
  @copyright GPL2 only
*/

#ifndef RBT_HEADER_DEBUG
#define RBT_HEADER_DEBUG

#ifndef RBT_DEBUG_USE_COLOR
#ifdef RBT_USE_COLOR
#define RBT_DEBUG_USE_COLOR 1
#endif //RBT_USE_COLOR
#endif //RBT_DEBUG_USE_COLOR

/*!
  Enable debugging messages if non-zero.
*/
#ifndef RBT_DEBUG
#define RBT_DEBUG 0
#endif //RBT_DEBUG

/*!
  The file descriptor to use for printing debugging messages.
*/
#ifndef RBT_DEBUG_FD
#define RBT_DEBUG_FD stderr
#endif //RBT_DEBUG_FD

/*!
  Use indentation to represent the nesting level of function calls.
*/
#ifndef RBT_DEBUG_INDENT
#define RBT_DEBUG_INDENT 0
#endif //RBT_DEBUG_INDENT

// Nested debug messages to facilitate tracing.
#if RBT_DEBUG_INDENT>0
/*!
  Track the nesting level of function calls.
*/
unsigned int RBT_NESTING_LEVEL = 0;
#ifdef __GNUC__
void
__cyg_profile_func_enter(void * this_fn, void * call_site)
{
  RBT_NESTING_LEVEL += RBT_DEBUG_INDENT;
}

void
__cyg_profile_func_exit(void * this_fn, void * call_site)
{
  RBT_NESTING_LEVEL -= RBT_DEBUG_INDENT;
}
#else
#error RBT_DEBUG_INDENT is currently only supported when using GCC.
#endif //__GNUC__
#endif //RBT_DEBUG_INDENT>0

#ifdef RBT_DEBUG_USE_COLOR
/*!
  The color used for printing file names.
*/
#define RBT_DEBUG_FILENAME_COLOR "\033[34m"
/*!
  The color used for printing line numbers.
*/
#define RBT_DEBUG_LINENUMBER_COLOR "\033[36m"
/*!
  The color used for printing function names.
*/
#define RBT_DEBUG_FUNCTIONNAME_COLOR "\033[35m"
/*!
  The sequence for resetting colors.
*/
#define RBT_DEBUG_RESET_COLOR "\033[0m"
#else
/*!
  @cond INTERNAL
*/
#define RBT_DEBUG_FILENAME_COLOR ""
#define RBT_DEBUG_LINENUMBER_COLOR ""
#define RBT_DEBUG_FUNCTIONNAME_COLOR ""
#define RBT_DEBUG_RESET_COLOR ""
/*!
  @endcond
*/
#endif //RBT_DEBUG_USE_COLOR



/*!
  Print the file name, line number and function name.
*/
#define debug_print_prefix_flat \
  fprintf(\
    RBT_DEBUG_FD, \
    "%s%s %s%d %s%s(): %s", \
    RBT_DEBUG_FILENAME_COLOR, \
    __FILE__, \
    RBT_DEBUG_LINENUMBER_COLOR, \
    __LINE__, \
    RBT_DEBUG_FUNCTIONNAME_COLOR, \
    __func__, \
    RBT_DEBUG_RESET_COLOR \
  )



/*!
  Precede `debug_print_prefix_flat` with indentation to represent the nesting
  level, if `RBT_DEBUG_INDENT` is greater than 0, otherwise this is the same
  as `debug_print_prefix_flat`.
*/
#if RBT_DEBUG_INDENT>0
#define debug_print_prefix \
  int i = RBT_NESTING_LEVEL; \
  while(i--) \
  { \
    fprintf(stderr, " "); \
  } \
  debug_print_prefix_flat
#else
#define debug_print_prefix debug_print_prefix_flat
#endif //RBT_DEBUG_INDENT>0



/*!
  Print a debugging message.

  @param
  fmt A `printf` format string.

  @param
  ... Parameters to pass to `printf`.
*/
#define debug_printf(fmt, ...) \
do \
{ \
  if (RBT_DEBUG) \
  { \
    debug_print_prefix; \
    fprintf(\
      RBT_DEBUG_FD, \
      fmt, \
      __VA_ARGS__ \
    ); \
  } \
} while (0)



/*!
  Print a debugging message.

  @param
  msg The string to print.
*/
#define debug_print(msg) \
do \
{ \
  if (RBT_DEBUG) \
  { \
    debug_print_prefix; \
    fprintf(\
      RBT_DEBUG_FD, \
      msg \
    ); \
  } \
} while (0)




/*!
  Print the debugging prefix then invoke a function to print the rest of the
  message.

  @param
  func A function that accepts a file descriptor as its first argument.

  @param
  print_newline A boolean parameter. If it evaluates to non-zero, a newline will
  be printed after the function call.

  @param
  ... Additional arguments to pass to the function.
*/
#define debug_print_func(func, print_newline, ...) \
do \
{ \
  if (RBT_DEBUG) \
  { \
    debug_print_prefix; \
    func(RBT_DEBUG_FD, ##__VA_ARGS__); \
    if (print_newline) \
    { \
      fprintf(RBT_DEBUG_FD, "\n"); \
    } \
  } \
} while (0)



/*!
  Identical to `debug_printf` except that it is independent of `RBT_DEBUG`.

  @param
  fmt A `printf` format string.

  @param
  ... Parameters to pass to `printf`.
*/
#define error_printf(fmt, ...) \
do \
{ \
  fprintf( \
    RBT_DEBUG_FD, \
    "%s %d %s(): " fmt, \
    __FILE__, __LINE__, __func__, fmt, __VA_ARGS__ \
  ); \
} \
while (0) \



/*!
  Identical to `debug_print` except that it is independent of `RBT_DEBUG`.

  @param
  msg The string to print.
*/
#define error_print(msg) \
do \
{ \
  fprintf( \
    RBT_DEBUG_FD, \
    "%s %d %s(): %s", \
    __FILE__, __LINE__, __func__, msg \
  ); \
} \
while (0) \



#endif // RBT_HEADER_DEBUG
