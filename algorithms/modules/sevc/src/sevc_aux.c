#include<stdio.h>
#include "AISP_TSL_base.h"
#include "AISP_TSL_str.h"
#include "AISP_TSL_common.h"
#include "AISP_TSL_complex.h"
#include "fft_wrapper.h"
#include "sevc_config.h"
#include "sevc_types.h"
#include "sevc_util.h"
#include "sevc_aux.h"

#if SEVC_SWITCH_AUX
U32 SEVC_AUX_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    U16 usFrameSize = pstSevcCfg->usFrameSize;
    U16 usFftBin = pstSevcCfg->usFftBin;
    uiTotalMemSize += SIZE_ALIGN(usFrameSize * sizeof(aisp_s32_t));
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_cpx_s32_t));
    return uiTotalMemSize;
}

U32 SEVC_AUX_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    U16 usFrameShift = pstSevcCfg->frameShift;
    uiTotalMemSize += SIZE_ALIGN(sizeof(SEVC_AUX_S));
    uiTotalMemSize += SIZE_ALIGN(usFrameShift * sizeof(aisp_s16_t));
#ifndef USE_SHARE_MEM
    uiTotalMemSize += SEVC_AUX_ShMemSizeGet(pstSevcCfg);
#endif
    return uiTotalMemSize;
}

SEVC_AUX_S *SEVC_AUX_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator)
{
    SEVC_AUX_S *pstAuxEng;
    U32 uiSize;
    U16 usFrameShift = pstSevcCfg->frameShift;
    U16 usFrameOldSize = pstSevcCfg->usFrameSize - usFrameShift;

    if (NULL == pstSevcCfg || NULL == pstMemLocator)
    {
        printf("%s [%d] params error\r\n", __func__, __LINE__);
        return NULL;
    }

    MEM_LOCATOR_SHOW(pstMemLocator);
    uiSize = sizeof(SEVC_AUX_S);
    pstAuxEng = (SEVC_AUX_S *)memLocatorGet(pstMemLocator, uiSize, AISP_ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstAuxEng, FAIL);
    pstAuxEng->pstFftEng = &pstSevcCfg->stFFTEng;
    pstAuxEng->pstSevcCfg = pstSevcCfg;
    uiSize = usFrameOldSize * sizeof(aisp_s16_t);
    BUFF_POINT_SET_ALIGN(pstAuxEng->pfFrameOld, uiSize, pstMemLocator, FAIL);
    uiSize = pstSevcCfg->usFrameSize * sizeof(aisp_s32_t);
    SH_BUFF_POINT_SET_ALIGN(pstAuxEng->pfFrameWin, uiSize, pstMemLocator, FAIL);
    uiSize = pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t);
    SH_BUFF_POINT_SET_ALIGN(pstAuxEng->pAuxBins, uiSize, pstMemLocator, FAIL);
    //MEM_LOCATOR_SHOW(pstMemLocator);
    return pstAuxEng;
FAIL:
    return NULL;
}

VOID SEVC_AUX_Feed(SEVC_AUX_S *pstAuxEng, aisp_cpx_s32_t *pMicFreq, aisp_s16_t *pfAuxFrame)
{
    U32 ii;
    U32 uiAuxBin1 = pstAuxEng->pstSevcCfg->uiAuxBin1;
    U32 uiAuxBin2 = pstAuxEng->pstSevcCfg->uiAuxBin2;
    U16 usFrameShift = pstAuxEng->pstSevcCfg->frameShift;
    aisp_s16_t *pfFrameOld = pstAuxEng->pfFrameOld;
    aisp_s32_t *pfFrameWin = pstAuxEng->pfFrameWin;
    aisp_s32_t *pfFrameWinHalf = pstAuxEng->pfFrameWin + usFrameShift;
    aisp_s16_t *pfHanningWinSqrt = pstAuxEng->pstSevcCfg->pfHanningWindowSqrt;
    aisp_s16_t *pfHanningWinSqrtHalf = pfHanningWinSqrt + usFrameShift;
    aisp_cpx_s32_t *pAuxBins = pstAuxEng->pAuxBins;
#ifdef JIELI_BR28
#ifdef FREQ_BIN_Q21
    sevcVecMUL16X16_Real(pfFrameWin, pfFrameOld, pfHanningWinSqrt, usFrameShift, 9);
    sevcVecMUL16X16_Real(pfFrameWinHalf, pfAuxFrame, pfHanningWinSqrtHalf, usFrameShift, 9);
#else
    sevcVecMUL16X16_Real(pfFrameWin, pfFrameOld, pfHanningWinSqrt, usFrameShift, 15);
    sevcVecMUL16X16_Real(pfFrameWinHalf, pfAuxFrame, pfHanningWinSqrtHalf, usFrameShift, 15);
#endif
#else
#ifdef FREQ_BIN_Q21
    sevcVecMUL16X16_Real(pfFrameWin, pfFrameOld, pfHanningWinSqrt, usFrameShift, 0);
    sevcVecMUL16X16_Real(pfFrameWinHalf, pfAuxFrame, pfHanningWinSqrtHalf, usFrameShift, 0);
#else
    sevcVecMUL16X16_Real(pfFrameWin, pfFrameOld, pfHanningWinSqrt, usFrameShift, 6);
    sevcVecMUL16X16_Real(pfFrameWinHalf, pfAuxFrame, pfHanningWinSqrtHalf, usFrameShift, 6);
#endif
#endif
    AISP_TSL_FFT((*pstAuxEng->pstFftEng), pfFrameWin, pAuxBins);

    for (ii = 0; ii < usFrameShift; ii++)
    {
        pfFrameOld[ii] = pfAuxFrame[ii];
    }

    for (ii = 0; ii < uiAuxBin1; ii++)
    {
        pMicFreq[ii].real = pAuxBins[ii].real;
        pMicFreq[ii].imag = pAuxBins[ii].imag;
    }

    for (ii = uiAuxBin1; ii < uiAuxBin2; ii++)
    {
        pMicFreq[ii].real = pMicFreq[ii].real / 2;
        pMicFreq[ii].imag = pMicFreq[ii].imag / 2;
    }

    pstAuxEng->uiFrameCount++;
    return ;
}

VOID SEVC_AUX_Reset(SEVC_AUX_S *pstAuxEng)
{
    U32 ii;
    U16 usFrameShift = pstAuxEng->pstSevcCfg->frameShift;
    aisp_s16_t *pfFrameOld = pstAuxEng->pfFrameOld;

    for (ii = 0; ii < usFrameShift; ii++)
    {
        pfFrameOld[ii] = 0;
    }

    return ;
}

VOID SEVC_AUX_Delete(SEVC_AUX_S *pstAuxEng)
{
    return ;
}

#endif
