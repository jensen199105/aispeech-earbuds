#include <stdio.h>
#include "AISP_TSL_base.h"
#include "AISP_TSL_str.h"
#include "AISP_TSL_complex.h"

#include "sevc_config.h"
#include "sevc_types.h"
#include "sevc_util.h"

#include "sevc_bfpost.h"

extern FILE *fpYyPcm;

U32 SEVC_BFPOST_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
#if SEVC_SWITCH_BF_POST
    U32 uiSize;
    U16 fftBins = pstSevcCfg->usFftBin;
    // power gsc
    uiSize = SIZE_ALIGN(sizeof(aisp_s64_t) * fftBins);
    uiTotalMemSize += uiSize;
    // srk gsc
    uiSize = SIZE_ALIGN(sizeof(aisp_s32_t) * fftBins);
    uiTotalMemSize += uiSize;
    // gammaLast
    uiSize = SIZE_ALIGN(sizeof(aisp_s32_t) * fftBins);
    uiTotalMemSize += uiSize;
#endif
    return uiTotalMemSize;
}

U32 SEVC_BFPOST_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    U32 uiSize;
    // bfpost Engine
    uiSize = SIZE_ALIGN(sizeof(SEVC_BFPOST_S));
    uiTotalMemSize += uiSize;
#if SEVC_SWITCH_BF_POST
    U16 fftBins = pstSevcCfg->usFftBin;
    // Stored Minimum Gsc
    uiSize = SIZE_ALIGN(sizeof(VOID *) * pstSevcCfg->ucU);
    uiTotalMemSize += uiSize;
    uiSize = SIZE_ALIGN(sizeof(aisp_s64_t) * fftBins) * pstSevcCfg->ucU;
    uiTotalMemSize += uiSize;
    // gsc smoothed
    uiSize = SIZE_ALIGN(sizeof(aisp_s64_t) * fftBins);
    uiTotalMemSize += uiSize;
    // minimum gsc
    uiSize = SIZE_ALIGN(sizeof(aisp_s64_t) * fftBins);
    uiTotalMemSize += uiSize;
    // temporary gsc
    uiSize = SIZE_ALIGN(sizeof(aisp_s64_t) * fftBins);
    uiTotalMemSize += uiSize;
    // pk gsc
    uiSize = SIZE_ALIGN(sizeof(aisp_s32_t) * fftBins);
    uiTotalMemSize += uiSize;
    // noise ps gsc
    uiSize = SIZE_ALIGN(sizeof(aisp_s64_t) * fftBins);
    uiTotalMemSize += uiSize;
    // gamma
    uiSize = SIZE_ALIGN(sizeof(aisp_s32_t) * fftBins);
    uiTotalMemSize += uiSize;
    // GH1
    uiSize = SIZE_ALIGN(sizeof(aisp_s32_t) * fftBins);
    uiTotalMemSize += uiSize;
    // noiseps
    uiSize = SIZE_ALIGN(sizeof(aisp_s64_t) * fftBins);
    uiTotalMemSize += uiSize;
#endif
#ifndef USE_SHARE_MEM
    uiTotalMemSize += SEVC_BFPOST_ShMemSizeGet(pstSevcCfg);
