#include "common.h"
#include "itoa.h"

int32_t itoa(int32_t n, char *s)
{
    int i =  0;         

    if(n / 10 != 0)
        i = itoa(n/10, s);
    else if(n < 0)
        s[i++] = '-';

    s[i++] = abs(n % 10) + '0';
    s[i] = '\0';

    return i;
}

