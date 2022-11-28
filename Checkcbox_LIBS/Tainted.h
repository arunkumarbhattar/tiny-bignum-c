#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#define SPRINTF_FORMAT_STR       "%.08x"
#define WORD_SIZE 4
#define BN_ARRAY_SIZE    (128 / WORD_SIZE)
#define DTYPE                    uint32_t
struct _T_bn
{
	DTYPE* array;
};

void bignum_to_string(struct _T_bn* n, char* str, int nbytes);
