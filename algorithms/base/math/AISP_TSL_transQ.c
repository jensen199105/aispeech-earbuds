/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : AISP_TSL_transQ.c
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

#include "AISP_TSL_transQ.h"

/************************************************************
  Function   : AISP_TSL_copy_q31()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/21, Youhai.Jiang create

************************************************************/
VOID AISP_TSL_copy_q31(const S32 *piSrc, S32 *piDst, U32 uiLen)
{
    U32 uiBlkCnt;                               /* Loop counter */
    /* Loop unrolling: Compute 4 outputs at a time */
    uiBlkCnt = uiLen >> 2U;

    while (uiBlkCnt > 0U)
    {
        /* C = A */
        /* Copy and store result in destination buffer */
        *piDst++ = *piSrc++;
        *piDst++ = *piSrc++;
        *piDst++ = *piSrc++;
        *piDst++ = *piSrc++;
        /* Decrement loop counter */
        uiBlkCnt--;
    }

    /* Loop unrolling: Compute remaining outputs */
    uiBlkCnt = uiLen & 3;

    while (uiBlkCnt > 0U)
    {
        /* C = A */
        /* Copy and store result in destination buffer */
        *piDst++ = *piSrc++;
        /* Decrement loop counter */
        uiBlkCnt--;
    }
}

/************************************************************
  Function   : AISP_TSL_max_q31()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/08/01, Youhai.Jiang create

************************************************************/
VOID AISP_TSL_max_q31(const S32 *piSrc, U32 uiBlockSize, S32 *piResult, U32 *puiIdx)
{
    S32 maxVal, out;        /* Temporary variables to store the output value. */
    U32 blkCnt, outIndex;   /* Loop counter */
    U32 index = 0;          /* index of maximum value */
    /* Initialise index value to zero. */
    outIndex = 0U;
    /* Load first input value that act as reference value for comparision */
    out = *piSrc++;
    /* Loop unrolling: Compute 4 outputs at a time */
    blkCnt = (uiBlockSize - 1U) >> 2U;

    while (blkCnt > 0U)
    {
        /* Initialize maxVal to next consecutive values one by one */
        maxVal = *piSrc++;

        /* compare for the maximum value */
        if (out < maxVal)
        {
            /* Update the maximum value and it's index */
            out = maxVal;
            outIndex = index + 1U;
        }

        maxVal = *piSrc++;

        if (out < maxVal)
        {
            out = maxVal;
            outIndex = index + 2U;
        }

        maxVal = *piSrc++;

        if (out < maxVal)
        {
            out = maxVal;
            outIndex = index + 3U;
        }

        maxVal = *piSrc++;

        if (out < maxVal)
        {
            out = maxVal;
            outIndex = index + 4U;
        }

        index += 4U;
        /* Decrement loop counter */
        blkCnt--;
    }

    /* Loop unrolling: Compute remaining outputs */
    blkCnt = (uiBlockSize - 1U) & 3;

    while (blkCnt > 0U)
    {
        /* Initialize maxVal to the next consecutive values one by one */
        maxVal = *piSrc++;

        /* compare for the maximum value */
        if (out < maxVal)
        {
            /* Update the maximum value and it's index */
            out = maxVal;
            outIndex = uiBlockSize - blkCnt;
        }

        /* Decrement loop counter */
        blkCnt--;
    }

    /* Store the maximum value and it's index into destination pointers */
    *piResult = out;
    *puiIdx = outIndex;
}

/************************************************************
  Function   : AISP_TSL_min_q31()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/08/09, Youhai.Jiang create

************************************************************/
VOID AISP_TSL_min_q31(const S32 *piSrc, U32 uiBlockSize, S32 *piResult, U32 *puiIdx)
{
    S32 minVal, out;        /* Temporary variables to store the output value. */
    U32 blkCnt, outIndex;   /* Loop counter */
    U32 index = 0;          /* index of maximum value */
    /* Initialise index value to zero. */
    outIndex = 0U;
    /* Load first input value that act as reference value for comparision */
    out = *piSrc++;
    /* Loop unrolling: Compute 4 outputs at a time */
    blkCnt = (uiBlockSize - 1U) >> 2U;

    while (blkCnt > 0U)
    {
        /* Initialize maxVal to next consecutive values one by one */
        minVal = *piSrc++;

        /* compare for the maximum value */
        if (out > minVal)
        {
            /* Update the maximum value and it's index */
            out = minVal;
            outIndex = index + 1U;
        }

        minVal = *piSrc++;

        if (out > minVal)
        {
            out = minVal;
            outIndex = index + 2U;
        }

        minVal = *piSrc++;

        if (out > minVal)
        {
            out = minVal;
            outIndex = index + 3U;
        }

        minVal = *piSrc++;

        if (out > minVal)
        {
            out = minVal;
            outIndex = index + 4U;
        }

        index += 4U;
        /* Decrement loop counter */
        blkCnt--;
    }

    /* Loop unrolling: Compute remaining outputs */
    blkCnt = (uiBlockSize - 1U) & 3;

    while (blkCnt > 0U)
    {
        /* Initialize maxVal to the next consecutive values one by one */
        minVal = *piSrc++;

        /* compare for the maximum value */
        if (out > minVal)
        {
            /* Update the maximum value and it's index */
            out = minVal;
            outIndex = uiBlockSize - blkCnt;
        }

        /* Decrement loop counter */
        blkCnt--;
    }

    /* Store the maximum value and it's index into destination pointers */
    *piResult = out;
    *puiIdx = outIndex;
}