#endif
#if MEM_BUFF_DBG
    MSG_DBG("%s uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}

SEVC_BFPOST_S *SEVC_BFPOST_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator)
{
    SEVC_BFPOST_S *pstBfpostEng = NULL;
    U32 uiSize;
    MEM_LOCATOR_SHOW(pstMemLocator);
    // allocate for bfpost engine
    uiSize = sizeof(SEVC_BFPOST_S);
    pstBfpostEng = (SEVC_BFPOST_S *)memLocatorGet(pstMemLocator, uiSize, ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstBfpostEng, FAIL);
    pstBfpostEng->pstSevcCfg = pstSevcCfg;
#if SEVC_SWITCH_BF_POST
    S32 iLoop;
    U32 fftBins = pstSevcCfg->usFftBin;
    aisp_s32_t *tmpPtr = NULL;
    // unshared memory
    // allocate for stored minimum gsc
    BUFF_POINT_SET_ALIGN(pstBfpostEng->pStoredMinGsc, pstSevcCfg->ucU * sizeof(VOID *), pstMemLocator, FAIL);

    for (iLoop = 0; iLoop < pstSevcCfg->ucU; ++iLoop)
    {
        BUFF_POINT_SET_ALIGN(pstBfpostEng->pStoredMinGsc[iLoop], sizeof(aisp_s64_t) * fftBins,
                             pstMemLocator, FAIL);
    }

    // allocate for smoothed gsc
    BUFF_POINT_SET_ALIGN(pstBfpostEng->pSmGsc, sizeof(aisp_s64_t) * fftBins, pstMemLocator, FAIL);
    // allocate for minimum gsc
    BUFF_POINT_SET_ALIGN(pstBfpostEng->pMinGsc, sizeof(aisp_s64_t) * fftBins, pstMemLocator, FAIL);
    // allocate for tmp gsc
    BUFF_POINT_SET_ALIGN(pstBfpostEng->pTmpGsc, sizeof(aisp_s64_t) * fftBins, pstMemLocator, FAIL);
    // allocate for pk gsc
    BUFF_POINT_SET_ALIGN(pstBfpostEng->pPkGsc, sizeof(aisp_s32_t) * fftBins, pstMemLocator, FAIL);
    // allocate for noise ps gsc
    BUFF_POINT_SET_ALIGN(pstBfpostEng->pNoisePsGsc, sizeof(aisp_s64_t) * fftBins, pstMemLocator, FAIL);
    // allocate for gamma
    BUFF_POINT_SET_ALIGN(pstBfpostEng->pGamma, sizeof(aisp_s32_t) * fftBins, pstMemLocator, FAIL);
    tmpPtr = pstBfpostEng->pGamma;

    for (iLoop = 0; iLoop < fftBins; ++iLoop)
    {
        *tmpPtr++ = AISP_TSL_Q24_ONE;
    }

    // allocate for GH1
    BUFF_POINT_SET_ALIGN(pstBfpostEng->pGH1, sizeof(aisp_s32_t) * fftBins, pstMemLocator, FAIL);
    tmpPtr = pstBfpostEng->pGH1;

    for (iLoop = 0; iLoop < fftBins; ++iLoop)
    {
        *tmpPtr++ = AISP_TSL_Q24_ONE;
    }

    // allocate for noiseps
    BUFF_POINT_SET_ALIGN(pstBfpostEng->pNoisePs, sizeof(aisp_s64_t) * fftBins, pstMemLocator, FAIL);
    // shared memory
    // noisy gsc power
    SH_BUFF_POINT_SET_ALIGN(pstBfpostEng->pNoisyPowerGsc, sizeof(aisp_s64_t) * fftBins, pstMemLocator, FAIL);
    // srk gsc
    SH_BUFF_POINT_SET_ALIGN(pstBfpostEng->pSrkGsc, sizeof(aisp_s32_t) * fftBins, pstMemLocator, FAIL);
    // gamma last
    SH_BUFF_POINT_SET_ALIGN(pstBfpostEng->pGammaLast, sizeof(aisp_s32_t) * fftBins, pstMemLocator, FAIL);
    // set uIdx and vIdx
    pstBfpostEng->uIdx = 0;
    pstBfpostEng->vIdx = 2;
    pstBfpostEng->frameCnt = 0;
#endif
    MEM_LOCATOR_SHOW(pstMemLocator);
    return pstBfpostEng;
FAIL:
    MSG_DBG("%s [%d] fail\r\n", __func__, __LINE__);
    return NULL;
}

VOID SEVC_BFPOST_CallBackFuncRegister(SEVC_BFPOST_S *pstBfpostEng,
                                      VOID *pCBFunc, VOID *pUsrData)
{
    if (NULL == pstBfpostEng || NULL == pCBFunc)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return ;
    }

    pstBfpostEng->pCallBackFunc = (CallBackFunc)pCBFunc;
    pstBfpostEng->pUsrData = pUsrData;
}

