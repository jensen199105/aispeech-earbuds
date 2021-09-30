#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include "AISP_TSL_str.h"
#include "AISP_TSL_types.h"
#include "AISP_TSL_base.h"
#include "AISP_TSL_sqrt.h"
#include "fft_wrapper.h"

#include "sevc_config.h"
#include "sevc_util.h"
#include "sevc_wn.h"
#include "sevc_vad.h"
#include "sevc_state.h"
#include "sevc_aec.h"
#include "sevc_bf.h"
#include "sevc_gsc.h"
#include "sevc_bfpost.h"
#include "sevc_aux.h"
#include "sevc_nr.h"
#include "sevc_nn.h"
#include "sevc_aes.h"
#include "sevc_eq.h"
#include "sevc_agc.h"
#include "sevc_func.h"

extern FILE *fpYyPcm;

VOID sevcApplyScalarGainToComplex(aisp_cpx_s32_t *pfFrame, aisp_s32_t gain, U16 len, U32 GainQ)
{
    U16 binIdx;

    for (binIdx = 0; binIdx < len; ++binIdx)
    {
        pfFrame->real = PSHR((aisp_s64_t)pfFrame->real * gain, GainQ);
        pfFrame->imag = PSHR((aisp_s64_t)pfFrame->imag * gain, GainQ);
        pfFrame++;
    }
}

VOID sevcApplyVectorGainToComplex(aisp_cpx_s32_t *pfFrame, aisp_s32_t *gain, U16 len, U32 GainQ)
{
    U16 binIdx;

    for (binIdx = 0; binIdx < len; ++binIdx)
    {
        pfFrame->real = PSHR((aisp_s64_t)pfFrame->real * gain[binIdx], GainQ);
        pfFrame->imag = PSHR((aisp_s64_t)pfFrame->imag * gain[binIdx], GainQ);
        pfFrame++;
    }
}

VOID sevcComputeOverlapGain(SEVC_CONFIG_S *pstSevcCfg, aisp_s32_t *overlapGain)
{
    U16 usFrameShift = pstSevcCfg->frameShift;
    U16 usFrameOldSize = pstSevcCfg->usFrameSize - usFrameShift;
    aisp_s16_t *hanningWin = pstSevcCfg->pfHanningWindow;
    U16 usLoop;

    for (usLoop = 0; usLoop < usFrameShift; ++usLoop)
    {
        overlapGain[usLoop] = hanningWin[usLoop];
    }

    for (usLoop = 0; usLoop < usFrameOldSize; ++usLoop)
    {
        overlapGain[usLoop] += hanningWin[usFrameShift + usLoop];
    }

    for (usLoop = 0; usLoop < usFrameShift; ++usLoop)
    {
        overlapGain[usLoop] = PDIV(AISP_TSL_Q15_ONE * AISP_TSL_Q15_ONE_MINI, overlapGain[usLoop]);
    }
}

VOID sevcFrameWindownAdd(aisp_s32_t *pfFramwWide, aisp_s16_t *pfWindow,
                         aisp_s16_t *pfFrameOld, aisp_s16_t *pfFrameNew, U16 usFrameOldSize, U16 usFrameShift)
{
#ifdef JIELI_BR28
#ifdef FREQ_BIN_Q21
    sevcVecMUL16X16_Real(pfFramwWide, pfFrameOld,
                         pfWindow, usFrameOldSize, 9);//W32Q21
#else
    sevcVecMUL16X16_Real(pfFramwWide, pfFrameOld,
                         pfWindow, usFrameOldSize, 15);//W32Q15
#endif
#else
#ifdef FREQ_BIN_Q21
    sevcVecMUL16X16_Real(pfFramwWide, pfFrameOld,
                         pfWindow, usFrameOldSize, 0); //W32Q30
#else
    sevcVecMUL16X16_Real(pfFramwWide, pfFrameOld,
                         pfWindow, usFrameOldSize, 6); //W32Q24
#endif
#endif
    pfWindow += usFrameOldSize;
    pfFramwWide += usFrameOldSize;
#ifdef JIELI_BR28
#ifdef FREQ_BIN_Q21
    sevcVecMUL16X16_Real(pfFramwWide, pfFrameNew,
                         pfWindow, usFrameShift, 9); //W32Q21
#else
    sevcVecMUL16X16_Real(pfFramwWide, pfFrameNew,
                         pfWindow, usFrameShift, 15); //W32Q15
#endif
#else
#ifdef FREQ_BIN_Q21
    sevcVecMUL16X16_Real(pfFramwWide, pfFrameNew,
                         pfWindow, usFrameShift, 0); //W32Q30
#else
    sevcVecMUL16X16_Real(pfFramwWide, pfFrameNew,
                         pfWindow, usFrameShift, 6); //W32Q24
#endif
#endif
}

VOID sevcFftProcess(SEVC_S *pstSevc, aisp_s16_t *pfFrame)
{
    S16 sLoop, sMicIdx;
    U16 usMixNum = pstSevc->pstSevcCfg->micChan;
    U16 usFftBin = pstSevc->pstSevcCfg->usFftBin;
    U16 usFrameShift = pstSevc->pstSevcCfg->frameShift;
    U16 usFrameOldSize = pstSevc->pstSevcCfg->usFrameSize - usFrameShift;
    aisp_s16_t *pfFrameNew;
    aisp_s16_t *pfFrameOld;
    aisp_s32_t *pfFrameWide;
    aisp_s16_t *pfHanningWinSqrt = pstSevc->pfHanningSqrt;
    aisp_cpx_s32_t *pMicBins = pstSevc->pMicBins;
    U16 shiftOldDiff = AISP_TSL_ABS(usFrameShift - usFrameOldSize);
#if SEVC_SWITCH_AEC
    aisp_cpx_s32_t *pRefBins = pstSevc->pRefBins;
#endif

    for (sMicIdx = 0; sMicIdx < usMixNum; sMicIdx++)
    {
        pfFrameWide = pstSevc->pfFrameWide;
        pfFrameNew = pfFrame + sMicIdx * usFrameShift;
        pfFrameOld = pstSevc->ppfMicFrameOld[sMicIdx];
        sevcFrameWindownAdd(pfFrameWide, pfHanningWinSqrt, pfFrameOld, pfFrameNew, usFrameOldSize, usFrameShift);

        for (sLoop = 0; sLoop < usFrameOldSize; sLoop++)
        {
            pfFrameOld[sLoop] = pfFrameNew[shiftOldDiff + sLoop];
        }

        AISP_TSL_FFT((*pstSevc->pstFftEng), pfFrameWide, pMicBins); //W32Q21

        if (pstSevc->pstSevcCfg->frameShift == 128)
        {
            for (sLoop = 0; sLoop < usFftBin; sLoop++)
            {
                pMicBins[sLoop].real = SHR(pMicBins[sLoop].real, 1);
                pMicBins[sLoop].imag = SHR(pMicBins[sLoop].imag, 1);
            }
        }

        for (sLoop = 0; sLoop < 2; sLoop++)
        {
            pMicBins[sLoop].real = 0;
            pMicBins[sLoop].imag = 0;
        }

        pMicBins += usFftBin;
    }

#if SEVC_SWITCH_AEC
    //Ref frame
    pfFrameWide = pstSevc->pfFrameWide;
    pfFrameNew = pfFrame + usMixNum * usFrameShift;
    pfFrameOld = pstSevc->pfRefFrameOld;
    sevcFrameWindownAdd(pfFrameWide, pfHanningWinSqrt, pfFrameOld, pfFrameNew, usFrameOldSize, usFrameShift);

    for (sLoop = 0; sLoop < usFrameOldSize; sLoop++)
    {
        pfFrameOld[sLoop] = pfFrameNew[shiftOldDiff + sLoop];
    }

    AISP_TSL_FFT((*pstSevc->pstFftEng), pfFrameWide, pRefBins); //W32Q21

    if (pstSevc->pstSevcCfg->frameShift == 128)
    {
        for (sLoop = 0; sLoop < usFftBin; sLoop++)
        {
            pRefBins[sLoop].real = SHR(pRefBins[sLoop].real, 1);
            pRefBins[sLoop].imag = SHR(pRefBins[sLoop].imag, 1);
        }
    }

#endif
}

VOID sevcIFftProcess(SEVC_S *pstSevc, aisp_s16_t *pfOutFrame, aisp_cpx_s32_t *pFreqFrame)
{
    U16 usBinIdx;
    U16 usFrameSize = pstSevc->pstSevcCfg->usFrameSize;
    aisp_s32_t *pfFrameWide = pstSevc->pfFrameWide;
    aisp_s32_t fOutTmp;
#ifndef  AISPEECH_FIXED
    aisp_s16_t fFftScalar = 1.0 / pstSevc->pstSevcCfg->usFftScalar;
#endif
    //usLoop = usFrameSize - 1;
    AISP_TSL_IFFT((*pstSevc->pstFftEng), pFreqFrame, pfFrameWide);
#ifndef  AISPEECH_FIXED

    for (usBinIdx = 0; usBinIdx < usFrameSize; usBinIdx++)
    {
        pfFrameWide[usBinIdx] *= fFftScalar;
    }

#endif

    for (usBinIdx = 0; usBinIdx < usFrameSize; usBinIdx++)
    {
#ifdef FREQ_BIN_Q21
        fOutTmp = PSHR(pfFrameWide[usBinIdx], 6);
#else
        fOutTmp = pfFrameWide[usBinIdx];
#endif

        if (fOutTmp > 32767)
        {
            pfOutFrame[usBinIdx] = 32767;
        }
        else
        {
            pfOutFrame[usBinIdx] = (fOutTmp < -32767 ? -32767 : fOutTmp);
        }
    }
}

