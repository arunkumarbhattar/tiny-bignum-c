/*

Big number library - arithmetic on multiple-precision unsigned integers.

This library is an implementation of arithmetic on arbitrarily large integers.

The difference between this and other implementations, is that the data structure
has optimal memory utilization (i.e. a 1024 bit integer takes up 128 bytes RAM),
and all memory is allocated statically: no dynamic allocation for better or worse.

Primary goals are correctness, clarity of code and clean, portable implementation.
Secondary goal is a memory footprint small enough to make it suitable for use in
embedded applications.


The current state is correct functionality and adequate performance.
There may well be room for performance-optimizations and improvements.

*/

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include "bn.h"



/* Functions for shifting number in-place. */
static void _lshift_one_bit(_TPtr<_T_bn> a);
static void _rshift_one_bit(_TPtr<_T_bn> a);
static void _lshift_word(_TPtr<_T_bn> a, int nwords);
static void _rshift_word(_TPtr<_T_bn> a, int nwords);



/* Public / Exported functions. */
void bignum_init(_TPtr<_T_bn> n)
{
    if (n == NULL) {
        n = (_TPtr<_T_bn>)t_malloc(sizeof(_T_bn));
    }
  require(n, "n is null");
  //sometimes you may  pass the same structure to bignum_init
  //hence check for null before you init
  if (n->array == NULL)
    n->array = (_TPtr<DTYPE>)t_malloc(BN_ARRAY_SIZE * sizeof(DTYPE));
  int i;
  for (i = 0; i < BN_ARRAY_SIZE; ++i)
  {
    n->array[i] = 0;
  }
}


void bignum_from_int(_TPtr<_T_bn> n, DTYPE_TMP i)
{
  require(n, "n is null");

  bignum_init(n);

  /* Endianness issue if machine is not little-endian? */
#ifdef WORD_SIZE
 #if (WORD_SIZE == 1)
  n->array[0] = (i & 0x000000ff);
  n->array[1] = (i & 0x0000ff00) >> 8;
  n->array[2] = (i & 0x00ff0000) >> 16;
  n->array[3] = (i & 0xff000000) >> 24;
 #elif (WORD_SIZE == 2)
  n->array[0] = (i & 0x0000ffff);
  n->array[1] = (i & 0xffff0000) >> 16;
 #elif (WORD_SIZE == 4)
  n->array[0] = i;
  DTYPE_TMP num_32 = 32;
  DTYPE_TMP tmp = i >> num_32; /* bit-shift with U64 operands to force 64-bit results */
  n->array[1] = tmp;
 #endif
#endif
}


int bignum_to_int(_TPtr<_T_bn> n)
{
  require(n, "n is null");

  int ret = 0;

  /* Endianness issue if machine is not little-endian? */
#if (WORD_SIZE == 1)
  ret += n->array[0];
  ret += n->array[1] << 8;
  ret += n->array[2] << 16;
  ret += n->array[3] << 24;  
#elif (WORD_SIZE == 2)
  ret += n->array[0];
  ret += n->array[1] << 16;
#elif (WORD_SIZE == 4)
  ret += n->array[0];
#endif

  return ret;
}


void bignum_from_string(_TPtr<_T_bn> n, char* str, int nbytes)
{
  require(n, "n is null");
  require(str, "str is null");
  require(nbytes > 0, "nbytes must be positive");
  require((nbytes & 1) == 0, "string format must be in hex -> equal number of bytes");
  require((nbytes % (sizeof(DTYPE) * 2)) == 0, "string length must be a multiple of (sizeof(DTYPE) * 2) characters");
  
  bignum_init(n);

  DTYPE tmp;                        /* DTYPE is defined in bn.h - uint{8,16,32,64}_t */
  int i = nbytes - (2 * WORD_SIZE); /* index into string */
  int j = 0;                        /* index into array */

  /* reading last hex-byte "MSB" from string first -> big endian */
  /* MSB ~= most significant byte / block ? :) */
  while (i >= 0)
  {
    tmp = 0;
    sscanf(&str[i], SSCANF_FORMAT_STR, &tmp);
    n->array[j] = tmp;
    i -= (2 * WORD_SIZE); /* step WORD_SIZE hex-byte(s) back in the string. */
    j += 1;               /* step one element forward in the array. */
  }
}

//we're finna gon move this to the sandbox

