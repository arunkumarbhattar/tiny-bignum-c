/*
  Testing bignum_from_string and bignum_from_int 

  Also testing bignum_cmp

*/


#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "bn.h"
#include <time.h>


int main()
{
  char sabuf[8192];
  char sbbuf[8192];
  char scbuf[8192];
  char sdbuf[8192];
  char iabuf[8192];
  char ibbuf[8192];
  char icbuf[8192];
  char idbuf[8192];

  _TPtr<_T_bn> sa = NULL, sb = NULL, sc = NULL, sd = NULL, se = NULL;
  _TPtr<_T_bn> ia = NULL, ib = NULL, ic = NULL, id = NULL;

    sa = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
    sb = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
    sc = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
    sd = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
    se = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
    ia = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
    ib = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
    ic = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
    id = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
    clock_t start, end;
    double cpu_time_used;
    start = clock();
  bignum_init(sa);
  bignum_init(sb);
  bignum_init(sc);
  bignum_init(sd);
  bignum_init(ia);
  bignum_init(ib);
  bignum_init(ic);
  bignum_init(id);

  printf("\nLoading numbers from strings and from int.\n");
  
  bignum_from_string(sa, "000000FF", 8);
  bignum_from_string(sb, "0000FF00", 8);
  bignum_from_string(sc, "00FF0000", 8);
  bignum_from_string(sd, "FF000000", 8);
 
  bignum_from_int(ia, 0x000000FF);
  bignum_from_int(ib, 0x0000FF00);
  bignum_from_int(ic, 0x00FF0000);
  bignum_from_int(id, 0xFF000000);

  printf("Verifying comparison function.\n");

  assert(bignum_cmp(ia, ib) == SMALLER);
  assert(bignum_cmp(ia, ic) == SMALLER);
  assert(bignum_cmp(ia, id) == SMALLER);

  assert(bignum_cmp(ib, ia) == LARGER);
  assert(bignum_cmp(ic, ia) == LARGER);
  assert(bignum_cmp(id, ia) == LARGER);

  assert(bignum_cmp(sa, sb) == SMALLER);
  assert(bignum_cmp(sa, sc) == SMALLER);
  assert(bignum_cmp(sa, sd) == SMALLER);

  assert(bignum_cmp(sb, sa) == LARGER);
  assert(bignum_cmp(sc, sa) == LARGER);
  assert(bignum_cmp(sd, sa) == LARGER);

  assert(bignum_cmp(ia, sa) == EQUAL);
  assert(bignum_cmp(ib, sb) == EQUAL);
  assert(bignum_cmp(ic, sc) == EQUAL);
  assert(bignum_cmp(id, sd) == EQUAL);

  printf("Verifying to_string function.\n");

  _TPtr<char> _T_sabuf = StaticUncheckedToTStrAdaptor(sabuf, sizeof(sabuf));
    _TPtr<char> _T_sbbuf = StaticUncheckedToTStrAdaptor(sbbuf, sizeof(sbbuf));
    _TPtr<char> _T_scbuf = StaticUncheckedToTStrAdaptor(scbuf, sizeof(scbuf));
    _TPtr<char> _T_sdbuf = StaticUncheckedToTStrAdaptor(sdbuf, sizeof(sdbuf));
    _TPtr<char> _T_iabuf = StaticUncheckedToTStrAdaptor(iabuf, sizeof(iabuf));
    _TPtr<char> _T_ibbuf = StaticUncheckedToTStrAdaptor(ibbuf, sizeof(ibbuf));
    _TPtr<char> _T_icbuf = StaticUncheckedToTStrAdaptor(icbuf, sizeof(icbuf));
    _TPtr<char> _T_idbuf = StaticUncheckedToTStrAdaptor(idbuf, sizeof(idbuf));

  bignum_to_string(sa, _T_sabuf, sizeof(sabuf));
  bignum_to_string(sb, _T_sbbuf, sizeof(sbbuf));
  bignum_to_string(sc, _T_scbuf, sizeof(scbuf));
  bignum_to_string(sd, _T_sdbuf, sizeof(sdbuf));

  bignum_to_string(ia, _T_iabuf, sizeof(iabuf));
  bignum_to_string(ib, _T_ibbuf, sizeof(ibbuf));
  bignum_to_string(ic, _T_icbuf, sizeof(icbuf));
  bignum_to_string(id, _T_idbuf, sizeof(idbuf));

  assert(t_strcmp(_T_sabuf, _T_iabuf) == 0);
  assert(t_strcmp(_T_sbbuf, _T_ibbuf) == 0);
  assert(t_strcmp(_T_scbuf, _T_icbuf) == 0);
  assert(t_strcmp(_T_sdbuf, _T_idbuf) == 0);

  printf("Verifying increment + decrement functions.\n");

  bignum_init(sd);
  int i;
  for (i = 0; i < 255; ++i)
  {
    bignum_inc(sd);
    assert(!bignum_is_zero(sd));
  }

  assert(bignum_cmp(sd, ia) == EQUAL);

  for (i = 0; i < 255; ++i)
  {
    assert(!bignum_is_zero(sd));
    bignum_dec(sd);
  }
  assert(bignum_is_zero(sd));

  bignum_init(sa);
  bignum_init(sb);
  bignum_init(sc);
  bignum_init(sd);

  char hex_1000[]    = "000003E8";
  char hex_1000000[] = "000F4240";

  /* Load 0x0308 into A and B from string */
  bignum_from_string(sa, hex_1000, 8);
  bignum_from_string(sb, hex_1000, 8);

  /* Load 0x0308 into C from integer */
  bignum_from_int(sc, 0x3e8);

  /* Check for agreement - does bignum_from_string match bignum_from_int? */
  assert(bignum_cmp(sa, sc) == EQUAL);
  assert(bignum_cmp(sb, sc) == EQUAL);

  /* Load comparison value: */
  bignum_from_string(sd, hex_1000000, 8);
  bignum_from_int(se, 0xf4240);

  /* Perform calculation:  C = A * B => C = 0x308 * 0x308 */
  bignum_mul(sa, sb, sc);

  /* Check if results agree: */
  assert(bignum_cmp(sc, sd) == EQUAL);
  assert(bignum_cmp(sc, se) == EQUAL);

  /* Perform calculation other way around: C = B * A */
  bignum_mul(sb, sa, sc);

  /* Check if results agree: */
  assert(bignum_cmp(sc, sd) == EQUAL);
  assert(bignum_cmp(sc, se) == EQUAL);

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("\nTests successful. With total time taken --> %f\n\n", cpu_time_used);

  __free__(sa);
    __free__(sb);
    __free__(sc);
    __free__(sd);__free__(se);
    __free__(ia);
    __free__(ib);
    __free__(ic);
    __free__(id);
#ifndef NOOP_SBX
    __free__(_T_sabuf);
    __free__(_T_sbbuf);
    __free__(_T_scbuf);
    __free__(_T_sdbuf);
    __free__(_T_iabuf);
    __free__(_T_ibbuf);
    __free__(_T_icbuf);
    __free__(_T_idbuf);
#endif

  return 0;
}

