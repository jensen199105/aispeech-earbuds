#include <stdio.h>
#include "AISP_TSL_str.h"
#include "sevc_config.h"
#include "sevc_types.h"
#include "sevc_util.h"

#include "sevc_vad.h"

extern FILE *fpYyPcm;

U32 SEVC_VAD_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    U32 uiSize;
#ifdef SEVC_SWITCH_VAD
    // ref signal sum
    uiSize = sizeof(aisp_s32_t) * pstSevcCfg->frameShift;
    uiSize = SIZE_ALIGN(uiSize);
    uiTotalMemSize += uiSize;
#endif
    return uiTotalMemSize;
}

U32 SEVC_VAD_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    U32 uiSize;
#ifdef SEVC_SWITCH_VAD
    uiSize = SIZE_ALIGN(sizeof(SEVC_VAD_S));
    uiTotalMemSize += uiSize;
#endif
#ifndef USE_SHARE_MEM
    uiTotalMemSize += SEVC_VAD_ShMemSizeGet(pstSevcCfg);
#endif
#if MEM_BUFF_DBG
    MSG_DBG("%s, uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}

SEVC_VAD_S *SEVC_VAD_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator)
{
    SEVC_VAD_S *pstSevcVadEng = NULL;
    U32 uiSize;
    MEM_LOCATOR_SHOW(pstMemLocator);
    uiSize = sizeof(SEVC_VAD_S);
    pstSevcVadEng = (SEVC_VAD_S *)memLocatorGet(pstMemLocator, uiSize, ALIGN_ON);
    pstSevcVadEng->pstSevcCfg = pstSevcCfg;
    AISP_TSL_PTR_CHECK(pstSevcVadEng, FAIL);
    MEM_LOCATOR_SHOW(pstMemLocator);
    // allocate for ref sum buffer
    uiSize = pstSevcCfg->frameShift * sizeof(aisp_s32_t);
    SH_BUFF_POINT_SET_ALIGN(pstSevcVadEng->pRefSum, uiSize, pstMemLocator, FAIL);
    // initialize member variables
    pstSevcVadEng->vadAmth = pstSevcCfg->fAmth; // amp threshold
    pstSevcVadEng->sil2Spe = pstSevcCfg->sil2spe;
    pstSevcVadEng->spe2Sil = pstSevcCfg->spe2sil;
    pstSevcVadEng->silCnt = 0;
    pstSevcVadEng->speCnt = 0;
    return pstSevcVadEng;
FAIL:
    MSG_DBG("%s, [%d], fails\r\n", __func__, __LINE__);
    return NULL;
}

S32 SEVC_VAD_Feed(SEVC_VAD_S *pstSevcVadEng, aisp_s16_t *pRefFrame)
{
    U16 usRefNum = pstSevcVadEng->pstSevcCfg->refChan;
    U16 usFrameShift = pstSevcVadEng->pstSevcCfg->frameShift;
    S32 refIdx;
    S32 frameIdx;
    aisp_s32_t *pRefSum;
    aisp_s32_t refSums = 0;
    aisp_s32_t refMean;
    aisp_s32_t vadAmp = 0;
    // sum all absolute values of ref along channels
    // set all zero
    AISP_TSL_memset(pstSevcVadEng->pRefSum, 0, sizeof(aisp_s32_t) * usFrameShift);

    for (refIdx = 0; refIdx < usRefNum; ++refIdx)
    {
        pRefSum = pstSevcVadEng->pRefSum;

        for (frameIdx = 0; frameIdx < usFrameShift; ++frameIdx)
        {
            *pRefSum += (aisp_s32_t)AISP_TSL_ABS(*pRefFrame);
            pRefFrame++;
            pRefSum++;
        }
    }

    pRefSum = pstSevcVadEng->pRefSum;

    for (frameIdx = 0; frameIdx < usFrameShift; ++frameIdx)
    {
        refSums += *pRefSum++;
    }

    refMean = PDIV(refSums, usFrameShift);
    pRefSum = pstSevcVadEng->pRefSum;

    for (frameIdx = 0; frameIdx < usFrameShift; ++frameIdx)
    {
        vadAmp += AISP_TSL_ABS(pRefSum[frameIdx] - refMean);
    }

    vadAmp = PDIV(vadAmp, usFrameShift);

    if (pstSevcVadEng->vadFlag == 0)
    {
        if (vadAmp > pstSevcVadEng->vadAmth)
        {
            pstSevcVadEng->speCnt += 1;

            if (pstSevcVadEng->speCnt >= pstSevcVadEng->sil2Spe)
            {
                pstSevcVadEng->vadFlag = 1;
                pstSevcVadEng->speCnt = 0;
            }
        }
        else
        {
            pstSevcVadEng->speCnt = 0;
        }
    }
    else
    {
        if (vadAmp <= pstSevcVadEng->vadAmth)
        {
            pstSevcVadEng->silCnt += 1;

            if (pstSevcVadEng->silCnt >= pstSevcVadEng->spe2Sil)
            {
                pstSevcVadEng->vadFlag = 0;
                pstSevcVadEng->silCnt = 0;
            }
        }
        else
        {
            pstSevcVadEng->silCnt = 0;
        }
    }

    pstSevcVadEng->uiFrameCount++;
    return pstSevcVadEng->vadFlag;
}