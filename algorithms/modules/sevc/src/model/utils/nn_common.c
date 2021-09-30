#include <stdio.h>
#include "AISP_TSL_base.h"
#include "sevc_types.h"
#include "AISP_TSL_sigmoid.h"
#include "sevc_util.h"
#include "nn_common.h"

#if SEVC_SWITCH_NN

#define NN_STATISTIC        0
#define NN_ADDR_CHECK       0
#define NN_DEBUG            0

//#define FIX_HERE()          MSG_DBG("%s [%d] please fix the code here\r\n", __func__, __LINE__)
//#define DBG_HERE()          MSG_DBG("%s [%d] +++++++++++\r\n", __func__, __LINE__)


#if NN_STATISTIC
S32 g_logsigX_Max = 0;
S32 g_logsigX_Min = 1 << 28;

S32 g_tansigX_Max = 0;
S32 g_tansigX_Min = 1 << 28;
S32 g_CalcCount = 0;

S32 g_nnVec32Add32_Real_Count = 0;
S32 g_nnVecMul32X8_Real_Count = 0;
S32 g_nnVecMul32X16_Real_Count = 0;

S32 g_nnVecMul32X32_Real_Count = 0;
S32 g_nnVecMulSum32X8_Real_Count = 0;
S32 g_nnVecMulSum32X16_Real_Count = 0;
S32 g_nnVecMulSum32X32_Real_Count = 0;
S32 g_nnVecMulAdd32X8_Real_Count = 0;
S32 g_nnVecMulAdd32X16_Real_Count = 0;
S32 g_nnVecMulAdd32X32_Real_Count = 0;
S32 g_nnLinearCalc_Count = 0;

#endif

#if NN_ADDR_CHECK
#define ADDR_CHECK(_addr_) do{if(((S64)_addr_)%4 != 0){\
    MSG_DBG("%s [%d] addr=%p fail\r\n", __func__, __LINE__, _addr_);\
    exit(0);}}while(0)
#else
#define ADDR_CHECK(_addr_)
#endif

#ifdef JIELI_BR28
extern long long exp_fix(int data, char iq);
extern int logsig_q24(int iData);
extern int tansig_q24(int iData);
extern void vector_real_zs32_ys32_add_xs32(volatile long *zptr, long *yptr,
        long *xptr, short len);
extern void vector_real_zs32_ys32_mul_xs8(long *zptr, long *yptr,
        long *xptr, short len, char q);
extern void vector_real_zs32_ys32_mul_xs16(long *zptr, long *yptr,
        long *xptr, short len, char q);
extern void vector_real_zs32_ys32_mul_xs32(long *zptr, long *yptr,
        long *xptr, short len, char q);
extern void vector_real_zs32_ys32_dotpdt_xs8(long *zptr, long *yptr,
        long *xptr, short len, char q);
#endif

int AISP_TSL_tanh_xW32Q24_yW32Q24(int x)
{
    int iXx2    = 0;
    int iResult = 0;
    int iTmp    = 0;
    int iAbsX   = NN_ABS(x);

    if (iAbsX >= (S32)167772160)
    {
        iAbsX = (S32)167772159;
    }

    iXx2  = iAbsX * 2;
    iTmp = (S32)16777216 + AISP_TSL_exp_minus_xW32Q24_yW32(iXx2, 24);
    iResult = (((long long)2 << 48) + (iTmp >> 1)) / iTmp - (S32)16777216;

    if (x < 0)
    {
        iResult = -iResult;
    }

    return iResult;
}

