/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : AISP_TSL_sqrt.c
  Project    :
  Module     :
  Version    :
  Date       : 2019/06/20
  Author     : Youhai.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Youhai.Jiang  2019/06/20         1.00              Create

************************************************************/
#include "AISP_TSL_sqrt.h"

#ifdef USE_CM4_OPTIMIZE
#include "arm_math.h"
#endif

/************************************************************
 *   Function   : AISP_TSL_sqrt_xW64_yW32()
 *   Description:
 *   Calls      :
 *   Called By  :
 *   Input      : x, stored in 64 bits
 *   Output     : y, stored in 32 bits
 *   Return     :
 *   Others     :
 *   History    :
 *       2017/05/08, changren.tan create
 *       2018/05/05, chao.xu      modify
 *************************************************************/
U32 AISP_TSL_sqrt_xW64_yW32(U64 x)
{
#ifdef USE_CM4_OPTIMIZE
    float tmpIn = x;
    float tmpOut = 0.0f;
    arm_sqrt_f32(tmpIn, &tmpOut);
    return (U32)(tmpOut + 0.5f);
#else
    U64 temp  = 0;
    U32 nHat  = 0;
    U32 b     = 0x80000000;
    U32 bshft = 31;
    U64 V     = (U64) x;

    do
    {
        if (V >= (temp = ((U64)((nHat << 1) + b) << bshft--)))
        {
            nHat += b;
            V    -= temp;
        }
    }
    while (b >>= 1);

    return (U32)nHat;
#endif
}

/************************************************************
  Function   : AISP_TSL_sqrt32()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2017/05/08, changren.tan create

************************************************************/
U32 AISP_TSL_sqrt32(U32 x)
{
#ifdef AISPEECH_FIXED
    U64 temp  = 0;
    U64 V     = (U64) x << 15;
    U32 nHat  = 0;
    U32 b     = 0x80000000;
    U32 bshft = 31;

    do
    {
        if (V >= (temp = ((U64)((nHat << 1) + b) << bshft--)))
        {
            nHat += b;
            V    -= temp;
        }
    }
    while (b >>= 1);

    return (U32)nHat;
#else
    return sqrtf(x);
#endif
}