VOID sevcPostEmphasisProcess(SEVC_S *pstSevc, aisp_s16_t *pFrame)
{
    U16 usBinIdx;
    U16 usFrameShift = pstSevc->pstSevcCfg->frameShift;
    aisp_s16_t fPreemph = pstSevc->pstSevcCfg->fAecPreemph;
    aisp_s16_t *pfErrFrame = pstSevc->pfErrFrame;
    aisp_s32_t fMemE = pstSevc->fMemE;
    aisp_s32_t fDataTmp;

    for (usBinIdx = 0; usBinIdx < usFrameShift; usBinIdx++)
    {
        fDataTmp = pFrame[usBinIdx] + MULT16_32_P15(fPreemph, fMemE);

        if (fDataTmp > 32767)
        {
            pfErrFrame[usBinIdx] = 32767;
        }
        else
        {
            pfErrFrame[usBinIdx] = (fDataTmp < -32767 ? -32767 : fDataTmp);
        }

        fMemE = fDataTmp;
    }

    pstSevc->fMemE = fMemE;
}

VOID sevcPostOverlapProcess(SEVC_S *pstSevc, aisp_s16_t *pfOutFrame, aisp_s16_t *pfInFrame, aisp_s16_t *pfFrameOld)
{
    U16 usBinIdx, usLoop;
    U16 usFrameShift = pstSevc->pstSevcCfg->frameShift;
    U16 usFrameOldSize = pstSevc->pstSevcCfg->usFrameSize - usFrameShift;
    aisp_s16_t *pfHanningWinSqrt = pstSevc->pfHanningSqrt;
    aisp_s32_t *pfFrameWide = pstSevc->pfFrameWide;
    aisp_s64_t fDataTmp;
    usLoop = usFrameShift;
    sevcVecMUL16X16_Real(pfFrameWide, pfInFrame, pfHanningWinSqrt, pstSevc->pstSevcCfg->usFrameSize, 15);

    for (usBinIdx = 0; usBinIdx < usFrameOldSize; ++usBinIdx)
    {
        pfFrameWide[usBinIdx] += pfFrameOld[usBinIdx];
        pfFrameOld[usBinIdx] = pfFrameWide[usFrameShift + usBinIdx];
    }

    for (usBinIdx = 0; usBinIdx < usFrameShift; usBinIdx++, usLoop++)
    {
        fDataTmp = (aisp_s64_t)pfFrameWide[usBinIdx] * pstSevc->usOverlapGain[usBinIdx];
        fDataTmp = PSHR64(fDataTmp, 15);
        fDataTmp = (fDataTmp > 32767 ? 32767 : fDataTmp);
        fDataTmp = (fDataTmp < -32767 ? -32767 : fDataTmp);
        pfOutFrame[usBinIdx] = fDataTmp;
        // fDataTmp = (pfFrameWide[usLoop] > 32767 ? 32767 : pfFrameWide[usLoop]);
        // fDataTmp = (fDataTmp < -32767 ? -32767 : fDataTmp);
        // pfFrameOld[usBinIdx] = fDataTmp;
    }
}

VOID sevcFrameConvertProcess(SEVC_S *pstSevc, aisp_s16_t *pFrame)
{
    U16 usLoop;
    U16 usFrameShift = pstSevc->pstSevcCfg->frameShift;
    S16 *psFrameOut = pstSevc->psFrameOut;

    for (usLoop = 0; usLoop < usFrameShift; usLoop++)
    {
#ifdef  AISPEECH_FIXED
        psFrameOut[usLoop] = pFrame[usLoop];
#else
        //psFrameOut[usLoop] = (S16)(pFrame[usLoop]*32767);
        psFrameOut[usLoop] = (S16)(pFrame[usLoop]);
#endif
    }
}

// static VOID AuxillaryGainUpdate(SEVC_S *pstSevc, aisp_cpx_s32_t *pfFrame)
// {
// #if SEVC_SWITCH_NN
//     U16 binIdx;
//     U16 fftBins = pstSevc->pstSevcCfg->usFftBin;
//     aisp_cpx_s32_t *firstMic              =            pfFrame;
//     aisp_cpx_s32_t *secondMic             =            pfFrame + fftBins;
//     aisp_cpx_s32_t *thirdMic              =            secondMic + fftBins;
//     aisp_s32_t alphaGain                  =            pstSevc->pstSevcCfg->fAlphaGain;
//     aisp_s32_t minusAlphaGain             =            AISP_TSL_Q20_ONE - alphaGain;
//     aisp_s32_t *eqGainSmooth              =            pstSevc->eqGainSmooth;
//     aisp_s32_t *innerMicGain              =            pstSevc->innerMicGain;
//     aisp_s64_t gainSum = 0;
//     aisp_s32_t gainMean;
//     aisp_s64_t tmpData;
//     U32 tmpSqrt;
//     for (binIdx = 15; binIdx < 48; ++binIdx) {
//         gainSum += pstSevc->pstSevcNnEng->pfGains[binIdx];
//     }

//     gainMean = PDIV(gainSum, 33);
//     // 943718 = 0.9 Q20
// #if SEVC_SWITCH_WN
//     if (gainMean > 943718 &&
//         pstSevc->pstSevcStateEng->windSpeed < 3 * AISP_TSL_Q16_ONE &&
//         pstSevc->refVadLast == 0) {
//             for (binIdx = 0; binIdx < 64; ++binIdx) {
//                 tmpData = PSHR((aisp_s64_t)firstMic->real * secondMic->real + (aisp_s64_t)firstMic->imag * secondMic->imag, 21);
//                 tmpData = AISP_TSL_ABS(tmpData) + 1;
//                 tmpData = PDIV(AISP_TSL_Q20_ONE * tmpData,
//                 PSHR((aisp_s64_t)thirdMic->real * thirdMic->real + (aisp_s64_t)thirdMic->imag * thirdMic->imag, 21) + 1);

//                 tmpSqrt = AISP_TSL_sqrt_xW64_yW32((U64)tmpData * (U64)AISP_TSL_Q20_ONE);
//                 *eqGainSmooth = PSHR((aisp_s64_t)alphaGain * (*eqGainSmooth), 20) + PSHR((aisp_s64_t)minusAlphaGain * (aisp_s64_t)tmpSqrt, 20);
//                 // fwrite(eqGainSmooth, sizeof(aisp_s32_t), 1, fpYyPcm);
//                 *eqGainSmooth = AISP_TSL_MIN((*eqGainSmooth), micGainLimit[binIdx]);

//                 firstMic++;
//                 secondMic++;
//                 thirdMic++;

//                 eqGainSmooth++;
//             }
//             eqGainSmooth = pstSevc->eqGainSmooth;

//             for (binIdx = 0; binIdx < 64; ++binIdx) {
//                 innerMicGain[binIdx] = AISP_TSL_sqrt_xW64_yW32((U64)innerMicGainNormalTest[binIdx] * (U64)eqGainSmooth[binIdx]);

//                 innerMicGain[binIdx] = AISP_TSL_MIN(innerMicGain[binIdx], 25 * AISP_TSL_Q20_ONE);

//                 // 10486 = 0.01 Q20
//                 innerMicGain[binIdx] = AISP_TSL_MAX(innerMicGain[binIdx], 10486);
//             }
//     }
//     // GetData(innerMicGain, 257);
// #endif
// #endif
// }

/************************************************************
  Function   : sevcAgcCbFunc()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/09/04, shizhang.tang create
************************************************************/
static VOID sevcAgcCbFunc(U8 *pucBuff, U32 uiSize, VOID *pUsrData)
{
    SEVC_S *pstSevc = (SEVC_S *)pUsrData;
    SEVC_CONFIG_S *pstSevcCfg = pstSevc->pstSevcCfg;
    U16 usFrameShift = pstSevc->pstSevcCfg->frameShift;
    aisp_s16_t *pfOutData = (aisp_s16_t *)pucBuff;
    aisp_s16_t fOutThreshold = 0, fOutThreshold2 = 0;
    // do fft and overlap add even all modules are off
    sevcPostOverlapProcess(pstSevc, pstSevc->pfErrFrame, pfOutData, pstSevc->pfErrFrameOld);
    pfOutData = pstSevc->pfErrFrame;
#if (SEVC_SWITCH_CNG)
    sevcPostOverlapProcess(pstSevc, pstSevc->pfCngFrame, pstSevc->pfCngFrame, pstSevc->pfCngFrameOld);
    sevcVec16Add16_Real(pstSevc->pfErrFrame, pstSevc->pfErrFrame, pstSevc->pfCngFrame,
                        usFrameShift, 0);
#endif
#if (SEVC_SWITCH_AEC && SEVC_SWITCH_AEC_PREEMPH)
    sevcPostEmphasisProcess(pstSevc, pstSevc->pfErrFrame);
    //pfOutData = pstSevc->pfErrFrame;
#endif

    if (pstSevcCfg->outGain != 32768)
    {
        int ii;
        fOutThreshold = 1073709056 / pstSevcCfg->outGain;
        fOutThreshold2 = 0 - fOutThreshold;

        for (ii = 0; ii < pstSevcCfg->frameShift; ii++)
        {
            pfOutData[ii] = (pfOutData[ii] >= fOutThreshold ? 32767
                             : (pfOutData[ii] <= fOutThreshold2 ? -32767
                                : MULT16_32_P15(pfOutData[ii], pstSevcCfg->outGain)));
        }
    }

    sevcFrameConvertProcess(pstSevc, pfOutData);

    if (NULL != pstSevc->pCallBackFunc)
    {
        pstSevc->pCallBackFunc((U8 *)pstSevc->psFrameOut, usFrameShift * 2, pstSevc->pUsrData);
    }
}

