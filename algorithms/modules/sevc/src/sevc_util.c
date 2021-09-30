#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <math.h>
#include "AISP_TSL_base.h"
#include "AISP_TSL_str.h"
//#include "AISP_TSL_sigmoid.h"
#include "AISP_TSL_math.h"

#include "sevc_config.h"
#include "sevc_types.h"
#include "sevc_util.h"

extern FILE *fpYyPcm;

#define SEVC_ADDR_CHECK     1
#if SEVC_ADDR_CHECK
#ifdef JIELI_BR28
#define ADDR_CHECK(_addr_) do{if(((S64)_addr_)%4 != 0){\
    MSG_DBG("%s [%d] addr=%p fail\r\n", __func__, __LINE__, _addr_);\
    while(1);}}while(0)
#else
#define ADDR_CHECK(_addr_) do{if(((S64)_addr_)%4 != 0){\
    MSG_DBG("%s [%d] addr=%p fail\r\n", __func__, __LINE__, _addr_);\
    exit(0);}}while(0)
#endif
#else
#define ADDR_CHECK(_addr_)
#endif


#ifndef AISPEECH_FIXED
static const aisp_s16_t g_TansigTable[201] =
{
    0.000000f, 0.039979f, 0.079830f, 0.119427f, 0.158649f,
    0.197375f, 0.235496f, 0.272905f, 0.309507f, 0.345214f,
    0.379949f, 0.413644f, 0.446244f, 0.477700f, 0.507977f,
    0.537050f, 0.564900f, 0.591519f, 0.616909f, 0.641077f,
    0.664037f, 0.685809f, 0.706419f, 0.725897f, 0.744277f,
    0.761594f, 0.777888f, 0.793199f, 0.807569f, 0.821040f,
    0.833655f, 0.845456f, 0.856485f, 0.866784f, 0.876393f,
    0.885352f, 0.893698f, 0.901468f, 0.908698f, 0.915420f,
    0.921669f, 0.927473f, 0.932862f, 0.937863f, 0.942503f,
    0.946806f, 0.950795f, 0.954492f, 0.957917f, 0.961090f,
    0.964028f, 0.966747f, 0.969265f, 0.971594f, 0.973749f,
    0.975743f, 0.977587f, 0.979293f, 0.980869f, 0.982327f,
    0.983675f, 0.984921f, 0.986072f, 0.987136f, 0.988119f,
    0.989027f, 0.989867f, 0.990642f, 0.991359f, 0.992020f,
    0.992631f, 0.993196f, 0.993718f, 0.994199f, 0.994644f,
    0.995055f, 0.995434f, 0.995784f, 0.996108f, 0.996407f,
    0.996682f, 0.996937f, 0.997172f, 0.997389f, 0.997590f,
    0.997775f, 0.997946f, 0.998104f, 0.998249f, 0.998384f,
    0.998508f, 0.998623f, 0.998728f, 0.998826f, 0.998916f,
    0.999000f, 0.999076f, 0.999147f, 0.999213f, 0.999273f,
    0.999329f, 0.999381f, 0.999428f, 0.999472f, 0.999513f,
    0.999550f, 0.999585f, 0.999617f, 0.999646f, 0.999673f,
    0.999699f, 0.999722f, 0.999743f, 0.999763f, 0.999781f,
    0.999798f, 0.999813f, 0.999828f, 0.999841f, 0.999853f,
    0.999865f, 0.999875f, 0.999885f, 0.999893f, 0.999902f,
    0.999909f, 0.999916f, 0.999923f, 0.999929f, 0.999934f,
    0.999939f, 0.999944f, 0.999948f, 0.999952f, 0.999956f,
    0.999959f, 0.999962f, 0.999965f, 0.999968f, 0.999970f,
    0.999973f, 0.999975f, 0.999977f, 0.999978f, 0.999980f,
    0.999982f, 0.999983f, 0.999984f, 0.999986f, 0.999987f,
    0.999988f, 0.999989f, 0.999990f, 0.999990f, 0.999991f,
    0.999992f, 0.999992f, 0.999993f, 0.999994f, 0.999994f,
    0.999994f, 0.999995f, 0.999995f, 0.999996f, 0.999996f,
    0.999996f, 0.999997f, 0.999997f, 0.999997f, 0.999997f,
    0.999997f, 0.999998f, 0.999998f, 0.999998f, 0.999998f,
    0.999998f, 0.999998f, 0.999999f, 0.999999f, 0.999999f,
    0.999999f, 0.999999f, 0.999999f, 0.999999f, 0.999999f,
    0.999999f, 0.999999f, 0.999999f, 0.999999f, 0.999999f,
    1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f,
    1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f,
    1.000000f,
};
#endif

#ifdef JIELI_BR28
extern long long ln_fix(int data, char iq);
extern long long exp_fix(int data, char iq);
extern void vector_real_zs32_ys32_add_xs32(volatile long *zptr, long *yptr,
        long *xptr, short len);
extern void vector_real_zs32_ys32_sub_xs32(volatile long *zptr, long *yptr,
        long *xptr, short len);
