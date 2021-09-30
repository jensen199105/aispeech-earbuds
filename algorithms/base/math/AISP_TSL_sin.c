#include <stdio.h>
#include <stdlib.h>

extern const unsigned int   sin_pol_table[];
extern const unsigned short sin_error_table[];

int AISP_TSL_sin_xW32Q24_yW32Q24(int x);

/*
 * Input: x is limited by [0, pi/2]
 * Accuracy: 10e-7
 */
int AISP_TSL_sin_xW32Q24_yW32Q24(int x)
{
    int abs_x, group, half, quarter, slope;
    int error;
    int y;
    abs_x   = abs(x);
    group   = abs_x >> 16;
    half    = abs_x & 0xffff;
    quarter = abs_x & 0x7fff;

    if (half < 0x8000)
    {
        quarter = 0x8000 - quarter;
    }

    error = sin_error_table[group];
    slope = sin_pol_table[group + 1] - sin_pol_table[group];
    y     = sin_pol_table[group] + error;
    y    += ((long long)slope * half) >> 16;
    y    -= ((long long)error * quarter * quarter) >> 30;   /* second order approximation y = m(1 - x*x) */
    y   >>= 7;
    return y;
}