#if SEVC_MULTI_CORE
static VOID sevcMulticoreIfftCb(U8 *pucBuff, U32 uiSize, VOID *pUsrData)
{
    SEVC_S *pstSevc = (SEVC_S *)pUsrData;
    sevcIFftProcess(pstSevc, pstSevc->pfErrFrame, (aisp_cpx_s32_t *)pucBuff);
#if SEVC_SWITCH_AGC
    SEVC_AGC_Feed(pstSevc->pstSevcAgcEng, (aisp_s16_t *)pstSevc->pfErrFrame, pstSevc->fVadStatus);
#else
    sevcAgcCbFunc((U8 *)pstSevc->pfErrFrame, pstSevc->pstSevcCfg->usFrameSize * sizeof(aisp_s16_t),
                  pUsrData);
#endif
}
#endif

/************************************************************
  Function   : sevcNnCbFunc()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/09/04, shizhang.tang create
************************************************************/
static VOID sevcNnCbFunc(U8 *pucBuff, U32 uiSize, VOID *pUsrData)
{
    SEVC_S *pstSevc = (SEVC_S *)pUsrData;
#if SEVC_SWITCH_NN
#if SEVC_SWITCH_CNG
    aisp_cpx_s32_t *pFreqFrame = (aisp_cpx_s32_t *)pucBuff;
    U16 usFftBin = pstSevc->pstSevcCfg->usFftBin;
    pFreqFrame = pFreqFrame + usFftBin;
    sevcIFftProcess(pstSevc, pstSevc->pfCngFrame, pFreqFrame);
#endif
    pstSevc->fVadStatus = SEVC_NN_VadGet(pstSevc->pstSevcNnEng);
#endif
#if SEVC_MIC_CHAN == 3

    if (pstSevc->pstSevcStateEng != NULL)
    {
        if (pstSevc->pstSevcStateEng->usWindStatus)
        {
            AISP_TSL_memset(pucBuff, 0, 4 * sizeof(aisp_cpx_s32_t));
        }
        else
        {
            sevcApplyVectorGainToComplex((aisp_cpx_s32_t *)pucBuff, eqGain2,
                                         pstSevc->pstSevcCfg->usFftBin, 20);
        }
    }

#endif
#if SEVC_SWITCH_EQ
    SEVC_EQ_Feed(pstSevc->pstSevcEqEng, (aisp_cpx_s32_t *)pucBuff);
#endif
#if SEVC_MULTI_CORE

    if (pstSevc->pCallBackFuncMiddle != NULL)
    {
        pstSevc->pCallBackFuncMiddle((U8 *)pucBuff, pstSevc->pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t),
                                     pstSevc->pUsrDataMiddle);
    }
    else
    {
        sevcMulticoreIfftCb((U8 *)pucBuff, pstSevc->pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t),
                            pstSevc);
    }

#else
    sevcIFftProcess(pstSevc, pstSevc->pfErrFrame, (aisp_cpx_s32_t *)pucBuff);
#if SEVC_SWITCH_AGC
    SEVC_AGC_Feed(pstSevc->pstSevcAgcEng, (aisp_s16_t *)pstSevc->pfErrFrame, pstSevc->fVadStatus);
#else
    sevcAgcCbFunc((U8 *)pstSevc->pfErrFrame, pstSevc->pstSevcCfg->usFrameSize * sizeof(aisp_s16_t),
                  pUsrData);
#endif
#endif // SEVC_MULTI_CORE
}

/************************************************************
  Function   : sevcNrCbFunc()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/09/04, shizhang.tang create
************************************************************/
static VOID sevcNrCbFunc(U8 *pucBuff, U32 uiSize, VOID *pUsrData)
{
#if SEVC_SWITCH_NN
    SEVC_S *pstSevc = (SEVC_S *)pUsrData;
    aisp_cpx_s32_t *pFreqFrame = (aisp_cpx_s32_t *)pucBuff;
    aisp_cpx_s32_t *pEstFrame = NULL;
    SEVC_NN_Feed(pstSevc->pstSevcNnEng, pFreqFrame, pEstFrame, pstSevc->uiWindStatus, pstSevc->uiQuietState);
#else
    sevcNnCbFunc(pucBuff, uiSize, pUsrData);
#endif
}

#if SEVC_SWITCH_BF
static VOID sevcSppCalc(SEVC_S *pstSevc, aisp_cpx_s32_t *pFreqFrame, aisp_cpx_s32_t *pAecEstFrame)
{
    S32 ii;
    U16 usFftBin = pstSevc->pstSevcCfg->usFftBin;
    aisp_s32_t *pfBfSpp = pstSevc->pfBfSpp;
    aisp_cpx_s32_t *pstSpeechFrame = pFreqFrame;
    aisp_cpx_s32_t *pstNoiseFrame = pFreqFrame + 2 * usFftBin;
    aisp_s64_t fPow1, fPow2, fEstPow = 0;
    //sevcVecAdd_Complex(pstNoiseFrame, pstNoiseFrame, pAecEstFrame, usFftBin, 0);
#if 0

    for (ii = 0; ii < usFftBin; ii++)
    {
        fPow1 = AISP_TSL_ABS(pstSpeechFrame[ii].real) + AISP_TSL_ABS(pstSpeechFrame[ii].imag) ;
        fPow2 = AISP_TSL_ABS(pstNoiseFrame[ii].real) + AISP_TSL_ABS(pstNoiseFrame[ii].imag) ;
        fEstPow = (AISP_TSL_ABS(pAecEstFrame[ii].real) + AISP_TSL_ABS(pAecEstFrame[ii].imag)) ;
        pfBfSpp[ii] = (fPow1 > (fPow2 + fEstPow * 4) ? 1 : 0);
    }

#endif

    for (ii = 0; ii < usFftBin; ii++)
    {
        fPow1 = PSHR64((MULT32_32(pstSpeechFrame[ii].real, (pstSpeechFrame[ii].real))
                        + MULT32_32(pstSpeechFrame[ii].imag , pstSpeechFrame[ii].imag)), 26);
        fPow2 = PSHR64((MULT32_32(pstNoiseFrame[ii].real, (pstNoiseFrame[ii].real))
                        + MULT32_32(pstNoiseFrame[ii].imag , pstNoiseFrame[ii].imag)), 26);
#if SEVC_SWITCH_AEC
        fEstPow = PSHR64((MULT32_32(pAecEstFrame[ii].real, (pAecEstFrame[ii].real))
                          + MULT32_32(pAecEstFrame[ii].imag , pAecEstFrame[ii].imag)), 26);
#endif
        pfBfSpp[ii] = (fPow1 > (fPow2 + fEstPow * 10) ? 1 : 0);
    }
}
#endif

static VOID sevcBfPostCbFunc(U8 *pucBuff, U32 uiSize, VOID *pUsrData)
{
#if SEVC_SWITCH_NR
    SEVC_S *pstSevc = (SEVC_S *)pUsrData;
    aisp_cpx_s32_t *pFreqFrame = (aisp_cpx_s32_t *)pucBuff;
    aisp_s32_t *pfGain = NULL;
#if SEVC_SWITCH_NN
    pfGain = pstSevc->pstSevcNnEng->pfGains;
#endif
    SEVC_NR_Feed(pstSevc->pstSevcNrEng, pFreqFrame, pfGain);
#else
    sevcNrCbFunc(pucBuff, uiSize, pUsrData);
#endif
}

#if SEVC_SWITCH_AES || SEVC_MIC_CHAN == 3