/************************************************************
  Function   : AISP_TSL_max_q15()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/08/01, Youhai.Jiang create

************************************************************/
VOID AISP_TSL_max_q15(const S16 *psSrc, U32 uiBlockSize, S16 *pResult, U32 *puiIdx)
{
    S16 maxVal, out;        /* Temporary variables to store the output value. */
    U32 blkCnt, outIndex;   /* Loop counter */
    U32 index;              /* index of maximum value */
    /* Initialise index value to zero. */
    outIndex = 0U;
    /* Load first input value that act as reference value for comparision */
    out = *psSrc++;
    /* Initialise index of maximum value. */
    index = 0U;
    /* Loop unrolling: Compute 4 outputs at a time */
    blkCnt = (uiBlockSize - 1U) >> 2U;

    while (blkCnt > 0U)
    {
        /* Initialize maxVal to next consecutive values one by one */
        maxVal = *psSrc++;

        /* compare for the maximum value */
        if (out < maxVal)
        {
            /* Update the maximum value and it's index */
            out = maxVal;
            outIndex = index + 1U;
        }

        maxVal = *psSrc++;

        if (out < maxVal)
        {
            out = maxVal;
            outIndex = index + 2U;
        }

        maxVal = *psSrc++;

        if (out < maxVal)
        {
            out = maxVal;
            outIndex = index + 3U;
        }

        maxVal = *psSrc++;

        if (out < maxVal)
        {
            out = maxVal;
            outIndex = index + 4U;
        }

        index += 4U;
        /* Decrement loop counter */
        blkCnt--;
    }

    /* Loop unrolling: Compute remaining outputs */
    blkCnt = (uiBlockSize - 1U) & 3U;

    while (blkCnt > 0U)
    {
        /* Initialize maxVal to the next consecutive values one by one */
        maxVal = *psSrc++;

        /* compare for the maximum value */
        if (out < maxVal)
        {
            /* Update the maximum value and it's index */
            out = maxVal;
            outIndex = uiBlockSize - blkCnt;
        }

        /* Decrement loop counter */
        blkCnt--;
    }

    /* Store the maximum value and it's index into destination pointers */
    *pResult = out;
    *puiIdx = outIndex;
}

/************************************************************
  Function   : AISP_TSL_min_q15()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/08/01, Youhai.Jiang create

************************************************************/
VOID AISP_TSL_min_q15(const S16 *psSrc, U32 uiBlockSize, S16 *pResult, U32 *puiIdx)
{
    S16 minVal, out;        /* Temporary variables to store the output value. */
    U32 blkCnt, outIndex;   /* Loop counter */
    U32 index;              /* index of maximum value */
    /* Initialise index value to zero. */
    outIndex = 0U;
    /* Load first input value that act as reference value for comparision */
    out = *psSrc++;
    /* Initialise index of maximum value. */
    index = 0U;
    /* Loop unrolling: Compute 4 outputs at a time */
    blkCnt = (uiBlockSize - 1U) >> 2U;

    while (blkCnt > 0U)
    {
        /* Initialize minVal to next consecutive values one by one */
        minVal = *psSrc++;

        /* compare for the minimum value */
        if (out > minVal)
        {
            /* Update the minimum value and it's index */
            out = minVal;
            outIndex = index + 1U;
        }

        minVal = *psSrc++;

        if (out > minVal)
        {
            out = minVal;
            outIndex = index + 2U;
        }

        minVal = *psSrc++;

        if (out > minVal)
        {
            out = minVal;
            outIndex = index + 3U;
        }

        minVal = *psSrc++;

        if (out > minVal)
        {
            out = minVal;
            outIndex = index + 4U;
        }

        index += 4U;
        /* Decrement loop counter */
        blkCnt--;
    }

    /* Loop unrolling: Compute remaining outputs */
    blkCnt = (uiBlockSize - 1U) & 3U;

    while (blkCnt > 0U)
    {
        /* Initialize minVal to the next consecutive values one by one */
        minVal = *psSrc++;

        /* compare for the minimum value */
        if (out > minVal)
        {
            /* Update the minimum value and it's index */
            out = minVal;
            outIndex = uiBlockSize - blkCnt;
        }

        /* Decrement loop counter */
        blkCnt--;
    }

    /* Store the minimum value and it's index into destination pointers */
    *pResult = out;
    *puiIdx = outIndex;
}

