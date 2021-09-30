#include <stdio.h>
#include "AISP_TSL_base.h"
#include "AISP_TSL_common.h"
#include "AISP_TSL_complex.h"

#include "sevc_config.h"
#include "sevc_types.h"
#include "sevc_util.h"

#include "sevc_state.h"

extern FILE *fpYyPcm;

enum DetectorType { WIND, WEAR };

VOID SD_DetectorInit(SEVC_CONFIG_S *pstSevcCfg, StateProbe_S *pstStateProbe, aisp_s32_t trig, enum DetectorType type)
{
    pstStateProbe->fAlpha = pstSevcCfg->fSdAlpha;
    pstStateProbe->fDetectTrig = trig;

    if (type == WIND)
    {
        pstStateProbe->zero2One = pstSevcCfg->wind2quit;
        pstStateProbe->one2Zero = pstSevcCfg->quit2wind;
    }
    else
        if (type == WEAR)
        {
            pstStateProbe->zero2One = pstSevcCfg->loose2tight;
            pstStateProbe->one2Zero = pstSevcCfg->tight2loose;
        }

    pstStateProbe->oneCnt = 0;
    pstStateProbe->zeroCnt = 0;
    pstStateProbe->usStatus = 0;
    pstStateProbe->fDStatus = 0;

    if (type == WIND)
    {
        pstStateProbe->usStatus = 1;
    }
}

S32 SD_Detect(StateProbe_S *detector, aisp_s32_t ratioMean)
{
    aisp_s32_t fAlpha = detector->fAlpha;
    aisp_s32_t fMinusAlpha = AISP_TSL_Q15_ONE - fAlpha;
    aisp_s32_t detectState;
    detector->fDStatus = PSHR((aisp_s64_t)fAlpha * detector->fDStatus + (aisp_s64_t)fMinusAlpha * ratioMean, 15);
    detectState = detector->fDStatus > detector->fDetectTrig ? 1 : 0;

    if (detector->usStatus == 0)
    {
        if (detectState == 1)
        {
            detector->oneCnt += 1;

            if (detector->oneCnt >= detector->zero2One)
            {
                detector->usStatus = 1;
                detector->oneCnt = 0;
            }
        }
        else
        {
            detector->oneCnt = 0;
        }
    }
    else
    {
        if (detectState == 0)
        {
            detector->zeroCnt += 1;

            if (detector->zeroCnt >= detector->one2Zero)
            {
                detector->usStatus = 0;
                detector->zeroCnt = 0;
            }
        }
        else
        {
            detector->zeroCnt = 0;
        }
    }

    return detector->usStatus;
}

S32 SD_SpeedCalc(aisp_s64_t powSum)
{
    if (powSum < 1.5e4)
    {
        return 0;
    }
    else
        if (powSum < 2e7)
        {
            return 2 * AISP_TSL_Q16_ONE;
        }
        else
            if (powSum < 1e8)
            {
                return 3 * AISP_TSL_Q16_ONE;
            }
            else
                if (powSum < 1.8e8)
                {
                    return 4 * AISP_TSL_Q16_ONE;
                }
                else
                    if (powSum < 4e8)
                    {
                        return 5 * AISP_TSL_Q16_ONE;
                    }
                    else
                        if (powSum < 1.1e9)
                        {
                            return 7 * AISP_TSL_Q16_ONE;
                        }
                        else
                            if (powSum < 2.1e9)
                            {
                                return 9 * AISP_TSL_Q16_ONE;
                            }
                            else
                            {
                                return 11 * AISP_TSL_Q16_ONE;
                            }
}

U32 SEVC_SD_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    return uiTotalMemSize;
}

U32 SEVC_SD_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    U32 uiSize;
    U16 detectBinSize = pstSevcCfg->usBinEnd - pstSevcCfg->usBinStart;
