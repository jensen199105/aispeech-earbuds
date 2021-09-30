/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : AISP_TSL_add.c
  Project    :
  Module     :
  Version    :
  Date       : 2019/07/15
  Author     : Youhai.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Youhai.Jiang  2019/07/15         1.00              Create

************************************************************/

#include "AISP_TSL_add.h"

/************************************************************
  Function   : AISP_TSL_add_vec_32()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/15, Youhai.Jiang create

************************************************************/
VOID AISP_TSL_add_vec_32(S32 *piSrcA, S32 *piSrcB, S32 *piDst, S32 len)
{
    /* Loop unrolling: Compute 4 outputs at a time */
    S32 blkCnt = len >> 2U;

    while (blkCnt > 0U)
    {
        /* C = A + B */
        /* Add and store result in destination buffer. */
        *piDst++ = (*piSrcA++) + (*piSrcB++);
        *piDst++ = (*piSrcA++) + (*piSrcB++);
        *piDst++ = (*piSrcA++) + (*piSrcB++);
        *piDst++ = (*piSrcA++) + (*piSrcB++);
        /* Decrement loop counter */
        blkCnt--;
    }

    /* Loop unrolling: Compute remaining outputs */
    blkCnt = len & 3;

    while (blkCnt > 0U)
    {
        /* C = A + B */
        /* Add and store result in destination buffer. */
        *piDst++ = (*piSrcA++) + (*piSrcB++);
        /* Decrement loop counter */
        blkCnt--;
    }
}

/************************************************************
  Function   : AISP_TSL_sub_vec_32()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/15, Youhai.Jiang create

************************************************************/
VOID AISP_TSL_sub_vec_32(S32 *piSrcA, S32 *piSrcB, S32 *piDst, S32 len)
{
    /* Loop unrolling: Compute 4 outputs at a time */
    S32 blkCnt = len >> 2U;

    while (blkCnt > 0U)
    {
        /* C = A - B */
        /* Add and store result in destination buffer. */
        *piDst++ = (*piSrcA++) - (*piSrcB++);
        *piDst++ = (*piSrcA++) - (*piSrcB++);
        *piDst++ = (*piSrcA++) - (*piSrcB++);
        *piDst++ = (*piSrcA++) - (*piSrcB++);
        /* Decrement loop counter */
        blkCnt--;
    }

    /* Loop unrolling: Compute remaining outputs */
    blkCnt = len & 3;

    while (blkCnt > 0U)
    {
        /* C = A - B */
        /* Add and store result in destination buffer. */
        *piDst++ = (*piSrcA++) - (*piSrcB++);
        /* Decrement loop counter */
        blkCnt--;
    }
}