extern void vector_real_scale_s32(volatile long *zptr, long *yptr, long xconst,
                                  short len, char q);
extern void vector_real_zs32_ys32_mul_xs8(long *zptr, long *yptr, long *xptr,
        short len, char q);
extern void vector_real_zs32_ys32_mul_xs16(long *zptr, long *yptr, long *xptr,
        short len, char q);
extern void vector_real_zs32_ys32_mul_xs32(long *zptr, long *yptr, long *xptr,
        short len, char q);
extern void vector_real_zs32_ys16_mul_xs16(volatile long *zptr, long *yptr,
        long *xptr, short len, char q);

extern void vector_complex_zs32_ys32_mac_xs32(long *zptr, long *yptr,
        long *xptr, short len, char q);
extern void vector_complex_zs32_ys32_msc_xs32(long *zptr, long *yptr,
        long *xptr, short len, char q);

extern void vector_complex_zs32_ys32_add_xs32(long *zptr, long *yptr,
        long *xptr, short len);
extern void vector_complex_zs32_ys32_sub_xs32(long *zptr, long *yptr,
        long *xptr, short len);

extern void vector_complex_zs32_ys32_mul_xs32(long *zptr, long *yptr,
        long *xptr, short len, char q);
extern void vector_complex_zs32_ys32_mac_conj_xs32(long *zptr, long *yptr,
        long *xptr, short len, char q);
extern void vector_complex_zs32_ys32_mul_conj_xs32(long *zptr, long *yptr,
        long *xptr, short len, char q);
extern void vector_complex_zs64_ys32_mul_conj_xs32(long *zptr, long *yptr,
        long *xptr, short len, char q);

extern void vector_real_zs32_sum_xs32(long *zptr, long *xptr, short len, char q);
extern void vector_real_zs32_ys32_dotpdt_xs8(long *zptr, long *yptr,
        long *xptr, short len, char q);
extern void vector_complex_zs32_qdt_xs32(long *zptr, long *xptr, short len, char q);
extern void vector_complex_zs64_qdt_xs32(long *zptr, long *xptr, short len, char q);
extern void vector_real_smooth_zs32_xs32(long *zptr,   long *xptr,  unsigned short len, char q, long lamda);
extern void vector_real_smooth_zs64_xs64(long long *zptr,  long long *xptr,  unsigned long len, char q, long lamda);
extern void vector_real_zs32_x32_min_scale(long *zptr,  long *xptr, short len, char q, long const_dat);
extern void vector_real_zs32_x32_max_scale(long *zptr,  long *xptr, short len, char q, long const_dat);
extern void vector_real_zs64_x64_min_scale(long  long *xptr,  long long min_const, long len);
extern void vector_real_zs64_x64_max_scale(long  long *xptr,  long long max_const, long len);
extern void complex_abs_s32(unsigned long *zptr, long *xptr, unsigned long len);
#endif
/************************************************************
  Function   : sevcCharToFloat()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/02, shizhang.tang create
************************************************************/
aisp_s16_t sevcCharToFloat(const U8 *p, U32 uiIndex)
{
    S16 tmp = 0;
    p = p + (uiIndex << 1);
    tmp = *((S16 *)p);
    return (aisp_s16_t)tmp;
}

/************************************************************
  Function   : sevc24bitToFLOAT_T()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/02, shizhang.tang create
************************************************************/
aisp_s16_t sevc24bitToFloat(const U8 *p, U32 uiIndex)
{
    U32 I0 = 0;
    U32 I1 = 0;
    U32 I2 = 0;
    p = p + ((uiIndex << 1) + uiIndex);
    I0 = p[0];
    I1 = p[1];
    I1 <<= 8;
    I2 = p[2];
    I2 <<= 16;
    I0 = I0 | I1 | I2;
    return (aisp_s16_t)(I0);
}

/************************************************************
  Function   : sevcFloatTo24bit()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/02, shizhang.tang create
************************************************************/
VOID sevcFloatTo24bit(U8 *p, aisp_s16_t fData)
{
    U32 uiData;
    uiData = (U32)fData;
    p[0] = uiData & 0xFF;
    p[1] = (uiData >> 8) & 0xFF;
    p[2] = (uiData >> 16) & 0xFF;
    return ;
}

VOID sevcComplexDataShow(S8 *pcFunc, S32 iLine, aisp_cpx_s32_t *pFrame, U16 usLen)
{
    U16 usLoop = 0;
    MSG_DBG("%s %d:\r\n", pcFunc, iLine);

    for (usLoop = 0; usLoop < usLen; usLoop++)
    {
#ifdef AISPEECH_FIXED
        MSG_DBG("%03d %d  %di\r\n", usLoop, pFrame[usLoop].real, pFrame[usLoop].imag);
#else
        MSG_DBG("%03d %13.6e  %13.6ei\r\n", usLoop, pFrame[usLoop].real, pFrame[usLoop].imag);
#endif
    }
}

