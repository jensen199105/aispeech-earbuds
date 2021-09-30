#include <stdio.h>
#include "AISP_TSL_base.h"
#include "AISP_TSL_str.h"
#include "AISP_TSL_complex.h"

#include "sevc_config.h"
#include "sevc_types.h"
#include "sevc_util.h"

#ifdef SEVC_SWITCH_GSC
#include "sevc_gsc.h"
#define SEVC_GSC_WEIGHT_CHECK   1

extern FILE *fpYyPcm;

/************************************************************
  Function   : sevcGscErrCalc()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2020/11/03, shizhang.tang create
************************************************************/
static inline VOID sevcGscErrCalc(SEVC_GSC_S *pstGscEng, aisp_cpx_s32_t *pstNoisyFrame)
{
    S32 ii;
    SEVC_CONFIG_S *pstSevcCfg = pstGscEng->pstSevcCfg;
    aisp_cpx_s32_t *pstErr = pstGscEng->pstErr;
    aisp_cpx_s32_t *pstEst = pstGscEng->pstEst;
    aisp_cpx_s32_t **ppstNoise = pstGscEng->ppstNoise;
    aisp_cpx_s32_t **ppstWeight = pstGscEng->ppstWeight;
    aisp_cpx_s32_t *pstNoiseRow;
    aisp_cpx_s32_t *pstWeightRow;
    U16 usFftBin = pstSevcCfg->usFftBin;
    U16 usGscTaps = pstSevcCfg->usGscTaps;
    pstWeightRow = ppstWeight[0];
    pstNoiseRow = ppstNoise[0];

    for (ii = 0; ii < usFftBin; ii++)
    {
        COMPLEX_MUL_SQ(pstEst[ii], pstNoiseRow[ii], pstWeightRow[ii], 24); //W32Q21
    }

    for (ii = 1; ii < usGscTaps; ii++)
    {
        pstWeightRow = ppstWeight[ii];
        pstNoiseRow = ppstNoise[ii];
        sevcVecMLA_Complex(pstEst, pstWeightRow, pstNoiseRow, usFftBin, 24); //W32Q21
    }

    sevcVecSub_Complex(pstErr, pstNoisyFrame, pstEst, usFftBin, 0);
}

/************************************************************
  Function   : sevcGscWeightUpdate()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2020/11/03, shizhang.tang create
************************************************************/
static inline VOID sevcGscWeightUpdate(SEVC_GSC_S *pstGscEng, aisp_s32_t *pfSpp)
{
    S32 ii;
    SEVC_CONFIG_S *pstSevcCfg = pstGscEng->pstSevcCfg;
    aisp_cpx_s32_t *pstErr = pstGscEng->pstErr;
    aisp_cpx_s32_t *pstErrNorm = pstGscEng->pstErrNorm;
    aisp_cpx_s32_t **ppstNoise = pstGscEng->ppstNoise;
    aisp_cpx_s32_t **ppstWeight = pstGscEng->ppstWeight;
    aisp_s32_t *pfNoisePowSum = pstGscEng->pfNoisePowSum;
    aisp_s64_t *pfErrPow = pstGscEng->pfErrPow;
    aisp_s32_t *pfDelta = pstGscEng->pfDelta;
    aisp_s32_t fGscDeltaGain = pstSevcCfg->fGscDeltaGain;
    aisp_s32_t fGscDeltaMin = pstSevcCfg->fGscDeltaMin;
    aisp_s32_t fGscDeltaMax = pstSevcCfg->fGscDeltaMax;
    aisp_s32_t fMuOffset = pstSevcCfg->fMuOffset;
    aisp_s32_t usGscMu = pstSevcCfg->usGscMu + fMuOffset;
    U16 usGscTaps = pstSevcCfg->usGscTaps;
    U16 usFftBin = pstSevcCfg->usFftBin;
    aisp_cpx_s32_t *pstNoiseRow;
    aisp_cpx_s32_t *pstWeightRow;
    aisp_s32_t fTmpData;
    U16 usTapIdx;
    aisp_s64_t tmpData64;

    for (ii = 0; ii < usFftBin; ii++)
    {
        tmpData64 = pfErrPow[ii] * fGscDeltaGain; //Q16
        tmpData64 = PSHR64(tmpData64, 16);
        tmpData64 = SEVC_MAX(tmpData64, fGscDeltaMin);
        pfDelta[ii] = SEVC_MIN(tmpData64, fGscDeltaMax);
        pfDelta[ii] += pfNoisePowSum[ii]; //Q16
    }

    sevcVecPowY64X32_Complex(pfErrPow, pstErr, usFftBin, 26); //W32Q16

    for (ii = 0; ii < usFftBin; ii++)
    {
        //fTmpData = (pfSpp[ii] < 16384 ? 0 : MULT32_32_SQ(pfSpp[ii], usGscMu, 17)); //Q16
        fTmpData = (pfSpp[ii] == 1 ? fMuOffset : usGscMu); //Q16
        pstErrNorm[ii].real = PDIV(MULT32_32(fTmpData, pstErr[ii].real), pfDelta[ii]); //W32Q21 16+21-16
        pstErrNorm[ii].imag = PDIV(MULT32_32(fTmpData, pstErr[ii].imag), pfDelta[ii]); //W32Q21 16+21-16
    }

    for (usTapIdx = 0; usTapIdx < usGscTaps; usTapIdx++)
    {
        pstWeightRow = ppstWeight[usTapIdx];
        pstNoiseRow = ppstNoise[usTapIdx];
#ifdef FREQ_BIN_Q21
        sevcVecConjMLA_Complex(pstWeightRow, pstErrNorm, pstNoiseRow, usFftBin, 18); //W32Q24
#else
        sevcVecConjMLA_Complex(pstWeightRow, pstErrNorm, pstNoiseRow, usFftBin, 14); //W32Q24
#endif
#ifdef SEVC_GSC_WEIGHT_CHECK

        for (ii = 0; ii < 50; ii++)
        {
            if (pstWeightRow[ii].real > 134217728
                    || pstWeightRow[ii].real < -134217728) //134217728=8 Q24
            {
                pstWeightRow[ii].real = 0;
                pstWeightRow[ii].imag = 0;
                //goto GSC_WEIGHT_RESET;
            }
        }

        for (ii = 50; ii < usFftBin; ii++)
        {
            if (pstWeightRow[ii].real > 67108864
                    || pstWeightRow[ii].real < -67108864) //67108864=4 Q24
            {
                pstWeightRow[ii].real = 0;
                pstWeightRow[ii].imag = 0;
                //goto GSC_WEIGHT_RESET;
            }
        }

#endif
    }

    return;
}

