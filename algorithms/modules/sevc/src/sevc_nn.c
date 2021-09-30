#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "AISP_TSL_base.h"
#include "AISP_TSL_math.h"
#include "AISP_TSL_str.h"
#include "AISP_TSL_complex.h"
#include "AISP_TSL_sigmoid.h"

#include "sevc_config.h"
#include "sevc_types.h"
#include "sevc_util.h"
#ifdef NN_FSMN
#include "nn_fsmn.h"
#else
#include "nn_gru.h"
#endif
#include "sevc_nn.h"
//#include "nn_common.h"

extern FILE *fpYyPcm;

#if SEVC_SWITCH_NN
extern VOID nnVec32Add32_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                              aisp_s32_t *pSrcX, U32 uiLen, S32 rq);
extern VOID nnVecMul32X32_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                               aisp_s32_t *pSrcX, U32 uiLen, S32 rq);

#if SEVC_SWITCH_NN_AES
#if 0
/************************************************************
 Function   : sevcResidualEchoCalc()

 Description:
 Calls      :
 Called By  :
 Input      :
 Output     :
 Return     :
 Others     :

 History    :
   2020/06/10, shizhang.tang create
************************************************************/
VOID sevcResidualEchoCalc(SEVC_NN_S *pstNnEng,
                          aisp_s32_t *pfResidualEcho,
                          aisp_cpx_s32_t *pFreqFrame,
                          aisp_cpx_s32_t *pEstFrame)
{
    S32 ii;
    SEVC_CONFIG_S *pstSevcCfg = pstNnEng->pstSevcCfg;
    U16 usFftBin = pstSevcCfg->usFftBin;
    aisp_s32_t fLamda_x = pstSevcCfg->lamda_x;
#ifdef SEVC_PHONE
    aisp_cpx_s32_t *pPey = pstNnEng->pPey;
    aisp_cpx_s32_t *pPeyTmp = pstNnEng->pPeyTmp;
    aisp_s32_t *pfPyy = pstNnEng->pfPyy;
    aisp_s32_t *pfPyyTmp = pstNnEng->pfPyyTmp;
    aisp_s32_t *pfAbsEst = pstNnEng->pfPeeTmp;
    aisp_s32_t *pfAbsPeyTmp = pstNnEng->pfAbsPeyTmp;
#else
    aisp_cpx_s64_t *pPey = pstNnEng->pPey;
    aisp_s64_t *pfPyy = pstNnEng->pfPyy;
    aisp_s64_t *pfPyyTmp = pstNnEng->pfPyyTmp;
    aisp_s64_t *pfAbsEst = pstNnEng->pfPeeTmp;
    aisp_s64_t *pfAbsPeyTmp = pstNnEng->pfAbsPeyTmp;
    aisp_cpx_s64_t *pPeyTmp = pstNnEng->pPeyTmp;
#endif
#ifdef SEVC_PHONE
    sevcVecPowY32X32_Complex(pfPyyTmp, pEstFrame, usFftBin, 21);                      //W32Q21
    sevcVec32ConjMUL_Complex(pPeyTmp, pEstFrame, pFreqFrame, usFftBin, 21);           //W32Q21
    sevcVec32Smooth((aisp_s32_t *)pPey, (aisp_s32_t *)pPeyTmp, fLamda_x, 2 * usFftBin, 24);
    sevcVecAbs32_Complex(pfAbsPeyTmp, pPey, usFftBin, 21);
    sevcVecAbs32_Complex(pfAbsEst, pEstFrame, usFftBin, 21);                            //W32Q21
    sevcVec32Smooth(pfPyy, pfPyyTmp, fLamda_x, usFftBin, 24);
    sevcVec32MinRestrain(pfAbsEst, 64, usFftBin);
    //sevcVec32MinRestrain(pfPyyTmp, 64, usFftBin);
    sevcVec32MinRestrain(pfPyy, 64, usFftBin);
    sevcVec32MinRestrain(pfAbsPeyTmp, 64, usFftBin);
#else
    sevcVecPowY64X32_Complex(pfPyyTmp, pEstFrame, usFftBin, 18);                      //W32Q24
    sevcVec64Smooth(pfPyy, pfPyyTmp, fLamda_x, usFftBin, 24);
    sevcVec64ConjMUL_Complex(pPeyTmp, pEstFrame, pFreqFrame, usFftBin, 18);           //W32Q24
    sevcVec64Smooth((aisp_s64_t *)pPey, (aisp_s64_t *)pPeyTmp, fLamda_x, 2 * usFftBin, 24);
    sevcVecAbs64_Complex(pfAbsPeyTmp, pPey, usFftBin, 24);
    sevcVecAbs64_Complex(pfAbsEst, pEstFrame, usFftBin, 21);                            //W32Q21
    sevcVec64MinRestrain(pfAbsEst, 64, usFftBin);
    //sevcVec64MinRestrain(pfPyyTmp, 64, usFftBin);
    sevcVec64MinRestrain(pfPyy, 64, usFftBin);
    sevcVec64MinRestrain(pfAbsPeyTmp, 64, usFftBin);
#endif

    for (ii = 0; ii < usFftBin; ii++)
    {
        pfResidualEcho[ii] = (aisp_s64_t)pfAbsEst[ii] * pfAbsPeyTmp[ii] / (pfPyy[ii] + 1);
    }
}