_Tainted void bignum_to_string(_TPtr<_T_bn> n, _TPtr<char> str, int nbytes)
{
    return w2c_bignum_to_string(c_fetch_sandbox_address(), (int)n, (int)str, nbytes);
}


void bignum_dec(_TPtr<_T_bn> n)
{
  require(n, "n is null");

  DTYPE tmp; /* copy of n */
  DTYPE res;

  int i;
  for (i = 0; i < BN_ARRAY_SIZE; ++i)
  {
    tmp = n->array[i];
    res = tmp - 1;
    n->array[i] = res;

    if (!(res > tmp))
    {
      break;
    }
  }
}


void bignum_inc(_TPtr<_T_bn> n)
{
  require(n, "n is null");

  DTYPE res;
  DTYPE_TMP tmp; /* copy of n */

  int i;
  for (i = 0; i < BN_ARRAY_SIZE; ++i)
  {
    tmp = n->array[i];
    res = tmp + 1;
    n->array[i] = res;

    if (res > tmp)
    {
      break;
    }
  }
}


void bignum_add(_TPtr<_T_bn> a, _TPtr<_T_bn> b, _TPtr<_T_bn> c)
{
  require(a, "a is null");
  require(b, "b is null");
  require(c, "c is null");

  DTYPE_TMP tmp;
  int carry = 0;
  int i;
  for (i = 0; i < BN_ARRAY_SIZE; ++i)
  {
    tmp = (DTYPE_TMP)a->array[i] + b->array[i] + carry;
    carry = (tmp > MAX_VAL);
    c->array[i] = (tmp & MAX_VAL);
  }
}


void bignum_sub(_TPtr<_T_bn> a, _TPtr<_T_bn> b, _TPtr<_T_bn> c)
{
  require(a, "a is null");
  require(b, "b is null");
  require(c, "c is null");

  DTYPE_TMP res;
  DTYPE_TMP tmp1;
  DTYPE_TMP tmp2;
  int borrow = 0;
  int i;
  for (i = 0; i < BN_ARRAY_SIZE; ++i)
  {
    tmp1 = (DTYPE_TMP)a->array[i] + (MAX_VAL + 1); /* + number_base */
    tmp2 = (DTYPE_TMP)b->array[i] + borrow;;
    res = (tmp1 - tmp2);
    c->array[i] = (DTYPE)(res & MAX_VAL); /* "modulo number_base" == "% (number_base - 1)" if number_base is 2^N */
    borrow = (res <= MAX_VAL);
  }
}


void bignum_mul(_TPtr<_T_bn> a, _TPtr<_T_bn> b, _TPtr<_T_bn> c)
{
  require(a, "a is null");
  require(b, "b is null");
  require(c, "c is null");

  _TPtr<_T_bn> row = NULL; //moving from static memory to heap --> SLOW!
  _TPtr<_T_bn> tmp = NULL;
  row = (_TPtr<_T_bn>)t_malloc(sizeof(_T_bn));
  tmp = (_TPtr<_T_bn>)t_malloc(sizeof(_T_bn));
  //memset these both to 0
  t_memset(row, 0, sizeof(_T_bn));
  t_memset(tmp, 0, sizeof(_T_bn));

  int i, j;

  bignum_init(c);

  for (i = 0; i < BN_ARRAY_SIZE; ++i)
  {
    bignum_init(row);

    for (j = 0; j < BN_ARRAY_SIZE; ++j)
    {
      if (i + j < BN_ARRAY_SIZE)
      {
        bignum_init(tmp);
        DTYPE_TMP intermediate = ((DTYPE_TMP)a->array[i] * (DTYPE_TMP)b->array[j]);
        bignum_from_int(tmp, intermediate);
        _lshift_word(tmp, i + j);
        bignum_add(tmp, row, row);
      }
    }
    bignum_add(c, row, c);
  }
  t_free(row->array);
  t_free(tmp->array);
  t_free(row);
  t_free(tmp);
}


