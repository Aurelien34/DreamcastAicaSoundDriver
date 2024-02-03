#include "common.h"

#include "BasicFunctions.h"

void *memcpy (void *dest, const void *src, size_t n)
{
    unsigned char *pSrc = (unsigned char *)src;
    unsigned char *pDest = (unsigned char *)dest;

    while (n--)
        *(pDest++) = *(pSrc++);
    
    return dest;
}

void *memmove (void *dest, const void *src, size_t n)
{
    unsigned char *pSrc = (unsigned char *)src;
    unsigned char *pDest = (unsigned char *)dest;

    if (pSrc >= pDest)
    {
        while (n--)
            *(pDest++) = *(pSrc++);
    }
    else
    {
        pSrc += n;
        pDest += n;
        while (n--)
            *(pDest--) = *(pSrc--);
    }

    return dest;
}

void *memset (void *s, int c, size_t n)
{
    unsigned char *pDest = (unsigned char *)s;
    unsigned char bC = (unsigned char) c;

    while (n--)
        *(pDest++) = bC;
    
    return s;
}

char * GetStringValue(unsigned int uValue)
{
    static char szReturn[] = "0x00000000";
    int nCounter;
    unsigned int nQuartet;
    for (nCounter = 9; nCounter > 1; --nCounter)
    {
        nQuartet = uValue & 0xF;
        szReturn[nCounter] = (char)( (nQuartet > 9) ? (nQuartet - 10 + 'A') : (nQuartet + '0') );
        uValue = uValue >> 4;
    }
    return szReturn;
}



/*

   These functions are examples of unsigned and signed integer division.
   The algorithms assume 32-bit integers.  The algorithms used are
   from

      Digital Computer Arithmetic, by Joseph J.F. Cavanaugh
      McGraw Hill, 1984.

   For an integer of size N, this algorithm will take N steps.  This
   makes it slower the "high-radix" division algorithms.  The algorithm
   has been written with pipelined hardware in mind and where ever
   possible, conditions are avoided in loops.  Assuming a decent
   compiler, this code should also perform well on hardware with 
   branch delay slots.

   If you compile this with Visual C++ and you keep the .c suffix
   use -Tp to default to C++.

   Ian Kaplan, October 1996

   Copyright stuff

   Use of this program, for any purpose, is granted the author,
   Ian Kaplan, as long as this copyright notice is included in
   the source code or any source code derived from this program.
   The user assumes all responsibility for using this code.

*/
/*
uint div(uint dividend, uint divisor)
{
  return dividend / divisor;

  unsigned int t, num_bits;
  unsigned int q, bit, d = 0;
  int i;

  unsigned remainder = 0;
  unsigned quotient = 0;

  if (divisor == 0)
    return quotient;

  if (divisor > dividend) {
    remainder = dividend;
    return quotient;
  }

  if (divisor == dividend) {
    quotient = 1;
    return quotient;
  }

  num_bits = 32;

  while (remainder < divisor) {
    bit = (dividend & 0x80000000) >> 31;
    remainder = (remainder << 1) | bit;
    d = dividend;
    dividend = dividend << 1;
    num_bits--;
  }

  // The loop, above, always goes one iteration too far.
  //   To avoid inserting an "if" statement inside the loop
  //   the last iteration is simply reversed.
  dividend = d;
  remainder = remainder >> 1;
  num_bits++;

  for (i = 0; i < num_bits; i++) {
    bit = (dividend & 0x80000000) >> 31;
    remainder = (remainder << 1) | bit;
    t = remainder - divisor;
    q = !((t & 0x80000000) >> 31);
    dividend = dividend << 1;
    quotient = (quotient << 1) | q;
    if (q) {
       remainder = t;
     }
  }
  return quotient;
}  // unsigned_divide

int sdiv(int dividend, int divisor)
{
    // Get signs
    unsigned int uSign = ((unsigned int)(divisor ^ dividend)) & 0x80000000;
    unsigned int uDividend = ((unsigned int)dividend) & 0x7FFFFFFF;
    unsigned int uDivisor = ((unsigned int)divisor) & 0x7FFFFFFF;
    return (int)(div(uDividend, uDivisor) + uSign);
}

int __divsi3(int a, int b)
{
    return sdiv(a, b);
}
*/