/************************************************************
  Function   : sevcGscNoiseProcess()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2020/11/03, shizhang.tang create
************************************************************/
static inline VOID sevcGscNoiseProcess(SEVC_GSC_S *pstGscEng, aisp_cpx_s32_t *pstNoiseFrame)
{
    S32 ii;
    U16 usGscTaps = pstGscEng->pstSevcCfg->usGscTaps;
    U16 usFftBin = pstGscEng->pstSevcCfg->usFftBin;
    aisp_cpx_s32_t *pstNoiseBins = pstGscEng->ppstNoise[usGscTaps];
    aisp_s32_t *pRefPower = pstGscEng->ppfNoisePow[usGscTaps];
    aisp_s32_t *pRefPowerOld;
    aisp_s32_t *pfNoisePowSum = pstGscEng->pfNoisePowSum;

    for (ii = 0; ii < usFftBin; ii++)
    {
        pstNoiseBins[ii].real = pstNoiseFrame[ii].real;
        pstNoiseBins[ii].imag = pstNoiseFrame[ii].imag;
    }

#ifdef FREQ_BIN_Q21

    // sevcVecPowY32X32_Complex(pRefPower, pstNoiseBins, usFftBin, 26); //W32Q16
    for (ii = 0; ii < usFftBin; ii++)
    {
        COMPLEX_POW_SQ(pRefPower[ii], pstNoiseBins[ii], 26);
    }

#else
    sevcVecPowY32X32_Complex(pRefPower, pstNoiseBins, usFftBin, 14); //W32Q16
#endif

    for (ii = usGscTaps; ii > 0; ii--)
    {
        pstGscEng->ppstNoise[ii] = pstGscEng->ppstNoise[ii - 1];
        pstGscEng->ppfNoisePow[ii] = pstGscEng->ppfNoisePow[ii - 1];
    }

    pstGscEng->ppstNoise[0] = pstNoiseBins;
    pstGscEng->ppfNoisePow[0] = pRefPower;

    if (usGscTaps == 1)
    {
        pRefPower = pstGscEng->ppfNoisePow[0];

        for (ii = 0; ii < usFftBin; ii++)
        {
            pfNoisePowSum[ii] = pRefPower[ii]; //W32Q16
        }
    }
    else
    {
        //sum
        pRefPowerOld = pstGscEng->ppfNoisePow[usGscTaps];
        pRefPower = pstGscEng->ppfNoisePow[0];
        sevcVec32Add32_Real(pfNoisePowSum, pfNoisePowSum, pRefPower, usFftBin, 0);
        sevcVec32Sub32_Real(pfNoisePowSum, pfNoisePowSum, pRefPowerOld, usFftBin, 0);
    }
}