/************************************************************
  Function   : sevcBandEnergyCalcReal()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2020/06/10, shizhang.tang create
************************************************************/
VOID sevcBandEnergyCalcReal(SEVC_NN_S *pstNnEng,
                            aisp_s64_t *pfNbBandsSum,
                            aisp_s32_t *pFrame)
{
    U16        *psFull2BarkMapping = pstNnEng->pstNn->psFull2BarkMapping;
    U16         usNbBands          = pstNnEng->pstNn->usNbBands;
    aisp_s32_t *pX = NULL;
    aisp_s32_t *pP = NULL;
    aisp_s64_t fData       = 0;
    aisp_s32_t iFrac       = 0;
    S32 usLoop, usLoop2;
    S32 iBandsSize;

    for (usLoop = 0; usLoop < usNbBands; usLoop++)
    {
        pfNbBandsSum[usLoop] = 0;
    }

#ifdef AISPEECH_FIXED

    for (usLoop = 0; usLoop < usNbBands - 1; usLoop++)
    {
        iBandsSize = psFull2BarkMapping[usLoop + 1] - psFull2BarkMapping[usLoop];
        pP      = pFrame + psFull2BarkMapping[usLoop];
        pX      = pFrame + psFull2BarkMapping[usLoop];

        for (usLoop2 = 0; usLoop2 < iBandsSize; usLoop2++)
        {
            iFrac = ((usLoop2 << 15) + (iBandsSize >> 1)) / iBandsSize;                     //W32Q15
            fData = PSHR((aisp_s64_t)((aisp_s64_t)pP[usLoop2] * pX[usLoop2]), 15);      //W32Q27
            pfNbBandsSum[usLoop + 0] += PSHR((aisp_s64_t)(32768 - iFrac) * fData, 15);      //W32Q27
            pfNbBandsSum[usLoop + 1] += PSHR((aisp_s64_t)iFrac * fData, 15);                //W32Q27
        }
    }

    for (usLoop = 0; usLoop < usNbBands; usLoop++)
    {
        pfNbBandsSum[usLoop] = PSHR(((aisp_s64_t)pfNbBandsSum[usLoop]), 10);                //W32Q17

        if (pfNbBandsSum[usLoop] > 0x7FFFFFF0 || pfNbBandsSum[usLoop] < 0)
        {
            //MSG_DBG("%s [%d] %d %d %lld ++++++\r\n", __func__, __LINE__, pstNnEng->uiFrameCount, usLoop, pfNbBandsSum[usLoop]);
            pfNbBandsSum[usLoop] = 0x7FFFFFF0;
        }
    }

#else

    for (usLoop = 0; usLoop < usNbBands - 1; usLoop++)
    {
        iBandsSize = psFull2BarkMapping[usLoop + 1] - psFull2BarkMapping[usLoop];
        pP      = pFrame + psFull2BarkMapping[usLoop];
        pX      = pFrame + psFull2BarkMapping[usLoop];

        for (usLoop2 = 0; usLoop2 < iBandsSize; usLoop2++)
        {
            iFrac = (aisp_s32_t)usLoop2 / iBandsSize;
            fData = (aisp_s64_t)pP[usLoop2] * pX[usLoop2];
            pfNbBandsSum[usLoop] += (1 - iFrac) * fData;
            pfNbBandsSum[usLoop + 1] += iFrac * fData;
        }
    }

    for (usLoop = 0; usLoop < usNbBands; usLoop++)
    {
        pfNbBandsSum[usLoop] = pfNbBandsSum[usLoop] / 262144; //262144=512*512
    }

#endif
}
#endif
VOID sevcRefEnergyCalc(SEVC_NN_S *pstNnEng,
                       aisp_s64_t *pfNbBandsEnergy,
                       aisp_cpx_s32_t *pFreqFrame)
{
    U16 usBinCount = pstNnEng->pstSevcCfg->usAesRefBinCount;
    U16 usBinIdx = pstNnEng->pstSevcCfg->usAesRefBinStart;
    S32 ii;

    for (ii = 0; ii < usBinCount; ii++)
    {
        COMPLEX_POW_SQ(pfNbBandsEnergy[ii], pFreqFrame[usBinIdx], 25);
        usBinIdx++;
    }
}

#endif

/************************************************************
  Function   : sevcBandEnergyCalc()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/02, shizhang.tang create
    2020/01/09, chao.xu check ok for fixed version
************************************************************/
VOID sevcBandEnergyCalc(SEVC_NN_S *pstNnEng,
                        aisp_s64_t *pfNbBandsSum,
                        aisp_cpx_s32_t *pFreqFrame)
{
    U16        *psFull2BarkMapping = pstNnEng->pstNn->psFull2BarkMapping;
    U16         usNbBands          = pstNnEng->pstNn->usNbBands;
    aisp_cpx_s32_t *pcpxX = NULL;
    aisp_cpx_s32_t *pcpxP = NULL;
    aisp_s64_t fDataTmp = 0;
    aisp_s64_t fData = 0;
    aisp_s32_t iFrac = 0;
    S32 usLoop, usLoop2;
    S32 iBandsSize;
    U8 bandEnergySmooth = pstNnEng->pstSevcCfg->ucNnBandEnergySmooth;
    aisp_s64_t *pfPowSum = (bandEnergySmooth ? pstNnEng->pstNn->pfNbBandsSumTmp : pfNbBandsSum);

    for (usLoop = 0; usLoop < usNbBands; usLoop++)
    {
        pfPowSum[usLoop] = 0;
    }

#ifdef AISPEECH_FIXED

    for (usLoop = 0; usLoop < usNbBands - 1; usLoop++)
    {
        iBandsSize = psFull2BarkMapping[usLoop + 1] - psFull2BarkMapping[usLoop];
        pcpxP       = pFreqFrame + psFull2BarkMapping[usLoop];
        pcpxX       = pFreqFrame + psFull2BarkMapping[usLoop];

        for (usLoop2 = 0; usLoop2 < iBandsSize; usLoop2++)
        {
            iFrac = ((usLoop2 << 15) + (iBandsSize >> 1)) / iBandsSize;                     //W32Q15
            fDataTmp = (aisp_s64_t)pcpxP->real * pcpxX->real + (aisp_s64_t)pcpxP->imag * pcpxX->imag;
            fData = PSHR((aisp_s64_t)fDataTmp, 15);        //W32Q27
            pfPowSum[usLoop + 0] += PSHR((aisp_s64_t)(32768 - iFrac) * fData, 15);      //W32Q27
            pfPowSum[usLoop + 1] += PSHR((aisp_s64_t)iFrac * fData, 15);                //W32Q27
            pcpxP++;
            pcpxX++;
        }
    }

    for (usLoop = 0; usLoop < usNbBands; usLoop++)
    {
        pfPowSum[usLoop] = PSHR(((aisp_s64_t)pfPowSum[usLoop]), 10);                //W32Q17

        if (bandEnergySmooth)
        {
            pfNbBandsSum[usLoop] = (pfNbBandsSum[usLoop] + pfPowSum[usLoop] + 1) / 2;
        }

        if (pfNbBandsSum[usLoop] > 0x7FFFFFF0 || pfNbBandsSum[usLoop] < 0)
        {
            //MSG_DBG("%s [%d] %d %d %lld ++++++\r\n", __func__, __LINE__, pstNnEng->uiFrameCount, usLoop, pfNbBandsSum[usLoop]);
            pfNbBandsSum[usLoop] = 0x7FFFFFF0;
        }
    }

#else

    for (usLoop = 0; usLoop < usNbBands - 1; usLoop++)
    {
        iBandsSize = psFull2BarkMapping[usLoop + 1] - psFull2BarkMapping[usLoop];
        pcpxP       = pFreqFrame + psFull2BarkMapping[usLoop];
        pcpxX       = pFreqFrame + psFull2BarkMapping[usLoop];

        for (usLoop2 = 0; usLoop2 < iBandsSize; usLoop2++)
        {
            iFrac = (aisp_s32_t)usLoop2 / iBandsSize;
            fData = (aisp_s64_t)pcpxP->real * pcpxX->real + (aisp_s64_t)pcpxP->imag * pcpxX->imag;
            pfNbBandsSum[usLoop] += (1 - iFrac) * fData;
            pfNbBandsSum[usLoop + 1] += iFrac * fData;
            pcpxP++;
            pcpxX++;
        }
    }

    for (usLoop = 0; usLoop < usNbBands; usLoop++)
    {
        pfNbBandsSum[usLoop] = pfNbBandsSum[usLoop] / 262144; //262144=512*512
    }

#endif
}

