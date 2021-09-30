#include <stdio.h>
#include <math.h>
#include "AISP_TSL_base.h"
#include "AISP_TSL_str.h"
#include "sevc_config.h"
#include "sevc_types.h"
#include "sevc_util.h"
#include "sevc_agc.h"

extern FILE *fpYyPcm;

#if SEVC_SWITCH_AGC

/************************************************************
  Function   : sevcAgcGainCalc()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
aisp_s32_t sevcAgcGainCalc(SEVC_AGC_S *pstAgcEng)
{
    SEVC_CONFIG_S *pstSevcCfg = pstAgcEng->pstSevcCfg;
    aisp_s32_t fVad = pstAgcEng->drc.fVadSmoothShort;
    aisp_s32_t fXDb = pstAgcEng->drc.fXDb;
    aisp_s32_t fGain;

    if (fXDb < pstSevcCfg->T1)
    {
        //fGain = fXDb;
        fGain = pstSevcCfg->G_low;
    }
    else
    {
        if ((pstSevcCfg->T1 <= fXDb) && (fXDb < pstSevcCfg->T2)
                && (fVad > pstSevcCfg->vad_thresh))
        {
            fGain = pstSevcCfg->T2 + pstSevcCfg->T2_gain
                    + PSHR((MULT32_32((fXDb - pstSevcCfg->T2), pstSevcCfg->ratio1)), 24); //W32Q24
        }
        else
        {
            if (fXDb >= pstSevcCfg->T2  && (fVad > pstSevcCfg->vad_thresh))
            {
                fGain = pstSevcCfg->T2 + pstSevcCfg->T2_gain
                        - PSHR((MULT32_32((pstSevcCfg->T2 - fXDb), pstSevcCfg->ratio2)), 24); //W32Q24
            }
            else
            {
                fGain = fXDb + pstSevcCfg->T2_gain;
            }
        }
    }

    return fGain;
}

/************************************************************
  Function   : sevcAgcDrc()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
VOID sevcAgcDrc(SEVC_AGC_S *pstAgcEng)
{
    //aisp_s32_t fYGain;
    SEVC_AGC_DRC_S *pDrc = &pstAgcEng->drc;
    aisp_s32_t fVadSmoothLongThr = pstAgcEng->pstSevcCfg->vad_smooth_long_thr;
    aisp_s32_t fVadSmoothShortThr = pstAgcEng->pstSevcCfg->vad_smooth_short_thr;
    aisp_s32_t fAlphaRInVAD = pstAgcEng->fAlphaRInVAD;
    aisp_s32_t fAlphaRSil = pstAgcEng->fAlphaRSil;
    aisp_s32_t fAlphaABeginVAD = pstAgcEng->fAlphaABeginVAD;
    aisp_s32_t fAlphaAInVAD = pstAgcEng->fAlphaAInVAD;
    aisp_s32_t fGainThr = pstAgcEng->pstSevcCfg->gainThreshold;
    aisp_s32_t iExpTmpIn    = 0;
    aisp_s64_t llTmp;
    aisp_s64_t llYGain, llGain;
    llYGain = sevcAgcGainCalc(pstAgcEng);
    llGain  = (aisp_s64_t)llYGain - pDrc->fXDb;

    if (llGain == pDrc->fGainSmooth)
    {
        pDrc->fGainSmooth = llGain;
    }
    else
    {
        if (llGain < pDrc->fGainSmooth && (pDrc->fVadSmoothLong >= fVadSmoothLongThr))
        {
            // release speech_based slow
            pDrc->fGainSmooth = PSHR((aisp_s64_t)(MULT32_32(fAlphaRInVAD, pDrc->fGainSmooth)
                                                  + (AGC_ONE - fAlphaRInVAD) * llGain), 24);
        }
        else
        {
            if (llGain < pDrc->fGainSmooth && (pDrc->fVadSmoothLong < fVadSmoothLongThr))
            {
                // % release   sil_based fast
                pDrc->fGainSmooth = PSHR((aisp_s64_t)(MULT32_32(fAlphaRSil, pDrc->fGainSmooth)
                                                      + (AGC_ONE - fAlphaRSil) * llGain), 24);
            }
            else
            {
                if ((llGain > pDrc->fGainSmooth) && (pDrc->fVadSmoothLong < fVadSmoothLongThr)
                        && (pDrc->fVadSmoothShort >= fVadSmoothShortThr))
                {
                    // % attack siltospeech fast
                    pDrc->fGainSmooth =
                        PSHR((aisp_s64_t)(MULT32_32(fAlphaABeginVAD, pDrc->fGainSmooth)
                                          + (AGC_ONE - fAlphaABeginVAD) * llGain), 24);
                }
                else
                {
                    if ((llGain > pDrc->fGainSmooth)  && (pDrc->fVadSmoothLong >= fVadSmoothLongThr))
                    {
                        // % attack speech_base slow
#ifdef AISPEECH_FIXED
                        pDrc->fGainSmooth =
                            PSHR((aisp_s64_t)(MULT32_32(fAlphaAInVAD, pDrc->fGainSmooth)
                                              + (AGC_ONE - fAlphaAInVAD) * llGain), 24);
#else
                        pDrc->fGainSmooth = fAlphaAInVAD * pDrc->fGainSmooth
                                            + (AGC_ONE - fAlphaAInVAD) * llGain;
#endif
                    }
                    else
                    {
                        pDrc->fGainSmooth = pDrc->fGainSmooth;
                    }
                }
            }
        }
    }

    if (((aisp_s64_t)pDrc->fGainSmooth + pDrc->fXDb) > fGainThr)
    {
        pDrc->fGainSmooth = fGainThr - pDrc->fXDb;
    }

    //Convert gain to linear
#ifdef AISPEECH_FIXED
    llTmp = PSHR(MULT32_32(1931548, pDrc->fGainSmooth), 24); //1931548=log(10)*0.05*16777216

    if (llTmp > 0x7FFFFFFF)
    {
        iExpTmpIn = 0x7FFFFFFF;
    }
    else
    {
        iExpTmpIn = (llTmp < -0x7FFFFFFF ? -0x7FFFFFFF : llTmp);
    }

    pDrc->fGainLinear = sevcExp(iExpTmpIn, 24, 24);
#else
    pDrc->fGainLinear = powf(10, pDrc->fGainSmooth * 0.05);
#endif
}

/************************************************************
  Function   : sevcAgcPreprocess()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
VOID sevcAgcPreprocess(SEVC_AGC_S *pstAgcEng, aisp_s16_t *pFrame, aisp_s32_t fVad)
{
    U16 usLoop;
    aisp_s32_t *pfVadBuff = (aisp_s32_t *)pstAgcEng->pfVadBuff;
    aisp_s32_t *pfVadShortBuff = pstAgcEng->pfVadShortBuff;
    SEVC_AGC_DRC_S *pDrc = &pstAgcEng->drc;
    //U16 usFrameShift = pstAgcEng->pstSevcCfg->frameShift;
    U16 usFrameSize = pstAgcEng->pstSevcCfg->usFrameSize;
    U16 usVadBuffLen = pstAgcEng->usVadBuffLen;
    U16 usVadShortLen = pstAgcEng->pstSevcCfg->vad_smooth_short_len;
    aisp_s32_t fDataTmp = 0, fDataMax = 0;
    aisp_s64_t llDataTmp = 0;
    // if (fVad > AGC_ONE)
    // {
    //     MSG_DBG("%s [%d] %d +++++\r\n", __func__, __LINE__, fVad);
    //     fVad = AGC_ONE;
    // }
    //vad calc
#if 0
    pDrc->fVadSmoothLong += DIV32_32((fVad - pfVadBuff[pstAgcEng->usVadIdx]), usVadBuffLen); //W32Q24
    pDrc->fVadSmoothShort += DIV32_32((fVad - pfVadBuff[usVadShortIdx]), usVadShortLen); //W32Q24
    pfVadBuff[pstAgcEng->usVadIdx] = fVad;
#else
    pfVadBuff[pstAgcEng->usVadIdx] = fVad;

    for (usLoop = 0; usLoop < usVadBuffLen; usLoop++)
    {
        llDataTmp += pfVadBuff[usLoop];
    }

    pDrc->fVadSmoothLong = (llDataTmp + (usVadBuffLen >> 1)) / usVadBuffLen; //W32Q24
    llDataTmp = 0;
    pfVadShortBuff[pstAgcEng->usVadShortIdx] = fVad;

    for (usLoop = 0; usLoop < usVadShortLen; usLoop++)
    {
        llDataTmp += pfVadShortBuff[usLoop];
    }

    pDrc->fVadSmoothShort = (llDataTmp + (usVadShortLen >> 1)) / usVadShortLen; //W32Q24
#endif
    pstAgcEng->usVadIdx = (pstAgcEng->usVadIdx + 1) % usVadBuffLen;
    pstAgcEng->usVadShortIdx = (pstAgcEng->usVadShortIdx + 1) % usVadShortLen;

    //xDb calc
    for (usLoop = 0; usLoop < usFrameSize; usLoop++)
    {
        fDataTmp = (aisp_s32_t)pFrame[usLoop];
        fDataTmp = sevcAbsf(fDataTmp);

        if (fDataTmp > fDataMax)
        {
            fDataMax = fDataTmp;
        }
    }

    if (fDataMax == 0)
    {
        pDrc->fXDb = -1677721600; //SHL32(-100, 24);
    }
    else
    {
        //xdB = 20 * log10(max(abs(sig_in)) + 1e-10 );
#ifdef AISPEECH_FIXED
#if 1
        fDataTmp = sevcLog(fDataMax, 15, 24);
        //pDrc->fXDb = (aisp_s32_t)MULT32_32_P15(284619, fDataTmp); //W32Q24 //284619=20.0/ln(10)*32768
        pDrc->fXDb = PSHR((aisp_s64_t)fDataTmp * 9326402983, 30);
#else
        llDataTmp = round(20 * log10(1.0 * fDataMax / (1 << 15)) * (1 << 24));

        if (llDataTmp > 0x7fffffff || llDataTmp < -0x7fffffff)
        {
            MSG_DBG("_________________________________\r\n");
        }

        pDrc->fXDb = llDataTmp;
#endif
#else
        pDrc->fXDb = 20.0f * log10f(fDataMax + 1e-10);
#endif
    }
}

/************************************************************
  Function   : SEVC_AGC_ShMemSizeGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/23/02, shizhang.tang create
************************************************************/
U32 SEVC_AGC_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    return uiTotalMemSize;
}