//x:
#if 0 //def SEVC_PHONE
aisp_s32_t nnSigmoidApprox(aisp_s32_t iX, S32 iInQ, S32 iOutQ)
{
    MSG_DBG("%s not support!\r\n", __func__);
    return 0;
}
#else
aisp_s32_t nnSigmoidApprox(aisp_s32_t iX, S32 iInQ, S32 iOutQ)
{
#if NN_STATISTIC

    if (g_logsigX_Max < iX)
    {
        g_logsigX_Max = iX;
    }
    else
        if (g_logsigX_Min > iX)
        {
            g_logsigX_Min = iX;
        }

#endif
    S32 iY;
    S32 iMax;
    S32 iQDiff;
    S32 aiMax[] =
    {
        20,         40,         80,         160,        320,        640,        1280,       2560,
        5120,       10240,      20480,      40960,      81920,      163840,     327680,     655360,
        1310720,    2621440,    5242880,    10485760,   20971520,   41943040,   83886080,   167772160,
        335544320,  671088640,  1342177280
    };

    if (26 < iInQ)
    {
        MSG_DBG("%s [%d] iInQ =%d not support\r\n", __func__, __LINE__, iInQ);
        return 0;
    }

    iMax = aiMax[iInQ];

    if (iX >= iMax)
    {
        return (1 << iOutQ);
    }
    else
        if (iX <= -iMax)
        {
            return 0;
        }

    if (24 == iInQ)
    {
#ifdef JIELI_BR28
        iY = logsig_q24(iX);
#else
        iY = AISP_TSL_sigmoid_xW32Q24_yW32Q24(iX);
#endif
    }
    else
    {
        iQDiff = 24 - iInQ;
        iX = (iQDiff > 0 ? iX << iQDiff : iX>>iQDiff);
#ifdef JIELI_BR28
        iY = logsig_q24(iX);
#else
        iY = AISP_TSL_sigmoid_xW32Q24_yW32Q24(iX);
#endif
    }

    if (24 == iOutQ)
    {
        return iY;
    }

    return (iOutQ > 24 ? iY << (iOutQ - 24) : iY >> (24 - iOutQ));
}
#endif
aisp_s32_t nnTansigApprox(aisp_s32_t iX, S32 iInQ, S32 iOutQ)
{
#if NN_STATISTIC

    if (g_tansigX_Max < iX)
    {
        g_tansigX_Max = iX;
    }
    else
        if (g_tansigX_Min > iX)
        {
            g_tansigX_Min = iX;
        }

#endif
#if 0 //def JIELI_BR28
    int iAbsX   = 0;
    int iXx2    = 0;
    int iResult = 0;
    aisp_s64_t iTmp    = 0;
    iAbsX = NN_ABS(x);
    iXx2  = iAbsX * 2;
    aisp_s64_t llY;
    aisp_s32_t iM, iN;
    llY = exp_fix(iXx2, 24);
    iN = llY & 0xFFFFFFFF;
    iM = (llY >> 32) & 0xFFFFFFFF;
    //return (iN>=4 ? iM<<(iN-4) : iM>>(4-iN));
    llY = iM << iN; //W64Q28
    iTmp = 268435456 + llY;
    iResult = (((long long)2 << 52) + (iTmp >> 1)) / iTmp - 16777216;

    if (x < 0)
    {
        iResult = -iResult;
    }

#else
    return AISP_TSL_tanh_xW32Q24_yW32Q24(iX);
    //return tanh(x);
#endif
}

aisp_s32_t nnRelu(aisp_s32_t iX, S32 iInQ, S32 iOutQs)
{
    return iX < 0 ? 0 : iX;
}