static VOID sevcAesCbFunc(U8 *pucBuff, U32 uiSize, VOID *pUsrData)
{
    SEVC_S *pstSevc = (SEVC_S *)pUsrData;
#if SEVC_SWITCH_AES || SEVC_SWITCH_BF_POST
    aisp_cpx_s32_t *pstFreqFrame = (aisp_cpx_s32_t *)pucBuff;
    U16 fftBins = pstSevc->pstSevcCfg->usFftBin;
    S32 binIdx;
#endif
#if SEVC_SWITCH_AEC && SEVC_SWITCH_AES && SEVC_MIC_CHAN == 3
    SEVC_STATE_S *pstStateEng = pstSevc->pstSevcStateEng;
    SEVC_AEC_S *pstAecEng = pstSevc->pstSevcAecEng;
    // aec third channel or aes output
    aisp_cpx_s32_t *pfFrame = pstFreqFrame;
    // aec first input
    aisp_cpx_s32_t *pfAecOut = pstAecEng->pErrOut;
    aisp_s32_t *innerMicGain = pstSevc->innerMicGain;
    aisp_s32_t *innerMicGain2 = pstSevc->innerMicGain2;
    aisp_s32_t tmpData;

    // apply innerMicGain
    if (pstSevc->refVad == 0)
    {
        sevcApplyVectorGainToComplex(pfFrame, innerMicGain, fftBins, 20);
        AISP_TSL_memset(pfFrame + 113, 0, sizeof(aisp_cpx_s32_t) * (fftBins - 113));
    }
    else
    {
        sevcApplyVectorGainToComplex(pfFrame, innerMicGain2, fftBins, 20);

        for (binIdx = 65; binIdx < 113; ++binIdx)
        {
            pfFrame[binIdx].real = PSHR(pfFrame[binIdx].real, 1);
            pfFrame[binIdx].imag = PSHR(pfFrame[binIdx].imag, 1);
        }

        AISP_TSL_memset(pfFrame + 113, 0, sizeof(aisp_cpx_s32_t) * (fftBins - 113));
    }

    if (pstStateEng->usWindStatus == 1)
    {
        if (pstStateEng->windSpeed <= 6 * AISP_TSL_Q16_ONE)
        {
            sevcApplyVectorGainToComplex(pfAecOut, micGain1, fftBins, 20);
        }
        else
            if (pstStateEng->windSpeed <= 8 * AISP_TSL_Q16_ONE)
            {
                // sevcApplyVectorGainToComplex(pfAecOut, micGain1, fftBins, 20);
                tmpData = PDIV(74 * AISP_TSL_Q16_ONE - 9 * pstStateEng->windSpeed, 20);

                // apply scalar gain Q16
                for (binIdx = 0; binIdx < fftBins; ++binIdx)
                {
                    pfAecOut[binIdx].real = PSHR((aisp_s64_t)pfAecOut[binIdx].real * tmpData, 16);
                    pfAecOut[binIdx].imag = PSHR((aisp_s64_t)pfAecOut[binIdx].imag * tmpData, 16);
                }
            }
            else
            {
                sevcApplyVectorGainToComplex(pfAecOut, micGain2, fftBins, 20);
            }

        if (pstStateEng->windSpeed <= 8 * AISP_TSL_Q16_ONE)
        {
            for (binIdx = 64; binIdx < fftBins; ++binIdx)
            {
                pfFrame[binIdx].real += pfAecOut[binIdx].real;
                pfFrame[binIdx].imag += pfAecOut[binIdx].imag;
            }

            // copy from bin 129
            // AISP_TSL_memcpy(pfFrame + 129, pfAecOut + 129, 128 * sizeof(aisp_cpx_s32_t));
        }
    }

    // if (pstStateEng->windSpeed != 0)
    // {
    for (binIdx = 0; binIdx < 3; ++binIdx)
    {
        pfFrame[binIdx].real = PSHR(pfFrame[binIdx].real, 2);
        pfFrame[binIdx].imag = PSHR(pfFrame[binIdx].imag, 2);
    }

    // apply aux gain
    for (binIdx = 0; binIdx < fftBins; ++binIdx)
    {
        pfFrame[binIdx].real = SHL(pfFrame[binIdx].real, 1);
        pfFrame[binIdx].imag = SHL(pfFrame[binIdx].imag, 1);
    }

    // tmpData = pstStateEng->windSpeed * 4;
    // if (pstStateEng->windSpeed <= 4 * AISP_TSL_Q16_ONE)
    // {
    //     for (binIdx = 0; binIdx < fftBins; ++binIdx)
    //     {
    //         pfAecOut[binIdx].real = PDIV64((aisp_s64_t)pfAecOut[binIdx].real * micGain1[binIdx], tmpData);
    //         pfAecOut[binIdx].imag = PDIV64((aisp_s64_t)pfAecOut[binIdx].imag * micGain1[binIdx], tmpData);
    //     }
    // }
    // else
    // {
    //     for (binIdx = 0; binIdx < fftBins; ++binIdx)
    //     {
    //         pfAecOut[binIdx].real = PDIV64((aisp_s64_t)pfAecOut[binIdx].real * micGain2[binIdx], tmpData);
    //         pfAecOut[binIdx].imag = PDIV64((aisp_s64_t)pfAecOut[binIdx].imag * micGain2[binIdx], tmpData);
    //     }
    // }
    // }
    // if (pstStateEng->windSpeed != 0)
    // {
    //     if (pstStateEng->windSpeed <= 7 * AISP_TSL_Q16_ONE)
    //     {
    //         // add 2 buffers and store in input buffer
    //         for (binIdx = 0; binIdx < fftBins; ++binIdx)
    //         {
    //             pfFrame[binIdx].real = pfFrame[binIdx].real + pfAecOut[binIdx].real;
    //             pfFrame[binIdx].imag = pfFrame[binIdx].imag + pfAecOut[binIdx].imag;
    //         }
    //     }
    //     // windspeed not equal 0, divide the first 8 bins by 4
    //     for (binIdx = 0; binIdx < 8; ++binIdx)
    //     {
    //         pfFrame[binIdx].real = PSHR(pfFrame[binIdx].real, 4);
    //         pfFrame[binIdx].imag = PSHR(pfFrame[binIdx].imag, 4);
    //     }
    // }
    // else
    // {
    //     // windspeed equals 0, divide the first 6 bins by 4
    //     for (binIdx = 0; binIdx < 6; ++binIdx)
    //     {
    //         pfFrame[binIdx].real = PSHR(pfFrame[binIdx].real, 4);
    //         pfFrame[binIdx].imag = PSHR(pfFrame[binIdx].imag, 4);
    //     }
    // }
    // if (pstStateEng->windSpeed <= 7 * AISP_TSL_Q16_ONE)
    // {
    //     if (pstStateEng->windSpeed <= 4 * AISP_TSL_Q16_ONE)
    //     {
    //         //sevcApplyVectorGainToComplex(pfAecOut, micGain1, fftBins, 18);
    //         for (binIdx = 0; binIdx < fftBins; ++binIdx)
    //         {
    //             llDataTmp = round(((S64)AISP_TSL_Q16_ONE * micGain1[binIdx]) / (pstStateEng->windSpeed + 1)); //Q20
    //             pfAecOut[binIdx].real = PSHR(llDataTmp * pfAecOut[binIdx].real, 18);
    //             pfAecOut[binIdx].imag = PSHR(llDataTmp * pfAecOut[binIdx].imag, 18);
    //         }
    //     }
    //     else
    //     {
    //         //sevcApplyVectorGainToComplex(pfAecOut, micGain2, fftBins, 18);
    //         for (binIdx = 0; binIdx < fftBins; ++binIdx)
    //         {
    //             llDataTmp = round(((S64)AISP_TSL_Q16_ONE * micGain2[binIdx]) / (pstStateEng->windSpeed + 1)); //Q20
    //             pfAecOut[binIdx].real = PSHR(llDataTmp * pfAecOut[binIdx].real, 18);
    //             pfAecOut[binIdx].imag = PSHR(llDataTmp * pfAecOut[binIdx].imag, 18);
    //         }
    //     }
    //     for (binIdx = 0; binIdx < fftBins; ++binIdx)
    //     {
    //         pfFrame[binIdx].real = pfFrame[binIdx].real + pfAecOut[binIdx].real;
    //         pfFrame[binIdx].imag = pfFrame[binIdx].imag + pfAecOut[binIdx].imag;
    //     }
    // }
    // pfFrame = pstFreqFrame;
    // for (binIdx = 0; binIdx < 8; ++binIdx)
    // {
    //     pfFrame[binIdx].real = PSHR(pfFrame[binIdx].real, 3);
    //     pfFrame[binIdx].imag = PSHR(pfFrame[binIdx].imag, 3);
    // }
    // for (binIdx = 8; binIdx < 11; ++binIdx)
    // {
    //     pfFrame[binIdx].real = PSHR(pfFrame[binIdx].real, 2);
    //     pfFrame[binIdx].imag = PSHR(pfFrame[binIdx].imag, 2);
    // }
#endif
#if SEVC_SWITCH_BF_POST
    aisp_s32_t *pfGain = NULL;

    // reset spp to 1
    for (binIdx = 0; binIdx < fftBins; ++binIdx)
    {
        pstSevc->pfBfSpp[binIdx] = 1;
    }

#if SEVC_SWITCH_NN
    pfGain = pstSevc->pstSevcNnEng->pfGains;
#endif
    SEVC_BFPOST_Feed(pstSevc->pstSevcBfPostEng, pstFreqFrame, pfGain,
                     pstSevc->pfBfSpp, 0, pstSevc->noiseUpdate);

    if (pstSevc->pstSevcBfPostEng->pCallBackFunc != NULL)
    {
        pstSevc->pstSevcBfPostEng->pCallBackFunc((U8 *)pstFreqFrame, pstSevc->pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t),
                pstSevc->pstSevcBfPostEng->pUsrData);
    }

#else
    sevcBfPostCbFunc(pucBuff, uiSize, pUsrData);
#endif
}

#endif

#if SEVC_SWITCH_BF || SEVC_SWITCH_GSC