aisp_s32_t *sevcNnNewFeatureBuffGet(SEVC_NN_S *pstNnEng)
{
    S32 ii;
    U16 usBinCnt;
    NN_STATE_S *pstNn = pstNnEng->pstNn;
    U16 usFeatureContext = pstNn->usFeatureContext;
    aisp_s32_t *pfFeaturesNew;
#ifndef NN_FSMN
    aisp_s32_t *pfFeaturesHistory = pstNn->pfFeatures;
#else
    aisp_s32_t *pfFeaturesHistory   = pstNn->pfFeaturesHistory;
#endif
    U16 usFeatureDim = pstNnEng->pstSevcCfg->ucNnFeatureDim;

    if (usFeatureContext == 1)
    {
        pfFeaturesNew = pfFeaturesHistory;
    }
    else
    {
        usBinCnt = usFeatureDim * (usFeatureContext - 1);
        pfFeaturesNew = pfFeaturesHistory + usFeatureDim;

        for (ii = 0; ii < usBinCnt; ii++)
        {
            pfFeaturesHistory[ii] = pfFeaturesNew[ii];
        }

        pfFeaturesNew = pfFeaturesHistory + usFeatureDim * (usFeatureContext - 1);
    }

    return pfFeaturesNew;
}

/*
    iLogTmpOut = log(Enx+1);
    Exn's Q value is m
    pfNbBandsSum's Q value is 17
    iLogTmpOut = sevcLog(((aisp_s32_t)pfNbBandsSum[usLoop] + 1), 17-m, iQValue);
*/
VOID sevcNnExnLogCalc(SEVC_NN_S *pstNnEng, aisp_s32_t *pfFeatures, aisp_s64_t *pExn)
{
    U16 usLoop;
    NN_STATE_S *pstNn = pstNnEng->pstNn;
    S32 iQValue = nnProcessorQValueDataGet(pstNn);
    U16 usFeatureDim = pstNnEng->pstSevcCfg->ucNnFeatureDim;

    for (usLoop = 0; usLoop < usFeatureDim; usLoop++)
    {
#ifdef AISPEECH_FIXED
        aisp_s32_t iLogTmpOut   = 0;
#ifdef NN_FSMN
        //iLogTmpOut = sevcLog(((aisp_s32_t)pfNbBandsSum[usLoop] + 1), 5, 20);
#ifdef FAR_FIELD_COMMUNICATION
        iLogTmpOut = sevcLog(((aisp_s32_t)pExn[usLoop] + 1), 5, iQValue);
#else
        iLogTmpOut = sevcLog(((aisp_s32_t)pExn[usLoop] + 1), 0, iQValue);
#endif
        /* log10(x) = ln(x)/ln(10) */
        pfFeatures[usLoop] = PSHR((aisp_s64_t)iLogTmpOut * 466320149, 30);
#else
        iLogTmpOut = sevcLog(((aisp_s32_t)pExn[usLoop] + 1), 7, iQValue);
        /* log10(x) = ln(x)/ln(10) */
        pfFeatures[usLoop] = PSHR((aisp_s64_t)iLogTmpOut * 466320149, 30)
                             - pstNn->pfMiu[usLoop];
#endif
#else
        pfFeatures[usLoop] = LOG10((aisp_s32_t)(pExn[usLoop] + 1e-1))
                             - pstNn->pfMiu[usLoop];
#endif
    }
}

aisp_s32_t *sevcNnCmnUpdate(SEVC_NN_S *pstNnEng)
{
    NN_STATE_S *pstNn = pstNnEng->pstNn;
    aisp_s32_t *pfCmnFeature;
    aisp_s32_t **ppCmnBuffer   = pstNn->ppCmnBuffer;
    aisp_s64_t *pCmnFeatureSum = pstNn->pCmnFeatureSum;
    U16 usFeatureDim = pstNnEng->pstSevcCfg->ucNnFeatureDim;
    S32 iCmnWinLen = pstNn->ucCmnLen;
    S32 ii;
    pfCmnFeature = ppCmnBuffer[0];

    for (ii = 0; ii < usFeatureDim; ii++)
    {
        pCmnFeatureSum[ii] -= pfCmnFeature[ii];
    }

    for (ii = 0; ii < iCmnWinLen; ii++)
    {
        ppCmnBuffer[ii] = ppCmnBuffer[ii + 1];
    }

    ppCmnBuffer[iCmnWinLen] = pfCmnFeature;
    return pfCmnFeature;
}

// VOID GetLogFeature(aisp_s32_t *logFeature, U32 length)
// {
//     static FILE *fpLogFeature = NULL;
//     static int cnt = 0;

//     if (fpLogFeature == NULL)
//     {
//         fpLogFeature = fopen("misc/2mics/logfeature.pcm", "rb");
//     }

//     if (fpLogFeature != NULL)
//     {
//         U32 uiSize = fread(logFeature, sizeof(aisp_s32_t), length, fpLogFeature);
//         cnt++;