void bignum_div(_TPtr<_T_bn> a, _TPtr<_T_bn> b, _TPtr<_T_bn> c)
{
  require(a, "a is null");
  require(b, "b is null");
  require(c, "c is null");

  _TPtr<_T_bn> current = NULL;
  _TPtr<_T_bn> denom = NULL;
  _TPtr<_T_bn> tmp = NULL;

  current = (_TPtr<_T_bn>)t_malloc(sizeof(_T_bn));
  denom = (_TPtr<_T_bn>)t_malloc(sizeof(_T_bn));
  tmp = (_TPtr<_T_bn>)t_malloc(sizeof(_T_bn));
  //memset these to zero
  t_memset(current, 0, sizeof(_T_bn));
  t_memset(denom, 0, sizeof(_T_bn));
  t_memset(tmp, 0, sizeof(_T_bn));
  bignum_from_int(current, 1);               // int current = 1;
  bignum_assign(denom, b);                   // denom = b
  bignum_assign(tmp, a);                     // tmp   = a


  const DTYPE_TMP half_max = 1 + (DTYPE_TMP)(MAX_VAL / 2);
  bool overflow = false;
  while (bignum_cmp(denom, a) != LARGER)     // while (denom <= a) {
  {
    if (denom->array[BN_ARRAY_SIZE - 1] >= half_max)
    {
      overflow = true;
      break;
    }
    _lshift_one_bit(current);                //   current <<= 1;
    _lshift_one_bit(denom);                  //   denom <<= 1;
  }
  if (!overflow)
  {
    _rshift_one_bit(denom);                  // denom >>= 1;
    _rshift_one_bit(current);                // current >>= 1;
  }
  bignum_init(c);                             // int answer = 0;

  while (!bignum_is_zero(current))           // while (current != 0)
  {
    if (bignum_cmp(tmp, denom) != SMALLER)  //   if (dividend >= denom)
    {
      bignum_sub(tmp, denom, tmp);         //     dividend -= denom;
      bignum_or(c, current, c);              //     answer |= current;
    }
    _rshift_one_bit(current);                //   current >>= 1;
    _rshift_one_bit(denom);                  //   denom >>= 1;
  }
  t_free(current->array); t_free(denom->array);t_free(tmp->array);
  t_free(current);
  t_free(denom);
  t_free(tmp);
}


void bignum_lshift(_TPtr<_T_bn> a, _TPtr<_T_bn> b, int nbits)
{
  require(a, "a is null");
  require(b, "b is null");
  require(nbits >= 0, "no negative shifts");

  bignum_assign(b, a);
  /* Handle shift in multiples of word-size */
  const int nbits_pr_word = (WORD_SIZE * 8);
  int nwords = nbits / nbits_pr_word;
  if (nwords != 0)
  {
    _lshift_word(b, nwords);
    nbits -= (nwords * nbits_pr_word);
  }

  if (nbits != 0)
  {
    int i;
    for (i = (BN_ARRAY_SIZE - 1); i > 0; --i)
    {
      b->array[i] = (b->array[i] << nbits) | (b->array[i - 1] >> ((8 * WORD_SIZE) - nbits));
    }
    b->array[i] <<= nbits;
  }
}


void bignum_rshift(_TPtr<_T_bn> a, _TPtr<_T_bn> b, int nbits)
{
  require(a, "a is null");
  require(b, "b is null");
  require(nbits >= 0, "no negative shifts");
  
  bignum_assign(b, a);
  /* Handle shift in multiples of word-size */
  const int nbits_pr_word = (WORD_SIZE * 8);
  int nwords = nbits / nbits_pr_word;
  if (nwords != 0)
  {
    _rshift_word(b, nwords);
    nbits -= (nwords * nbits_pr_word);
  }

  if (nbits != 0)
  {
    int i;
    for (i = 0; i < (BN_ARRAY_SIZE - 1); ++i)
    {
      b->array[i] = (b->array[i] >> nbits) | (b->array[i + 1] << ((8 * WORD_SIZE) - nbits));
    }
    b->array[i] >>= nbits;
  }
  
}


void bignum_mod(_TPtr<_T_bn> a, _TPtr<_T_bn> b, _TPtr<_T_bn> c)
{
  /*
    Take divmod and throw away div part
  */
  require(a, "a is null");
  require(b, "b is null");
  require(c, "c is null");

  _TPtr<_T_bn> tmp = NULL;

  tmp = (_TPtr<_T_bn>)t_malloc(sizeof(_T_bn));
    //memset this to zero
    t_memset(tmp, 0, sizeof(_T_bn));
  bignum_divmod(a,b,tmp,c);
    t_free(tmp->array);
  t_free(tmp);
}