static VOID sevcGscCbFunc(U8 *pucBuff, U32 uiSize, VOID *pUsrData)
{
#if SEVC_SWITCH_BF_POST
    SEVC_S *pstSevc = (SEVC_S *)pUsrData;
    aisp_cpx_s32_t *pFreqFrame = (aisp_cpx_s32_t *)pucBuff;
    aisp_s32_t *pfGain = NULL;
#if SEVC_SWITCH_NN
    pfGain = pstSevc->pstSevcNnEng->pfGains;
#endif
#if SEVC_SWITCH_BF
    SEVC_BFPOST_Feed(pstSevc->pstSevcBfPostEng, pFreqFrame, pfGain, pstSevc->pfBfSpp, 1, pstSevc->noiseUpdate);
#else
    SEVC_BFPOST_Feed(pstSevc->pstSevcBfPostEng, pFreqFrame, pfGain, pstSevc->pfBfSpp, 0, pstSevc->noiseUpdate);
#endif

    if (pstSevc->pstSevcBfPostEng->pCallBackFunc != NULL)
    {
        pstSevc->pstSevcBfPostEng->pCallBackFunc((U8 *)pFreqFrame, pstSevc->pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t),
                pstSevc->pstSevcBfPostEng->pUsrData);
    }

#else
    sevcBfPostCbFunc(pucBuff, uiSize, pUsrData);
#endif
}

#endif
/************************************************************
  Function   : sevcBfCbFunc()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/09/04, shizhang.tang create
************************************************************/
#if SEVC_SWITCH_BF

static VOID sevcBfCbFunc(U8 *pucBuff, U32 uiSize, VOID *pUsrData)
{
#if SEVC_SWITCH_BF
    SEVC_S *pstSevc = (SEVC_S *)pUsrData;
    aisp_cpx_s32_t *pFreqFrame = (aisp_cpx_s32_t *)pucBuff;
    U16 usMicNum = pstSevc->pstSevcCfg->micChan;
    sevcSppCalc(pstSevc, pFreqFrame, pstSevc->pstSevcAecEng->pErrOut + usMicNum * pstSevc->pstSevcCfg->usFftBin);
#endif
#if SEVC_SWITCH_GSC
    aisp_cpx_s32_t *pstBfBlock = pFreqFrame + pstSevc->pstSevcCfg->usFftBin;
    SEVC_GSC_Feed(pstSevc->pstSevcGscEng, pFreqFrame, pstBfBlock, pstSevc->pfBfSpp);
#else
    sevcGscCbFunc(pucBuff, sizeof(aisp_cpx_s32_t) * pstSevc->pstSevcCfg->usFftBin, pUsrData);
#endif
}

#endif

/************************************************************
  Function   : sevcAecCbFunc()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/09/04, shizhang.tang create
************************************************************/
static VOID sevcAecCbFunc(U8 *pucBuff, U32 uiSize, VOID *pUsrData)
{
    SEVC_S *pstSevc = (SEVC_S *)pUsrData;
    // AuxillaryGainUpdate(pstSevc, (aisp_cpx_s32_t *)pucBuff);
#if SEVC_SWITCH_WN
    SEVC_SD_Feed(pstSevc->pstSevcStateEng, (aisp_cpx_s32_t *)pucBuff);
    pstSevc->uiWindStatus = pstSevc->pstSevcStateEng->usWindStatus;
#endif

    // assign noise update
    if (pstSevc->uiWindStatus == 1 && pstSevc->refVad == 1)
    {
        pstSevc->noiseUpdate = 0;
    }
    else
    {
        pstSevc->noiseUpdate = 1;
    }

    if (pstSevc->uiWindStatus == 0)
    {
#if SEVC_SWITCH_BF
        aisp_cpx_s32_t *pFreqFrame = (aisp_cpx_s32_t *)pucBuff;
        SEVC_BF_Feed(pstSevc->pstSevcBfEng, pFreqFrame);
#else
#if SEVC_MIC_CHAN == 3
        U16 fftBins = pstSevc->pstSevcCfg->usFftBin;
        U16 usMicNum = pstSevc->pstSevcCfg->micChan;
        aisp_cpx_s32_t *pLastAec = pstSevc->pstSevcAecEng->pErrOut + (usMicNum - 1) * fftBins;
#if SEVC_SWITCH_AES

        if (pstSevc->refVad == 1)
        {
            SEVC_AES_Feed(pstSevc->pstSevcAesEng, pstSevc->pMicBins + (usMicNum - 1) * fftBins,
                          pLastAec,
                          pstSevc->pstSevcAecEng->pErrOut + (usMicNum + 1) * fftBins,
                          pstSevc->refVad);
        }
        else
        {
            sevcAesCbFunc((U8 *)pLastAec, fftBins * sizeof(aisp_cpx_s32_t), pUsrData);
        }

#else
        sevcAesCbFunc((U8 *)pLastAec, fftBins * sizeof(aisp_cpx_s32_t), pUsrData);
#endif
#else
#if SEVC_SWITCH_BF_POST
        aisp_cpx_s32_t *pfFrame = (aisp_cpx_s32_t *)pucBuff;
        aisp_s32_t *pfGain = NULL;
#if SEVC_SWITCH_NN
        pfGain = pstSevc->pstSevcNnEng->pfGains;
#endif
        SEVC_BFPOST_Feed(pstSevc->pstSevcBfPostEng, pfFrame, pfGain, pstSevc->pfBfSpp, 0, pstSevc->noiseUpdate);

        if (pstSevc->pstSevcBfPostEng->pCallBackFunc != NULL)
        {
            pstSevc->pstSevcBfPostEng->pCallBackFunc((U8 *)pfFrame, pstSevc->pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t),
                    pstSevc->pstSevcBfPostEng->pUsrData);
        }

#else
        sevcBfPostCbFunc(pucBuff, pstSevc->pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t), pUsrData);
#endif
#endif // if SEVC_MIC_CHAN == 3
#endif // if SEVC_SWITCH_BF
    }
    else
    {
#if SEVC_MIC_CHAN == 3
        U16 fftBins = pstSevc->pstSevcCfg->usFftBin;
        U16 usMicNum = pstSevc->pstSevcCfg->micChan;
        aisp_cpx_s32_t *pLastAec = pstSevc->pstSevcAecEng->pErrOut + (usMicNum - 1) * fftBins;
#if SEVC_SWITCH_AES

        if (pstSevc->refVad == 1)
        {
            SEVC_AES_Feed(pstSevc->pstSevcAesEng, pstSevc->pMicBins + (usMicNum - 1) * fftBins,
                          pLastAec,
                          pstSevc->pstSevcAecEng->pErrOut + (usMicNum + 1) * fftBins,
                          pstSevc->refVad);
        }
        else
        {
            sevcAesCbFunc((U8 *)pLastAec, fftBins * sizeof(aisp_cpx_s32_t), pUsrData);
        }

#else
        sevcAesCbFunc((U8 *)pLastAec, fftBins * sizeof(aisp_cpx_s32_t), pUsrData);
#endif
#else
#if SEVC_SWITCH_BF_POST
        aisp_cpx_s32_t *pfFrame = (aisp_cpx_s32_t *)pucBuff;
        aisp_s32_t *pfGain = NULL;
#if SEVC_SWITCH_NN
        pfGain = pstSevc->pstSevcNnEng->pfGains;
#endif
        SEVC_BFPOST_Feed(pstSevc->pstSevcBfPostEng, pfFrame, pfGain, pstSevc->pfBfSpp, 0, pstSevc->noiseUpdate);

        if (pstSevc->pstSevcBfPostEng->pCallBackFunc != NULL)
        {
            pstSevc->pstSevcBfPostEng->pCallBackFunc((U8 *)pfFrame, pstSevc->pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t),
                    pstSevc->pstSevcBfPostEng->pUsrData);
        }

#else
        sevcBfPostCbFunc(pucBuff, pstSevc->pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t), pUsrData);
#endif
        // if (pstSevc->pCallBackFuncMiddle)
        // {
        //     pstSevc->pCallBackFuncMiddle(pucBuff, pstSevc->pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t),
        //                                  pstSevc->pUsrDataMiddle);
        // }
        // else
        // {
        //     sevcGscCbFunc((U8 *)pucBuff, uiSize, pUsrData);
        // }
#endif // if SEVC_MIC_CHAN == 3
    }
}

S8 *SEVC_ModelInfoGet(SEVC_S *pstSevc)
{
#if SEVC_SWITCH_NN
    return SEVC_NN_ModelInfoGet(pstSevc->pstSevcNnEng);
#else
    return "No info";
#endif
}

VOID SEVC_PreProcess(SEVC_CONFIG_S *pstSevcCfg, aisp_s16_t *pFrame, aisp_s32_t fMicGain, aisp_s32_t fRefGain)
{
    U16 usFrameShift = pstSevcCfg->frameShift;
    U16 usMicNum = pstSevcCfg->micChan;
    U16 usRefNum = pstSevcCfg->refChan;
    aisp_s16_t *pRefFrame = pFrame + usMicNum * usFrameShift;
    S32 frameIdx;
    S32 sampleIdx;

    if (fMicGain != 32768)   // if gain is not one
    {
        for (frameIdx = 0; frameIdx < usMicNum; ++frameIdx)
        {
            for (sampleIdx = 0; sampleIdx < usFrameShift; ++sampleIdx)
            {
                *pFrame = MULT16_32_P15(*pFrame, fMicGain);
                pFrame++;
            }
        }
    }

    if (fRefGain != 32768)
    {
        for (frameIdx = 0; frameIdx < usRefNum; ++frameIdx)
        {
            for (sampleIdx = 0; sampleIdx < usFrameShift; ++sampleIdx)
            {
                *pRefFrame = MULT16_32_P15(*pRefFrame, fRefGain);
                pRefFrame++;
            }
        }
    }
}

