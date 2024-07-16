  #include <stdio.h>

typedef union {
  int i;
  float f;
} intfloat;

void float_bits(intfloat i)
{
  int j;

  for (j = 31; j >= 0; j--) {
    printf("%d", (i.i >> j) & 0x1);
  }
  printf("\n");
}

/*   abcdefghijkl  <- these letters are variables for bits
 * & 001101001010
 * --------------
 *   00cd0f00i0k0
 */

void print_ieee754_fields(intfloat i)
{
  // Printing mantissa prints a leading 0 bit as an artifact of the
  // hex output.  That highest bit is not actually part of the mantissa.
  printf("f: %f\ts: %d\tbe: %d\tue: %d\tm: %#x\n", i.f,
         (i.i >> 31) & 0x1, (i.i >> 23) & 0xff, ((i.i >> 23) & 0xff) - 127,
         i.i & 0x7fffff);
}

int main(int argc, char *argv[])
{
  intfloat i;
  int j;
  int k;
  
  i.f = 14.0625; // 1.1100001 * 2^3

  printf("i: %d\tf: %f\n", i.i, i.f);
  float_bits(i);
  print_ieee754_fields(i);

  k = 4;
  for (i.f = 0, j = k; ; i.f += k) {
    if (i.f == (0x1 << j)) {
      print_ieee754_fields(i);
      j++;
    }
  }
  
  return 0;
}