VOID nnActivation(aisp_s32_t *pOutput, aisp_s32_t *pInput, U32 uiLen, U8 ucAction,
                  U32 usReluMax, S32 iInQ, S32 iOutQ)
{
    S32 iLoop;
    aisp_s32_t xTmp;
    aisp_s32_t (*pActFuncArr[3])(aisp_s32_t, S32, S32) =
    {
        nnSigmoidApprox, nnTansigApprox, nnRelu
    };

    if (ucAction < ACTIVATION_NONE)
    {
        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            xTmp = pInput[iLoop];
            pOutput[iLoop] = pActFuncArr[ucAction](xTmp, iInQ, iOutQ);
        }
    }
    else
    {
        if (ucAction > ACTIVATION_NONE)
        {
            MSG_DBG("%s:%d: %d The activation function is not supported.\n", __func__, __LINE__, ucAction);
        }
    }

    if (ACTIVATION_RELU == ucAction && usReluMax != 0)
    {
        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            pOutput[iLoop] = (pOutput[iLoop] < usReluMax ? pOutput[iLoop] : usReluMax);
        }
    }
}
/************************************************************
  Function   : nnVec32Add32_Real()

  Description: Z = Y + X
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
VOID nnVec32Add32_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                       aisp_s32_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_real_zs32_ys32_add_xs32(pDstZ, pSrcY, pSrcX, uiLen);
#else
    S32 iLoop;
    S32 iLenTmp = uiLen >> 2;
    uiLen = (uiLen & 0x03);

    if (0 < rq)
    {
        for (iLoop = 0; iLoop < iLenTmp; iLoop++)
        {
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++ + *pSrcX++)) >> rq;
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++ + *pSrcX++)) >> rq;
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++ + *pSrcX++)) >> rq;
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++ + *pSrcX++)) >> rq;
        }

        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++ + *pSrcX++)) >> rq;
        }
    }
    else
    {
        if (0 == rq)
        {
            for (iLoop = 0; iLoop < iLenTmp; iLoop++)
            {
                *pDstZ++ = *pSrcY++ + *pSrcX++;
                *pDstZ++ = *pSrcY++ + *pSrcX++;
                *pDstZ++ = *pSrcY++ + *pSrcX++;
                *pDstZ++ = *pSrcY++ + *pSrcX++;
            }

            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                *pDstZ++ = *pSrcY++ + *pSrcX++;
            }
        }
        else
        {
            rq = -rq;

            for (iLoop = 0; iLoop < iLenTmp; iLoop++)
            {
                *pDstZ++ = (*pSrcY++ + *pSrcX++) << rq;
                *pDstZ++ = (*pSrcY++ + *pSrcX++) << rq;
                *pDstZ++ = (*pSrcY++ + *pSrcX++) << rq;
                *pDstZ++ = (*pSrcY++ + *pSrcX++) << rq;
            }

            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                *pDstZ++ = (*pSrcY++ + *pSrcX++) << rq;
            }
        }
    }

#endif
#if NN_STATISTIC
    g_nnVec32Add32_Real_Count++;
#endif
}

/************************************************************
  Function   : nnVecMul32X8_Real()

  Description: Z = Y.*X
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
VOID nnVecMul32X8_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                       aisp_s8_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_real_zs32_ys32_mul_xs8(pDstZ, pSrcY, pSrcX, uiLen, rq);
#else
    S32 iLoop;
    S32 iLenTmp = uiLen >> 2;
    uiLen = (uiLen & 0x03);

    if (0 < rq)
    {
        for (iLoop = 0; iLoop < iLenTmp; iLoop++)
        {
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq;
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq;
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq;
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq;
        }

        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq;
        }
    }
    else
    {
        if (0 == rq)
        {
            for (iLoop = 0; iLoop < iLenTmp; iLoop++)
            {
                *pDstZ++ = (*pSrcY++) * (*pSrcX++);
                *pDstZ++ = (*pSrcY++) * (*pSrcX++);
                *pDstZ++ = (*pSrcY++) * (*pSrcX++);
                *pDstZ++ = (*pSrcY++) * (*pSrcX++);
            }

            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                *pDstZ++ = (*pSrcY++) * (*pSrcX++);
            }
        }
        else
        {
            rq = -rq;

            for (iLoop = 0; iLoop < iLenTmp; iLoop++)
            {
                *pDstZ++ = ((*pSrcY++) * (*pSrcX++)) << rq;
                *pDstZ++ = ((*pSrcY++) * (*pSrcX++)) << rq;
                *pDstZ++ = ((*pSrcY++) * (*pSrcX++)) << rq;
                *pDstZ++ = ((*pSrcY++) * (*pSrcX++)) << rq;
            }

            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                *pDstZ++ = ((*pSrcY++) * (*pSrcX++)) << rq;
            }
        }
    }

#endif
#if NN_STATISTIC
    g_nnVecMul32X8_Real_Count++;
#endif
}

/************************************************************
  Function   : nnVecMul32X16_Real()

  Description: Z = Y.*X
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
VOID nnVecMul32X16_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                        aisp_s16_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_real_zs32_ys32_mul_xs16(pDstZ, pSrcY, pSrcX, uiLen, rq);
#else
    S32 iLoop;
    S32 iLenTmp = uiLen >> 2;
    uiLen = (uiLen & 0x03);

    if (0 < rq)
    {
        for (iLoop = 0; iLoop < iLenTmp; iLoop++)
        {
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq;
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq;
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq;
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq;
        }

        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq;
        }
    }
    else
    {
        if (0 == rq)
        {
            for (iLoop = 0; iLoop < iLenTmp; iLoop++)
            {
                *pDstZ++ = (*pSrcY++) * (*pSrcX++);
                *pDstZ++ = (*pSrcY++) * (*pSrcX++);
                *pDstZ++ = (*pSrcY++) * (*pSrcX++);
                *pDstZ++ = (*pSrcY++) * (*pSrcX++);
            }

            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                *pDstZ++ = (*pSrcY++) * (*pSrcX++);
            }
        }
        else
        {
            rq = -rq;

            for (iLoop = 0; iLoop < iLenTmp; iLoop++)
            {
                *pDstZ++ = ((*pSrcY++) * (*pSrcX++)) << rq;
                *pDstZ++ = ((*pSrcY++) * (*pSrcX++)) << rq;
                *pDstZ++ = ((*pSrcY++) * (*pSrcX++)) << rq;
                *pDstZ++ = ((*pSrcY++) * (*pSrcX++)) << rq;
            }

            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                *pDstZ++ = ((*pSrcY++) * (*pSrcX++)) << rq;
            }
        }
    }

#endif
#if NN_STATISTIC
    g_nnVecMul32X16_Real_Count++;
#endif
}

/************************************************************
  Function   : nnVecMul32X32_Real()

  Description: Z = Y.*X
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
VOID nnVecMul32X32_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                        aisp_s32_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    //sevcVec32MUL32_Real(pDstZ, pSrcY, pSrcX, uiLen, rq); //W32Q24
    vector_real_zs32_ys32_mul_xs32(pDstZ, pSrcY, pSrcX, uiLen, rq);
#else
    S32 iLoop;
    S32 iLenTmp = uiLen >> 2;
    uiLen = (uiLen & 0x03);

    if (0 < rq)
    {
        for (iLoop = 0; iLoop < iLenTmp; iLoop++)
        {
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq;
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq;
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq;
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq;
        }

        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            *pDstZ++ = ((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq;
        }
    }
    else
    {
        if (0 == rq)
        {
            for (iLoop = 0; iLoop < iLenTmp; iLoop++)
            {
                *pDstZ++ = (*pSrcY++) * (*pSrcX++);
                *pDstZ++ = (*pSrcY++) * (*pSrcX++);
                *pDstZ++ = (*pSrcY++) * (*pSrcX++);
                *pDstZ++ = (*pSrcY++) * (*pSrcX++);
            }

            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                *pDstZ++ = (*pSrcY++) * (*pSrcX++);
            }
        }
        else
        {
            rq = -rq;

            for (iLoop = 0; iLoop < iLenTmp; iLoop++)
            {
                *pDstZ++ = ((*pSrcY++) * (*pSrcX++)) << rq;
                *pDstZ++ = ((*pSrcY++) * (*pSrcX++)) << rq;
                *pDstZ++ = ((*pSrcY++) * (*pSrcX++)) << rq;
                *pDstZ++ = ((*pSrcY++) * (*pSrcX++)) << rq;
            }

            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                *pDstZ++ = ((*pSrcY++) * (*pSrcX++)) << rq;
            }
        }
    }

#endif
#if NN_STATISTIC
    g_nnVecMul32X32_Real_Count++;
#endif
}

/************************************************************
  Function   : nnVecMulSum32X8_Real()

  Description: Z = sum(Y.*X)
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
VOID nnVecMulSum32X8_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                          aisp_s8_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    vector_real_zs32_ys32_dotpdt_xs8(pDstZ, pSrcY, pSrcX, uiLen, rq);
#else
    aisp_s64_t llSum = 0;
    S32 iLoop;
    S32 iLenTmp = uiLen >> 2;
    uiLen = (uiLen & 0x03);

    for (iLoop = 0; iLoop < iLenTmp; iLoop++)
    {
        llSum += (aisp_s64_t)(*pSrcY++) * (*pSrcX++);
        llSum += (aisp_s64_t)(*pSrcY++) * (*pSrcX++);
        llSum += (aisp_s64_t)(*pSrcY++) * (*pSrcX++);
        llSum += (aisp_s64_t)(*pSrcY++) * (*pSrcX++);
    }

    if (uiLen)
    {
        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            llSum += (aisp_s64_t)(*pSrcY++) * (*pSrcX++);
        }
    }

    *pDstZ = PSHR(llSum, rq);
#endif
#if NN_STATISTIC
    g_nnVecMulSum32X8_Real_Count++;
#endif
}

/************************************************************
  Function   : nnVecMulSum32X16_Real()

  Description: Z = sum(Y.*X)
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
VOID nnVecMulSum32X16_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                           aisp_s16_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    FIX_HERE();
#else
    aisp_s64_t llSum = 0;
    S32 iLoop;
    S32 iLenTmp = uiLen >> 2;
    uiLen = (uiLen & 0x03);

    for (iLoop = 0; iLoop < iLenTmp; iLoop++)
    {
        llSum += (aisp_s64_t)(*pSrcY++) * (*pSrcX++);
        llSum += (aisp_s64_t)(*pSrcY++) * (*pSrcX++);
        llSum += (aisp_s64_t)(*pSrcY++) * (*pSrcX++);
        llSum += (aisp_s64_t)(*pSrcY++) * (*pSrcX++);
    }

    for (iLoop = 0; iLoop < uiLen; iLoop++)
    {
        llSum += (aisp_s64_t)(*pSrcY++) * (*pSrcX++);
    }

    *pDstZ = PSHR(llSum, rq);
#endif
#if NN_STATISTIC
    g_nnVecMulSum32X16_Real_Count++;
#endif
}

/************************************************************
  Function   : nnVecMulSum32X32_Real()

  Description: Z = sum(Y.*X)
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
VOID nnVecMulSum32X32_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                           aisp_s32_t *pSrcX, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    FIX_HERE();
#else
    aisp_s64_t llSum = 0;
    S32 iLoop;
    S32 iLenTmp = uiLen >> 2;
    uiLen = (uiLen & 0x03);

    for (iLoop = 0; iLoop < iLenTmp; iLoop++)
    {
        llSum += (aisp_s64_t)(*pSrcY++) * (*pSrcX++);
        llSum += (aisp_s64_t)(*pSrcY++) * (*pSrcX++);
        llSum += (aisp_s64_t)(*pSrcY++) * (*pSrcX++);
        llSum += (aisp_s64_t)(*pSrcY++) * (*pSrcX++);
    }

    for (iLoop = 0; iLoop < uiLen; iLoop++)
    {
        llSum += (aisp_s64_t)(*pSrcY++) * (*pSrcX++);
    }

    *pDstZ = PSHR(llSum, rq);
#endif
#if NN_STATISTIC
    g_nnVecMulSum32X32_Real_Count++;
#endif
}

/************************************************************
  Function   : nnVecMulAdd32X8_Real()

  Description: Z = Y.*X +B
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
VOID nnVecMulAdd32X8_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                          aisp_s8_t *pSrcX, aisp_s32_t *pSrcB, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28

    if (pDstZ == pSrcB)
    {
        //vector_real_zs32_ys32_dotpdt_xs8(pDstZ, pSrcY, pSrcX, uiLen, rq);
        MSG_DBG("%s [%d] not support. \r\n", __func__, __LINE__);
    }
    else
    {
        vector_real_zs32_ys32_mul_xs8(pDstZ, pSrcY, pSrcX, uiLen, rq);
        vector_real_zs32_ys32_add_xs32(pDstZ, pDstZ, pSrcB, uiLen);
    }

#else
#if 0
    nnVecMul32X8_Real(pDstZ, pSrcY, pSrcX, uiLen, rq);
    nnVec32Add32_Real(pDstZ, pDstZ, pSrcB, uiLen, 0);
#else
    S32 iLoop;
    S32 iLenTmp = uiLen >> 2;
    uiLen = (uiLen & 0x03);

    if (0 < rq)
    {
        for (iLoop = 0; iLoop < iLenTmp; iLoop++)
        {
            *pDstZ++ = (((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq) + (*pSrcB++);
            *pDstZ++ = (((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq) + (*pSrcB++);
            *pDstZ++ = (((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq) + (*pSrcB++);
            *pDstZ++ = (((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq) + (*pSrcB++);
        }

        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            *pDstZ++ = (((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq) + (*pSrcB++);
        }
    }
    else
    {
        if (0 == rq)
        {
            for (iLoop = 0; iLoop < iLenTmp; iLoop++)
            {
                *pDstZ++ = (*pSrcY++) * (*pSrcX++) + (*pSrcB++);
                *pDstZ++ = (*pSrcY++) * (*pSrcX++) + (*pSrcB++);
                *pDstZ++ = (*pSrcY++) * (*pSrcX++) + (*pSrcB++);
                *pDstZ++ = (*pSrcY++) * (*pSrcX++) + (*pSrcB++);
            }

            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                *pDstZ++ = (*pSrcY++) * (*pSrcX++) + (*pSrcB++);
            }
        }
        else
        {
            rq = -rq;

            for (iLoop = 0; iLoop < iLenTmp; iLoop++)
            {
                *pDstZ++ = (((*pSrcY++) * (*pSrcX++)) << rq) + (*pSrcB++);
                *pDstZ++ = (((*pSrcY++) * (*pSrcX++)) << rq) + (*pSrcB++);
                *pDstZ++ = (((*pSrcY++) * (*pSrcX++)) << rq) + (*pSrcB++);
                *pDstZ++ = (((*pSrcY++) * (*pSrcX++)) << rq) + (*pSrcB++);
            }

            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                *pDstZ++ = (((*pSrcY++) * (*pSrcX++)) << rq) + (*pSrcB++);
            }
        }
    }

#endif
#endif
#if NN_STATISTIC
    g_nnVecMulAdd32X8_Real_Count++;
#endif
}

/************************************************************
  Function   : nnVecMulAdd32X16_Real()

  Description: Z = Y.*X +B
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
VOID nnVecMulAdd32X16_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                           aisp_s16_t *pSrcX, aisp_s32_t *pSrcB, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    FIX_HERE();
#else
#if 0
    nnVecMul32X16_Real(pDstZ, pSrcY, pSrcX, uiLen, rq);
    nnVec32Add32_Real(pDstZ, pDstZ, pSrcB, uiLen, 0);
#else
    S32 iLoop;
    S32 iLenTmp = uiLen >> 2;
    uiLen = (uiLen & 0x03);

    if (0 < rq)
    {
        for (iLoop = 0; iLoop < iLenTmp; iLoop++)
        {
            *pDstZ++ = (((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq) + (*pSrcB++);
            *pDstZ++ = (((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq) + (*pSrcB++);
            *pDstZ++ = (((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq) + (*pSrcB++);
            *pDstZ++ = (((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq) + (*pSrcB++);
        }

        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            *pDstZ++ = (((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq) + (*pSrcB++);
        }
    }
    else
    {
        if (0 == rq)
        {
            for (iLoop = 0; iLoop < iLenTmp; iLoop++)
            {
                *pDstZ++ = (*pSrcY++) * (*pSrcX++) + (*pSrcB++);
                *pDstZ++ = (*pSrcY++) * (*pSrcX++) + (*pSrcB++);
                *pDstZ++ = (*pSrcY++) * (*pSrcX++) + (*pSrcB++);
                *pDstZ++ = (*pSrcY++) * (*pSrcX++) + (*pSrcB++);
            }

            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                *pDstZ++ = (*pSrcY++) * (*pSrcX++) + (*pSrcB++);
            }
        }
        else
        {
            rq = -rq;

            for (iLoop = 0; iLoop < iLenTmp; iLoop++)
            {
                *pDstZ++ = (((*pSrcY++) * (*pSrcX++)) << rq) + (*pSrcB++);
                *pDstZ++ = (((*pSrcY++) * (*pSrcX++)) << rq) + (*pSrcB++);
                *pDstZ++ = (((*pSrcY++) * (*pSrcX++)) << rq) + (*pSrcB++);
                *pDstZ++ = (((*pSrcY++) * (*pSrcX++)) << rq) + (*pSrcB++);
            }

            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                *pDstZ++ = (((*pSrcY++) * (*pSrcX++)) << rq) + (*pSrcB++);
            }
        }
    }

#endif
#endif
#if NN_STATISTIC
    g_nnVecMulAdd32X16_Real_Count++;
#endif
}

/************************************************************
  Function   : nnVecMulAdd32X32_Real()

  Description: Z = Y.*X + B
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
VOID nnVecMulAdd32X32_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                           aisp_s32_t *pSrcX, aisp_s32_t *pSrcB, U32 uiLen, S32 rq)
{
    ADDR_CHECK(pDstZ);
    ADDR_CHECK(pSrcY);
    ADDR_CHECK(pSrcX);
#ifdef JIELI_BR28
    FIX_HERE();
#else
#if 0
    nnVecMul32X32_Real(pDstZ, pSrcY, pSrcX, uiLen, rq);
    nnVec32Add32_Real(pDstZ, pDstZ, pSrcB, uiLen, 0);
#else
    S32 iLoop;
    S32 iLenTmp = uiLen >> 2;
    uiLen = (uiLen & 0x03);

    if (0 < rq)
    {
        for (iLoop = 0; iLoop < iLenTmp; iLoop++)
        {
            *pDstZ++ = (((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq) + (*pSrcB++);
            *pDstZ++ = (((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq) + (*pSrcB++);
            *pDstZ++ = (((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq) + (*pSrcB++);
            *pDstZ++ = (((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq) + (*pSrcB++);
        }

        for (iLoop = 0; iLoop < uiLen; iLoop++)
        {
            *pDstZ++ = (((aisp_s64_t)(*pSrcY++) * (*pSrcX++)) >> rq) + (*pSrcB++);
        }
    }
    else
    {
        if (0 == rq)
        {
            for (iLoop = 0; iLoop < iLenTmp; iLoop++)
            {
                *pDstZ++ = (*pSrcY++) * (*pSrcX++) + (*pSrcB++);
                *pDstZ++ = (*pSrcY++) * (*pSrcX++) + (*pSrcB++);
                *pDstZ++ = (*pSrcY++) * (*pSrcX++) + (*pSrcB++);
                *pDstZ++ = (*pSrcY++) * (*pSrcX++) + (*pSrcB++);
            }

            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                *pDstZ++ = (*pSrcY++) * (*pSrcX++) + (*pSrcB++);
            }
        }
        else
        {
            rq = -rq;

            for (iLoop = 0; iLoop < iLenTmp; iLoop++)
            {
                *pDstZ++ = (((*pSrcY++) * (*pSrcX++)) << rq) + (*pSrcB++);
                *pDstZ++ = (((*pSrcY++) * (*pSrcX++)) << rq) + (*pSrcB++);
                *pDstZ++ = (((*pSrcY++) * (*pSrcX++)) << rq) + (*pSrcB++);
                *pDstZ++ = (((*pSrcY++) * (*pSrcX++)) << rq) + (*pSrcB++);
            }

            for (iLoop = 0; iLoop < uiLen; iLoop++)
            {
                *pDstZ++ = (((*pSrcY++) * (*pSrcX++)) << rq) + (*pSrcB++);
            }
        }
    }

#endif
#endif
#if NN_STATISTIC
    g_nnVecMulAdd32X32_Real_Count++;
#endif
}

/************************************************************
  Function   : nnLinearCalc()

  Description: Z = Y.*X + B
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     : pW: Q=iWQ

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
VOID nnLinearCalc(S32 iInSize, S32 iOutSize, aisp_s32_t *pOutput, aisp_s32_t *pInput,
                  aisp_s8_t *pW, aisp_s32_t *pBias, S32 iWQ)
{
    S32 iIdx;
    aisp_s8_t *pWPoint = pW;

    for (iIdx = 0; iIdx < iOutSize; iIdx++)
    {
        nnVecMulSum32X8_Real(&pOutput[iIdx], pInput, pWPoint, iInSize, iWQ);
        pWPoint += iInSize;
    }

    if (NULL != pBias)
    {
        nnVec32Add32_Real(pOutput, pOutput, pBias, iOutSize, 0);
    }

#if NN_STATISTIC
    g_nnLinearCalc_Count++;
#endif
}

/************************************************************
  Function   : nnDenseCalc()

  Description: Z = activation(Y.*X + B)
  Calls      :
  Called By  :
  Input      : Q=ucDataQ
  Output     : Q=ucDataQ
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
VOID nnFcCalc(FC_LAYER_S *pLayer, aisp_s32_t *pfOutputs, aisp_s32_t *pfInputs)
{
    U32 uiOutput = pLayer->iOutputs;
    U32 uiInput = pLayer->iInputs;
    U8 ucWQ = pLayer->ucWQ;
    U8 ucDataQ = pLayer->ucDataQ;
    nnLinearCalc(uiInput, uiOutput, pfOutputs, pfInputs, pLayer->pfInputWeights, pLayer->pfBias, ucWQ);
    nnActivation(pfOutputs, pfOutputs, uiOutput, pLayer->iActivation, 0, ucDataQ, ucDataQ);
}


VOID nnStatisticShow(VOID)
{
#if NN_STATISTIC
    MSG_DBG("g_nnVec32Add32_Real_Count       = %d\r\n", g_nnVec32Add32_Real_Count);
    MSG_DBG("g_nnVecMul32X8_Real_Count       = %d\r\n", g_nnVecMul32X8_Real_Count);
    MSG_DBG("g_nnVecMul32X16_Real_Count      = %d\r\n", g_nnVecMul32X16_Real_Count);
    MSG_DBG("g_nnVecMul32X32_Real_Count      = %d\r\n", g_nnVecMul32X32_Real_Count);
    MSG_DBG("g_nnVecMulSum32X8_Real_Count    = %d\r\n", g_nnVecMulSum32X8_Real_Count);
    MSG_DBG("g_nnVecMulSum32X16_Real_Count   = %d\r\n", g_nnVecMulSum32X16_Real_Count);
    MSG_DBG("g_nnVecMulSum32X32_Real_Count   = %d\r\n", g_nnVecMulSum32X32_Real_Count);
    MSG_DBG("g_nnVecMulAdd32X8_Real_Count    = %d\r\n", g_nnVecMulAdd32X8_Real_Count);
    MSG_DBG("g_nnVecMulAdd32X16_Real_Count   = %d\r\n", g_nnVecMulAdd32X16_Real_Count);
    MSG_DBG("g_nnVecMulAdd32X32_Real_Count   = %d\r\n", g_nnVecMulAdd32X32_Real_Count);
    MSG_DBG("g_nnLinearCalc_Count            = %d\r\n", g_nnLinearCalc_Count);
#endif
}
#endif