/************************************************************
  Function   : SEVC_ShMemSizeGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/09/04, shizhang.tang create
************************************************************/
U32 SEVC_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize  = 0;
    //pfFrameWide;
    uiTotalMemSize = SIZE_ALIGN(pstSevcCfg->usFftFrameSize * sizeof(aisp_s32_t));
#ifdef USE_SHARE_MEM
#if (SEVC_SWITCH_AEC || SEVC_SWITCH_BF || SEVC_SWITCH_NN || SEVC_SWITCH_AGC)
    U32 uiMemSize  = 0;
#endif
#if SEVC_SWITCH_WN
    uiTotalMemSize += SEVC_SD_ShMemSizeGet(pstSevcCfg);
#endif
#if SEVC_SWITCH_VAD
    uiMemSize = SEVC_VAD_ShMemSizeGet(pstSevcCfg);
    uiTotalMemSize = SEVC_MAX(uiTotalMemSize, uiMemSize);
#endif
#if SEVC_SWITCH_AEC
    uiMemSize = SEVC_AEC_ShMemSizeGet(pstSevcCfg);
    uiTotalMemSize = SEVC_MAX(uiTotalMemSize, uiMemSize);
#endif
#if SEVC_SWITCH_BF
    uiMemSize += SEVC_BF_ShMemSizeGet(pstSevcCfg);
    uiTotalMemSize = SEVC_MAX(uiTotalMemSize, uiMemSize);
#endif
#if SEVC_SWITCH_GSC
    uiMemSize = SEVC_GSC_ShMemSizeGet(pstSevcCfg);
    uiTotalMemSize = SEVC_MAX(uiTotalMemSize, uiMemSize);
#endif
#if SEVC_SWITCH_BF_POST
    uiMemSize = SEVC_BFPOST_ShMemSizeGet(pstSevcCfg);
    uiTotalMemSize = SEVC_MAX(uiTotalMemSize, uiMemSize);
#endif
#if SEVC_SWITCH_AUX
    uiTotalMemSize += SEVC_AUX_ShMemSizeGet(pstSevcCfg);
#endif
#if SEVC_SWITCH_NR
    uiTotalMemSize += SEVC_NR_ShMemSizeGet(pstSevcCfg);
#endif
#if SEVC_SWITCH_NN
    uiMemSize = SEVC_NN_ShMemSizeGet(pstSevcCfg);
    uiTotalMemSize = SEVC_MAX(uiTotalMemSize, uiMemSize);
#endif
#if SEVC_SWITCH_AES
    uiMemSize = SEVC_AES_ShMemSizeGet(pstSevcCfg);
    uiTotalMemSize = SEVC_MAX(uiTotalMemSize, uiMemSize);
#endif
#if SEVC_SWITCH_EQ
    uiMemSize = SEVC_EQ_ShMemSizeGet(pstSevcCfg);
    uiTotalMemSize = SEVC_MAX(uiTotalMemSize, uiMemSize);
#endif
#if SEVC_SWITCH_AGC
    uiMemSize = SEVC_AGC_ShMemSizeGet(pstSevcCfg);
    uiTotalMemSize = SEVC_MAX(uiTotalMemSize, uiMemSize);
#endif
#endif
#if MEM_BUFF_DBG
    MSG_DBG("%s [%d] uiTotalMemSize=%d\r\n", __func__, __LINE__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}

/************************************************************
  Function   : SEVC_LocMemSizeGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/09/04, shizhang.tang create
************************************************************/
U32 SEVC_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize  = 0;
    U16 usFrameShift = pstSevcCfg->frameShift;
    U16 usFrameSize = pstSevcCfg->usFftFrameSize;
    U16 usFrameOldSize = pstSevcCfg->usFrameSize - usFrameShift;
    uiTotalMemSize += SIZE_ALIGN(sizeof(SEVC_S));
    //ppfMicFrameOld;
    uiTotalMemSize += SIZE_ALIGN(pstSevcCfg->micChan * sizeof(VOID *));
    uiTotalMemSize += pstSevcCfg->micChan * SIZE_ALIGN(usFrameOldSize * sizeof(aisp_s16_t));
    //pMicBins;
    uiTotalMemSize += SIZE_ALIGN(pstSevcCfg->micChan * pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t));

    if (pstSevcCfg->AEC_flag == 1)
    {
        //pfRefFrameOld;
        uiTotalMemSize += SIZE_ALIGN(usFrameOldSize * sizeof(aisp_s16_t));
        //pRefBins;
        uiTotalMemSize += SIZE_ALIGN(pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t));
    }

    // hanning sqrt
    uiTotalMemSize += SIZE_ALIGN(pstSevcCfg->usFrameSize * sizeof(aisp_s16_t));
    // overlap gain
    uiTotalMemSize += SIZE_ALIGN(usFrameShift * sizeof(aisp_s32_t));
    //psFrameOut;
    uiTotalMemSize += SIZE_ALIGN(usFrameShift * 2);
    //pfErrFrameOld;
    uiTotalMemSize += SIZE_ALIGN(usFrameOldSize * sizeof(aisp_s16_t));
    //pfErrFrame;
    uiTotalMemSize += SIZE_ALIGN(usFrameSize * sizeof(aisp_s16_t));
#if SEVC_MIC_CHAN == 3
    // aux innerMicGain2
    uiTotalMemSize += SIZE_ALIGN(pstSevcCfg->usFftBin * sizeof(aisp_s32_t));
    // aux innerMicGain
    uiTotalMemSize += SIZE_ALIGN(pstSevcCfg->usFftBin * sizeof(aisp_s32_t));
#endif
#if (SEVC_SWITCH_BF || SEVC_SWITCH_GSC || SEVC_SWITCH_BF_POST)
    //pfBfSpp
    uiTotalMemSize += SIZE_ALIGN(pstSevcCfg->usFftBin * sizeof(aisp_s32_t));
#endif
#if SEVC_SWITCH_AUX
    //pfAuxFrame;
    uiTotalMemSize += SIZE_ALIGN(usFrameShift * sizeof(aisp_s16_t));
#endif
#if (SEVC_SWITCH_NN && SEVC_SWITCH_CNG)
    //pfCngFrameOld;
    uiTotalMemSize += SIZE_ALIGN(usFrameOldSize * sizeof(aisp_s16_t));
    //pfCngFrame
    uiTotalMemSize += SIZE_ALIGN(usFrameSize * sizeof(aisp_s16_t));
#endif
// #if (SEVC_SWITCH_AEC)
//     //pAecEstBuff
//     uiTotalMemSize += SIZE_ALIGN(pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t));
// #endif
#ifndef USE_SHARE_MEM
    uiTotalMemSize += SEVC_ShMemSizeGet(pstSevcCfg);
