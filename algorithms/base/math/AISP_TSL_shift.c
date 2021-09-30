/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : AISP_TSL_shift.c
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
#include "AISP_TSL_shift.h"
#include "AISP_TSL_common.h"

#ifdef AISPEECH_FIXED
/************************************************************
  Function   : AISP_TSL_PSHR64()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/06/21, Youhai.Jiang create

************************************************************/
U64 AISP_TSL_PSHR64(S64 a, S32 shift)
{
    S64_WITH_SYMBLE_T *aa = (S64_WITH_SYMBLE_T *)&a;
    return SHR((a) + (((S64)1 << shift) >> 1) - aa->sign, (shift));
}

/************************************************************
  Function   : AISP_TSL_shr_vec_32_s16()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/16, Youhai.Jiang create

************************************************************/
VOID AISP_TSL_shr_vec_32_16(S16 *psSrc, S32 *piDst, U32 uiLen, S8 shiftBits)
{
    int i = 0;
    U8 sign = shiftBits & 0x80;
    U32 blkCnt; /* Loop counter */
    /* Loop unrolling: Compute 4 outputs at a time */
    blkCnt = uiLen >> 2U;

    /* if shift is negative, then left shift */
    if (sign)
    {
        shiftBits = -shiftBits;

        for (i = 0; i < blkCnt; i++)
        {
            *piDst++ = ((int) * psSrc++) << shiftBits;
            *piDst++ = ((int) * psSrc++) << shiftBits;
            *piDst++ = ((int) * psSrc++) << shiftBits;
            *piDst++ = ((int) * psSrc++) << shiftBits;
        }

        blkCnt = uiLen & 3;

        for (i = 0; i < blkCnt; i++)
        {
            *piDst++ = ((int) * psSrc++) << shiftBits;
        }
    }
    else
    {
        for (i = 0; i < blkCnt; i++)
        {
            *piDst++ = ((int) * psSrc++) >> shiftBits;
            *piDst++ = ((int) * psSrc++) >> shiftBits;
            *piDst++ = ((int) * psSrc++) >> shiftBits;
            *piDst++ = ((int) * psSrc++) >> shiftBits;
        }

        blkCnt = uiLen & 3;

        for (i = 0; i < blkCnt; i++)
        {
            *piDst++ = ((int) * psSrc++) >> shiftBits;
        }
    }
}
#endif
