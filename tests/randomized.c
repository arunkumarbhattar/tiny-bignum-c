#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "bn.h"

enum { ADD, SUB, MUL, DIV, AND, OR, XOR, POW, MOD, RSHFT, LSHFT, ISQRT };

int main(int argc, char** argv)
{

  if (argc < 5)
  {
    printf("ERROR\n\nUsage:\n    %s [oper] [operand1] [operand2] [result]\n\nWhere oper means:\n    0 = add, 1 = sub, 2 = mul, 3 = div\n\n", argv[0]);

    return -1;
  }  

  int oper = atoi(argv[1]);

/*
  printf("program  = %s \n", argv[0]);
  printf("operator = %s [%d]\n", argv[1], oper);
  printf("operand1 = %s \n", argv[2]);
  printf("operand2 = %s \n", argv[3]);
  printf("expected = %s \n", argv[4]);
*/

  _TPtr<_T_bn> a = NULL, b = NULL, c = NULL, res = NULL;
  a = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
  b = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
  c = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
  res = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));

  bignum_init(a);
  bignum_init(b);
  bignum_init(c);
  bignum_init(res);
  bignum_from_string(a, argv[2], strlen(argv[2]));
  bignum_from_string(b, argv[3], strlen(argv[3]));
  bignum_from_string(c, argv[4], strlen(argv[4]));

  _TPtr<_T_bn> a_before = NULL, b_before = NULL;
  a_before = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
  b_before = (_TPtr<_T_bn>)__malloc__(sizeof(_T_bn));
  bignum_assign(a_before, a);
  bignum_assign(b_before, b);


  switch (oper)
  {
    case ADD:   bignum_add(a, b, res);   break;
    case SUB:   bignum_sub(a, b, res);   break;
    case MUL:   bignum_mul(a, b, res);   break;
    case DIV:   bignum_div(a, b, res);   break;
    case AND:   bignum_and(a, b, res);   break;
    case OR:    bignum_or (a, b, res);   break;
    case XOR:   bignum_xor(a, b, res);   break;
    case POW:   bignum_pow(a, b, res);   break;
    case MOD:   bignum_mod(a, b, res);   break;
    case ISQRT: bignum_isqrt(a, res);     break;
    case RSHFT:
    {
      bignum_rshift(a, res, bignum_to_int(b));
    } break;
    case LSHFT: 
    {
      bignum_lshift(a, res, bignum_to_int(b));
    } break;
    

    default:
      printf("default switch-case hit: unknown operator '%d' \n", oper);
      assert(0); 
  }

  int cmp_result = (bignum_cmp(res, c) == EQUAL);

  if (!cmp_result)
  {
    char buf[8192];
    _TPtr<char>_T_buf = StaticUncheckedToTStrAdaptor(buf, sizeof(buf));
    bignum_to_string(res, _T_buf, sizeof(buf));
    printf("\ngot %s\n", _T_buf);
    printf(" a  = %d \n", bignum_to_int(a));
    printf(" b  = %d \n", bignum_to_int(b));
    printf("res = %d \n", bignum_to_int(res));
    printf("\n");
    __free__(_T_buf);
    return 1;
  }

  assert(bignum_cmp(a_before, a) == EQUAL);
  assert(bignum_cmp(b_before, b) == EQUAL);

    __free__(a);
    __free__(b);
    __free__(c);
    __free__(res);
    __free__(a_before);
    __free__(b_before);

  return 0;
}


