#include<stdio.h>
#include "AISP_TSL_base.h"
#include "AISP_TSL_str.h"
#include "AISP_TSL_common.h"
#include "AISP_TSL_complex.h"
#include "fft_wrapper.h"
#include "sevc_config.h"
#include "sevc_types.h"
#include "sevc_util.h"
#include "sevc_nr.h"

#if SEVC_SWITCH_NR
U32 SEVC_NR_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    U16 usFftBin = pstSevcCfg->usFftBin;
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s32_t));
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s32_t));
    return uiTotalMemSize;
}

U32 SEVC_NR_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    U16 usFftBin = pstSevcCfg->usFftBin;
    uiTotalMemSize += SIZE_ALIGN(sizeof(SEVC_NR_S));
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s32_t));
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_cpx_s32_t));
#ifndef USE_SHARE_MEM
    uiTotalMemSize += SEVC_NR_ShMemSizeGet(pstSevcCfg);
#endif
#if MEM_BUFF_DBG
    MSG_DBG("%s uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}

VOID SEVC_NR_CallBackFuncRegister(SEVC_NR_S *pstNrEng,
                                  VOID *pCBFunc, VOID *pUsrData)
{
    if (NULL == pstNrEng || NULL == pCBFunc)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return ;
    }

    pstNrEng->pCallBackFunc = (CallBackFunc)pCBFunc;
    pstNrEng->pUsrData = pUsrData;
}

SEVC_NR_S *SEVC_NR_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator)
{
    SEVC_NR_S *pstNrEng;
    U32 uiSize;
    U16 usFftBin = pstSevcCfg->usFftBin;

    if (NULL == pstSevcCfg || NULL == pstMemLocator)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return NULL;
    }

    MEM_LOCATOR_SHOW(pstMemLocator);
    uiSize = sizeof(SEVC_NR_S);
    pstNrEng = (SEVC_NR_S *)memLocatorGet(pstMemLocator, uiSize, AISP_ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstNrEng, FAIL);
    pstNrEng->pstSevcCfg = pstSevcCfg;
    uiSize = usFftBin * sizeof(aisp_cpx_s32_t);
    BUFF_POINT_SET_ALIGN(pstNrEng->pOutFreq, uiSize, pstMemLocator, FAIL);
    uiSize = usFftBin * sizeof(aisp_s32_t);
    BUFF_POINT_SET_ALIGN(pstNrEng->pfNoisePs, uiSize, pstMemLocator, FAIL);
    SH_BUFF_POINT_SET_ALIGN(pstNrEng->pfAdk, uiSize, pstMemLocator, FAIL);
    SH_BUFF_POINT_SET_ALIGN(pstNrEng->pfNoisyPower, uiSize, pstMemLocator, FAIL);
    MEM_LOCATOR_SHOW(pstMemLocator);
    return pstNrEng;
FAIL:
    return NULL;
}

VOID SEVC_NR_Feed(SEVC_NR_S *pstNrEng, aisp_cpx_s32_t *pMicFreq, aisp_s32_t *pfGain)
{
    S32 ii;
    U16 usFftBin = pstNrEng->pstSevcCfg->usFftBin;
    aisp_s32_t *pfGainNew = pstNrEng->pfAdk;
    aisp_s32_t *pfSpp = pstNrEng->pfAdk;
    aisp_s32_t *pfAdk = pstNrEng->pfAdk;
    aisp_s32_t fNrGainTrig = pstNrEng->pstSevcCfg->fNrGainTrig;
    aisp_s32_t fNrMcraAd = pstNrEng->pstSevcCfg->fNrMcraAd;
    aisp_s32_t *pfNoisyPower = pstNrEng->pfNoisyPower;
    aisp_s32_t *pfNoisePs = pstNrEng->pfNoisePs;
    aisp_s32_t fData = 0;
    aisp_cpx_s32_t *pOutFreq = pstNrEng->pOutFreq;

    if (NULL != pfGain)
    {
        for (ii = 0; ii < usFftBin; ii++)
        {
            pfSpp[ii] = (pfGain[ii] > fNrGainTrig ? 1048576 : pfGain[ii]); //W32Q20
        }
    }
    else
    {
        for (ii = 0; ii < usFftBin; ii++)
        {
            pfSpp[ii] = 0;
        }
    }

    for (ii = 0; ii < usFftBin; ii++)
    {
        fData = MULT32_32_SQ(fNrMcraAd, pfSpp[ii], 20);
        pfAdk[ii] = fNrMcraAd + pfSpp[ii] - fData;                      //W32Q20
    }

    for (ii = 0; ii < usFftBin; ii++)
    {
        fData = 1048576 - pfAdk[ii];
        COMPLEX_POW_SQ(pfNoisyPower[ii], pMicFreq[ii], 28);             //W32Q14
        pfNoisePs[ii] = MULT32_32_SQ(pfNoisePs[ii], pfAdk[ii], 20)
                        + MULT32_32_SQ(pfNoisyPower[ii], fData, 20);
    }

    //gain = min(1, max(0.5, 1- noise_mu2./Noisy_POWER));
    for (ii = 0; ii < usFftBin; ii++)
    {
        pfGainNew[ii] = (((aisp_s64_t)pfNoisePs[ii] << 1) >= pfNoisyPower[ii] ? 524288
                         : (1048576 - (aisp_s64_t)1048576 * pfNoisePs[ii] / (pfNoisyPower[ii] + 1))); //W32Q20
        pfGainNew[ii] = (pfGainNew[ii] < 1048576 ? pfGainNew[ii] : 1048576);
    }

    for (ii = 0; ii < usFftBin; ii++)
    {
        COMPLEX_MUL_FLOAT_SQ(pOutFreq[ii], pMicFreq[ii], pfGainNew[ii], 20);
    }

    if (pstNrEng->pCallBackFunc)
    {
        pstNrEng->pCallBackFunc((U8 *)pOutFreq, usFftBin * sizeof(aisp_cpx_s32_t), pstNrEng->pUsrData);
    }

    pstNrEng->uiFrameCount++;
    return ;
}

VOID SEVC_NR_Reset(SEVC_NR_S *pstNrEng)
{
    S32 ii;
    U16 usFftBin = pstNrEng->pstSevcCfg->usFftBin;
    aisp_s32_t *pfNoisePs = pstNrEng->pfNoisePs;

    for (ii = 0; ii < usFftBin; ii++)
    {
        pfNoisePs[ii] = 0;
    }

    return ;
}

VOID SEVC_NR_Delete(SEVC_NR_S *pstNrEng)
{
    return ;
}

#endif
