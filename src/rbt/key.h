/*!
  @file
  @author Xyne
  @copyright GPL2 only

  # Required Macro Definitions:
  - RBT_KEY_H_PREFIX_

    The prefix for visible variable and function declarations in this header.

  - RBT_PIN_T

    The type of array element used to hold the key bits interally. This is
    independent of the actual key type. It must be an unsigned integer type.
    Smaller types use space more efficiently but they are not always the fastest
    type. For example, GCC supports built-in functions (`__builtin_clz()` etc.)
    for some larger integer types but not for 8-bit integer types. These
    functions can have a significant effect on tree traversal.

    Note that the pin type is independent of the key type and size.

  - RBT_KEY_SIZE_T

    An unsigned integer type large enough to count the bits in all possible
    keys. Let \f$n\f$ be the (maximum) number of *bytes* in a key and let \f$b\f$
    be the number of bits per byte. The maximum number of bits in a key is then
    \f$n * b\f$. The key size type must be an unsigned integer type that can
    store this number.

    Let \f$s\f$ be the size in bytes of our key size type. The maximum value
    that this type can represent is \f$2^{s*b}-1\f$ and thus the necessary
    condition is \f$2^{s*b}-1 \ge n*b\f$

    The minimum required size in bytes of the key size type is therefore
    \f[
      s_\textup{min} \ge \frac {\log_{2}(n*b + 1)} b
    \f]

    of, if you prefer thinking in bits,
    \f[
      S_\textup{min} \ge \log_{2}(N + 1)
    \f]
    where \f$S\f$ is the size of the key size type in bits and \f$N\f$ is the
    maximum number of bits in the key.

    For fixed-width types, \f$n\f$ will be equal to the value returned by
    `sizeof`. For variable width types, an upper limit will have to be
    anticipated.

  - RBT_KEY_SIZE_T_FORMAT

    The printf format parameter for RBT_KEY_SIZE_T, e.g. "u". When using integer
    formats from stdint.h, use the matching parameters from inttypes.h.



  # Examples
  ## Fixed Width 128 Bit Keys
  `uint_fast32_t` will make use of GCC's `__builtin_clz()`. The minimum required
  size of the key size type is 7.011..., so an 8-bit type will suffice.

      #define RBT_PIN_T uint_fast32_t
      #define RBT_KEY_SIZE_T uint_fast8_t

  ## Variable Width String Keys
  Again, `uint_fast_32_t is used for the reason given above. If string keys
  would not exceed 10 characters then `uing_fast8_t` would suffice as the key
  size type, but 10 characters is very restrictive. By using `uint_fast32_t`,
  keys up to 100000000 characters can be used.

      #define RBT_PIN_T uint_fast32_t
      #define RBT_KEY_SIZE_T uint_fast32_t

*/

#include <stdio.h>

#include "common.h"
#include "debug.h"

/*!
  @cond INTERNAL
*/

#ifndef RBT_KEY_H_PREFIX_
#error RBT_KEY_H_PREFIX_ is not defined.
#endif // RBT_KEY_H_PREFIX_

#ifndef RBT_PIN_T
#error RBT_PIN_T is not defined.
#endif // RBT_PIN_T

#ifndef RBT_KEY_SIZE_T
#error RBT_KEY_SIZE_T is not defined.
#endif // RBT_KEY_SIZE_T

#ifndef RBT_KEY_SIZE_T_FORMAT
#error RBT_KEY_SIZE_T_FORMAT is not defined.
#endif // RBT_KEY_SIZE_T_FORMAT


/*
  Typedef these so that they are not lost if the preceding macros are redefined
  for another inclusion.
*/
typedef RBT_PIN_T      RBT_TOKEN_2_W(RBT_KEY_H_PREFIX_, pin_t);
typedef RBT_KEY_SIZE_T RBT_TOKEN_2_W(RBT_KEY_H_PREFIX_, key_size_t);

#undef RBT_FPRINT_BITS
#define RBT_FPRINT_BITS           RBT_TOKEN_2_W(RBT_KEY_H_PREFIX_, fprint_bits)

#undef RBT_COMMON_BIT_PREFIX_LEN
#define RBT_COMMON_BIT_PREFIX_LEN RBT_TOKEN_2_W(RBT_KEY_H_PREFIX_, common_bit_prefix_len)
/*!
  @endcond
*/


/*
  The conditional definitions may be unnecessary if multiplication by 1 would be
  optimized by the compiler, but it does not hurt and it assures that the more
  efficient method is used.
*/
/*!
  The number of bytes in the pin type.
*/
#undef RBT_PIN_SIZE
#define RBT_PIN_SIZE sizeof(RBT_PIN_T)
/*!
  The number of bits in the pin type.
*/
#undef RBT_PIN_SIZE_BITS
#define RBT_PIN_SIZE_BITS (RBT_PIN_SIZE * BITS_PER_BYTE)