VOID SEVC_BFPOST_Feed(SEVC_BFPOST_S *pstBfpostEng, aisp_cpx_s32_t *pNoisyGsc,
                      aisp_s32_t *pfGain, aisp_s32_t *pfSpp, aisp_s32_t iBfFlag, aisp_s32_t noiseUpdate)
{
    aisp_s64_t *pPowerGsc      = pstBfpostEng->pNoisyPowerGsc;
    aisp_s64_t *pSmGsc         = pstBfpostEng->pSmGsc;
    aisp_s64_t *pMinGsc        = pstBfpostEng->pMinGsc;
    aisp_s64_t *pTmpGsc        = pstBfpostEng->pTmpGsc;
    aisp_s64_t **pStoredMinGsc = pstBfpostEng->pStoredMinGsc;
    aisp_s32_t *pSrkGsc        = pstBfpostEng->pSrkGsc;
    aisp_s32_t *pPkGsc         = pstBfpostEng->pPkGsc;
    aisp_s64_t *pNoisePsGsc    = pstBfpostEng->pNoisePsGsc;
    aisp_s32_t *pGamma         = pstBfpostEng->pGamma;
    aisp_s64_t *pNoisePs       = pstBfpostEng->pNoisePs;
    aisp_s32_t *pGammaLast     = pstBfpostEng->pGammaLast;
    aisp_s32_t *pGH1           = pstBfpostEng->pGH1;
    aisp_cpx_s32_t *postOut    = pNoisyGsc;
    SEVC_CONFIG_S *pstSevcCfg  = pstBfpostEng->pstSevcCfg;
    // variables
    aisp_s32_t mcraAs         = pstSevcCfg->fMcraAs;
    aisp_s32_t mcraAsMinus    = AISP_TSL_Q24_ONE - mcraAs;
    aisp_s32_t mcraAp         = pstSevcCfg->fMcraAp;
    aisp_s32_t mcraApMinus    = AISP_TSL_Q24_ONE - mcraAp;
    aisp_s32_t mcraDelta      = pstSevcCfg->usMcraDelta;
    U8         U              = pstSevcCfg->ucU;
    U8         V              = pstSevcCfg->ucV;
    aisp_s32_t mcraAd         = pstSevcCfg->fBfpostMcraAd;
    aisp_s32_t mcraAdMinus    = AISP_TSL_Q24_ONE - mcraAd;
    aisp_s32_t alpha          = pstSevcCfg->fBfpostAlpha;
    aisp_s32_t alphaMinus     = AISP_TSL_Q24_ONE - alpha;
    aisp_s32_t ratioYuZhi     = pstSevcCfg->fRatioYuZhi;
    aisp_s32_t k0             = pstSevcCfg->usK0;
    aisp_s32_t k1             = pstSevcCfg->usK1;
    aisp_s32_t pesai0         = pstSevcCfg->fPesai0;
    aisp_s32_t beta           = pstSevcCfg->fBeta;
    aisp_s32_t priorMin       = pstSevcCfg->fPriorMin;
    aisp_s32_t gainFloor      = pstSevcCfg->fGainFloor;
    aisp_s32_t postGain       = pstSevcCfg->postGain;
    // local variables
    S32 iBinIdx;
    S32 iLoop;
    aisp_s64_t *tmp = NULL;
    aisp_s64_t tmpVal;
    // aisp_s32_t tmpValMinus;
    aisp_s64_t tmp1;
    aisp_s32_t epsHat;
    aisp_s64_t tmp64;
    aisp_s32_t iQuietDetect = 0;
    U32 fftBins = pstBfpostEng->pstSevcCfg->usFftBin;
    U32 halfFftBins = fftBins >> 1;
    // quiet detection starts
    tmp1 = 0;

    for (iBinIdx = 14; iBinIdx < 60; ++iBinIdx)
    {
        tmp1 += pNoisePsGsc[iBinIdx];
    }

    // compute noiselevel for use externally
    pstBfpostEng->stQuietDetectEng.noiseLevel = PDIV(tmp1, 46);

    // 2300 = 46 * 50
    if (tmp1 <= 2300)
    {
        iQuietDetect = 1;
    }

    if (pstBfpostEng->stQuietDetectEng.uiQuietState == 0)
    {
        if (iQuietDetect == 1)
        {
            pstBfpostEng->stQuietDetectEng.uiSpeCnt += 1;

            // sil2spe = 1200
            if (pstBfpostEng->stQuietDetectEng.uiSpeCnt >= 1200)
            {
                pstBfpostEng->stQuietDetectEng.uiQuietState = 1;
                pstBfpostEng->stQuietDetectEng.uiSpeCnt = 0;
            }
        }
        else
        {
            pstBfpostEng->stQuietDetectEng.uiSpeCnt = 0;
        }
    }
    else
    {
        if (iQuietDetect == 0)
        {
            pstBfpostEng->stQuietDetectEng.uiSilenceCnt += 1;

            if (pstBfpostEng->stQuietDetectEng.uiSilenceCnt >= 100)
            {
                pstBfpostEng->stQuietDetectEng.uiQuietState = 0;
                pstBfpostEng->stQuietDetectEng.uiSilenceCnt = 0;
            }
        }
        else
        {
            pstBfpostEng->stQuietDetectEng.uiSilenceCnt = 0;
        }
    }

    // noise detect
    // if (pstBfpostEng->stQuietDetectEng.highNoise == 0)
    // {
    //     // mean > 1500
    //     if (tmp1 > 69000)
    //     {
    //         pstBfpostEng->stQuietDetectEng.highCnt += 1;

    //         if (pstBfpostEng->stQuietDetectEng.highCnt >= 100)
    //         {
    //             pstBfpostEng->stQuietDetectEng.highNoise = 1;
    //             pstBfpostEng->stQuietDetectEng.highCnt = 0;
    //         }
    //     }
    //     else
    //     {
    //         pstBfpostEng->stQuietDetectEng.highCnt = 0;
    //     }
    // }
    // else
    // {
    //     if (tmp1 <= 69000)
    //     {
    //         pstBfpostEng->stQuietDetectEng.lowCnt += 1;

    //         if (pstBfpostEng->stQuietDetectEng.lowCnt >= 100)
    //         {
    //             pstBfpostEng->stQuietDetectEng.highNoise = 0;
    //             pstBfpostEng->stQuietDetectEng.lowCnt = 0;
    //         }
    //     }
    //     else
    //     {
    //         pstBfpostEng->stQuietDetectEng.lowCnt = 0;
    //     }
    // }

    // quiet detection ends
    for (iBinIdx = 0; iBinIdx < fftBins; ++iBinIdx)
    {
        *pGammaLast++ = *pGamma++;
    }

    sevcVecPowY64X32_Complex(pPowerGsc, pNoisyGsc, fftBins, 26); // W32Q19
    pPowerGsc = pstBfpostEng->pNoisyPowerGsc;

    for (iBinIdx = 0; iBinIdx < fftBins; ++iBinIdx)
    {
#if SEVC_SWITCH_NN
        tmp64 = (aisp_s64_t)mcraAsMinus * (1048576 - (PSHR(pfGain[iBinIdx], 1)));
        tmp64 = PSHR64(tmp64, 20);       //Q24
#else
        tmp64 = mcraAsMinus;
#endif
        *pSmGsc = PSHR64(((aisp_s64_t)mcraAs * (*pSmGsc)), 24)
                  + PSHR64(((aisp_s64_t)tmp64 * (*pPowerGsc)), 24);
        pSmGsc++;
        pPowerGsc++;
    }

    if (noiseUpdate == 1)
    {
        // get minimum
        pSmGsc = pstBfpostEng->pSmGsc;
        pPowerGsc = pstBfpostEng->pNoisyPowerGsc;

        for (iBinIdx = 0; iBinIdx < fftBins; ++iBinIdx)
        {
            *pMinGsc = AISP_TSL_MIN(*pMinGsc, *pSmGsc);
            *pTmpGsc = AISP_TSL_MIN(*pTmpGsc, *pSmGsc);
            pMinGsc++;
            pSmGsc++;
            pTmpGsc++;
        }

        pSmGsc = pstBfpostEng->pSmGsc;
        pMinGsc = pstBfpostEng->pMinGsc;
        pTmpGsc = pstBfpostEng->pTmpGsc;

        if ((++pstBfpostEng->vIdx) % V == 0)
        {
            // shift and update storedMinGsc
            tmp = pStoredMinGsc[0];

            for (iLoop = 1; iLoop < U; ++iLoop)
            {
                pStoredMinGsc[iLoop - 1] = pStoredMinGsc[iLoop];
            }

            pStoredMinGsc[U - 1] = tmp;
            AISP_TSL_memcpy(pStoredMinGsc[U - 1], pTmpGsc, sizeof(aisp_s64_t) * fftBins);

            for (iBinIdx = 0; iBinIdx < fftBins; ++iBinIdx)
            {
                tmpVal = pStoredMinGsc[0][iBinIdx];

                for (iLoop = 1; iLoop < U; ++iLoop)
                {
                    tmpVal = AISP_TSL_MIN(tmpVal, pStoredMinGsc[iLoop][iBinIdx]);
                }

                pMinGsc[iBinIdx] = tmpVal;
                pTmpGsc[iBinIdx] = pSmGsc[iBinIdx];
            }
        }

        pstBfpostEng->vIdx = pstBfpostEng->vIdx % V;
        pTmpGsc = pstBfpostEng->pTmpGsc;
        pMinGsc = pstBfpostEng->pMinGsc;
        pPowerGsc  = pstBfpostEng->pNoisyPowerGsc;

        // do compare and update
        for (iBinIdx = 0; iBinIdx < fftBins; ++iBinIdx)
        {
            tmpVal = ((*pPowerGsc + 1) > (aisp_s64_t)mcraDelta * *pMinGsc ? 1 : 0);
            *pPkGsc = PSHR64((aisp_s64_t)mcraAp * (*pPkGsc), 24) + mcraApMinus * tmpVal; // Q24
            tmpVal = mcraAd + PSHR64((aisp_s64_t)(*pPkGsc) * mcraAdMinus, 24); // Q24
            *pNoisePsGsc = PSHR64((aisp_s64_t)tmpVal * (*pNoisePsGsc), 24)
                           + PSHR64(((aisp_s64_t)(*pPowerGsc) * (AISP_TSL_Q24_ONE - tmpVal)), 24); // Q16
            pMinGsc++;
            pPkGsc++;
            pNoisePsGsc++;
            pPowerGsc++;
        }
    }

    pSrkGsc = pstBfpostEng->pSrkGsc; // use srkGsc for pesai
    pNoisePsGsc = pstBfpostEng->pNoisePsGsc;
    pPowerGsc = pstBfpostEng->pNoisyPowerGsc;

    if (pstBfpostEng->frameCnt != 0)
    {
        for (iBinIdx = 0; iBinIdx < fftBins; ++iBinIdx)
        {
            tmp64 = (aisp_s64_t)(*pNoisePsGsc) * ratioYuZhi;
            *pSrkGsc = (*pPowerGsc < PSHR64(tmp64, 24) ? 0 : AISP_TSL_Q24_ONE);
            pSrkGsc++;
            pNoisePsGsc++;
            pPowerGsc++;
        }

        pSrkGsc = pstBfpostEng->pSrkGsc;

//#if SEVC_SWITCH_BF
        if (iBfFlag)
        {
            for (iBinIdx = 31; iBinIdx < fftBins; ++iBinIdx)
            {
                pSrkGsc[iBinIdx] = (pfSpp[iBinIdx] == 0 ?  SEVC_MIN(pSrkGsc[iBinIdx], 5033165)
                                    : pSrkGsc[iBinIdx]); //5033165=round(0.3*2^24)
            }
        }

//#endif
        // denote pesai
        pSrkGsc = pstBfpostEng->pSrkGsc;
        tmp64 = 0;

        for (iBinIdx = k0; iBinIdx <= k1; ++iBinIdx)
        {
            tmp64 += pSrkGsc[iBinIdx];
        }

        tmp64 /= (k1 - k0 + 1);
        // pSrkGsc denote q_hat in matlab
        pSrkGsc = pstBfpostEng->pSrkGsc; // pk Q24

        if (tmp64 <= pesai0)
        {
            AISP_TSL_memset(pSrkGsc, 0, sizeof(aisp_s32_t) * fftBins);
        }

        pPowerGsc  = pstBfpostEng->pNoisyPowerGsc;
        pGamma     = pstBfpostEng->pGamma;
        pGammaLast = pstBfpostEng->pGammaLast;
        pNoisePs   = pstBfpostEng->pNoisePs;
        pGH1       = pstBfpostEng->pGH1;

//#if SEVC_SWITCH_BF
        if (iBfFlag)
        {
            for (iLoop = 0; iLoop < 8; ++iLoop)
            {
                pfSpp[iLoop] = 1;
            }
        }

//#endif

        // compute posteriori
        for (iBinIdx = 0; iBinIdx < fftBins; ++iBinIdx)
        {
            aisp_s32_t GH1Pow;
            tmp64 = PDIV64((aisp_s64_t)(*pPowerGsc) * AISP_TSL_Q24_ONE, (aisp_s64_t)(*pNoisePs + EPS));
            *pGamma = tmp64 > 671088640 ? 671088640 : (aisp_s32_t)tmp64; //671088640=40 * AISP_TSL_Q24_ONE // assign gamma Q24
            GH1Pow = PSHR64(((aisp_s64_t) * pGH1 * (*pGH1)), 24); // Q24
            tmpVal = PSHR64((aisp_s64_t)alpha * (*pGammaLast), 24); // Q24
            tmp1 = PSHR64(((aisp_s64_t)GH1Pow * tmpVal), 24);
            tmp64 = ((*pGamma > AISP_TSL_Q24_ONE) ? *pGamma - AISP_TSL_Q24_ONE : 0);
            epsHat = tmp1 +  PSHR64(((aisp_s64_t)alphaMinus * tmp64), 24);// Q24
#if 0
#if SEVC_SWITCH_BF
            epsHat = AISP_TSL_MAX(epsHat * pfSpp[iBinIdx], priorMin);
#else
            epsHat = AISP_TSL_MAX(epsHat, priorMin);
#endif
#else

            if (iBfFlag)
            {
                epsHat = AISP_TSL_MAX(epsHat * pfSpp[iBinIdx], priorMin);
            }
            else
            {
                epsHat = AISP_TSL_MAX(epsHat, priorMin);
            }

#endif
            // epsHat = AISP_TSL_MAX(epsHat, priorMin);
            *pGH1 = PDIV64(((aisp_s64_t)epsHat * AISP_TSL_Q24_ONE), (AISP_TSL_Q24_ONE + (aisp_s64_t)epsHat));
            pPowerGsc++;
            pNoisePs++;
            pGammaLast++;
            pGH1++;
            pGamma++;
        }

        // absence probability
        pSrkGsc = pstBfpostEng->pSrkGsc; // pk Q24
        pGH1    = pstBfpostEng->pGH1;
        pNoisePs = pstBfpostEng->pNoisePs;
        pPowerGsc = pstBfpostEng->pNoisyPowerGsc;

        // for the first half bin, gainFloor is unchanged
        for (iBinIdx = 0; iBinIdx < halfFftBins; ++iBinIdx)
        {
            aisp_s32_t fadk;
            // aisp_s32_t fpk;
            // fpk = AISP_TSL_Q24_ONE - *pSrkGsc;
            fadk = mcraAd + PSHR64((aisp_s64_t)mcraAdMinus * (*pSrkGsc), 24); // Q24
            tmpVal = PSHR64((aisp_s64_t)beta * (AISP_TSL_Q24_ONE - fadk), 24); // Q24
            *pNoisePs = PSHR64((aisp_s64_t)fadk * (*pNoisePs), 24) + PSHR64(((aisp_s64_t)tmpVal * (*pPowerGsc)), 24); // Q16
            tmp1 = AISP_TSL_MAX(*pGH1, gainFloor);
            tmp64 = (aisp_s64_t)postOut->real * tmp1;
            postOut->real = PSHR64(tmp64, 24); // Q21
            tmp64 = (aisp_s64_t)postOut->imag * tmp1;
            postOut->imag = PSHR64(tmp64, 24); // Q21
            pSrkGsc++;
            pNoisePs++;
            pPowerGsc++;
            postOut++;
            pGH1++;
        }

        // for the second half bin, gainFloor is depending on quiet state
        gainFloor = (pstBfpostEng->stQuietDetectEng.uiQuietState == 1 && fftBins == 257) ? AISP_TSL_Q23_ONE : gainFloor;

        for (iBinIdx = halfFftBins; iBinIdx < fftBins; ++iBinIdx)
        {
            aisp_s32_t fadk;
            // aisp_s32_t fpk;
            // fpk = AISP_TSL_Q24_ONE - *pSrkGsc;
            fadk = mcraAd + PSHR64((aisp_s64_t)mcraAdMinus * (*pSrkGsc), 24); // Q24
            tmpVal = PSHR64((aisp_s64_t)beta * (AISP_TSL_Q24_ONE - fadk), 24); // Q24
            *pNoisePs = PSHR64((aisp_s64_t)fadk * (*pNoisePs), 24) + PSHR64(((aisp_s64_t)tmpVal * (*pPowerGsc)), 24); // Q16
            tmp1 = AISP_TSL_MAX(*pGH1, gainFloor);
            tmp64 = (aisp_s64_t)postOut->real * tmp1;
            postOut->real = PSHR64(tmp64, 24); // Q21
            tmp64 = (aisp_s64_t)postOut->imag * tmp1;
            postOut->imag = PSHR64(tmp64, 24); // Q21
            pSrkGsc++;
            pNoisePs++;
            pPowerGsc++;
            postOut++;
            pGH1++;
        }
    }
    else
    {
        pNoisePs = pstBfpostEng->pNoisePs;
        pPowerGsc = pstBfpostEng->pNoisyPowerGsc;

        for (iBinIdx = 0; iBinIdx < fftBins; ++iBinIdx)
        {
            *pNoisePs++ = *pPowerGsc++;
        }
    }

    // apply post gain
    postOut = pNoisyGsc;

    for (iBinIdx = 0; iBinIdx < fftBins; ++iBinIdx)
    {
        postOut->real = PSHR64((aisp_s64_t)postOut->real * postGain, 15);
        postOut->imag = PSHR64((aisp_s64_t)postOut->imag * postGain, 15);
        postOut++;
    }

    // fwrite(pNoisyGsc, sizeof(aisp_cpx_s32_t), 257, fpYyPcm);
    SEVC_S *pstSevcEng = (SEVC_S *)pstBfpostEng->pUsrData;
    pstSevcEng->uiQuietState = pstBfpostEng->stQuietDetectEng.uiQuietState;
    pstBfpostEng->frameCnt++;
    pstBfpostEng->frameCnt %= AISP_TSL_Q31_ONE_MINI;
}

VOID SEVC_BFPOST_Reset(SEVC_BFPOST_S *pstBfpostEng)
{
}

VOID SEVC_BFPOST_Delete(SEVC_BFPOST_S *pstBfpostEng)
{
}