VOID sevcFloatDataShow(S8 *pcFunc, S32 iLine, aisp_s16_t *pfFrame, U16 usLen)
{
    U16 usLoop = 0;
    MSG_DBG("%s %d:\r\n", pcFunc, iLine);

    for (usLoop = 0; usLoop < usLen; usLoop++)
    {
#ifdef AISPEECH_FIXED
        MSG_DBG("%03d %d\r\n", usLoop, pfFrame[usLoop]);
#else
        MSG_DBG("%03d %13.6e\r\n", usLoop, pfFrame[usLoop]);
#endif
    }
}

VOID sevcShortDataShow(S8 *pcFunc, S32 iLine, S16 *psFrame, U16 usLen)
{
    U16 usLoop = 0;
    MSG_DBG("%s %d:\r\n", pcFunc, iLine);

    for (usLoop = 0; usLoop < usLen; usLoop++)
    {
        MSG_DBG("%03d %d\r\n", usLoop, psFrame[usLoop]);
    }
}

/************************************************************
  Function   : sevcMemLocatorInfoShow()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/02, shizhang.tang create
************************************************************/
VOID memLocatorInfoShow(const S8 *pStr, U32 uiLine, MEM_LOCATOR_S *pstMemLocator)
{
#if MEM_BUFF_DBG
    MSG_DBG("%s [%d] base=%p size=%d offset=%d (base=%p size=%d offset=%d)\r\n", pStr, uiLine,
            pstMemLocator->pcBase, pstMemLocator->uiSize, pstMemLocator->uiOffset,
            pstMemLocator->pcShMemBase, pstMemLocator->uiShMemSize, pstMemLocator->uiShMemOffset);
#else
    S8 *pPoint = pstMemLocator->pcBase + pstMemLocator->uiOffset;
    MSG_DBG("%s [%d] base=%p size=%d offset=%d (%p)\r\n", pStr, uiLine,
            pstMemLocator->pcBase, pstMemLocator->uiSize, pstMemLocator->uiOffset, pPoint);
#endif
}

/************************************************************
  Function   : SEVC_ComplexAbs()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/02, shizhang.tang create
************************************************************/
aisp_s32_t SEVC_ComplexAbs(aisp_cpx_s32_t cpx)
{
    aisp_s64_t fTmp = (aisp_s64_t)cpx.real * cpx.real + (aisp_s64_t)cpx.imag * cpx.imag;
    return AISP_TSL_sqrt_xW64_yW32(fTmp);
}

/************************************************************
  Function   : sevcMemLocatorInit()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/02, shizhang.tang create
************************************************************/
VOID memLocatorInit(MEM_LOCATOR_S *pstMemLocator,
                    S8 *pcBase, S32 uiSize, S32 uiShMemSize)
{
    if (pstMemLocator)
    {
        pstMemLocator->pcBase = pcBase;
        pstMemLocator->uiSize  = uiSize;
        pstMemLocator->uiOffset  = 0;
        pstMemLocator->pcShMemBase = pcBase + uiSize;
        pstMemLocator->uiShMemSize  = uiShMemSize;
        pstMemLocator->uiShMemOffset  = 0;
    }
}

/************************************************************
  Function   : sevcMemLocatorGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/02, shizhang.tang create
************************************************************/
VOID *memLocatorGet(MEM_LOCATOR_S *pstMemLocator, U32 uiSize, U8 ucIsAlign)
{
    VOID *ptr = NULL;

    if (pstMemLocator)
    {
        if (ucIsAlign)
        {
            uiSize = SIZE_ALIGN(uiSize);
        }

        if (pstMemLocator->uiOffset + uiSize <= pstMemLocator->uiSize)
        {
            ptr = pstMemLocator->pcBase + pstMemLocator->uiOffset;
            pstMemLocator->uiOffset += uiSize;
        }
        else
        {
            MSG_DBG("%s [%d] no mem can be alloc total=%d offset=%d uiSize=%d\r\n",
                    __func__, __LINE__,
                    pstMemLocator->uiSize, pstMemLocator->uiOffset, uiSize);
        }
    }

    return ptr;
}

/************************************************************
  Function   : sevcMemLocatorShMemGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/02, shizhang.tang create
************************************************************/
VOID *memLocatorShMemGet(MEM_LOCATOR_S *pstMemLocator, U32 uiSize, U8 ucIsAlign)
{
    VOID *ptr = NULL;

    if (pstMemLocator)
    {
        if (ucIsAlign)
        {
            uiSize = SIZE_ALIGN(uiSize);
        }

        if (pstMemLocator->uiShMemOffset + uiSize <= pstMemLocator->uiShMemSize)
        {
            ptr = pstMemLocator->pcShMemBase + pstMemLocator->uiShMemOffset;
            pstMemLocator->uiShMemOffset += uiSize;
        }
        else
        {
            MSG_DBG("%s [%d] no share mem can be alloc total=%d offset=%d uiSize=%d\r\n",
                    __func__, __LINE__,
                    pstMemLocator->uiShMemSize, pstMemLocator->uiShMemOffset, uiSize);
        }
    }

    return ptr;
}