#ifdef SEVC_SWITCH_WN
    uiSize = SIZE_ALIGN(sizeof(SEVC_STATE_S));
    uiTotalMemSize += uiSize;
    // coh11
    uiSize = SIZE_ALIGN(sizeof(aisp_s64_t) * detectBinSize);
    uiTotalMemSize += uiSize;
    // coh22
    uiSize = SIZE_ALIGN(sizeof(aisp_s64_t) * detectBinSize);
    uiTotalMemSize += uiSize;
    // coh33
    uiSize = SIZE_ALIGN(sizeof(aisp_s64_t) * detectBinSize);
    uiTotalMemSize += uiSize;
    // coh12
    uiSize = SIZE_ALIGN(sizeof(aisp_cpx_s32_t) * detectBinSize);
    uiTotalMemSize += uiSize;
    // coh13
    uiSize = SIZE_ALIGN(sizeof(aisp_cpx_s32_t) * detectBinSize);
    uiTotalMemSize += uiSize;
#endif
#ifndef USE_SHARE_MEM
    uiTotalMemSize += SEVC_SD_ShMemSizeGet(pstSevcCfg);
#endif
#if MEM_BUFF_DBG
    MSG_DBG("%s uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}

SEVC_STATE_S *SEVC_SD_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator)
{
    U32 uiSize;
    SEVC_STATE_S *pstSevcStateEng = NULL;
    MEM_LOCATOR_SHOW(pstMemLocator);
    uiSize = sizeof(SEVC_STATE_S);
    pstSevcStateEng = (SEVC_STATE_S *)memLocatorGet(pstMemLocator, uiSize, ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstSevcStateEng, FAIL);
    pstSevcStateEng->pstSevcCfg = pstSevcCfg;
#if SEVC_SWITCH_WN
    U32 uiBinSize = pstSevcCfg->usBinEnd - pstSevcCfg->usBinStart;
    uiSize = sizeof(aisp_s64_t) * uiBinSize;
    // allocate for coh11
    BUFF_POINT_SET_ALIGN(pstSevcStateEng->coh11, uiSize, pstMemLocator, FAIL);
    // allocate for coh22
    BUFF_POINT_SET_ALIGN(pstSevcStateEng->coh22, uiSize, pstMemLocator, FAIL);
    // allocate for coh33
    BUFF_POINT_SET_ALIGN(pstSevcStateEng->coh33, uiSize, pstMemLocator, FAIL);
    uiSize = sizeof(aisp_cpx_s32_t) * uiBinSize;
    // allocate for coh12
    BUFF_POINT_SET_ALIGN(pstSevcStateEng->coh12, uiSize, pstMemLocator, FAIL);
    // allocate for coh13
    BUFF_POINT_SET_ALIGN(pstSevcStateEng->coh13, uiSize, pstMemLocator, FAIL);
    pstSevcStateEng->uiFrameCount = 0;
    pstSevcStateEng->usWindSwitch = pstSevcCfg->usWindSwitch;
    pstSevcStateEng->usWearSwitch = pstSevcCfg->usWearSwitch;
    pstSevcStateEng->usWindStatus = 0;
    pstSevcStateEng->usWearStatus = 0;
    pstSevcStateEng->windSpeed = 0;
    SD_DetectorInit(pstSevcCfg, &pstSevcStateEng->stWindDetector, pstSevcCfg->fWindTrig, WIND);
    SD_DetectorInit(pstSevcCfg, &pstSevcStateEng->stWearDetector, pstSevcCfg->fWearTrig, WEAR);
    return pstSevcStateEng;
#endif
FAIL:
    MSG_DBG("%s [%d] fail\r\n", __func__, __LINE__);
    return NULL;
}

VOID SEVC_SD_CallBackFuncRegister(SEVC_STATE_S *pstSevcStateEng, VOID *pCBFunc, VOID *pUsrData)
{
    if (pstSevcStateEng == NULL || pCBFunc == NULL)
    {
        MSG_DBG("%s, [%d] param error \r\n", __func__, __LINE__);
        return;
    }

    pstSevcStateEng->pCallBackFunc = pCBFunc;
    pstSevcStateEng->pUsrData = pUsrData;
}