#endif
#if MEM_BUFF_DBG
    MSG_DBG("%s uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
#if SEVC_SWITCH_WN
    uiTotalMemSize += SEVC_SD_LocMemSizeGet(pstSevcCfg);
#endif
#if SEVC_SWITCH_VAD
    uiTotalMemSize += SEVC_VAD_LocMemSizeGet(pstSevcCfg);
#endif
#if SEVC_SWITCH_AEC
    uiTotalMemSize += SEVC_AEC_LocMemSizeGet(pstSevcCfg);
#endif
#if SEVC_SWITCH_BF
    uiTotalMemSize += SEVC_BF_LocMemSizeGet(pstSevcCfg);
#endif
#if SEVC_SWITCH_GSC
    uiTotalMemSize += SEVC_GSC_LocMemSizeGet(pstSevcCfg);
#endif
#if SEVC_SWITCH_BF_POST
    uiTotalMemSize += SEVC_BFPOST_LocMemSizeGet(pstSevcCfg);
#endif
#if SEVC_SWITCH_AUX
    uiTotalMemSize += SEVC_AUX_LocMemSizeGet(pstSevcCfg);
#endif
#if SEVC_SWITCH_NR
    uiTotalMemSize += SEVC_NR_LocMemSizeGet(pstSevcCfg);
#endif
#if SEVC_SWITCH_NN
    uiTotalMemSize += SEVC_NN_LocMemSizeGet(pstSevcCfg);
#endif
#if SEVC_SWITCH_AES
    uiTotalMemSize += SEVC_AES_LocMemSizeGet(pstSevcCfg);
#endif
#if SEVC_SWITCH_EQ
    uiTotalMemSize += SEVC_EQ_LocMemSizeGet(pstSevcCfg);
#endif
#if SEVC_SWITCH_AGC
    uiTotalMemSize += SEVC_AGC_LocMemSizeGet(pstSevcCfg);
#endif
#if MEM_BUFF_DBG
    MSG_DBG("%s uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}

/************************************************************
  Function   : SEVC_New()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/09/04, shizhang.tang create
************************************************************/
SEVC_S *SEVC_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator)
{
    SEVC_S *pstSevc  = NULL;
    U16 usLoop;
    U16 usSizeComp = sizeof(aisp_cpx_s32_t);
    U16 usMicNum = pstSevcCfg->micChan;
    U16 usFrameShift = pstSevcCfg->frameShift;
    // overlap size
    U16 usFrameOldSize = pstSevcCfg->usFrameSize - usFrameShift;
    U16 usFrameSize = pstSevcCfg->usFftFrameSize;
    MEM_LOCATOR_SHOW(pstMemLocator);
    BUFF_POINT_SET_ALIGN(pstSevc, sizeof(SEVC_S), pstMemLocator, FAIL);
    pstSevc->pstSevcCfg = pstSevcCfg;
    pstSevc->pstFftEng = &pstSevcCfg->stFFTEng;
    pstSevc->uiFrameCount = 0;
    pstSevc->uiWindStatus = 0;
    //pMicBins
    BUFF_POINT_SET_ALIGN(pstSevc->pMicBins,
                         (usMicNum * pstSevcCfg->usFftBin * usSizeComp),
                         pstMemLocator, FAIL);
    //ppfMicFrameOld
    BUFF_POINT_SET_ALIGN(pstSevc->ppfMicFrameOld,
                         (usMicNum * sizeof(VOID *)),
                         pstMemLocator, FAIL);

    for (usLoop = 0; usLoop < usMicNum; usLoop++)
    {
        BUFF_POINT_SET_ALIGN(pstSevc->ppfMicFrameOld[usLoop],
                             (usFrameOldSize * sizeof(aisp_s16_t)),
                             pstMemLocator, FAIL);
    }

    if (pstSevcCfg->AEC_flag == 1)
    {
        //pRefBins
        BUFF_POINT_SET_ALIGN(pstSevc->pRefBins,
                             (pstSevcCfg->usFftBin * usSizeComp),
                             pstMemLocator, FAIL);
        //pfRefFrameOld
        BUFF_POINT_SET_ALIGN(pstSevc->pfRefFrameOld,
                             (usFrameOldSize * sizeof(aisp_s16_t)),
                             pstMemLocator, FAIL);
    }

    // hanningSqrt
    BUFF_POINT_SET_ALIGN(pstSevc->pfHanningSqrt, usFrameSize * sizeof(aisp_s16_t), pstMemLocator, FAIL);
    AISP_TSL_memcpy(pstSevc->pfHanningSqrt, pstSevcCfg->pfHanningWindowSqrt, usFrameSize * sizeof(aisp_s16_t));
    // overlap gain
    BUFF_POINT_SET_ALIGN(pstSevc->usOverlapGain, usFrameShift * sizeof(aisp_s32_t), pstMemLocator, FAIL);
    sevcComputeOverlapGain(pstSevcCfg, pstSevc->usOverlapGain);
    //psFrameOut;
    BUFF_POINT_SET_ALIGN(pstSevc->psFrameOut, usFrameShift * sizeof(aisp_s16_t), pstMemLocator, FAIL);
    //pfErrFrameOld;
    BUFF_POINT_SET_ALIGN(pstSevc->pfErrFrameOld,
                         (usFrameOldSize * sizeof(aisp_s16_t)), pstMemLocator, FAIL);
    //pfErrFrame;
    BUFF_POINT_SET_ALIGN(pstSevc->pfErrFrame,
                         (usFrameSize * sizeof(aisp_s16_t)), pstMemLocator, FAIL);
#if SEVC_MIC_CHAN == 3
    // eqGainSmooth
    BUFF_POINT_SET_ALIGN(pstSevc->innerMicGain2, pstSevcCfg->usFftBin * sizeof(aisp_s32_t), pstMemLocator, FAIL);
    AISP_TSL_memcpy(pstSevc->innerMicGain2, innerGainNormalRef, pstSevcCfg->usFftBin * sizeof(aisp_s32_t));
    // innerMicGain
    BUFF_POINT_SET_ALIGN(pstSevc->innerMicGain, pstSevcCfg->usFftBin * sizeof(aisp_s32_t), pstMemLocator, FAIL);
    AISP_TSL_memcpy(pstSevc->innerMicGain, innerMicGainNormal, pstSevcCfg->usFftBin * sizeof(aisp_s32_t));
#endif
#if (SEVC_SWITCH_GSC || SEVC_SWITCH_BF_POST || SEVC_SWITCH_BF)
    //pfBfPostSpp
    BUFF_POINT_SET_ALIGN(pstSevc->pfBfSpp,
                         (pstSevcCfg->usFftBin * sizeof(aisp_s32_t)), pstMemLocator, FAIL);
#endif
#if SEVC_SWITCH_AUX
    //pfAuxFrame;
    BUFF_POINT_SET_ALIGN(pstSevc->pfAuxFrame,
                         (usFrameShift * sizeof(aisp_s16_t)), pstMemLocator, FAIL);
#endif
#if (SEVC_SWITCH_NN && SEVC_SWITCH_CNG)
    //pfCngFrameOld
    BUFF_POINT_SET_ALIGN(pstSevc->pfCngFrameOld,
                         (usFrameOldSize * sizeof(aisp_s16_t)), pstMemLocator, FAIL);
    //pfCngFrame
    BUFF_POINT_SET_ALIGN(pstSevc->pfCngFrame,
                         (usFrameSize * sizeof(aisp_s16_t)), pstMemLocator, FAIL);
#endif
    memLocatorShMemReset(pstMemLocator);
    //shared mem
    //pfFrameWide;
    SH_BUFF_POINT_SET_ALIGN(pstSevc->pfFrameWide,
                            (usFrameSize * sizeof(aisp_s32_t)), pstMemLocator, FAIL);
    //init sub eng
#if SEVC_SWITCH_WN
    memLocatorShMemReset(pstMemLocator);
    pstSevc->pstSevcStateEng = SEVC_SD_New(pstSevcCfg, pstMemLocator);
    AISP_TSL_PTR_CHECK(pstSevc->pstSevcStateEng, FAIL);
#endif
#if SEVC_SWITCH_VAD
    memLocatorShMemReset(pstMemLocator);
    pstSevc->pstSevcVadEng = SEVC_VAD_New(pstSevcCfg, pstMemLocator);
    AISP_TSL_PTR_CHECK(pstSevc->pstSevcVadEng, FAIL);
#endif
#if SEVC_SWITCH_AEC
    memLocatorShMemReset(pstMemLocator);
    pstSevc->pstSevcAecEng = SEVC_AEC_New(pstSevcCfg, pstMemLocator);
    AISP_TSL_PTR_CHECK(pstSevc->pstSevcAecEng, FAIL);
    SEVC_AEC_CallBackFuncRegister(pstSevc->pstSevcAecEng,
                                  (VOID *)sevcAecCbFunc, (VOID *)pstSevc);
#endif
#if SEVC_SWITCH_BF
    memLocatorShMemReset(pstMemLocator);
    pstSevc->pstSevcBfEng = SEVC_BF_New(pstSevcCfg, pstMemLocator);
    AISP_TSL_PTR_CHECK(pstSevc->pstSevcBfEng, FAIL);
    SEVC_BF_CallBackFuncRegister(pstSevc->pstSevcBfEng,
                                 (VOID *)sevcBfCbFunc, (VOID *)pstSevc);
#endif
#if SEVC_SWITCH_GSC
    memLocatorShMemReset(pstMemLocator);
    pstSevc->pstSevcGscEng = SEVC_GSC_New(pstSevcCfg, pstMemLocator);
    AISP_TSL_PTR_CHECK(pstSevc->pstSevcGscEng, FAIL);
    SEVC_GSC_CallBackFuncRegister(pstSevc->pstSevcGscEng,
                                  (VOID *)sevcGscCbFunc, (VOID *)pstSevc);
#endif
#if SEVC_SWITCH_BF_POST
    memLocatorShMemReset(pstMemLocator);
    pstSevc->pstSevcBfPostEng = SEVC_BFPOST_New(pstSevcCfg, pstMemLocator);
    AISP_TSL_PTR_CHECK(pstSevc->pstSevcBfPostEng, FAIL);
    SEVC_BFPOST_CallBackFuncRegister(pstSevc->pstSevcBfPostEng,
                                     (VOID *)sevcBfPostCbFunc, (VOID *)pstSevc);
#endif
#if SEVC_SWITCH_AUX
    memLocatorShMemReset(pstMemLocator);
    pstSevc->pstSevcAuxEng = SEVC_AUX_New(pstSevcCfg, pstMemLocator);
    AISP_TSL_PTR_CHECK(pstSevc->pstSevcAuxEng, FAIL);
#endif
#if SEVC_SWITCH_NR
    memLocatorShMemReset(pstMemLocator);
    pstSevc->pstSevcNrEng = SEVC_NR_New(pstSevcCfg, pstMemLocator);
    SEVC_NR_CallBackFuncRegister(pstSevc->pstSevcNrEng,
                                 (VOID *)sevcNrCbFunc, (VOID *)pstSevc);
    AISP_TSL_PTR_CHECK(pstSevc->pstSevcNrEng, FAIL);
#endif
#if SEVC_SWITCH_NN
    memLocatorShMemReset(pstMemLocator);
    pstSevc->pstSevcNnEng = SEVC_NN_New(pstSevcCfg, pstMemLocator);
    AISP_TSL_PTR_CHECK(pstSevc->pstSevcNnEng, FAIL);
    SEVC_NN_CallBackFuncRegister(pstSevc->pstSevcNnEng,
                                 (VOID *)sevcNnCbFunc, (VOID *)pstSevc);
#endif
#if SEVC_SWITCH_AES
    memLocatorShMemReset(pstMemLocator);
    pstSevc->pstSevcAesEng = SEVC_AES_New(pstSevcCfg, pstMemLocator);
    AISP_TSL_PTR_CHECK(pstSevc->pstSevcAesEng, FAIL);
    SEVC_AES_CallBackFuncRegister(pstSevc->pstSevcAesEng,
                                  (VOID *)sevcAesCbFunc, (VOID *)pstSevc);
#endif
#if SEVC_SWITCH_EQ
    memLocatorShMemReset(pstMemLocator);
    pstSevc->pstSevcEqEng = SEVC_EQ_New(pstSevcCfg, pstMemLocator);
    AISP_TSL_PTR_CHECK(pstSevc->pstSevcEqEng, FAIL);
#endif
#if SEVC_SWITCH_AGC
    memLocatorShMemReset(pstMemLocator);
    pstSevc->pstSevcAgcEng = SEVC_AGC_New(pstSevcCfg, pstMemLocator);
    AISP_TSL_PTR_CHECK(pstSevc->pstSevcAgcEng, FAIL);
    SEVC_AGC_CallBackFuncRegister(pstSevc->pstSevcAgcEng,
                                  (VOID *)sevcAgcCbFunc, (VOID *)pstSevc);
#endif
    MEM_LOCATOR_SHOW(pstMemLocator);
    return pstSevc;
FAIL:
    MSG_DBG("%s [%d] fail\r\n", __func__, __LINE__);
    return NULL;
}

/************************************************************
  Function   : SEVC_CbFuncRegister()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/09/04, shizhang.tang create
************************************************************/
VOID SEVC_CbFuncRegister(SEVC_S *pstSevc, VOID *pCBFunc, VOID *pUsrData)
{
    if (NULL == pstSevc || NULL == pCBFunc)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return ;
    }

    pstSevc->pCallBackFunc = (CallBackFunc)pCBFunc;
    pstSevc->pUsrData = pUsrData;
}