#ifdef USE_SHARE_MEM
/************************************************************
  Function   : sevcMemLocatorShMemReset()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/02, shizhang.tang create
************************************************************/
VOID memLocatorShMemReset(MEM_LOCATOR_S *pstMemLocator)
{
    if (pstMemLocator)
    {
        pstMemLocator->uiShMemOffset = 0;
    }
}
#endif

U32 sevcFftSizeConvert(U32 uiFrameSize)
{
    U32 ii;
    U32 uiSize = 1;

    for (ii = 1; ii < 32; ii++)
    {
        if (uiSize >= uiFrameSize)
        {
            break;
        }

        uiSize = uiSize << 1;
    }

    return uiSize;
}

VOID sevcVec16Add16_Real(aisp_s16_t *pDstZ, S16 *pSrcY, S16 *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    //vector_real_zs32_ys32_add_xs32(pDstZ, pSrcY, pSrcX, uiLen);
    ///
#else
    S32 iLoop;
    S32 iDataTmp;

    if (rq)
    {
        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            pDstZ[iLoop] = PSHR((aisp_s32_t)((aisp_s32_t)(pSrcX[iLoop])
                                             + (aisp_s32_t)(pSrcY[iLoop])), rq);
        }
    }
    else
    {
        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            iDataTmp = pSrcX[iLoop] + pSrcY[iLoop];
            iDataTmp = (iDataTmp > 32767 ? 32767 : iDataTmp);
            pDstZ[iLoop] = (iDataTmp < -32767 ? -32767 : iDataTmp);
        }
    }

#endif
}

VOID sevcVec32Add32_Real(aisp_s32_t *pDstZ, S32 *pSrcY, S32 *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_real_zs32_ys32_add_xs32(pDstZ, pSrcY, pSrcX, uiLen);
#else
    S32 iLoop;

    if (rq)
    {
        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            pDstZ[iLoop] = PSHR((aisp_s64_t)((aisp_s64_t)(pSrcX[iLoop])
                                             + (aisp_s64_t)(pSrcY[iLoop])), rq);
        }
    }
    else
    {
        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            pDstZ[iLoop] = pSrcX[iLoop] + pSrcY[iLoop];
        }
    }

#endif
}

VOID sevcVec32Sub32_Real(aisp_s32_t *pDstZ, S32 *pSrcY, S32 *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_real_zs32_ys32_sub_xs32(pDstZ, pSrcY, pSrcX, uiLen);
#else
    S32 iLoop;

    for (iLoop = 0; iLoop < uiLen; iLoop++)
    {
        pDstZ[iLoop] = pSrcY[iLoop] - pSrcX[iLoop];
    }

#endif
}

VOID sevcVecMulScalorShiftLeft(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                               aisp_s32_t srcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    rq = (rq < 0 ? -rq : rq);
#ifdef JIELI_BR28
    vector_real_scale_s32(pDstZ, pSrcY, srcX, uiLen, -rq);
#else
    S32 iLoop;

    if (rq)
    {
        if (1 == srcX)
        {
            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                pDstZ[iLoop] = SHL(pSrcY[iLoop], rq);
            }
        }
        else
        {
            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                pDstZ[iLoop] = SHL(MULT32_32(srcX, pSrcY[iLoop]), rq);
            }
        }
    }
    else
    {
        if (!(1 == srcX && pDstZ == pSrcY))
        {
            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                pDstZ[iLoop] = MULT32_32(srcX, pSrcY[iLoop]);
            }
        }
    }

#endif
}

VOID sevcVecMulScalorShiftRight(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                                aisp_s32_t srcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    rq = (rq < 0 ? -rq : rq);
#ifdef JIELI_BR28
    vector_real_scale_s32(pDstZ, pSrcY, srcX, uiLen, rq);
#else
    S32 iLoop;

    if (rq)
    {
        if (1 == srcX)
        {
            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                pDstZ[iLoop] = PSHR(pSrcY[iLoop], rq);
            }
        }
        else
        {
            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                pDstZ[iLoop] = MULT32_32_SQ(srcX, pSrcY[iLoop], rq);
            }
        }
    }
    else
    {
        if (!(1 == srcX && pDstZ == pSrcY))
        {
            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                pDstZ[iLoop] = MULT32_32(srcX, pSrcY[iLoop]);
            }
        }
    }

#endif
}

VOID sevcVecMUL16X16_Real(aisp_s32_t *pDstZ, aisp_s16_t *pSrcY,
                          aisp_s16_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_real_zs32_ys16_mul_xs16(pDstZ, pSrcY, pSrcX, uiLen, rq);
#else
    S32 iLoop;

    if (rq)
    {
        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            pDstZ[iLoop] = PSHR(MULT16_16(pSrcX[iLoop], pSrcY[iLoop]), rq);
        }
    }
    else
    {
        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            pDstZ[iLoop] = MULT16_16(pSrcX[iLoop], pSrcY[iLoop]);
        }
    }

#endif
}

