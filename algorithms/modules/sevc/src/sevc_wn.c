#include <stdio.h>
#include "AISP_TSL_base.h"
#include "AISP_TSL_str.h"
#include "AISP_TSL_common.h"
#include "AISP_TSL_complex.h"
#include "sevc_config.h"
#include "sevc_types.h"
#include "sevc_util.h"
#include "sevc_wn.h"

#if SEVC_SWITCH_WN
VOID wnStatusUpdate(SEVC_WN_S *pstWnEng, U8 ucWindDetect)
{
    SEVC_CONFIG_S *pstSevcCfg = pstWnEng->pstSevcCfg;

    if (0 == pstWnEng->uiWindNoiseState)
    {
        if (ucWindDetect)
        {
            if (++pstWnEng->uiSpeCnt >= pstSevcCfg->uiWnSil2Spe)
            {
                pstWnEng->uiWindNoiseState = 1;
                pstWnEng->uiSpeCnt = 0;
            }
        }
        else
        {
            pstWnEng->uiSpeCnt = 0;
        }
    }
    else
    {
        if (0 == ucWindDetect)
        {
            if (++pstWnEng->uiSilCnt >= pstSevcCfg->uiWnSpe2Sil)
            {
                pstWnEng->uiWindNoiseState = 0;
                pstWnEng->uiSilCnt = 0;
            }
        }
        else
        {
            pstWnEng->uiSilCnt = 0;
        }
    }
}

U32 SEVC_WN_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    //U16 usFftBin = pstSevcCfg->usFftBin;
    U16 usMicNum = pstSevcCfg->micChan;
    uiTotalMemSize += SIZE_ALIGN(sizeof(SEVC_WN_S));
    uiTotalMemSize += SIZE_ALIGN(usMicNum * sizeof(aisp_s32_t));
#if MEM_BUFF_DBG
    MSG_DBG("%s uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}

U32 SEVC_WN_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    return 0;
}

SEVC_WN_S *SEVC_WN_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator)
{
    SEVC_WN_S *pstWnEng;
    U32 uiSize;
    U16 usMicNum = pstSevcCfg->micChan;

    if (NULL == pstSevcCfg || NULL == pstMemLocator)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return NULL;
    }

    MEM_LOCATOR_SHOW(pstMemLocator);
    uiSize = sizeof(SEVC_WN_S);
    pstWnEng = (SEVC_WN_S *)memLocatorGet(pstMemLocator, uiSize, AISP_ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstWnEng, FAIL);
    pstWnEng->pstSevcCfg = pstSevcCfg;
    BUFF_POINT_SET_ALIGN(pstWnEng->pfMicAmpSmooth, (usMicNum * sizeof(aisp_s32_t)),
                         pstMemLocator, FAIL);
    MEM_LOCATOR_SHOW(pstMemLocator);
    return pstWnEng;
FAIL:
    return NULL;
}