//         if (uiSize != length || uiSize == 0)
//         {
//             printf("input file ends at %d\t\n", cnt);
//             fclose(fpLogFeature);
//             fpLogFeature = NULL;
//         }
//     }
// }

/************************************************************
  Function   : sevcNnFeaturesCalc()

  Description:
  Calls      :
  Called By  :
  Input      : pFreqFrame Q21
  Output     :
  Return     :
  Others     : pfFeatures iQValue

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
static S32 sevcNnFeaturesCalc(SEVC_NN_S *pstNnEng,
                              aisp_cpx_s32_t *pFreqFrame,
                              aisp_cpx_s32_t *pEstFrame)
{
    NN_STATE_S *pstNn = pstNnEng->pstNn;
    aisp_s64_t *pfNbBandsSum = pstNn->pfNbBandsSum;
    aisp_s32_t *pfFeaturesNew;
    aisp_s32_t *pfFeaturesCalc;
    aisp_s64_t *pCmnFeatureSum = pstNn->pCmnFeatureSum;
    S32 iCmnWinLen = pstNn->ucCmnLen;
    U16 usFeatureDim = pstNnEng->pstSevcCfg->ucNnFeatureDim;
#ifndef NN_FSMN
    U16 usFftBin  = pstNnEng->pstSevcCfg->usFftBin;
    aisp_s32_t *pfFeaturesHistory = pstNn->pfFeatures;
    aisp_s16_t fNoisePow    = 0;
#else
    aisp_s32_t *pfFeatures  = pstNn->pfFeatures;
    aisp_s32_t *pfFeaturesHistory   = pstNn->pfFeaturesHistory;
    S32 nnDataQ = pstNnEng->pstNn->modelInfo.ucDataQ;
#endif
    S32 iSilence = 0;
    U16 usLoop;
    U16 usBinCnt;
    U16 usFeatureContext = pstNn->usFeatureContext;
// #if SEVC_SWITCH_NN_AES
// #if 0
//     sevcResidualEchoCalc(pstNnEng, pstNnEng->pfResidualEcho, pFreqFrame, pEstFrame);
//     sevcBandEnergyCalcReal(pstNnEng, pfNbBandsSum + pstNn->usNbBands, pstNnEng->pfResidualEcho);
// #endif
//     sevcRefEnergyCalc(pstNnEng, pfNbBandsSum + pstNn->usNbBands, pEstFrame);
// #endif
    pfFeaturesNew = sevcNnNewFeatureBuffGet(pstNnEng);
    //cmn update
    pfFeaturesCalc = (pstNn->ucCmnSupport ? sevcNnCmnUpdate(pstNnEng) : pfFeaturesNew);

    if (pstNnEng->pstSevcCfg->ucNnBarkSupport)
    {
        sevcBandEnergyCalc(pstNnEng, pfNbBandsSum, pFreqFrame);
        sevcNnExnLogCalc(pstNnEng, pfFeaturesCalc, pfNbBandsSum);
        // debug
        // GetLogFeature(pfFeaturesCalc, 64);
    }
    else
    {
#ifndef NN_FSMN
        usBinCnt = SEVC_MIN(usFftBin, pstNn->pstModel->pDenseInLayer->iInputs);

        for (usLoop = 0; usLoop < usBinCnt; usLoop++)
        {
            //COMPLEX_POW(fNoisePow, pFreqFrame[usLoop]);
            fNoisePow = (aisp_s64_t)(pFreqFrame[usLoop].real * pFreqFrame[usLoop].real
                                     + pFreqFrame[usLoop].imag *
                                     pFreqFrame[usLoop].imag) / 262144;
            pfFeaturesCalc[usLoop] = LOG10((fNoisePow + 1e-2)) - pstNn->pfMiu[usLoop];
        }

#endif
    }

    if (pstNn->ucCmnSupport)
    {
        iCmnWinLen += 1;

        for (usLoop = 0; usLoop < usFeatureDim; usLoop++)
        {
            pCmnFeatureSum[usLoop] += pfFeaturesCalc[usLoop];
            pfFeaturesNew[usLoop] = pfFeaturesCalc[usLoop] - (pCmnFeatureSum[usLoop] + (iCmnWinLen >> 1)) / iCmnWinLen;
        }
    }

#ifdef NN_FSMN
    usBinCnt = usFeatureDim * usFeatureContext;

    for (usLoop = 0; usLoop < usBinCnt; usLoop++)
    {
        pfFeatures[usLoop] = PSHR((aisp_s64_t)(pfFeaturesHistory[usLoop]
                                               + pstNn->piFeatureMean[usLoop])
                                  * pstNn->piFeatureVar[usLoop],
                                  nnDataQ);
    }

    // fwrite(pstNnEng->pstNn->pfFeatures, sizeof(aisp_s32_t), 64, fpYyPcm);
#endif
    return iSilence;
}

/************************************************************
  Function   : sevcInterpBandGainCalc()

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
VOID sevcInterpBandGainCalc(SEVC_NN_S *pstNnEng, aisp_s32_t *pfGains, aisp_s32_t *pfRnnOut)
{
    U16 usLoop, usLoop2, usIdx;
    U16 usBandsSize;
    U16 usNbBands = pstNnEng->pstNn->usNbBands;
    U16 *psFull2BarkMapping = pstNnEng->pstNn->psFull2BarkMapping;
    aisp_s32_t frac;
#ifdef AISPEECH_FIXED

    for (usLoop = 0; usLoop < usNbBands - 1; usLoop++)
    {
        usBandsSize = psFull2BarkMapping[usLoop + 1] - psFull2BarkMapping[usLoop];

        for (usLoop2 = 0; usLoop2 < usBandsSize; usLoop2++)
        {
            frac  = (((usLoop2) << 24) + (usBandsSize >> 1)) / usBandsSize;
            usIdx = psFull2BarkMapping[usLoop] + usLoop2;
            pfGains[usIdx] = PSHR((aisp_s64_t)(16777216 - frac) * pfRnnOut[usLoop]
                                  + (aisp_s64_t)frac * pfRnnOut[usLoop + 1], 24);
        }
    }

#else

    for (usLoop = 0; usLoop < usNbBands - 1; usLoop++)
    {
        usBandsSize = psFull2BarkMapping[usLoop + 1] - psFull2BarkMapping[usLoop];

        for (usLoop2 = 0; usLoop2 < usBandsSize; usLoop2++)
        {
            frac = (F32)usLoop2 / usBandsSize;
            usIdx = psFull2BarkMapping[usLoop] + usLoop2;
            pfGains[usIdx] = (1 - frac) * pfRnnOut[usLoop] + frac * pfRnnOut[usLoop + 1];
        }
    }

#endif
}

/************************************************************
  Function   : sevcNNGainAdjustment()

  Description:
  Calls      :
  Called By  :
  Input      : pfNNGain Q20
  Output     : pfNNGain Q20
  Return     :
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
VOID sevcNNGainAdjustment(SEVC_NN_S *pstNnEng, aisp_s32_t *pfNNGain, S32 iNNGainLen)
{
    S32 ii;

    for (ii = 0; ii < iNNGainLen; ii++)
    {
        pfNNGain[ii] = PSHR64(MULT32_32(pfNNGain[ii], pfNNGain[ii]), 20);
    }
}

/************************************************************
  Function   : sevcVadCalc()

  Description:
  Calls      :
  Called By  :
  Input      : pfNNGain Q20
  Output     :
  Return     : fVad :Q24
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
aisp_s32_t sevcVadCalc(SEVC_NN_S *pstNnEng, aisp_s32_t *pfNNGain, S32 iNNGainLen)
{
    aisp_s64_t fGainSum = 0;
    aisp_s32_t fVad;
    S32 ii;

    for (ii = 0; ii < iNNGainLen; ii++)
    {
        fGainSum += (aisp_s64_t)pfNNGain[ii] * pfNNGain[ii];
    }

    fGainSum = (fGainSum + (iNNGainLen >> 1)) / iNNGainLen;
    fVad = PSHR((aisp_s64_t)fGainSum, 16); //W32Q24
    return fVad;
}

#if SEVC_SWITCH_CNG
/************************************************************
Function   : sevcAngCalc()

Description:
Calls      :
Called By  :
Input      :
Output     :
Return     :
Others     :

History    :
  2020/04/02, shizhang.tang create
************************************************************/
VOID sevcAngCalc(aisp_s32_t *pfAng, U32 *piSeed, U32 uiLen)
{
    U32 ii;
    pfAng[0] = ((aisp_s64_t)(*piSeed) * 69069 + 1) % (0x7FFFFFFF);

    for (ii = 1; ii < uiLen; ii++)
    {
        pfAng[ii] = ((aisp_s64_t)pfAng[ii - 1] * 69069 + 1) % (0x7FFFFFFF);
    }

    *piSeed = pfAng[uiLen - 1];
}