VOID sevcVecMUL32X16_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                          aisp_s16_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_real_zs32_ys32_mul_xs16(pDstZ, pSrcY, pSrcX, uiLen, rq);
#else
    S32 iLoop;

    if (rq)
    {
        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            pDstZ[iLoop] = PSHR(MULT16_32(pSrcX[iLoop], pSrcY[iLoop]), rq);
        }
    }
    else
    {
        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            //pDstZ[iLoop] = (aisp_s32_t)MULT16_32(pSrcX[iLoop], pSrcY[iLoop]);
            pDstZ[iLoop] = pSrcX[iLoop] * pSrcY[iLoop];
        }
    }

#endif
}

VOID sevcVecMUL32X32_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                          aisp_s32_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_real_zs32_ys32_mul_xs32(pDstZ, pSrcY, pSrcX, uiLen, rq);
#else
    S32 iLoop;

    if (rq)
    {
        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            pDstZ[iLoop] = PSHR(MULT32_32(pSrcX[iLoop], pSrcY[iLoop]), rq);
        }
    }
    else
    {
        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            pDstZ[iLoop] = pSrcX[iLoop] * pSrcY[iLoop];
        }
    }

#endif
}

VOID sevcVecComplexMulReal(aisp_cpx_s32_t *pDstZ, aisp_cpx_s32_t *pSrcY,
                           aisp_s32_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#if 0 //def JIELI_BR28
    vector_complex_zs32_ys32_mul_xs32(pDstZ, pSrcY, pSrcX, uiLen, rq);
#else
    S32 ii;

    for (ii = 0; ii < uiLen; ii++)
    {
        pDstZ[ii].real = PSHR64(MULT32_32(pSrcY[ii].real, pSrcX[ii]), rq);
        pDstZ[ii].imag = PSHR64(MULT32_32(pSrcY[ii].imag, pSrcX[ii]), rq);
    }

#endif
}

VOID sevcVecMUL_Complex(aisp_cpx_s32_t *pDstZ, aisp_cpx_s32_t *pSrcY,
                        aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_complex_zs32_ys32_mul_xs32(pDstZ, pSrcY, pSrcX, uiLen, rq);
#else
    FIX_HERE();
#endif
}

VOID sevcVecConjMUL_Complex(aisp_cpx_s32_t *pDstZ, aisp_cpx_s32_t *pSrcY,
                            aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_complex_zs32_ys32_mul_conj_xs32(pDstZ, pSrcY, pSrcX, uiLen, rq);
#else
    S32 ii;

    for (ii = 0; ii < uiLen; ii++)
    {
        pDstZ[ii].real = PSHR64(((aisp_s64_t)pSrcY[ii].real * pSrcX[ii].real
                                 + (aisp_s64_t)pSrcY[ii].imag * pSrcX[ii].imag), rq); //W32Q24
        pDstZ[ii].imag = PSHR64(((aisp_s64_t)pSrcY[ii].imag * pSrcX[ii].real
                                 - (aisp_s64_t)pSrcY[ii].real * pSrcX[ii].imag), rq); //W32Q24
    }

#endif
}

VOID sevcVec32ConjMUL_Complex(aisp_cpx_s32_t *pDstZ, aisp_cpx_s32_t *pSrcY,
                              aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_complex_zs32_ys32_mul_conj_xs32(pDstZ, pSrcY, pSrcX, uiLen, rq);
#else
    S32 ii;

    for (ii = 0; ii < uiLen; ii++)
    {
        pDstZ[ii].real = SHR((aisp_s64_t)((aisp_s64_t)pSrcY[ii].real * pSrcX[ii].real
                                          + (aisp_s64_t)pSrcY[ii].imag * pSrcX[ii].imag), rq); //W32Q24
        pDstZ[ii].imag = SHR((aisp_s64_t)((aisp_s64_t)pSrcY[ii].imag * pSrcX[ii].real
                                          - (aisp_s64_t)pSrcY[ii].real * pSrcX[ii].imag), rq); //W32Q24
    }

#endif
}

VOID sevcVec64ConjMUL_Complex(aisp_cpx_s64_t *pDstZ, aisp_cpx_s32_t *pSrcY,
                              aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_complex_zs64_ys32_mul_conj_xs32(pDstZ, pSrcY, pSrcX, uiLen, rq);
#else
    S32 ii;

    for (ii = 0; ii < uiLen; ii++)
    {
        pDstZ[ii].real = PSHR64(((aisp_s64_t)pSrcY[ii].real * pSrcX[ii].real
                                 + (aisp_s64_t)pSrcY[ii].imag * pSrcX[ii].imag), rq); //W32Q24
        pDstZ[ii].imag = PSHR64(((aisp_s64_t)pSrcY[ii].imag * pSrcX[ii].real
                                 - (aisp_s64_t)pSrcY[ii].real * pSrcX[ii].imag), rq); //W32Q24
    }

#endif
}


VOID sevcVecAdd_Complex(aisp_cpx_s32_t *pDstZ, aisp_cpx_s32_t *pSrcY,
                        aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_complex_zs32_ys32_add_xs32(pDstZ, pSrcY, pSrcX, uiLen);
#else
    S32 iLoop;

    for (iLoop = 0; iLoop < uiLen; iLoop++)
    {
        COMPLEX_ADD(pDstZ[iLoop], pSrcY[iLoop], pSrcX[iLoop]);
    }

#endif
}

