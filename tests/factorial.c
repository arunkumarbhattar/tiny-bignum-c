/*

    Testing Big-Number library by calculating factorial(100) a.k.a. 100!
    ====================================================================

    For the uninitiated:

        factorial(N) := N * (N-1) * (N-2) * ... * 1


    Example:

        factorial(5) = 5 * 4 * 3 * 2 * 1 = 120



    Validated by Python implementation of big-numbers:
    --------------------------------------------------

        In [1]: import math

        In [2]: "%x" % math.factorial(100)
        Out[]: '1b30964ec395dc24069528d54bbda40d16e966ef9a70eb21b5b2943a321cdf10391745570cca9420c6ecb3b72ed2ee8b02ea2735c61a000000000000000000000000'


    ... which should also be the result of this program's calculation


*/


#include <stdio.h>
#include "bn.h"
#include <time.h>


void factorial(_TPtr<_T_bn> n, _TPtr<_T_bn> res)
{
  _TPtr<_T_bn> tmp = NULL;
#ifndef NOOP_SBX
  tmp = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
  t_memset(tmp, 0, sizeof(_T_bn));
#else
  _T_bn _c_tmp;
  _T_bn _c_tmp_array[BN_ARRAY_SIZE];
#pragma TAINTED_SCOPE push
#pragma TAINTED_SCOPE on
    tmp = (_TPtr<_T_bn>)&_c_tmp;
    tmp->array = (_TPtr<DTYPE>)&_c_tmp_array;
#pragma TAINTED_SCOPE pop
#endif
  /* Copy n -> tmp */
  bignum_assign(tmp, n);

  /* Decrement n by one */
  bignum_dec(n);
  
  /* Begin summing products: */
  while (!bignum_is_zero(n))
  {
    /* res = tmp * n */
    bignum_mul(tmp, n, res);

    /* n -= 1 */
    bignum_dec(n);
    
    /* tmp = res */
    bignum_assign(tmp, res);
  }

  /* res = tmp */
  bignum_assign(res, tmp);
#ifndef NOOP_SBX
  __free__(tmp);
#endif
}


int main()
{
  _TPtr<_T_bn> num = NULL;
  _TPtr<_T_bn> result = NULL;
#ifndef NOOP_SBX
  num = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
  t_memset(num, 0, sizeof(_T_bn));
  result = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
  t_memset(result, 0, sizeof(_T_bn));
#else
  _T_bn _c_num;
  _T_bn _c_result;
  _T_bn _c_num_array[BN_ARRAY_SIZE];
  _T_bn _c_result_array[BN_ARRAY_SIZE];
#pragma TAINTED_SCOPE push
#pragma TAINTED_SCOPE on
    num = (_TPtr<_T_bn>)&_c_num;
    result = (_TPtr<_T_bn>)&_c_result;
    num->array = (_TPtr<DTYPE>)&_c_num_array;
    result->array = (_TPtr<DTYPE>)&_c_result_array;
#pragma TAINTED_SCOPE pop
#endif
  char buf[8192];
  clock_t start, end;
  double cpu_time_used = 0.0;
  _TPtr<char> _T_buf = StaticUncheckedToTStrAdaptor(buf, sizeof(buf));
    //perform this 10 times to get an average
  start = clock();
  for (int i = 0; i < 10; i++)
  {
      bignum_from_int(num, 100);
      factorial(num, result);
      bignum_to_string(result, _T_buf, sizeof(buf));
  }
  end = clock();
  t_printf("10x factorial(100) using bignum = %s\n: Experiment took time %f", _T_buf, ((double) (end - start)) / CLOCKS_PER_SEC);
#ifndef NOOP_SBX
  __free__(num);
  __free__(result);
  __free__(_T_buf);
#endif
  return 0;
}