/************************************************************
Function   : sevcCngCalc()

Description:
Calls      :
Called By  :
Input      : pErrFrame : Q21
Output     :
Return     :
Others     : pCng : Q21

History    :
  2020/04/02, shizhang.tang create
************************************************************/
VOID sevcCngCalc(SEVC_NN_S *pstNnEng, aisp_cpx_s32_t *pErrFrame)
{
    SEVC_CONFIG_S *pstSevcCfg = pstNnEng->pstSevcCfg;
    S32 ii;
    U16 usFftBin = pstSevcCfg->usFftBin;
    aisp_s32_t *pfGains  = pstNnEng->pfGains;
    aisp_s32_t *pfPNoisy = pstNnEng->pfPNoisy;
    aisp_s32_t *pfPMin;
    aisp_s32_t *pfNoisePs = pstNnEng->pfNoisePs;
    aisp_s32_t fMcraAs = pstSevcCfg->mcra_as;
    aisp_s32_t fMcraAs1 = pstSevcCfg->mcra_as1;
    aisp_s32_t fMcraAs2 = pstSevcCfg->mcra_as2;
    aisp_s32_t fMcraAs1Residual = (1 << 24) - fMcraAs1;
    aisp_s32_t fMcraAs2Residual = (1 << 24) - fMcraAs2;
    aisp_s32_t fAng;
    aisp_cpx_s32_t  *pCng = pstNnEng->pCng;
    aisp_s32_t *pfPTmp = pstNnEng->pfDataTmp;

    //P_noisy = mcra_as*P_noisy+(1-mcra_as)*real(X)
    for (ii = 0; ii < usFftBin; ii++)
    {
        pfPTmp[ii] = 1048576 - pfGains[ii]; //W32Q20
    }

    for (ii = 0; ii < usFftBin; ii++)
    {
        pfPTmp[ii] = PSHR64(MULT32_32(pErrFrame[ii].real, pfPTmp[ii]), 20);     //W32Q21
        pfPTmp[ii] = sevcAbsf(pfPTmp[ii]);
    }

    sevcVec32Smooth(pfPNoisy, pfPTmp, fMcraAs, usFftBin, 24);
    pfPMin = pfPNoisy;

    for (ii = 0; ii < usFftBin; ii++)
    {
        if (pfNoisePs[ii] < pfPMin[ii])
        {
            pfNoisePs[ii] = PSHR((MULT32_32(fMcraAs1, pfNoisePs[ii])
                                  + MULT32_32(fMcraAs1Residual, pfPMin[ii])), 24);  ////W32Q24
        }
        else
        {
            pfNoisePs[ii] = PSHR((MULT32_32(fMcraAs2, pfNoisePs[ii])
                                  + MULT32_32(fMcraAs2Residual, pfPMin[ii])), 24);    //W32Q24
        }
    }

    sevcAngCalc(pstNnEng->pfAng, &pstNnEng->uiSeed, usFftBin);

    for (ii = 0; ii < usFftBin; ii++)
    {
        fAng = PSHR64(((aisp_s64_t)pstNnEng->pfAng[ii] * (1 << 24) + (1 << 29)), 30) - (1 << 24); //W32Q24
        //CNG = noise_ps.*ang + (-1)^0.5*noise_ps.*ang;
        pCng[ii].real = MULT32_32_SQ(pfNoisePs[ii], fAng, 24); //W32Q21
        pCng[ii].imag = pCng[ii].real; //W32Q21
    }
}
#endif
VOID sevcDataPop(SEVC_NN_S *pstNnEng, aisp_cpx_s32_t *pFreqFrame)
{
    S32 iIdx;
    SEVC_CONFIG_S *pstSevcCfg = pstNnEng->pstSevcCfg;
    U16 usFftBin = pstSevcCfg->usFftBin;
    U16 usSizeComp = sizeof(aisp_cpx_s32_t);
    aisp_cpx_s32_t *pNnOut = pstNnEng->pNnOut;
#if SEVC_SWITCH_CNG
    aisp_cpx_s32_t *pCng;
#endif

    if (NULL != pstNnEng->pCallBackFunc)
    {
        if (pNnOut != pFreqFrame)
        {
            for (iIdx = 0; iIdx < usFftBin; iIdx++)
            {
                pNnOut[iIdx].real = pFreqFrame[iIdx].real;
                pNnOut[iIdx].imag = pFreqFrame[iIdx].imag;
            }
        }

#if SEVC_SWITCH_CNG
        //
        pCng = pstNnEng->pCng;
        pNnOut = pNnOut + usFftBin;

        for (iIdx = 0; iIdx < usFftBin; iIdx++)
        {
            pNnOut[iIdx].real = pCng[iIdx].real;
            pNnOut[iIdx].imag = pCng[iIdx].imag;
        }

        pNnOut = pstNnEng->pNnOut;
#endif
        pstNnEng->pCallBackFunc((U8 *)pNnOut,
                                usFftBin * usSizeComp, pstNnEng->pUsrData);
    }
}
/************************************************************
  Function   : SEVC_NN_ShMemSizeGet()

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
U32 SEVC_NN_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    //U32 uiMemSize = 0;
#if (SEVC_SWITCH_CNG || SEVC_SWITCH_AES || SEVC_SWITCH_NN_AES || SEVC_SWITCH_NN_BARK)
    U16 usFftBin = pstSevcCfg->usFftBin;
#endif
#if SEVC_SWITCH_CNG
    //pfDataTmp;
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s32_t));
    //pfAng;
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s32_t));
    //pCng;
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_cpx_s32_t));
#endif
#if (SEVC_SWITCH_AES || SEVC_SWITCH_NN_AES)
#if (SEVC_SWITCH_NN_AES)
    //pfResidualEcho
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s32_t));
#endif
#ifdef SEVC_PHONE
    //pfPyyTmp
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s32_t));
    //pfPeeTmp
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s32_t));
    //pPeyTmp
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_cpx_s32_t));
    //pfAbsPeyTmp
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s32_t));
#else
    //pfPyyTmp
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s64_t));
    //pfPeeTmp
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s64_t));
    //pPeyTmp
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_cpx_s64_t));
    //pfAbsPeyTmp
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s64_t));
#endif
    //pfPostGains
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s32_t));
#endif
#if SEVC_SWITCH_NN_BARK
    /* pfGainNew */
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s32_t));
#endif
#ifdef USE_SHARE_MEM
    uiTotalMemSize += nnProcessorSharedMemSizeGet(pstSevcCfg);