VOID SEVC_SD_Feed(SEVC_STATE_S *pstSevcStateEng, aisp_cpx_s32_t *pMic)
{
#if SEVC_SWITCH_WN
    SEVC_CONFIG_S *pstSevcCfg = pstSevcStateEng->pstSevcCfg;
    U32 uiBinStart = pstSevcCfg->usBinStart;
    U32 uiBinEnd = pstSevcCfg->usBinEnd;
    U32 uiBinSize = uiBinEnd - uiBinStart;
    U32 usFftBin = pstSevcCfg->usFftBin;
    U32 binIdx;
    aisp_cpx_s32_t *firstMic            =                pMic;
    aisp_cpx_s32_t *secMic              =                pMic + usFftBin;
    aisp_cpx_s32_t *thirdMic            =                secMic + usFftBin;
    aisp_cpx_s32_t *startPtr1           =                firstMic + uiBinStart;
    aisp_cpx_s32_t *startPtr2           =                secMic + uiBinStart;
    aisp_cpx_s32_t *startPtr3           =                thirdMic + uiBinStart;
    aisp_s64_t *pCoh11                  =                pstSevcStateEng->coh11;
    aisp_s64_t *pCoh22                  =                pstSevcStateEng->coh22;
    aisp_cpx_s32_t *pCoh12               =                pstSevcStateEng->coh12;
    aisp_cpx_s32_t *pCoh13               =                pstSevcStateEng->coh13;
    aisp_s32_t falpha                   =                pstSevcCfg->fSdAlpha;
    aisp_s32_t fMinusAlpha              =                AISP_TSL_Q15_ONE - falpha;
    aisp_s32_t fWindSmooth              =                pstSevcCfg->fWindSmooth;
    aisp_s32_t fMinusWindSmooth         =                AISP_TSL_Q15_ONE - fWindSmooth;
    aisp_s32_t tmpReal;
    aisp_s32_t tmpImag;
    aisp_s64_t tmp1;
    aisp_s64_t powerSum;
    aisp_s32_t speedCalc;
    aisp_s64_t ratioSum;
    aisp_s32_t ratioMean;
    aisp_s32_t windStatus = 0;

    for (binIdx = 0; binIdx < uiBinSize; ++binIdx)
    {
        tmp1 = (aisp_s64_t)startPtr1->real * startPtr1->real + (aisp_s64_t)startPtr1->imag * startPtr1->imag;
        tmp1 = PSHR(tmp1, 26);
        *pCoh11 = (aisp_s64_t)falpha * (*pCoh11) + (aisp_s64_t)fMinusAlpha * tmp1;
        *pCoh11 = PSHR(*pCoh11, 15);
        pCoh11++;
        startPtr1++;
    }

    startPtr1 = firstMic + uiBinStart;
    pCoh11 = pstSevcStateEng->coh11;
    powerSum = 0;

    if (pstSevcStateEng->usWindSwitch == 1)
    {
        // sum up
        for (binIdx = 0; binIdx < uiBinSize; ++binIdx)
        {
            powerSum += *pCoh11;
            pCoh11++;
        }

        // compute coherence of second mic
        for (binIdx = 0; binIdx < uiBinSize; ++binIdx)
        {
            tmp1 = (aisp_s64_t)startPtr2->real * startPtr2->real + (aisp_s64_t)startPtr2->imag * startPtr2->imag;
            tmp1 = PSHR(tmp1, 26);
            *pCoh22 = (aisp_s64_t)falpha * (*pCoh22) + (aisp_s64_t)fMinusAlpha * tmp1;
            *pCoh22 = PSHR(*pCoh22, 15);
            pCoh22++;
            startPtr2++;
        }

        // compute cross coh of first and second mic
        startPtr2 = secMic + uiBinStart;

        for (binIdx = 0; binIdx < uiBinSize; ++binIdx)
        {
            tmp1 = (aisp_s64_t)startPtr1->real * startPtr2->real + (aisp_s64_t)startPtr1->imag * startPtr2->imag;
            tmpReal = PSHR(tmp1, 26);
            tmp1 = (aisp_s64_t)startPtr1->imag * startPtr2->real - (aisp_s64_t)startPtr1->real * startPtr2->imag;
            tmpImag = PSHR(tmp1, 26);
            pCoh12->real = PSHR((aisp_s64_t)falpha * pCoh12->real + (aisp_s64_t)fMinusAlpha * tmpReal, 15);
            pCoh12->imag = PSHR((aisp_s64_t)falpha * pCoh12->imag + (aisp_s64_t)fMinusAlpha * tmpImag, 15);
            startPtr1++;
            startPtr2++;
            pCoh12++;
        }

        if (powerSum > 15000)
        {
            pCoh11 =  pstSevcStateEng->coh11;
            pCoh22 =  pstSevcStateEng->coh22;
            pCoh12 =  pstSevcStateEng->coh12;
            ratioSum = 0;

            for (binIdx = 0; binIdx < uiBinSize; ++binIdx)
            {
                tmp1 = PDIV((aisp_s64_t)pCoh12->real * pCoh12->real, (*pCoh11) + 1);
                ratioSum += PDIV(tmp1 * AISP_TSL_Q16_ONE, (*pCoh22) + 1);
                pCoh11++;
                pCoh22++;
                pCoh12++;
            }

            ratioMean = PDIV(ratioSum, uiBinSize); // Q16
            SD_Detect(&pstSevcStateEng->stWindDetector, ratioMean);
            windStatus = 1 - pstSevcStateEng->stWindDetector.usStatus;
        }
        else
        {
            windStatus = 0;
            pstSevcStateEng->windSpeed = 0;
        }

        if (windStatus == 1)
        {
            speedCalc = SD_SpeedCalc(powerSum);
            pstSevcStateEng->windSpeed = PSHR((aisp_s64_t)fWindSmooth * pstSevcStateEng->windSpeed + \
                                              (aisp_s64_t)fMinusWindSmooth * speedCalc, 15); // windSpeed Q16

            for (binIdx = 0; binIdx < usFftBin; ++binIdx)
            {
                if ((aisp_s64_t)AISP_TSL_ABS(firstMic[binIdx].real) + AISP_TSL_ABS(firstMic[binIdx].imag) > \
                        (aisp_s64_t)AISP_TSL_ABS(secMic[binIdx].real) + AISP_TSL_ABS(secMic[binIdx].imag))
                {
                    firstMic[binIdx].real = secMic[binIdx].real;
                    firstMic[binIdx].imag = secMic[binIdx].imag;
                }
            }
        }
    }

    if (pstSevcStateEng->usWearSwitch == 1)
    {
        startPtr1 = firstMic + uiBinStart;
        startPtr3 = thirdMic + uiBinStart;
        pCoh13 = pstSevcStateEng->coh13;

        for (binIdx = 0; binIdx < uiBinSize; ++binIdx)
        {
            tmp1 = (aisp_s64_t)startPtr1->real * startPtr3->real + (aisp_s64_t)startPtr1->imag * startPtr3->imag;
            tmpReal = PSHR(tmp1, 26);
            tmp1 = (aisp_s64_t)startPtr1->imag * startPtr3->real - (aisp_s64_t)startPtr1->real * startPtr3->imag;
            tmpImag = PSHR(tmp1, 26);
            pCoh13->real = PSHR((aisp_s64_t)falpha * pCoh13->real + (aisp_s64_t)fMinusAlpha * tmpReal, 15);
            pCoh13->imag = PSHR((aisp_s64_t)falpha * pCoh13->imag + (aisp_s64_t)fMinusAlpha * tmpImag, 15);
            startPtr1++;
            startPtr3++;
            pCoh13++;
        }

        pCoh13 = pstSevcStateEng->coh13;
        pCoh11 = pstSevcStateEng->coh11;
        ratioSum = 0;

        for (binIdx = 0; binIdx < uiBinSize; ++binIdx)
        {
            ratioSum += PDIV((aisp_s64_t)pCoh13->real * pCoh13->real, *pCoh11 + 1);
        }

        ratioMean = PDIV(ratioSum, uiBinSize);
        SD_Detect(&pstSevcStateEng->stWearDetector, ratioMean);
    }

    //pstSevcStateEng->usWindStatus = 1 - pstSevcStateEng->stWindDetector.usStatus;
    pstSevcStateEng->usWindStatus = windStatus;
    pstSevcStateEng->uiFrameCount++;
#endif
}

VOID SEVC_SD_Reset(SEVC_STATE_S *pstSevcStateEng)
{
}