/************************************************************
  Function   : SEVC_WN_Feed()

  Description:
  Calls      :
  Called By  :
  Input      : pfFrame
  Output     : pfFrame
  Return     :
  Others     :

  History    :
    2019/09/04, shizhang.tang create
************************************************************/
VOID SEVC_WN_Feed(SEVC_WN_S *pstWnEng, aisp_s16_t *pfMicFrame)
{
    S32 ii, sMicIdx;
    U8 ucWindDetect = 0;
    SEVC_CONFIG_S *pstSevcCfg = pstWnEng->pstSevcCfg;
    U16 usMicNum =  pstSevcCfg->micChan;
    aisp_s32_t fMicSmoothAlpha = pstSevcCfg->fWnMicSmoothAlpha;
    aisp_s32_t fWnRateSmoothAlpha = pstSevcCfg->fWnRateSmoothAlpha;
    aisp_s32_t fRefThresh = pstSevcCfg->fRefThresh;
    aisp_s32_t fMinPower = 0xEFFF;
    aisp_s32_t fMaxPower = 0;
    aisp_s32_t fRefPower = 0;
    aisp_s32_t fFrameSum = 0;
    aisp_s32_t fMeanData = 0;
    aisp_s32_t fData = 0;
    aisp_s32_t fRate = 0;
    aisp_s16_t *pDst = NULL;
    aisp_s16_t *pSrc = NULL;
    aisp_s16_t *pfFrameNew = pfMicFrame;
    aisp_s16_t *pfRefFrame =  pfMicFrame + pstSevcCfg->frameShift * usMicNum;

    for (sMicIdx = 0; sMicIdx < usMicNum; sMicIdx++)
    {
        fFrameSum = 0;

        for (ii = 0; ii < pstSevcCfg->frameShift; ii++)
        {
            fFrameSum += (pfFrameNew[ii] < 0 ? -pfFrameNew[ii] : pfFrameNew[ii]); //W32Q15
        }

        fMeanData = fFrameSum / pstSevcCfg->frameShift; //W32Q15
        fMeanData -= pstWnEng->pfMicAmpSmooth[sMicIdx];
        fData = MULT32_32_P15(fMeanData, fMicSmoothAlpha);
        pstWnEng->pfMicAmpSmooth[sMicIdx] += fData;
        pfFrameNew += pstSevcCfg->frameShift;
    }

    for (sMicIdx = 0; sMicIdx < usMicNum; sMicIdx++)
    {
        fMaxPower = MAX(pstWnEng->pfMicAmpSmooth[sMicIdx], fMaxPower);
        fMinPower = MIN(pstWnEng->pfMicAmpSmooth[sMicIdx], fMinPower);
    }

    fFrameSum = 0;

    for (ii = 0; ii < pstSevcCfg->frameShift; ii++)
    {
        fFrameSum += (pfRefFrame[ii] < 0 ? -pfRefFrame[ii] : pfRefFrame[ii]); //W32Q15
    }

    fRefPower = fFrameSum / pstSevcCfg->frameShift; //W32Q15
    fRate = (aisp_s64_t)1048576 * fMaxPower / (fMinPower + 1); //W32Q20
    fData = fRate - pstWnEng->fRateSmooth;
    fData = MULT32_32_P15(fData, fWnRateSmoothAlpha);
    pstWnEng->fRateSmooth += fData;
    fRate = pstWnEng->fRateSmooth;

    if ((fMaxPower > pstSevcCfg->fWnPowerThresh1 && fRate > pstSevcCfg->fWnRateThresh1 && fRefPower < fRefThresh)
            || (fMaxPower > pstSevcCfg->fWnPowerThresh2 && fRate > pstSevcCfg->fWnRateThresh2 && fRefPower < fRefThresh)
            || (fMaxPower > pstSevcCfg->fWnPowerThresh3 && fRate > pstSevcCfg->fWnRateThresh3 && fRefPower < fRefThresh))
    {
        ucWindDetect = 1;

        //just support dual mic
        for (sMicIdx = 0; sMicIdx < usMicNum; sMicIdx++)
        {
            if (fMinPower == pstWnEng->pfMicAmpSmooth[sMicIdx])
            {
                pstWnEng->uiWindIdx = sMicIdx;
                break;
            }
        }
    }

    wnStatusUpdate(pstWnEng, ucWindDetect);

    if (pstWnEng->uiWindNoiseState)
    {
        pSrc = pfMicFrame + pstWnEng->uiWindIdx * pstSevcCfg->frameShift;

        for (sMicIdx = 0; sMicIdx < usMicNum; sMicIdx++)
        {
            if (pstWnEng->uiWindIdx != sMicIdx)
            {
                pDst = pfMicFrame + sMicIdx * pstSevcCfg->frameShift;

                for (ii = 0; ii < pstSevcCfg->frameShift; ii++)
                {
                    pDst[ii] = pSrc[ii];
                }
            }
        }
    }

    pstWnEng->uiFrameCount++;
    return ;
}

VOID SEVC_WN_Reset(SEVC_WN_S *pstWnEng)
{
    S32 ii;
    pstWnEng->uiFrameCount = 0;
    pstWnEng->uiSilCnt = 0;
    pstWnEng->uiSpeCnt = 0;
    pstWnEng->uiWindNoiseState = 0;
    pstWnEng->fRateSmooth = 0;

    for (ii = 0; ii < pstWnEng->pstSevcCfg->micChan; ii++)
    {
        pstWnEng->pfMicAmpSmooth[ii] = 0;
    }
}

VOID SEVC_WN_Delete(SEVC_WN_S *pstWnEng)
{
}

#endif

