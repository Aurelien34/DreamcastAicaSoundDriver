#ifndef BASIC_FUNCTIONS_H
#define BASIC_FUNCTIONS_H

void *memcpy (void *dest, const void *src, size_t n);
void *memmove (void *dest, const void *src, size_t n);
void *memset (void *s, int c, size_t n);

char *GetStringValue(unsigned int uValue);

/*
uint div(uint dividend, uint divisor);
int sdiv(int dividend, int divisor);
int __divsi3(int a, int b);
*/

#endif