void bignum_divmod(_TPtr<_T_bn> a, _TPtr<_T_bn> b, _TPtr<_T_bn> c, _TPtr<_T_bn> d)
{
  /*
    Puts a%b in d
    and a/b in c

    mod(a,b) = a - ((a / b) * b)

    example:
      mod(8, 3) = 8 - ((8 / 3) * 3) = 2
  */
  require(a, "a is null");
  require(b, "b is null");
  require(c, "c is null");

  _TPtr<_T_bn> tmp = (_TPtr<_T_bn>)t_malloc(sizeof(_T_bn));
  //memset this to 0
  t_memset(tmp, 0, sizeof(_T_bn));
  /* c = (a / b) */
  bignum_div(a, b, c);

  /* tmp = (c * b) */
  bignum_mul(c, b, tmp);

  /* c = a - tmp */
  bignum_sub(a, tmp, d);

        t_free(tmp->array);
  t_free(tmp);
}


void bignum_and(_TPtr<_T_bn> a, _TPtr<_T_bn> b, _TPtr<_T_bn> c)
{
  require(a, "a is null");
  require(b, "b is null");
  require(c, "c is null");

  int i;
  for (i = 0; i < BN_ARRAY_SIZE; ++i)
  {
    c->array[i] = (a->array[i] & b->array[i]);
  }
}


void bignum_or(_TPtr<_T_bn> a, _TPtr<_T_bn> b, _TPtr<_T_bn> c)
{
  require(a, "a is null");
  require(b, "b is null");
  require(c, "c is null");

  int i;
  for (i = 0; i < BN_ARRAY_SIZE; ++i)
  {
    c->array[i] = (a->array[i] | b->array[i]);
  }
}


void bignum_xor(_TPtr<_T_bn> a, _TPtr<_T_bn> b, _TPtr<_T_bn> c)
{
  require(a, "a is null");
  require(b, "b is null");
  require(c, "c is null");

  int i;
  for (i = 0; i < BN_ARRAY_SIZE; ++i)
  {
    c->array[i] = (a->array[i] ^ b->array[i]);
  }
}


int bignum_cmp(_TPtr<_T_bn> a, _TPtr<_T_bn> b)
{
  require(a, "a is null");
  require(b, "b is null");

  //printf("Re-entering function\n");
  int i = BN_ARRAY_SIZE;
  do
  {
      //print value of i
    //t_printf("i = %d, a->array[i] = %d, b->array[i] = %d\n", i, a->array[i], b->array[i]);
    i -= 1; /* Decrement first, to start with last array element */
    if (a->array[i] > b->array[i])
    {
      return LARGER;
    }
    else if (a->array[i] < b->array[i])
    {
      return SMALLER;
    }
  }
  while (i != 0);

  return EQUAL;
}


int bignum_is_zero(_TPtr<_T_bn> n)
{
  require(n, "n is null");

  int i;
  for (i = 0; i < BN_ARRAY_SIZE; ++i)
  {
    if (n->array[i])
    {
      return 0;
    }
  }

  return 1;
}


void bignum_pow(_TPtr<_T_bn> a, _TPtr<_T_bn> b, _TPtr<_T_bn> c)
{
  require(a, "a is null");
  require(b, "b is null");
  require(c, "c is null");

  _TPtr<_T_bn> tmp = NULL;
  tmp = (_TPtr<_T_bn>)t_malloc(sizeof(_T_bn));
    //memset this to 0
    t_memset(tmp, 0, sizeof(_T_bn));
  bignum_init(c);

  if (bignum_cmp(b, c) == EQUAL)
  {
    /* Return 1 when exponent is 0 -- n^0 = 1 */
    bignum_inc(c);
  }
  else
  {
    _TPtr<_T_bn> bcopy = (_TPtr<_T_bn>)t_malloc(sizeof(_T_bn));
    //memset this to 0
    t_memset(bcopy, 0, sizeof(_T_bn));
    bignum_assign(bcopy, b);

    /* Copy a -> tmp */
    bignum_assign(tmp, a);

    bignum_dec(bcopy);
 
    /* Begin summing products: */
    while (!bignum_is_zero(bcopy))
    {

      /* c = tmp * tmp */
      bignum_mul(tmp, a, c);
      /* Decrement b by one */
      bignum_dec(bcopy);

      bignum_assign(tmp, c);
    }

    /* c = tmp */
    bignum_assign(c, tmp);
        t_free(bcopy->array);
        t_free(tmp->array);
    t_free(bcopy);
    t_free(tmp);
  }
}