#endif
#if MEM_BUFF_DBG
    MSG_DBG("%s [%d] uiTotalMemSize=%d\r\n", __func__, __LINE__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}

/************************************************************
  Function   : SEVC_NN_LocMemSizeGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/02, shizhang.tang create
    2020/01/09, chao.xu modified
************************************************************/
U32 SEVC_NN_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    U16 usSizeComp = sizeof(aisp_cpx_s32_t);
    U16 usFftBin = pstSevcCfg->usFftBin;
    U16 usFeatureContextR = nnProcessorFeatureContextRGet(pstSevcCfg->fs);
    U16 usSize;

    if (NULL == pstSevcCfg)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return 0;
    }

    uiTotalMemSize += SIZE_ALIGN(sizeof(SEVC_NN_S));
#if SEVC_SWITCH_NN_BARK
    /* pfGains */
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s32_t));
#endif
#if SEVC_SWITCH_CNG
    //pfPNoisy;
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s32_t));
    //pfNoisePs;
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s32_t));
#endif
#if (SEVC_SWITCH_AES || SEVC_SWITCH_NN_AES)
#if SEVC_SWITCH_AES
    //ppEst
    usSize = usFeatureContextR * sizeof(VOID *);
    uiTotalMemSize += SIZE_ALIGN(usSize);
    uiTotalMemSize += usFeatureContextR * SIZE_ALIGN(usFftBin * usSizeComp);
#endif
#ifdef SEVC_PHONE
    //pPey
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_cpx_s32_t));
    //pfPyy
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s32_t));
#else
    //pPey
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_cpx_s64_t));
    //pfPyy
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s64_t));
#endif
#endif
    /*ppLastX*/
    uiTotalMemSize += SIZE_ALIGN(usFeatureContextR * sizeof(VOID *));
    uiTotalMemSize += usFeatureContextR * SIZE_ALIGN(usFftBin * usSizeComp);
    usSize = usFftBin * usSizeComp;
#if SEVC_SWITCH_CNG
    /* pNnOut  = freqFrame + CNGframe */
    usSize *= 2;
#endif
    uiTotalMemSize += SIZE_ALIGN(usSize);
#ifndef USE_SHARE_MEM
    uiTotalMemSize += SEVC_NN_ShMemSizeGet(pstSevcCfg);
#endif
    uiTotalMemSize += nnProcessorMemSizeGet(pstSevcCfg);