//Z = Z + Y.*X
VOID sevcVecMLA_Complex(aisp_cpx_s32_t *pDstZ, aisp_cpx_s32_t *pSrcY,
                        aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_complex_zs32_ys32_mac_xs32(pDstZ, pSrcY, pSrcX, uiLen, rq);
#else
    S32 iLoop;
    aisp_cpx_s32_t tmpData;

    for (iLoop = 0; iLoop < uiLen; iLoop++)
    {
        COMPLEX_MUL_SQ(tmpData, pSrcY[iLoop], pSrcX[iLoop], rq);
        COMPLEX_ADD(pDstZ[iLoop], pDstZ[iLoop], tmpData);
    }

#endif
}

//Z = Z - Y.*X
VOID sevcVecMLS_Complex(aisp_cpx_s32_t *pDstZ, aisp_cpx_s32_t *pSrcY,
                        aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_complex_zs32_ys32_msc_xs32(pDstZ, pSrcY, pSrcX, uiLen, rq);
#else
    S32 iLoop;
    aisp_cpx_s32_t tmpData;

    for (iLoop = 0; iLoop < uiLen; iLoop++)
    {
        COMPLEX_MUL_SQ(tmpData, pSrcY[iLoop], pSrcX[iLoop], rq);
        COMPLEX_SUB(pDstZ[iLoop], pDstZ[iLoop], tmpData);
    }

#endif
}

//Z = Y-X
VOID sevcVecSub_Complex(aisp_cpx_s32_t *pDstZ, aisp_cpx_s32_t *pSrcY,
                        aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_complex_zs32_ys32_sub_xs32(pDstZ, pSrcY, pSrcX, uiLen);
#else
    S32 iLoop;

    for (iLoop = 0; iLoop < uiLen; iLoop++)
    {
        COMPLEX_SUB(pDstZ[iLoop], pSrcY[iLoop], pSrcX[iLoop]);
    }

    if (rq)
    {
        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            pDstZ[iLoop].real = PSHR(pDstZ[iLoop].real, rq);
            pDstZ[iLoop].imag = PSHR(pDstZ[iLoop].imag, rq);
        }
    }

#endif
}


//Z = Y.*Conj(X)/scale
VOID sevcVecConjMLA_Complex(aisp_cpx_s32_t *pDstZ, aisp_cpx_s32_t *pSrcY,
                            aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
    //MSG_DBG("%s need fix +++++++\r\n", __func__);
#ifdef JIELI_BR28
    vector_complex_zs32_ys32_mac_conj_xs32(pDstZ, pSrcY, pSrcX, uiLen, rq);
#else
    S32 iLoop;
    aisp_cpx_s32_t tmpData;

    for (iLoop = 0; iLoop < uiLen; iLoop++)
    {
        COMPLEX_CONJ_MUL_SQ(tmpData, pSrcY[iLoop], pSrcX[iLoop], rq);
        COMPLEX_ADD(pDstZ[iLoop], pDstZ[iLoop], tmpData);
    }

#endif
}


VOID sevcVecPowY32X32_Complex(aisp_s32_t *pDstY, aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_complex_zs32_qdt_xs32(pDstY, pSrcX, uiLen, rq);
#else
    S32 iLoop;

    for (iLoop = 0; iLoop < uiLen; iLoop++)
    {
        COMPLEX_POW_SQ(pDstY[iLoop], pSrcX[iLoop], rq);
    }

#endif
}

VOID sevcVecPowY64X32_Complex(aisp_s64_t *pDstY, aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_complex_zs64_qdt_xs32(pDstY, pSrcX, uiLen, rq);
#else
    S32 iLoop;

    if (rq == 0)
    {
        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            pDstY[iLoop] = MULT32_32(pSrcX[iLoop].real, pSrcX[iLoop].real)
                           + MULT32_32(pSrcX[iLoop].imag , pSrcX[iLoop].imag);
        }
    }
    else
    {
        if (rq < 0)
        {
            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                pDstY[iLoop] = SHL((MULT32_32(pSrcX[iLoop].real, pSrcX[iLoop].real)
                                    + MULT32_32(pSrcX[iLoop].imag , pSrcX[iLoop].imag)), rq);
            }
        }
        else
        {
            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                pDstY[iLoop] = PSHR64((MULT32_32(pSrcX[iLoop].real, pSrcX[iLoop].real)
                                       + MULT32_32(pSrcX[iLoop].imag , pSrcX[iLoop].imag)), rq);
            }
        }
    }

#endif
}

VOID sevcVecPowY64X64_Complex(aisp_s64_t *pDstY, aisp_cpx_s64_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstY);
    ADDR_CHECK(pSrcX);
#if 0 //def JIELI_BR28
    vector_complex_zs64_qdt_xs32(pDstZ, pSrcX, uiLen, rq);