/************************************************************
  Function   : sevcGscDataPop()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2020/11/03, shizhang.tang create
************************************************************/
static inline VOID sevcGscDataPop(SEVC_GSC_S *pstGscEng)
{
    U16 usFftBin = pstGscEng->pstSevcCfg->usFftBin;
    U16 usSizeComp = sizeof(aisp_cpx_s32_t);
    U32 uiSize = usFftBin * usSizeComp;
    aisp_cpx_s32_t *pstErr = pstGscEng->pstErr;

    if (NULL != pstGscEng->pCallBackFunc)
    {
        pstGscEng->pCallBackFunc((U8 *)pstErr, uiSize, pstGscEng->pUsrData);
    }
}

/************************************************************
  Function   : SEVC_GSC_ShMemSizeGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2020/11/03, shizhang.tang create
************************************************************/
U32 SEVC_GSC_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    U32 uiSize;
    U16 usSizeComp = sizeof(aisp_cpx_s32_t);
    U16 usFftBin = pstSevcCfg->usFftBin;
    //pstErrNorm
    uiSize = usFftBin * usSizeComp;
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    //pstEst
    uiSize = usFftBin * usSizeComp;
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    //pfDelta
    uiSize = usFftBin * sizeof(aisp_s32_t);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    return uiTotalMemSize;
}

/************************************************************
  Function   : SEVC_GSC_LocMemSizeGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2020/11/03, shizhang.tang create
************************************************************/
U32 SEVC_GSC_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    U32 uiSize;
    U16 usSizeComp = sizeof(aisp_cpx_s32_t);
    U16 usGscTaps = pstSevcCfg->usGscTaps;
    U16 usFftBin = pstSevcCfg->usFftBin;
    uiSize = sizeof(SEVC_GSC_S);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    //pstErr
    uiSize = usFftBin * usSizeComp;
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    //ppstWeight
    uiSize = usGscTaps * sizeof(VOID *);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    uiSize = usFftBin * usSizeComp;
    uiTotalMemSize += usGscTaps * SIZE_ALIGN(uiSize);
    //ppstNoise
    uiSize = (usGscTaps + 1) * sizeof(VOID *);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    uiSize = usFftBin * usSizeComp;
    uiTotalMemSize += (usGscTaps + 1) * SIZE_ALIGN(uiSize);
    //ppfNoisePow
    uiSize = (usGscTaps + 1) * sizeof(VOID *);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    uiSize = usFftBin * sizeof(aisp_s32_t);
    uiTotalMemSize += (usGscTaps + 1) * SIZE_ALIGN(uiSize);
    //pfNoisePowSum
    uiSize = usFftBin * sizeof(aisp_s32_t);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    //pfErrPow
    uiSize = usFftBin * sizeof(aisp_s64_t);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
#ifndef USE_SHARE_MEM
    uiTotalMemSize += SEVC_GSC_ShMemSizeGet(pstSevcCfg);
#endif
#if MEM_BUFF_DBG
    MSG_DBG("%s uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}

/************************************************************
  Function   : SEVC_GSC_New()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2020/11/03, shizhang.tang create
************************************************************/
SEVC_GSC_S *SEVC_GSC_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator)
{
    SEVC_GSC_S *pstGscEng = NULL;
    S32 ii;
    U16 usSize;
    U16 usSizeComp = sizeof(aisp_cpx_s32_t);
    U16 usGscTaps = pstSevcCfg->usGscTaps;
    U16 usFftBin = pstSevcCfg->usFftBin;
    MEM_LOCATOR_SHOW(pstMemLocator);
    usSize = sizeof(SEVC_GSC_S);
    pstGscEng = (SEVC_GSC_S *)memLocatorGet(pstMemLocator, usSize, AISP_ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstGscEng, FAIL);
    pstGscEng->pstSevcCfg = pstSevcCfg;
    //pstErr
    BUFF_POINT_SET_ALIGN(pstGscEng->pstErr, (usFftBin * usSizeComp), pstMemLocator, FAIL);
    //ppstWeight
    BUFF_POINT_SET_ALIGN(pstGscEng->ppstWeight, (usGscTaps * sizeof(VOID *)), pstMemLocator, FAIL);

    for (ii = 0; ii < usGscTaps; ii++)
    {
        BUFF_POINT_SET_ALIGN(pstGscEng->ppstWeight[ii], (usFftBin * usSizeComp), pstMemLocator, FAIL);
    }

    //ppstNoise
    BUFF_POINT_SET_ALIGN(pstGscEng->ppstNoise, ((usGscTaps + 1) * sizeof(VOID *)), pstMemLocator, FAIL);

    for (ii = 0; ii <= usGscTaps; ii++)
    {
        BUFF_POINT_SET_ALIGN(pstGscEng->ppstNoise[ii], (usFftBin * usSizeComp), pstMemLocator, FAIL);
    }

    //ppfNoisePow
    BUFF_POINT_SET_ALIGN(pstGscEng->ppfNoisePow, ((usGscTaps + 1) * sizeof(VOID *)), pstMemLocator, FAIL);

    for (ii = 0; ii <= usGscTaps; ii++)
    {
        BUFF_POINT_SET_ALIGN(pstGscEng->ppfNoisePow[ii], (usFftBin * sizeof(aisp_s32_t)), pstMemLocator, FAIL);
    }

    //pfNoisePowSum
    BUFF_POINT_SET_ALIGN(pstGscEng->pfNoisePowSum, (usFftBin * sizeof(aisp_s32_t)), pstMemLocator, FAIL);
    //pfErrPow
    BUFF_POINT_SET_ALIGN(pstGscEng->pfErrPow, (usFftBin * sizeof(aisp_s64_t)), pstMemLocator, FAIL);
    //Shared mem info
    //pstErrNorm
    SH_BUFF_POINT_SET_ALIGN(pstGscEng->pstErrNorm, (usFftBin * usSizeComp), pstMemLocator, FAIL);
    //pstEst
    SH_BUFF_POINT_SET_ALIGN(pstGscEng->pstEst, (usFftBin * usSizeComp), pstMemLocator, FAIL);
    //pfDelta
    SH_BUFF_POINT_SET_ALIGN(pstGscEng->pfDelta, (usFftBin * sizeof(aisp_s32_t)), pstMemLocator, FAIL);
    MEM_LOCATOR_SHOW(pstMemLocator);
    return pstGscEng;
FAIL:
    return NULL;
}

