#include <stdio.h>
#include <time.h>
#include "bn.h"


int npassed = 0;
int ntests = 0;


static void test_evil(void)
{
  ntests += 1;
  /*
    Evil test case triggering infinite-loop - reported by jepler@github
    https://github.com/kokke/tiny-bignum-c/issues/2
  */
  {
    _TPtr<_T_bn> a = NULL, b = NULL , c = NULL;
    a = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
    b = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
    c = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));

    bignum_from_int(a, 1);
    bignum_init(b); bignum_dec(b); // b now holds biggest bignum
    bignum_div(b, a, c);
    __free__(a); __free__(b); __free__(c);
  }
  /* test passed if it doesn't go into infinite-loop... */
  npassed += 1;
}


static void test_over_and_underflow(void)
{
  ntests += 1;
  /*
    Test case triggering overflow-bug - reported by XYlearn@github
    https://github.com/kokke/tiny-bignum-c/issues/3
  */
  {
    _TPtr<_T_bn> a = NULL;
    _TPtr<_T_bn> b = NULL;
    _TPtr<_T_bn> c = NULL;

    a = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
    b = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
    c = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));

    bignum_from_int(a, 0);
    bignum_from_int(b, 1);
    bignum_sub(a, b, a);
    bignum_from_int(b, 3);
    bignum_add(a, b, a);
    bignum_from_int(c, 2);
    assert(bignum_cmp(a, c) == EQUAL);
    __free__(a); __free__(b); __free__(c);
  }
  /* test passed if assertion doesn't fail. */
  npassed += 1;
}

static void test_rshift_largish_number(void)
{
  ntests += 1;
  /*
    Test case triggering rshift-bug - reported by serpilliere
    https://github.com/kokke/tiny-bignum-c/pull/7
  */
  {
    _TPtr<_T_bn> n1 = NULL, n2 = NULL, n3 = NULL;
    n1 = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
    t_memset(n1, 0, sizeof(_T_bn));
    n2 = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
    t_memset(n2, 0, sizeof(_T_bn));
    n3 = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
    t_memset(n3, 0, sizeof(_T_bn));

    bignum_from_string(n1, "11112222333344445555666677778888", 32);
    bignum_from_string(n3, "1111222233334444", 16);
    bignum_rshift(n1, n2, 64);

    /* Check that (0x11112222333344445555666677778888 >> 64) == 0x1111222233334444 */
    assert(bignum_cmp(n2, n3) == EQUAL);
    __free__(n1);__free__(n2);__free__(n3);
  }
  /* test passed if assertion doesn't fail. */
  npassed += 1;
}



int main()
{
  printf("\nRunning hand-picked test cases:\n");
  clock_t start, end;
  double cpu_time_used;
  start = clock();
  test_evil();
 //test_over_and_underflow();
  test_rshift_largish_number();
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("\n%d/%d tests successful.\n --> Taking total time %f", npassed, ntests, cpu_time_used);
  printf("\n");

  return (ntests - npassed); /* 0 if all tests passed */
}