#else
    S32 iLoop;

    if (rq == 0)
    {
        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            pDstY[iLoop] = MULT32_32(pSrcX[iLoop].real, pSrcX[iLoop].real)
                           + MULT32_32(pSrcX[iLoop].imag , pSrcX[iLoop].imag);
        }
    }
    else
    {
        if (rq < 0)
        {
            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                pDstY[iLoop] = SHL((MULT32_32(pSrcX[iLoop].real, pSrcX[iLoop].real)
                                    + MULT32_32(pSrcX[iLoop].imag , pSrcX[iLoop].imag)), rq);
            }
        }
        else
        {
            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                pDstY[iLoop] = PSHR64((MULT32_32(pSrcX[iLoop].real, pSrcX[iLoop].real)
                                       + MULT32_32(pSrcX[iLoop].imag , pSrcX[iLoop].imag)), rq);
            }
        }
    }

#endif
}

VOID sevcVecAbs32_Complex(aisp_s32_t *pDstY, aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 iInQ)
{
#ifdef JIELI_BR28
    complex_abs_s32(pDstY, pSrcX, uiLen);
#else
    S32 ii;
    aisp_s64_t llData;

    for (ii = 0; ii < uiLen; ii++)
    {
        llData = (aisp_s64_t)pSrcX[ii].real * pSrcX[ii].real
                 + (aisp_s64_t)pSrcX[ii].imag * pSrcX[ii].imag;
        pDstY[ii] = AISP_TSL_sqrt_xW64_yW32(llData);
    }

#endif
}

VOID sevcVecAbs64_Complex(aisp_s64_t *pDstY, aisp_cpx_s64_t *pSrcX, U32 uiLen, S32 iInQ)
{
    S32 ii;
    S32 iQ;
    aisp_s64_t llReal, llImag;

    if (iInQ > 20)
    {
        iQ = iInQ - 20;

        for (ii = 0; ii < uiLen; ii++)
        {
            llReal = PSHR(pSrcX[ii].real, iQ); //Q20
            llImag = PSHR(pSrcX[ii].imag, iQ);
            llReal = llReal * llReal + llImag * llImag; //W64Q40
            pDstY[ii] = SHL(AISP_TSL_sqrt_xW64_yW32(llReal), iQ);  //W32Q24
        }
    }
    else
    {
        for (ii = 0; ii < uiLen; ii++)
        {
            llReal = pSrcX[ii].real * pSrcX[ii].real + pSrcX[ii].imag * pSrcX[ii].imag;
            pDstY[ii] = AISP_TSL_sqrt_xW64_yW32(llReal);
        }
    }
}

VOID sevcVec32Smooth(aisp_s32_t *pDstZ, aisp_s32_t *pSrcX, S32 iLamda, U32 uiLen, S32 rq)
{
#ifdef JIELI_BR28
    vector_real_smooth_zs32_xs32(pDstZ, pSrcX, uiLen, rq, iLamda);
#else
    S32 ii;
    aisp_s64_t dataTmp;

    for (ii = 0; ii < uiLen; ii++)
    {
        dataTmp = (aisp_s64_t)pDstZ[ii] - (aisp_s64_t)pSrcX[ii];
        pDstZ[ii] = PSHR((aisp_s64_t)(dataTmp * iLamda), rq) + pSrcX[ii];
    }

#endif
}

VOID sevcVec64Smooth(aisp_s64_t *pDstZ, aisp_s64_t *pSrcX, S32 iLamda, U32 uiLen, S32 rq)
{
#ifdef JIELI_BR28
    vector_real_smooth_zs64_xs64(pDstZ, pSrcX, uiLen, rq, iLamda);
#else
    S32 ii;

    for (ii = 0; ii < uiLen; ii++)
    {
        pDstZ[ii] = PSHR64(((pDstZ[ii] - pSrcX[ii]) * iLamda), rq) + pSrcX[ii];
    }

#endif
}

VOID sevcVec32MaxRestrain(S32 *pSrcX, S32 iMax, U32 uiLen)
{
#ifdef JIELI_BR28
    vector_real_zs32_x32_min_scale(pSrcX, pSrcX, uiLen, 0, iMax);
#else
    S32 ii;

    for (ii = 0; ii < uiLen; ii++)
    {
        pSrcX[ii] = (pSrcX[ii] > iMax ? iMax : pSrcX[ii]);
    }

#endif
}

VOID sevcVec32MinRestrain(S32 *pSrcX, S32 iMin, U32 uiLen)
{
#ifdef JIELI_BR28
    vector_real_zs32_x32_max_scale(pSrcX, pSrcX, uiLen, 0, iMin);
#else
    S32 ii;

    for (ii = 0; ii < uiLen; ii++)
    {
        pSrcX[ii] = (pSrcX[ii] < iMin ? iMin : pSrcX[ii]);
    }

#endif
}

VOID sevcVec64MaxRestrain(S64 *pSrcX, S64 illMax, U32 uiLen)
{
#ifdef JIELI_BR28
    vector_real_zs64_x64_min_scale(pSrcX, illMax, uiLen);
#else
    S32 ii;

    for (ii = 0; ii < uiLen; ii++)
    {
        pSrcX[ii] = (pSrcX[ii] > illMax ? illMax : pSrcX[ii]);
    }

#endif
}