#if MEM_BUFF_DBG
    MSG_DBG("%s uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}

/************************************************************
  Function   : SEVC_NN_VadGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     : fVad: Q24
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
aisp_s32_t SEVC_NN_VadGet(SEVC_NN_S *pstNnEng)
{
    return pstNnEng->fVad;
}

/************************************************************
  Function   : SEVC_NN_ModelInfoGet()

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
S8 *SEVC_NN_ModelInfoGet(SEVC_NN_S *pstNnEng)
{
    return nnProcessorResourceInfoGet(pstNnEng->pstSevcCfg->fs);
}


/************************************************************
  Function   : SEVC_NN_New()

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
SEVC_NN_S *SEVC_NN_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator)
{
    SEVC_NN_S *pstNnEng = NULL;
    U16 usSize;
    U16 usSizeComp = sizeof(aisp_cpx_s32_t);
    U16 usFftBin = pstSevcCfg->usFftBin;
    S32 ii;

    if (NULL == pstSevcCfg || NULL == pstMemLocator)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
    }

    MEM_LOCATOR_SHOW(pstMemLocator);
    usSize = sizeof(SEVC_NN_S);
    pstNnEng = (SEVC_NN_S *)memLocatorGet(pstMemLocator, usSize, AISP_ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstNnEng, FAIL);
    pstNnEng->pstSevcCfg = pstSevcCfg;
    pstNnEng->uiFrameIdx = 0;
    pstNnEng->uiSeed = 1;
    pstNnEng->pstNn = nnProcessorNew(pstSevcCfg, pstMemLocator);
#if SEVC_SWITCH_CNG
    //pfPNoisy;
    BUFF_POINT_SET_ALIGN(pstNnEng->pfPNoisy, (usFftBin * sizeof(aisp_s32_t)),
                         pstMemLocator, FAIL);
    //pfNoisePs;
    BUFF_POINT_SET_ALIGN(pstNnEng->pfNoisePs, (usFftBin * sizeof(aisp_s32_t)),
                         pstMemLocator, FAIL);
#endif
#if (SEVC_SWITCH_NN_AES)
#ifdef SEVC_PHONE
    usSize = usFftBin * sizeof(aisp_cpx_s32_t);
    //pPey
    BUFF_POINT_SET_ALIGN(pstNnEng->pPey, usSize, pstMemLocator, FAIL);
    //pfPyy
    usSize = usFftBin * sizeof(aisp_s32_t);
    BUFF_POINT_SET_ALIGN(pstNnEng->pfPyy, usSize, pstMemLocator, FAIL);
#else
    usSize = usFftBin * sizeof(aisp_cpx_s64_t);
    //pPey
    BUFF_POINT_SET_ALIGN(pstNnEng->pPey, usSize, pstMemLocator, FAIL);
    //pfPyy
    usSize = usFftBin * sizeof(aisp_s64_t);
    BUFF_POINT_SET_ALIGN(pstNnEng->pfPyy, usSize, pstMemLocator, FAIL);
#endif
    //usSize = usFftBin * sizeof(aisp_s32_t);
#endif
    //ppLastX
    usSize = pstNnEng->pstNn->usFeatureContextR * sizeof(VOID *);
    BUFF_POINT_SET_ALIGN(pstNnEng->ppLastX, usSize, pstMemLocator, FAIL);
    usSize = usFftBin * usSizeComp;

    for (ii = 0; ii < pstNnEng->pstNn->usFeatureContextR; ii++)
    {
        BUFF_POINT_SET_ALIGN(pstNnEng->ppLastX[ii], usSize, pstMemLocator, FAIL);
    }

    usSize = usFftBin * usSizeComp;
#if SEVC_SWITCH_CNG
    //pNnOut = freqFrame + CNGFrame
    usSize *= 2;
#endif
    BUFF_POINT_SET_ALIGN(pstNnEng->pNnOut, usSize, pstMemLocator, FAIL);
    //MEM_LOCATOR_SHOW(pstMemLocator);
    //shared mem
#if !(defined SEVC_SWITCH_AEC || defined SEVC_SWITCH_BF)
    memLocatorShMemReset(pstMemLocator);
#endif
#if SEVC_SWITCH_NN_BARK
    //pfGains
    BUFF_POINT_SET_ALIGN(pstNnEng->pfGains, (usFftBin * sizeof(aisp_s32_t)), pstMemLocator, FAIL);
    //pfGainNew
    SH_BUFF_POINT_SET_ALIGN(pstNnEng->pfGainNew, (usFftBin * sizeof(aisp_s32_t)), pstMemLocator, FAIL);
#endif
#if SEVC_SWITCH_CNG
    //CNG
#if !(defined SEVC_SWITCH_AEC || defined SEVC_SWITCH_BF)
    memLocatorShMemReset(pstMemLocator);
#endif
    //pfDataTmp
    SH_BUFF_POINT_SET_ALIGN(pstNnEng->pfDataTmp, (usFftBin * sizeof(aisp_s32_t)),
                            pstMemLocator, FAIL);
    //pfAng;
    SH_BUFF_POINT_SET_ALIGN(pstNnEng->pfAng, (usFftBin * sizeof(aisp_s32_t)),
                            pstMemLocator, FAIL);
    //pCng;
    SH_BUFF_POINT_SET_ALIGN(pstNnEng->pCng, (usFftBin * usSizeComp),
                            pstMemLocator, FAIL);
#endif
#if (SEVC_SWITCH_NN_AES)
    //memLocatorShMemReset(pstMemLocator);
    //pfResidualEcho
    SH_BUFF_POINT_SET_ALIGN(pstNnEng->pfResidualEcho, usFftBin * sizeof(aisp_s32_t), pstMemLocator, FAIL);
#ifdef SEVC_PHONE
    //pfPyyTmp
    SH_BUFF_POINT_SET_ALIGN(pstNnEng->pfPyyTmp, usFftBin * sizeof(aisp_s32_t), pstMemLocator, FAIL);
    //pfPeeTmp
    SH_BUFF_POINT_SET_ALIGN(pstNnEng->pfPeeTmp, usFftBin * sizeof(aisp_s32_t), pstMemLocator, FAIL);
    //pPeyTmp
    SH_BUFF_POINT_SET_ALIGN(pstNnEng->pPeyTmp, usFftBin * sizeof(aisp_cpx_s32_t), pstMemLocator, FAIL);
    //pfAbsPeyTmp
    SH_BUFF_POINT_SET_ALIGN(pstNnEng->pfAbsPeyTmp, usFftBin * sizeof(aisp_s32_t), pstMemLocator, FAIL);
#else
    //pfPyyTmp
    SH_BUFF_POINT_SET_ALIGN(pstNnEng->pfPyyTmp, usFftBin * sizeof(aisp_s64_t), pstMemLocator, FAIL);
    //pfPeeTmp
    SH_BUFF_POINT_SET_ALIGN(pstNnEng->pfPeeTmp, usFftBin * sizeof(aisp_s64_t), pstMemLocator, FAIL);
    //pPeyTmp
    SH_BUFF_POINT_SET_ALIGN(pstNnEng->pPeyTmp, usFftBin * sizeof(aisp_cpx_s64_t), pstMemLocator, FAIL);
    //pfAbsPeyTmp
    SH_BUFF_POINT_SET_ALIGN(pstNnEng->pfAbsPeyTmp, usFftBin * sizeof(aisp_s64_t), pstMemLocator, FAIL);
#endif
    //pfPostGains
    SH_BUFF_POINT_SET_ALIGN(pstNnEng->pfPostGains, usFftBin * sizeof(aisp_s32_t), pstMemLocator, FAIL);
#endif
    MEM_LOCATOR_SHOW(pstMemLocator);
    return pstNnEng;
FAIL:
    MSG_DBG("%s [%d] fail\r\n", __func__, __LINE__);
    return NULL;
}

VOID SEVC_NN_CallBackFuncRegister(SEVC_NN_S *pstNnEng, VOID *pCBFunc, VOID *pUsrData)
{
    if (NULL == pstNnEng || NULL == pCBFunc)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return ;
    }

    pstNnEng->pCallBackFunc = (CallBackFunc)pCBFunc;
    pstNnEng->pUsrData = pUsrData;
}

/************************************************************
  Function   : SEVC_NN_Feed()

  Description:
  Calls      :
  Called By  :
  Input      : pFreqFrame : Q21
               pEstFrame : Q21
  Output     :
  Return     :
  Others     : pfGains : Q20

  History    :
    2019/12/02, shizhang.tang create
    2020/01/07, chao.xu modify
************************************************************/
S32 SEVC_NN_Feed(SEVC_NN_S *pstNnEng, aisp_cpx_s32_t *pFreqFrame, aisp_cpx_s32_t *pEstFrame,
                 U32 uiWindState, U32 uiQuietState)
{
    /* Engine */
    NN_STATE_S *pstNn = pstNnEng->pstNn;
    S32 iSilence  = 0;
    U16 usLoop = 0;
    U16 usFftBin = pstNnEng->pstSevcCfg->usFftBin;
    U16 usFeatureContextR = pstNnEng->pstNn->usFeatureContextR;
    S32 iNNGainLen;
    aisp_s32_t  *pfNNGain  = NULL;
    aisp_s32_t  *pfGains  = NULL;
    aisp_s32_t  *pfGainNew  = pstNnEng->pfGainNew;
    aisp_cpx_s32_t *pXCru = pstNnEng->ppLastX[0];
    aisp_cpx_s32_t *pOut = pstNnEng->pNnOut;
#ifndef FAR_FIELD_COMMUNICATION
    S32 iQValue = nnProcessorQValueDataGet(pstNn);
    aisp_s64_t fGainFloor = (uiWindState ? pstNnEng->pstSevcCfg->fNnWindGainFloor
                             : pstNnEng->pstSevcCfg->fNnGainFloor);  //W64Q40
    aisp_s64_t fGainFloorTmp = PSHR64(fGainFloor, iQValue);
    aisp_s64_t fGainTmp;
#endif
    iSilence = sevcNnFeaturesCalc(pstNnEng, pFreqFrame, pEstFrame);

    if (0 == usFeatureContextR)
    {
        pXCru = pFreqFrame;
    }

    if (0 == iSilence)
    {
        nnProcessorProc(pstNn, pstNnEng->pstNn->pfFeatures);
        iNNGainLen = nnProcessorOutDataGet(pstNnEng->pstNn, &pfNNGain);
        pstNnEng->fVad = sevcVadCalc(pstNnEng, pfNNGain, iNNGainLen);
#if SEVC_SWITCH_NN_BARK
        pfGains  = pstNnEng->pfGains;

        for (usLoop = 0; usLoop < usFftBin; usLoop++)
        {
            pfGains[usLoop] = 0;
        }

        sevcInterpBandGainCalc(pstNnEng, pfGains, pfNNGain);

        for (usLoop = 0; usLoop < usFftBin; usLoop++)
        {
            pfGains[usLoop] = AISP_TSL_MIN(pfGains[usLoop], AISP_TSL_Q20_ONE);
        }

#else
        pfGains = pfNNGain;
#endif
#ifndef FAR_FIELD_COMMUNICATION

        for (usLoop = 0; usLoop < usFftBin; usLoop++)
        {
            fGainTmp = (aisp_s64_t)pfGains[usLoop] * pfGains[usLoop];
#ifdef AISPEECH_FIXED
            pfGainNew[usLoop] = (fGainTmp < fGainFloor ? fGainFloorTmp : PSHR64(fGainTmp, iQValue));
#else
            pfGainNew[usLoop] = (fGainTmp < fGainFloor ? fGainFloor : fGainTmp);
#endif
        }

        sevcVecComplexMulReal(pstNnEng->pNnOut, pXCru, pfGainNew, usFftBin, iQValue);

        if (uiQuietState == 1 && uiWindState == 0)
        {
            pOut = pFreqFrame;
        }

#if SEVC_SWITCH_CNG
        sevcCngCalc(pstNnEng, pFreqFrame);
#endif
        // fwrite(pOut, sizeof(aisp_cpx_s32_t), 257, fpYyPcm);
        sevcDataPop(pstNnEng, pOut);
#endif
    }
    else
    {
        pstNnEng->fVad = 0;
#if SEVC_SWITCH_CNG
        sevcCngCalc(pstNnEng, pFreqFrame);
#endif
        sevcDataPop(pstNnEng, pXCru);
    }

    //update x fifo
    for (usLoop = 0; usLoop < usFftBin; usLoop++)
    {
        pXCru[usLoop].real = pFreqFrame[usLoop].real;
        pXCru[usLoop].imag = pFreqFrame[usLoop].imag;
    }

    if (usFeatureContextR)
    {
        for (usLoop = 1; usLoop < usFeatureContextR; usLoop++)
        {
            pstNnEng->ppLastX[usLoop - 1] = pstNnEng->ppLastX[usLoop];
        }

        pstNnEng->ppLastX[usFeatureContextR - 1] = pXCru;
    }

    pstNnEng->uiFrameCount++;
    return 0;
}

/************************************************************
  Function   : SEVC_NN_Reset()

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
VOID SEVC_NN_Reset(SEVC_NN_S *pstNnEng)
{
    return;
}

/************************************************************
  Function   : SEVC_NN_Delete()

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
VOID SEVC_NN_Delete(SEVC_NN_S *pstNnEng)
{
    extern void nnStatisticShow(void);
    nnStatisticShow();
    return;
}
#endif

