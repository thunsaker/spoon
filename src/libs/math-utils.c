#include "libs/math-utils.h"

// From http://stackoverflow.com/questions/15265230/c-writing-a-function-for-an-exponent-without-using-pow
int x_to_the_n(int x,int n)
{
    int i; /* Variable used in loop counter */
    int number = 1;

    for (i = 0; i < n; ++i)
        number *= x;

    return(number);
}