/************************************************************
  Function   : SEVC_AGC_LocMemSizeGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
U32 SEVC_AGC_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    //U16 usSizeComp = sizeof(aisp_cpx_s32_t);
    //U16 usFftBins = pstSevcCfg->usFftBin;
    //U16 usFrameShift = pstSevcCfg->frameShift;
    U16 usFrameSize = pstSevcCfg->usFrameSize;
    uiTotalMemSize += SIZE_ALIGN(sizeof(SEVC_AGC_S));
    //pfFrameOut
    uiTotalMemSize += SIZE_ALIGN(usFrameSize * sizeof(aisp_s16_t));
    //pfVadBuff
    uiTotalMemSize += SIZE_ALIGN(pstSevcCfg->vad_smooth_long_len * sizeof(aisp_s32_t));
    //pfVadShortBuff
    uiTotalMemSize += SIZE_ALIGN(pstSevcCfg->vad_smooth_short_len * sizeof(aisp_s32_t));
#ifndef USE_SHARE_MEM
    uiTotalMemSize += SEVC_AGC_ShMemSizeGet(pstSevcCfg);
#endif
#if MEM_BUFF_DBG
    MSG_DBG("%s uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}

/************************************************************
  Function   : SEVC_AGC_New()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
SEVC_AGC_S *SEVC_AGC_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator)
{
    U16 usSize;
    //U16 usFftBins = pstSevcCfg->usFftBin;
    U16 usFrameSize = pstSevcCfg->usFrameSize;
    //U16 usSizeComp = sizeof(aisp_cpx_s32_t);
    SEVC_AGC_S *pstAgcEng = NULL;
#ifndef AISPEECH_FIXED
    U16 usFrameShift = pstSevcCfg->frameShift;
    U16 usFs = pstSevcCfg->fs;
#endif
    //S32 LOG9Q24 = 36863311;

    if (NULL == pstSevcCfg || NULL == pstMemLocator)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
    }

    MEM_LOCATOR_SHOW(pstMemLocator);
    usSize = sizeof(SEVC_AGC_S);
    pstAgcEng = (SEVC_AGC_S *)memLocatorGet(pstMemLocator, usSize, AISP_ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstAgcEng, FAIL);
    pstAgcEng->pstSevcCfg = pstSevcCfg;
    pstAgcEng->usVadBuffLen = pstSevcCfg->vad_smooth_long_len;
    pstAgcEng->fGainSmooth = 0;
    pstAgcEng->uiFrameCount = 0;
    pstAgcEng->usVadIdx = 0;
    pstAgcEng->usVadShortIdx = 0;
#ifdef AISPEECH_FIXED
#if 1
    pstAgcEng->fAlphaABeginVAD = pstSevcCfg->agcAlphaABeginVAD;
    pstAgcEng->fAlphaAInVAD    = pstSevcCfg->agcAlphaAInVAD;
    pstAgcEng->fAlphaRSil      = pstSevcCfg->agcAlphaRSil;
    pstAgcEng->fAlphaRInVAD    = pstSevcCfg->agcAlphaRInVAD;
#else
    S64 LOG9Q48 = 618463736733793;
    S32 fDataTmp;
    S64 llDataTmp1, llDataTmp2;
    //alphaA_beginVAD = exp(-log(9)/( AttackTime_beginVAD * fs/Lk));
    llDataTmp1 = LOG9Q48 * usFrameShift;
    llDataTmp2 = MULT32_32(pstSevcCfg->AttackTime_beginVAD, usFs);
    fDataTmp = PDIV64((-llDataTmp1), llDataTmp2);
    //fDataTmp = -(LOG9Q24*usFrameShift+)/(pstSevcCfg->AttackTime_beginVAD * usFs);
    pstAgcEng->fAlphaABeginVAD = sevcExp(fDataTmp, 24, 24);
    //alphaA_inVAD    = exp(-log(9)/( AttackTime_inVAD * fs/Lk));
    llDataTmp2 = MULT32_32(pstSevcCfg->AttackTime_inVAD, usFs);
    fDataTmp = PDIV64((-llDataTmp1), llDataTmp2);
    pstAgcEng->fAlphaAInVAD    = sevcExp(fDataTmp, 24, 24);
    //alphaR_sil      = exp(-log(9)/( ReleaseTime_sil* fs/Lk));
    llDataTmp2 = MULT32_32(pstSevcCfg->ReleaseTime_inVAD, usFs);
    pstAgcEng->fAlphaRInVAD    = sevcExp(fDataTmp, 24, 24);
    //alphaR_inVAD    = exp(-log(9)/( ReleaseTime_inVAD* fs/Lk));
    llDataTmp2 = MULT32_32(pstSevcCfg->ReleaseTime_sil, usFs);
    pstAgcEng->fAlphaRSil      = sevcExp(fDataTmp, 24, 24);
#endif
#else
    pstAgcEng->fAlphaABeginVAD = exp(-LOG(9)
                                     / (pstSevcCfg->AttackTime_beginVAD * usFs / usFrameShift));
    pstAgcEng->fAlphaAInVAD    = exp(-LOG(9)
                                     / (pstSevcCfg->AttackTime_inVAD * usFs / usFrameShift));
    pstAgcEng->fAlphaRSil      = exp(-LOG(9)
                                     / (pstSevcCfg->ReleaseTime_sil * usFs / usFrameShift));
    pstAgcEng->fAlphaRInVAD    = exp(-LOG(9)
                                     / (pstSevcCfg->ReleaseTime_inVAD * usFs / usFrameShift));
#endif
    AISP_TSL_memset((VOID *)&pstAgcEng->drc, 0, (size_t)sizeof(pstAgcEng->drc));
    pstAgcEng->drc.fGainSmooth = pstAgcEng->fGainSmooth;
    //pfFrameOut
    BUFF_POINT_SET_ALIGN(pstAgcEng->pfFrameOut, (usFrameSize * sizeof(aisp_s16_t)),
                         pstMemLocator, FAIL);
    //pfVadBuff
    BUFF_POINT_SET_ALIGN(pstAgcEng->pfVadBuff,
                         (pstAgcEng->usVadBuffLen * sizeof(aisp_s32_t)),
                         pstMemLocator, FAIL);
    //pfVadShortBuff
    BUFF_POINT_SET_ALIGN(pstAgcEng->pfVadShortBuff,
                         (pstSevcCfg->vad_smooth_short_len * sizeof(aisp_s32_t)),
                         pstMemLocator, FAIL);
    MEM_LOCATOR_SHOW(pstMemLocator);
    return pstAgcEng;
FAIL:
    MSG_DBG("%s [%d] fail\r\n", __func__, __LINE__);
    return NULL;
}

/************************************************************
  Function   : SEVC_AGC_CallBackFuncRegister()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
VOID SEVC_AGC_CallBackFuncRegister(SEVC_AGC_S *pstAgcEng,
                                   VOID *pCBFunc, VOID *pUsrData)
{
    if (NULL == pstAgcEng || NULL == pCBFunc)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return ;
    }

    pstAgcEng->pCallBackFunc = (CallBackFunc)pCBFunc;
    pstAgcEng->pUsrData = pUsrData;
}


/************************************************************
  Function   : SEVC_AGC_Feed()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
S32 SEVC_AGC_Feed(SEVC_AGC_S *pstAgcEng, aisp_s16_t *pFrame, aisp_s32_t fVad)
{
    aisp_s64_t llTmpY;
    U16 usLoop;
    //U16 usFrameShift = pstAgcEng->pstSevcCfg->frameShift;
    U16 usFrameSize = pstAgcEng->pstSevcCfg->usFrameSize;
    SEVC_AGC_DRC_S *pDrc = &pstAgcEng->drc;
    //vad calc
    sevcAgcPreprocess(pstAgcEng, pFrame, fVad);
    //gain calc
    sevcAgcDrc(pstAgcEng);

    //signal process
    for (usLoop = 0; usLoop < usFrameSize; usLoop++)
    {
#ifdef AISPEECH_FIXED
        llTmpY = PSHR((aisp_s64_t)pFrame[usLoop] * pDrc->fGainLinear, 24);

        if (llTmpY > 32767)
        {
            pstAgcEng->pfFrameOut[usLoop] = 32767;
        }
        else
        {
            pstAgcEng->pfFrameOut[usLoop] = (llTmpY < -32767 ? -32767 : llTmpY);
        }

#else
        pstAgcEng->pfFrameOut[usLoop] = pFrame[usLoop] * pDrc->fGainLinear;
#endif
    }

    //callback
    if (NULL != pstAgcEng->pCallBackFunc)
    {
        pstAgcEng->pCallBackFunc((U8 *)pstAgcEng->pfFrameOut, usFrameSize * sizeof(aisp_s16_t),
                                 pstAgcEng->pUsrData);
    }

    pstAgcEng->uiFrameCount++;
    return 0;
}

/************************************************************
  Function   : SEVC_AGC_Reset()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
VOID SEVC_AGC_Reset(SEVC_AGC_S *pstAgcEng)
{
}

/************************************************************
  Function   : SEVC_AGC_Delete()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
VOID SEVC_AGC_Delete(SEVC_AGC_S *pstAgcEng)
{
}

#endif