/************************************************************
  Function   : SEVC_GSC_CallBackFuncRegister()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2020/11/03, shizhang.tang create
************************************************************/
VOID SEVC_GSC_CallBackFuncRegister(SEVC_GSC_S *pstGscEng, VOID *pCBFunc, VOID *pUsrData)
{
    if (NULL == pstGscEng || NULL == pCBFunc)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return ;
    }

    pstGscEng->pCallBackFunc = (CallBackFunc)pCBFunc;
    pstGscEng->pUsrData = pUsrData;
}

/************************************************************
  Function   : SEVC_GSC_Feed()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2020/11/03, shizhang.tang create
************************************************************/
S32 SEVC_GSC_Feed(SEVC_GSC_S *pstGscEng, aisp_cpx_s32_t *pNoisyFrame,
                  aisp_cpx_s32_t *pNoiseFrame, aisp_s32_t *pfSpp)
{
    sevcGscNoiseProcess(pstGscEng, pNoiseFrame);
    sevcGscErrCalc(pstGscEng, pNoisyFrame);
    sevcGscWeightUpdate(pstGscEng, pfSpp);
    sevcGscDataPop(pstGscEng);
    pstGscEng->uiFrameCount++;
    return 0;
}

/************************************************************
Function   : SEVC_GSC_Reset()

Description:
Calls      :
Called By  :
Input      :
Output     :
Return     :
Others     :

History    :
  2020/11/03, shizhang.tang create
************************************************************/
VOID SEVC_GSC_Reset(SEVC_GSC_S *pstGscEng)
{
    S32 ii, iTaps;
    SEVC_CONFIG_S *pstSevcCfg = pstGscEng->pstSevcCfg;
    aisp_s64_t *pfErrPow = pstGscEng->pfErrPow;
    U16 usGscTaps = pstSevcCfg->usGscTaps;
    U16 usFftBin = pstSevcCfg->usFftBin;
    aisp_cpx_s32_t *pstWeight;
    aisp_cpx_s32_t *pstNoiseBins;
    aisp_s32_t *pRefPower;
    pstGscEng->uiFrameCount = 0;

    for (iTaps = 0; iTaps < usGscTaps; iTaps++)
    {
        pstWeight = pstGscEng->ppstWeight[iTaps];
        pstNoiseBins = pstGscEng->ppstNoise[iTaps];
        pRefPower = pstGscEng->ppfNoisePow[iTaps];

        for (ii = 0; ii < usFftBin; ii++)
        {
            pstWeight[ii].real = 0;
            pstWeight[ii].imag = 0;
            pstNoiseBins[ii].real = 0;
            pstNoiseBins[ii].imag = 0;
            pRefPower[ii] = 0;
        }
    }

    for (ii = 0; ii < usFftBin; ii++)
    {
        pfErrPow[ii] = 0;
    }
}

/************************************************************
  Function   : SEVC_GSC_Delete()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2020/11/03, shizhang.tang create
************************************************************/
VOID SEVC_GSC_Delete(SEVC_GSC_S *pstGscEng)
{
}

#endif
