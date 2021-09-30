#include "AISP_TSL_common.h"
#include "AISP_TSL_log.h"

int AISP_TSL_log_xW32_yW32Q24(int x, int q_in)
{
    int t;
    int y;    /* W32Q24 output */
    int y_tmp[] = {360501479, 348872399, 337243319, 325614239, 313985159, 302356079, 290726999, 279097919,
                   267468839, 255839759, 244210679, 232581599, 220952519, 209323439, 197694359, 186065279,
                   174436200, 162807120, 151178040, 139548960, 127919880, 116290800, 104661720, 93032640,
                   81403560, 69774480, 58145400, 46516320, 34887240, 23258160, 11629080, 0
                  };
    //y = 360501479;  /* the max ln(x) value, ln(2^31)*Q24 = 360501479 */
    y = y_tmp[q_in];

    if (x < 0x00008000)
    {
        x <<= 16,         y -= 0xB17217F;
    }

    if (x < 0x00800000)
    {
        x <<= 8,          y -= 0x58B90C0;
    }

    if (x < 0x08000000)
    {
        x <<= 4,          y -= 0x2C5C860;
    }

    if (x < 0x20000000)
    {
        x <<= 2,          y -= 0x162E430;
    }

    if (x < 0x40000000)
    {
        x <<= 1,          y -= 0x0B17218;
    }

    t = x + (x >> 1);

    if (t < 0x80000000)
    {
        x = t, y -= 0x067CC90;
    }

    t = x + (x >> 2);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0391FF0;
    }

    t = x + (x >> 3);

    if (t < 0x80000000)
    {
        x = t, y -= 0x01E2707;
    }

    t = x + (x >> 4);

    if (t < 0x80000000)
    {
        x = t, y -= 0x00F8518;
    }

    t = x + (x >> 5);

    if (t < 0x80000000)
    {
        x = t, y -= 0x007E0A7;
    }

    t = x + (x >> 6);

    if (t < 0x80000000)
    {
        x = t, y -= 0x003F815;
    }

    t = x + (x >> 7);

    if (t < 0x80000000)
    {
        x = t, y -= 0x001FE03;
    }

    t = x + (x >> 8);

    if (t < 0x80000000)
    {
        x = t, y -= 0x000FF80;
    }

    t = x + (x >> 9);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0007FE0;
    }

    t = x + (x >> 10);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0003FF8;
    }

    t = x + (x >> 11);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0001FFE;
    }

    t = x + (x >> 12);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0001000;
    }

    t = x + (x >> 13);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0000800;
    }

    t = x + (x >> 14);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0000400;
    }

    t = x + (x >> 15);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0000200;
    }

    t = x + (x >> 16);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0000100;
    }

    t = x + (x >> 17);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0000080;
    }

    t = x + (x >> 18);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0000040;
    }

    t = x + (x >> 19);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0000020;
    }

    t = x + (x >> 20);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0000010;
    }

    t = x + (x >> 21);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0000008;
    }

    t = x + (x >> 22);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0000004;
    }

    t = x + (x >> 23);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0000002;
    }

    t = x + (x >> 24);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0000001;
    }

    x = 0x80000000 - x;
    y -= x >> 7;
    return y;
}

int AISP_TSL_log_xW32_yW32Q20(int x, int q_in)
{
    int t;
    int y;    /* W32Q24 output */
    int y_tmp[] = {22531342, 21804525, 21077707, 20350890, 19624072, 18897255, 18170437, 17443620,
                   16716802, 15989985, 15263167, 14536350, 13809532, 13082715, 12355897, 11629080,
                   10902262, 10175445, 9448627, 8721810, 7994992, 7268175, 6541357, 5814540,
                   5087722, 4360905, 3634087, 2907270, 2180452, 1453635, 726817, 0
                  };
    //y = 360501479;  /* the max ln(x) value, ln(2^31)*Q24 = 360501479 */
    y = y_tmp[q_in];

    if (x < 0x00008000)
    {
        x <<= 16,         y -= 0xB17218;
    }

    if (x < 0x00800000)
    {
        x <<= 8,          y -= 0x58B90C;
    }

    if (x < 0x08000000)
    {
        x <<= 4,          y -= 0x2C5C86;
    }

    if (x < 0x20000000)
    {
        x <<= 2,          y -= 0x162E43;
    }

    if (x < 0x40000000)
    {
        x <<= 1,          y -= 0x0B1722;
    }

    t = x + (x >> 1);

    if (t < 0x80000000)
    {
        x = t, y -= 0x067CC9;
    }

    t = x + (x >> 2);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0391FF;
    }

    t = x + (x >> 3);

    if (t < 0x80000000)
    {
        x = t, y -= 0x01E270;
    }

    t = x + (x >> 4);

    if (t < 0x80000000)
    {
        x = t, y -= 0x00F852;
    }

    t = x + (x >> 5);

    if (t < 0x80000000)
    {
        x = t, y -= 0x007E0A;
    }

    t = x + (x >> 6);

    if (t < 0x80000000)
    {
        x = t, y -= 0x003F81;
    }

    t = x + (x >> 7);

    if (t < 0x80000000)
    {
        x = t, y -= 0x001FE0;
    }

    t = x + (x >> 8);

    if (t < 0x80000000)
    {
        x = t, y -= 0x000FF8;
    }

    t = x + (x >> 9);

    if (t < 0x80000000)
    {
        x = t, y -= 0x0007FE;
    }

    t = x + (x >> 10);

    if (t < 0x80000000)
    {
        x = t, y -= 0x000400;
    }

    t = x + (x >> 11);

    if (t < 0x80000000)
    {
        x = t, y -= 0x000200;
    }

    t = x + (x >> 12);

    if (t < 0x80000000)
    {
        x = t, y -= 0x000100;
    }

    t = x + (x >> 13);

    if (t < 0x80000000)
    {
        x = t, y -= 0x000080;
    }

    t = x + (x >> 14);

    if (t < 0x80000000)
    {
        x = t, y -= 0x000040;
    }

    t = x + (x >> 15);

    if (t < 0x80000000)
    {
        x = t, y -= 0x000020;
    }

    t = x + (x >> 16);

    if (t < 0x80000000)
    {
        x = t, y -= 0x000010;
    }

    t = x + (x >> 17);

    if (t < 0x80000000)
    {
        x = t, y -= 0x000008;
    }

    t = x + (x >> 18);

    if (t < 0x80000000)
    {
        x = t, y -= 0x000004;
    }

    t = x + (x >> 19);

    if (t < 0x80000000)
    {
        x = t, y -= 0x000002;
    }

    t = x + (x >> 20);

    if (t < 0x80000000)
    {
        x = t, y -= 0x000001;
    }

    t = x + (x >> 21);

    if (t < 0x80000000)
    {
        x = t, y -= 0x000001;
    }

    x = 0x80000000 - x;
    y -= x >> 11;
    return y;
}