void bignum_isqrt(_TPtr<_T_bn> a, _TPtr<_T_bn> b)
{
  require(a, "a is null");
  require(b, "b is null");

  _TPtr<_T_bn> low = (_TPtr<_T_bn>)t_malloc(sizeof(_T_bn));
  _TPtr<_T_bn> high = (_TPtr<_T_bn>)t_malloc(sizeof(_T_bn));
  _TPtr<_T_bn> mid = (_TPtr<_T_bn>)t_malloc(sizeof(_T_bn));
  _TPtr<_T_bn> tmp =  (_TPtr<_T_bn>)t_malloc(sizeof(_T_bn));
//memset these to 0
t_memset(low, 0, sizeof(_T_bn));
t_memset(high, 0, sizeof(_T_bn));
t_memset(mid, 0, sizeof(_T_bn));
t_memset(tmp, 0, sizeof(_T_bn));

  bignum_init(low);
  bignum_assign(high, a);
  bignum_rshift(high, mid, 1);
  bignum_inc(mid);

  while (bignum_cmp(high, low) > 0)
  {
    bignum_mul(mid, mid, tmp);
    if (bignum_cmp(tmp, a) > 0)
    {
      bignum_assign(high, mid);
      bignum_dec(high);
    }
    else 
    {
      bignum_assign(low, mid);
    }
    bignum_sub(high,low,mid);
    _rshift_one_bit(mid);
    bignum_add(low,mid,mid);
    bignum_inc(mid);
  }
  bignum_assign(b,low);
  t_free(low->array);
    t_free(high->array);
    t_free(mid->array);
    t_free(tmp->array);
  t_free(low); t_free(high);t_free(mid);t_free(tmp);

}


void bignum_assign(_TPtr<_T_bn> dst, _TPtr<_T_bn> src)
{
  require(dst, "dst is null");
  require(src, "src is null");

  //since we are dealing with heap pointer as member instead of array, we need to check for NULL and allocate
  if (dst->array == NULL)
  {
      dst->array = (_TPtr<uint32_t>)t_malloc(BN_ARRAY_SIZE*sizeof(uint32_t));
  }
  int i;
  for (i = 0; i < BN_ARRAY_SIZE; ++i)
  {
    dst->array[i] = src->array[i];
  }
}


/* Private / Static functions. */
static void _rshift_word(_TPtr<_T_bn> a, int nwords)
{
  /* Naive method: */
  require(a, "a is null");
  require(nwords >= 0, "no negative shifts");

  int i;
  if (nwords >= BN_ARRAY_SIZE)
  {
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
      a->array[i] = 0;
    }
    return;
  }

  for (i = 0; i < BN_ARRAY_SIZE - nwords; ++i)
  {
    a->array[i] = a->array[i + nwords];
  }
  for (; i < BN_ARRAY_SIZE; ++i)
  {
    a->array[i] = 0;
  }
}


static void _lshift_word(_TPtr<_T_bn> a, int nwords)
{
  require(a, "a is null");
  require(nwords >= 0, "no negative shifts");

  int i;
  /* Shift whole words */
  for (i = (BN_ARRAY_SIZE - 1); i >= nwords; --i)
  {
    a->array[i] = a->array[i - nwords];
  }
  /* Zero pad shifted words. */
  for (; i >= 0; --i)
  {
    a->array[i] = 0;
  }  
}


static void _lshift_one_bit(_TPtr<_T_bn> a)
{
  require(a, "a is null");

  int i;
  for (i = (BN_ARRAY_SIZE - 1); i > 0; --i)
  {
    a->array[i] = (a->array[i] << 1) | (a->array[i - 1] >> ((8 * WORD_SIZE) - 1));
  }
  a->array[0] <<= 1;
}


static void _rshift_one_bit(_TPtr<_T_bn> a)
{
  require(a, "a is null");

  int i;
  for (i = 0; i < (BN_ARRAY_SIZE - 1); ++i)
  {
    a->array[i] = (a->array[i] >> 1) | (a->array[i + 1] << ((8 * WORD_SIZE) - 1));
  }
  a->array[BN_ARRAY_SIZE - 1] >>= 1;
}