/*!
  Calculate the number of pins required to hold a given number of bits.

  @param[in]
  x The number of bits.
*/
#undef BITS_TO_PINS
#define BITS_TO_PINS(x) DIV_UP(x, RBT_PIN_SIZE_BITS)

/*!
  Calculate the number of bytes required to hold a given number of pins.

  @param[in]
  x The number of pins.
*/
#undef PINS_TO_BYTES
#define PINS_TO_BYTES(x) ((x) * RBT_PIN_SIZE)

/*!
  Calculate the number of bytes required to hold a number of bits using an array
  of pins. This will differ from the number of bytes required to hold the bits
  directly if the number of bits is not an integral multiple of the pin size.

  @param[in]
  x The number of bits.
*/
#undef BITS_TO_PINS_TO_BYTES
#define BITS_TO_PINS_TO_BYTES(x) PINS_TO_BYTES(BITS_TO_PINS(x))









/*!
  Print a string representation of the bits in a key.

  @param[in]
  fd The output file descriptor.

  @param[in]
  key The key to print.

  @param[in]
  n The number of bits to print.

  @param[in]
  skip The number of bits from the start to skip.
*/

void
RBT_FPRINT_BITS(FILE * fd, RBT_PIN_T * key, RBT_KEY_SIZE_T n, RBT_KEY_SIZE_T skip)
{
  int i;
  for (i = skip; n > 0; i++,n--)
  {
    while (i >= RBT_PIN_SIZE_BITS)
    {
      i -= RBT_PIN_SIZE_BITS;
      key ++;
    }
    fprintf(fd, "%d", (MOST_SIGNIFICANT_BIT(typeof(key[0])) & (key[0] << i)) > 0);
  }
}





/*!
  Calculate the quotient and remainder of integer division. This should be
  optimized by the compiler to a single division operation.

  @param[in]
  a The dividend.

  @param[in]
  b The divisor.

  @param[out]
  c The quotient.

  @param[out]
  d The remainder.
*/
#undef RBT_DIVMOD
#define RBT_DIVMOD(a,b,c,d) \
c = a / b; \
d = a % b




/*!
  Determine the number of common bits at the beginning of two keys.

  @param[in]
  a The first key.

  @param[in]
  b The second key.

  @param[in]
  max The maximum number of bits to check.

  @return
  The number of common bits.

  @todo
  Investigate ways to optimize this (e.g. asm).
*/
RBT_KEY_SIZE_T
RBT_COMMON_BIT_PREFIX_LEN(
  RBT_PIN_T * a,
  RBT_PIN_T * b,
  RBT_KEY_SIZE_T max
)
{
  RBT_PIN_T c;
  RBT_KEY_SIZE_T length;

  length = 0;

  while (length < max && * a == * b)
  {
    length += RBT_PIN_SIZE_BITS;
    a ++;
    b ++;
  }
  if (length < max)
  {
    c = *a ^ *b;
#ifdef __GNUC__
    /*
      Make use of the GCC builtins:
      http://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html

      TODO
      At some point check if this is even worth it. Also consider checking
      compatibility of the returned int with RBT_KEY_SIZE_T. Under normal operation
      RBT_KEY_SIZE_T will be able to hold the value, but there may be overhead due
      to conversion between types.
    */
    if (__builtin_types_compatible_p(RBT_PIN_T, unsigned int))
    {
      debug_print("__builtin_clz\n");
      length += (RBT_KEY_SIZE_T) __builtin_clz(c);
    }
    else if (__builtin_types_compatible_p(RBT_PIN_T, unsigned long int))
    {
      debug_print("__builtin_clzl\n");
      length += (RBT_KEY_SIZE_T) __builtin_clzl(c);
    }
    else if (__builtin_types_compatible_p(RBT_PIN_T, unsigned long long int))
    {
      debug_print("__builtin_clzll\n");
      length += (RBT_KEY_SIZE_T) __builtin_clzll(c);
    }
    else
    {
#endif // __GNUC__
//       debug_print("loop\n");
    /*
      The condition above ensures that the length is less than the max, so it
      cannot be exceeded here and there is no need to check before returning it.

      The leading 0's represent common bits due to the XOR operation above. The
      previous loop ensures that *a and *b are not identical so there must be
      at least one bit in c. By shifting it left and checking if the value is
      less than MOST_SIGNIFICANT_BIT we can count the leading zeros and thus the
      number of common bits.
    */
    while (c < MOST_SIGNIFICANT_BIT_W(RBT_PIN_T) && length < max)
    {
      c <<= 1;
      length += 1;
    }
    return length;
#ifdef __GNUC__
    }
    return (length > max) ? max : length;
#endif // __GNUC__
  }
  else
  {
    return max;
  }
}