VOID sevcVec64MinRestrain(S64 *pSrcX, S64 illMin, U32 uiLen)
{
#ifdef JIELI_BR28
    vector_real_zs64_x64_max_scale(pSrcX, illMin, uiLen);
#else
    S32 ii;

    for (ii = 0; ii < uiLen; ii++)
    {
        pSrcX[ii] = (pSrcX[ii] < illMin ? illMin : pSrcX[ii]);
    }

#endif
}


/************************************************************
  Function   : fftInit()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/11/15, Youhai.Jiang create

************************************************************/
S32 fftInit(AISP_TSL_FFTENG_S *pstFftEng, S32 fftSize)
{
#ifndef JIELI_BR28
    AISP_TSL_fft_alloc(pstFftEng, fftSize);
#endif
    return 0;
}



/************************************************************
  Function   : fftDelete()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/11/15, Youhai.Jiang create

************************************************************/
VOID fftDelete(AISP_TSL_FFTENG_S *pstFftEng)
{
#ifndef JIELI_BR28
    AISP_TSL_fft_free(pstFftEng);
#endif
}

//OUT:W32Q24
aisp_s32_t sevcLog(aisp_s32_t iData, aisp_s32_t iInQ, S32 iOutQ)
{
    aisp_s64_t llY = 0;
#ifdef JIELI_BR28
    llY = ln_fix(iData, iInQ);//W64Q28
    return (aisp_s32_t)(llY >> (28 - iOutQ));
#else

    if (iOutQ == 24)
    {
        llY = AISP_TSL_log_xW32_yW32Q24(iData, iInQ);
    }
    else
    {
        if (20 == iOutQ)
        {
            llY = AISP_TSL_log_xW32_yW32Q20(iData, iInQ);
        }
        else
        {
            llY = AISP_TSL_log_xW32_yW32Q20(iData, iInQ);
            llY = llY << (iOutQ - 20);
        }
    }

    //return (aisp_s32_t)(llY<<(iOutQ-24));
    return (aisp_s32_t)llY;
#endif
}

//OUT W32Q24
aisp_s32_t sevcExp(aisp_s32_t iData, S32 iInQ, S32 iOutQ)
{
#ifdef JIELI_BR28
    aisp_s64_t llY;
    aisp_s32_t iM, iN;
    llY = exp_fix(iData, iInQ);
    iN = llY & 0xFFFFFFFF;
    iM = (llY >> 32) & 0xFFFFFFFF;
    return (iN >= 4 ? iM << (iN - 4) : iM >> (4 - iN));
#else
    return AISP_TSL_exp_xW32Q24_yW32(iData, iOutQ);
#endif
}


//W32Q24
aisp_s32_t sevcSin(aisp_s32_t iData)
{
    aisp_s32_t f2PI = 105414357; //W32Q24
    aisp_s32_t fx;
    aisp_s32_t fDataTmp;
    S8 sFlag = (0 < iData ? 1 : -1);
    fx = sevcAbsf(iData) % f2PI;

    if (0 == fx || 52707178 == fx)
    {
        return 0;
    }

    //PI/2
    if (26353589 > fx)
    {
        return sFlag * AISP_TSL_sin_xW32Q24_yW32Q24(fx);
    }

    if (26353589 == fx)
    {
        return sFlag;
    }

    //0<fx<PI
    if (52707178 > fx)
    {
        //sin(PI/2+x) = cos(x)
        fDataTmp = AISP_TSL_sin_xW32Q24_yW32Q24(fx - 26353589);
        return sFlag * AISP_TSL_sqrt_xW64_yW32(281474976710656 - (aisp_s64_t)fDataTmp * fDataTmp);
    }

    //PI<fx<3*PI/2
    if (79060768 > fx)
    {
        //sin(PI+x) = -sin(x)
        fDataTmp = AISP_TSL_sin_xW32Q24_yW32Q24(fx - 52707178);
        sFlag = -1 * sFlag;
        return sFlag * fDataTmp;
    }

    //3*PI/2
    if (79060768 == fx)
    {
        return -1 * sFlag;
    }

    //fx>3*PI/2
    if (fx >= 79060768)
    {
        sFlag = -1 * sFlag;
        return sFlag * AISP_TSL_sin_xW32Q24_yW32Q24(f2PI - fx);
    }

    return 0;
}

VOID SevcSortDescend(aisp_s32_t *arr, aisp_s32_t len)
{
    U16 i, j;

    for (i = 1; i < len; ++i)
    {
        aisp_s32_t tmp = arr[i];

        for (j = i; j > 0 && arr[j - 1] < tmp; j--)
        {
            arr[j] = arr[j - 1];
        }

        arr[j] = tmp;
    }
}

VOID SevcSortDescendLongLong(aisp_s64_t *arr, aisp_s32_t len)
{
    U16 i, j;

    for (i = 1; i < len; ++i)
    {
        aisp_s64_t tmp = arr[i];

        for (j = i; j > 0 && arr[j - 1] < tmp; j--)
        {
            arr[j] = arr[j - 1];
        }

        arr[j] = tmp;
    }
}