#ifdef SEVC_MULTI_CORE
VOID SEVC_MiddleFreqBinsFeed(SEVC_S *pstSevcEng, U8 *pucBuff, U32 uiSize)
{
    pstSevcEng->pCallBackFuncBackUp(pucBuff, uiSize, pstSevcEng->pUsrDataBackup);
}

VOID SEVC_MiddleCbFuncRegister(SEVC_S *pstSevcEng, VOID *pCBFunc, VOID *pUsrData)
{
    if (NULL == pstSevcEng || NULL == pCBFunc)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return ;
    }

#if 0
    pstSevcEng->pUsrDataBackup = pstSevcEng->pstSevcBfEng->pUsrData;
    pstSevcEng->pCallBackFuncBackUp = pstSevcEng->pstSevcBfEng->pCallBackFunc;
    SEVC_BF_CallBackFuncRegister(pstSevcEng->pstSevcBfEng,
                                 (VOID *)pCBFunc, (VOID *)pUsrData);
#else
    pstSevcEng->pUsrDataMiddle = pUsrData;
    pstSevcEng->pCallBackFuncMiddle = pCBFunc;
    // pstSevcEng->pUsrDataBackup = pstSevcEng->pstSevcGscEng->pUsrData;
    // pstSevcEng->pCallBackFuncBackUp = pstSevcEng->pstSevcGscEng->pCallBackFunc;
    pstSevcEng->pUsrDataBackup = pstSevcEng;
    pstSevcEng->pCallBackFuncBackUp = sevcMulticoreIfftCb;
    // SEVC_GSC_CallBackFuncRegister(pstSevcEng->pstSevcGscEng,
    //                               (VOID *)pCBFunc, (VOID *)pUsrData);
#endif
}

#endif

/************************************************************
  Function   : SEVC_Feed()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/09/04, shizhang.tang create
************************************************************/
VOID SEVC_Feed(SEVC_S *pstSevc, aisp_s16_t *pfFrame)
{
    aisp_s32_t fMicGain = pstSevc->pstSevcCfg->micGain;
    aisp_s32_t fRefGain = pstSevc->pstSevcCfg->refGain;
    aisp_s16_t *pfMicFrame = pfFrame;
#if SEVC_SWITCH_VAD
    U16 usFrameShift = pstSevc->pstSevcCfg->frameShift;
    U16 usMicNum = pstSevc->pstSevcCfg->micChan;
    aisp_s16_t *pRefFrame = pfMicFrame + usMicNum * usFrameShift;
#endif
    SEVC_PreProcess(pstSevc->pstSevcCfg, pfMicFrame, fMicGain, fRefGain);
    sevcFftProcess(pstSevc, (aisp_s16_t *)pfFrame);
#if SEVC_SWITCH_VAD
    aisp_s32_t vad = SEVC_VAD_Feed(pstSevc->pstSevcVadEng, pRefFrame);
    pstSevc->refVadLast = pstSevc->refVad;
    pstSevc->refVad = vad;
#else
    // set ref vad to 1 for aec
    pstSevc->pstSevcAecEng->refVad = 1;
#endif
// #if SEVC_SWITCH_WN
    // SEVC_SD_Feed(pstSevc->pstSevcStateEng, pstSevc->pMicBins);
    // pstSevc->uiWindStatus = pstSevc->pstSevcStateEng->usWindStatus;
// #endif
#if SEVC_SWITCH_AEC
    pstSevc->pstSevcAecEng->refVad = pstSevc->refVadLast;
    SEVC_AEC_Feed(pstSevc->pstSevcAecEng, pstSevc->pMicBins, pstSevc->pRefBins);
#else
    sevcAecCbFunc((U8 *)pstSevc->pMicBins, usMicNum * pstSevc->pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t),
                  (VOID *)pstSevc);
#endif
    pstSevc->uiFrameCount++;
}

/************************************************************
  Function   : SEVC_Reset()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/09/04, shizhang.tang create
************************************************************/
VOID SEVC_Reset(SEVC_S *pstSevc)
{
    pstSevc->uiWindStatus = 0;
#if SEVC_SWITCH_WN
    SEVC_SD_Reset(pstSevc->pstSevcStateEng);
#endif
#if SEVC_SWITCH_BF
    SEVC_BF_Reset(pstSevc->pstSevcBfEng);
#endif
#if SEVC_SWITCH_AEC
    SEVC_AEC_Reset(pstSevc->pstSevcAecEng);
#endif
#if SEVC_SWITCH_GSC
    SEVC_GSC_Reset(pstSevc->pstSevcGscEng);
#endif
#if SEVC_SWITCH_BF_POST
    SEVC_BFPOST_Reset(pstSevc->pstSevcBfPostEng);
#endif
#if SEVC_SWITCH_AUX
    SEVC_AUX_Reset(pstSevc->pstSevcAuxEng);
#endif
#if SEVC_SWITCH_NR
    SEVC_NR_Reset(pstSevc->pstSevcNrEng);
#endif
#if SEVC_SWITCH_NN
    SEVC_NN_Reset(pstSevc->pstSevcNnEng);
#endif
#if SEVC_SWITCH_AES
    SEVC_AES_Reset(pstSevc->pstSevcAesEng);
#endif
#if SEVC_SWITCH_EQ
    SEVC_EQ_Reset(pstSevc->pstSevcEqEng);
#endif
#if SEVC_SWITCH_AGC
    SEVC_AGC_Reset(pstSevc->pstSevcAgcEng);
#endif
}

/************************************************************
  Function   : SEVC_Delete()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/09/04, shizhang.tang create
************************************************************/
VOID SEVC_Delete(SEVC_S *pstSevc)
{
    if (NULL == pstSevc)
    {
        return ;
    }

#if SEVC_SWITCH_WN
    SEVC_WN_Delete(pstSevc->pstSevcWnEng);
#endif
#if SEVC_SWITCH_BF
    SEVC_BF_Delete(pstSevc->pstSevcBfEng);
#endif
#if SEVC_SWITCH_AEC
    SEVC_AEC_Delete(pstSevc->pstSevcAecEng);
#endif
#if SEVC_SWITCH_GSC
    SEVC_GSC_Delete(pstSevc->pstSevcGscEng);
#endif
#if SEVC_SWITCH_BF_POST
    SEVC_BFPOST_Delete(pstSevc->pstSevcBfPostEng);
#endif
#if SEVC_SWITCH_AUX
    SEVC_AUX_Delete(pstSevc->pstSevcAuxEng);
#endif
#if SEVC_SWITCH_NR
    SEVC_NR_Delete(pstSevc->pstSevcNrEng);
#endif
#if SEVC_SWITCH_NN
    SEVC_NN_Delete(pstSevc->pstSevcNnEng);
#endif
#if SEVC_SWITCH_AES
    SEVC_AES_Delete(pstSevc->pstSevcAesEng);
#endif
#if SEVC_SWITCH_EQ
    SEVC_EQ_Delete(pstSevc->pstSevcEqEng);
#endif
#if SEVC_SWITCH_AGC
    SEVC_AGC_Delete(pstSevc->pstSevcAgcEng);
#endif
}


