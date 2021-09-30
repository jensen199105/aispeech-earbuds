#include <stdio.h>
#include "AISP_TSL_base.h"
#include "AISP_TSL_str.h"
#include "AISP_TSL_complex.h"

#include "sevc_config.h"
#include "sevc_types.h"
#include "sevc_util.h"

//#include "AISP_TSL_circular_buffer.h"
#include "sevc_bf.h"

extern FILE *fpYyPcm;

#if SEVC_SWITCH_BF

#if SEVC_BF_MIC_DISTANCE == 35
#include "sevc_bf_resource_phone_2mic_35mm_nb_wq24_20210104.data"
#include "sevc_bf_resource_phone_2mic_35mm_wq24_20210104.data"
#elif SEVC_BF_MIC_DISTANCE == 33
#include "sevc_bf_resource_phone_2mic_33mm_nb_wq24_20210414.data"
#include "sevc_bf_resource_phone_2mic_33mm_wq24_20210414.data"
#elif SEVC_BF_MIC_DISTANCE == 30
#include "sevc_bf_resource_phone_2mic_30mm_nb_wq24_20210414.data"
#include "sevc_bf_resource_phone_2mic_30mm_wq24_20210414.data"
#elif SEVC_BF_MIC_DISTANCE == 29
#include "sevc_bf_resource_phone_2mic_29mm_wq24_20201201.data"
#include "sevc_bf_resource_phone_2mic_29mm_nb_wq24_20201201.data"
#elif SEVC_BF_MIC_DISTANCE == 25
#include "sevc_bf_resource_phone_2mic_25mm_wq24_20201228.data"
#include "sevc_bf_resource_phone_2mic_25mm_nb_wq24_20201228.data"
#elif SEVC_BF_MIC_DISTANCE == 20
#include "sevc_bf_resource_phone_2mic_20mm_wq24_20201124.data"
#include "sevc_bf_resource_phone_2mic_20mm_nb_wq24_20201124.data"
#elif SEVC_BF_MIC_DISTANCE == 18
#include "sevc_bf_resource_phone_2mic_18mm_wq24_20210401.data"
#include "sevc_bf_resource_phone_2mic_18mm_nb_wq24_20210401.data"
#elif SEVC_BF_MIC_DISTANCE == 15
#include "sevc_bf_resource_phone_2mic_15mm_wq24_20210414.data"
#include "sevc_bf_resource_phone_2mic_15mm_nb_wq24_20210414.data"
#else
#error "SEVC_BF_MIC_DISTANCE error"
#endif

#define COMPLEX_MUL_AND_CONJ_MUL_SQ(Z, Y, A, B, Q) \
    do { \
        aisp_s64_t ARBR = MULT32_32((A).real , (B).real); \
        aisp_s64_t AIBI = MULT32_32((A).imag , (B).imag); \
        aisp_s64_t ARBI = MULT32_32((A).real , (B).imag); \
        aisp_s64_t AIBR = MULT32_32((A).imag , (B).real); \
        aisp_s64_t tmpRz = ARBR - AIBI; \
        aisp_s64_t tmpIz = AIBR + ARBI; \
        aisp_s64_t tmpRy = ARBR + AIBI; \
        aisp_s64_t tmpIy = AIBR - ARBI; \
        Z.real = PSHR64(tmpRz, Q); \
        Z.imag = PSHR64(tmpIz, Q); \
        Y.real = PSHR64(tmpRy, Q); \
        Y.imag = PSHR64(tmpIy, Q); \
    } while (0)

VOID sevcBfBeamProcess(SEVC_BF_S *pstBfEng, aisp_cpx_s32_t *pFreqFrame)
{
    S32 ii, jj;
    SEVC_CONFIG_S *pstSevcCfg = pstBfEng->pstSevcCfg;
    U16 usFftBins = pstSevcCfg->usFftBin;
#if SEVC_MIC_CHAN == 2
    U16 usMicChan = pstSevcCfg->micChan;
#else
    U16 usMicChan = 2; // 2mic bf
#endif
    aisp_cpx_s32_t *pW;
    aisp_cpx_s32_t *pOut = pstBfEng->pOut;
    aisp_cpx_s32_t *pFrame = pFreqFrame;

    for (ii = 0; ii < pstSevcCfg->ucBfBeamCount; ii++)
    {
        for (jj = 0; jj < usFftBins; jj++)
        {
            pOut[jj].real  = 0;
            pOut[jj].imag  = 0;
        }

        pOut += usFftBins;
    }

    pOut = pstBfEng->pOut;
#ifdef JIELI_BR28

    for (ii = 0; ii < pstSevcCfg->ucBfBeamCount; ii++)
    {
        pW = pstBfEng->ppW[ii];                                            //W32Q24
        pFrame = pFreqFrame;                                                //W32Q21

        for (jj = 0; jj < usMicChan; jj++)
        {
            sevcVecMLA_Complex(pOut, pW, pFrame, usFftBins, 24);
            pFrame += usFftBins;
            pW += usFftBins;
        }

        pOut += usFftBins;
    }

#else
    aisp_cpx_s32_t *pBeam1Out = pstBfEng->pOut;
    aisp_cpx_s32_t *pBeam2Out = pstBfEng->pOut + usFftBins;
    aisp_cpx_s32_t *pBeam3Out = pBeam2Out + usFftBins;
    aisp_cpx_s32_t cpxData1;
    aisp_cpx_s32_t cpxData2;
    //process beam1 and beam3
    pW = pstBfEng->ppW[0];
    pFrame = pFreqFrame;

    for (jj = 0; jj < usMicChan; jj++)
    {
        for (ii = 0; ii < usFftBins; ii++)
        {
            COMPLEX_MUL_AND_CONJ_MUL_SQ(cpxData1, cpxData2, pFrame[ii], pW[ii], 24);
            COMPLEX_ADD(pBeam1Out[ii], pBeam1Out[ii], cpxData1);
            COMPLEX_ADD(pBeam3Out[ii], pBeam3Out[ii], cpxData2);
        }

        pFrame += usFftBins;
        pW += usFftBins;
    }

    //process beam2
    pW = pstBfEng->ppW[1];                                              //W32Q24
    pFrame = pFreqFrame;                                                //W32Q21

    for (jj = 0; jj < usMicChan; jj++)
    {
        sevcVecMLA_Complex(pBeam2Out, pW, pFrame, usFftBins, 24);
        pFrame += usFftBins;
        pW += usFftBins;
    }

#endif
}
#if 0
#if SEVC_SWITCH_BF_SPR
VOID sevcBfSpr(SEVC_BF_S *pstBfEng)
{
    SEVC_CONFIG_S *pstSevcCfg = pstBfEng->pstSevcCfg;
    aisp_cpx_s32_t *pOut;
    aisp_s32_t *pfPsd = pstBfEng->pfPsd;
    aisp_s32_t *pfPsdSum = pstBfEng->pfPsdSum;
    aisp_s32_t bfGainFloor = pstSevcCfg->fBfGainFloor;
    //aisp_s32_t *fGain = pstBfEng->pfGain;
    aisp_s32_t fGain;
    U16 usFftBins = pstSevcCfg->usFftBin;
    U16 usMicChan = pstSevcCfg->micChan;
    S32 ii, jj;
    pOut = pstBfEng->pOut;

    for (ii = 0; ii < pstSevcCfg->ucBfBeamCount; ii++)
    {
        sevcVecPowY32X32_Complex(pfPsd, pOut, usFftBins, 26);                     //W322Q16
        pOut += usFftBins;
        pfPsd += usFftBins;
    }

    pfPsd = pstBfEng->pfPsd;
    pOut = pstBfEng->pOut;

    if (2 == usMicChan)
    {
        sevcVec32Add32_Real(pfPsdSum, pfPsd, pfPsd + usFftBins, usFftBins, 0);         //W322Q16
        sevcVec32MinRestrain(pfPsdSum, 32, usFftBins);

        for (ii = 0; ii < pstSevcCfg->ucBfBeamCount; ii++)
        {
            for (jj = 0; jj < usFftBins; jj++)
            {
                fGain = (aisp_s32_t)((aisp_s64_t)16777216 * pfPsd[jj] / (pfPsdSum[jj] + 1)); //W32Q24
                fGain = (fGain < bfGainFloor ? bfGainFloor : fGain);
                //fGain = (fGain>16777216 ? 16777216 : fGain);                //W32Q24
                fGain = (fGain > 8388608 ? 16777216 : fGain);                 //W32Q24
                COMPLEX_MUL_FLOAT_SQ(pOut[jj], pOut[jj], fGain, 24);
            }

            pOut += usFftBins;
            pfPsd += usFftBins;
        }
    }
    else
    {
        MSG_DBG("%s [%d] not support +++++\r\n", __func__, __LINE__);
    }
}

#else
VOID sevcBfOutCalc(SEVC_BF_S *pstBfEng)
{
    SEVC_CONFIG_S *pstSevcCfg = pstBfEng->pstSevcCfg;
    aisp_cpx_s32_t *pEst = pstBfEng->pEst;
    aisp_cpx_s32_t *pBeamSpeech = pstBfEng->pOut;
    aisp_cpx_s32_t *pBeamNoise = pstBfEng->pOut + pstSevcCfg->usFftBin;
    aisp_cpx_s32_t **ppX = pstBfEng->ppX;
    aisp_cpx_s32_t **ppWGsc = pstBfEng->ppWGsc;
    aisp_cpx_s32_t *pX;
    aisp_cpx_s32_t *pWGsc;
    U16  usTaps = pstSevcCfg->uiBfTaps;
    S32 ii;
    pX = ppX[0];

    for (ii = 0; ii < usTaps - 1; ii++)
    {
        ppX[ii] = ppX[ii + 1];
    }

    ppX[usTaps - 1] = pX;

    for (ii = 0; ii < pstSevcCfg->usFftBin; ii++)
    {
        pX[ii].real = pBeamNoise[ii].real;
        pX[ii].imag = pBeamNoise[ii].imag;
    }

    //EST = round(sum(X.*W_gsc,2)/2^24);
    pX = ppX[0];
    pWGsc = ppWGsc[0];

    for (ii = 0; ii < pstSevcCfg->usFftBin; ii++)
    {
        COMPLEX_MUL_SQ(pEst[ii], pX[ii], pWGsc[ii], 24);
    }

    for (ii = 1; ii < pstSevcCfg->uiBfTaps; ii++)
    {
        pX = ppX[ii];
        pWGsc = ppWGsc[ii];
        sevcVecMLA_Complex(pEst, pWGsc, pX, pstSevcCfg->usFftBin, 24);
    }

    for (ii = 0; ii < pstSevcCfg->usFftBin; ii++)
    {
        COMPLEX_SUB(pBeamSpeech[ii], pBeamSpeech[ii], pEst[ii]);
    }
}

VOID sevcBfWUpdate(SEVC_BF_S *pstBfEng)
{
    S32 ii, jj;
    SEVC_CONFIG_S *pstSevcCfg = pstBfEng->pstSevcCfg;
    U16 usFftBin = pstSevcCfg->usFftBin;
    U16 usTaps = pstSevcCfg->uiBfTaps;
    aisp_s64_t *pfPowSum = pstBfEng->pfPowSum;
    aisp_s32_t **ppfXPow = pstBfEng->ppfXPow;
    aisp_s32_t *pfXPow;
    aisp_cpx_s32_t *pErrNorm = pstBfEng->pEst;
    aisp_cpx_s32_t **ppX = pstBfEng->ppX;
    aisp_cpx_s32_t **ppWGsc = pstBfEng->ppWGsc;
    aisp_cpx_s32_t *pX = pstBfEng->ppX[usTaps - 1];
    aisp_cpx_s32_t *pWGsc;
    aisp_cpx_s32_t *pErr = pstBfEng->pOut;
    aisp_s32_t *pfLamda = pstBfEng->pfLamda;
    aisp_cpx_s32_t cmpData;
    pfXPow = ppfXPow[0];

    for (ii = 0; ii < pstSevcCfg->usFftBin; ii++)
    {
        pfPowSum[ii] -= pfXPow[ii];
    }

    for (ii = 0; ii < usTaps - 1; ii++)
    {
        ppfXPow[ii] = ppfXPow[ii + 1];
    }

    ppfXPow[usTaps - 1] = pfXPow;

    for (ii = 0; ii < usFftBin; ii++)
    {
        COMPLEX_POW_SQ(pfXPow[ii], pX[ii], 26);             //W32Q16
    }

    for (ii = 0; ii < pstSevcCfg->usFftBin; ii++)
    {
        pfPowSum[ii] += pfXPow[ii];
    }

    for (ii = 0; ii < pstSevcCfg->usFftBin; ii++)
    {
        pErrNorm[ii].real = (aisp_s64_t)65536 * pErr[ii].real / pfPowSum[ii]; //W32Q21
        pErrNorm[ii].imag = (aisp_s64_t)65536 * pErr[ii].imag / pfPowSum[ii]; //W32Q21
    }

    for (ii = 0; ii < usTaps; ii++)
    {
        pX = ppX[ii];
        pWGsc = ppWGsc[ii];

        for (jj = 0; jj < usFftBin; jj++)
        {
            COMPLEX_CONJ_MUL_SQ(cmpData, pErrNorm[jj], pX[jj], 21);             //W32Q21
            COMPLEX_MUL_FLOAT_SQ(cmpData, cmpData, pfLamda[jj], 21);            //W32Q24
            COMPLEX_ADD(pWGsc[jj], pWGsc[jj], cmpData);                         //W32Q24
        }
    }
}

VOID sevcBfProcess(SEVC_BF_S *pstBfEng, aisp_cpx_s32_t *pFreqFrame)
{
    SEVC_CONFIG_S *pstSevcCfg = pstBfEng->pstSevcCfg;
    U16 usFftBin = pstSevcCfg->usFftBin;
    aisp_cpx_s32_t *pBeamSpeech = pstBfEng->pOut;
    aisp_cpx_s32_t *pBeamNoise = pstBfEng->pOut + usFftBin;
    aisp_cpx_s32_t *pMic0 = pFreqFrame;
    aisp_cpx_s32_t *pMic1 = pFreqFrame + usFftBin;
    aisp_s32_t *pfSigPowMean = (aisp_s32_t *)pstBfEng->pEst;
    aisp_s32_t *pfLamda = pstBfEng->pfLamda;
    aisp_s32_t fTrigTmp = 2097152 - pstSevcCfg->fBfRatioTrig1;
    aisp_s32_t fMuRatio2 = pstBfEng->fMuRatio2;
    aisp_s32_t *pfRatio = pstBfEng->pfRatio;
    aisp_s32_t *pfSpeechPow = pstBfEng->pfLamda;
    aisp_s32_t fRatioTmp;
    aisp_s32_t fSpeechPow, fNoisePow;
    S32 ii;

    for (ii = 0; ii < usFftBin; ii++)
    {
        pfSigPowMean[ii] = (SEVC_COMPLEX_POW_SQ(pMic0[ii], 28)
                            + SEVC_COMPLEX_POW_SQ(pMic1[ii], 28)) / 2 + 16;    //W32Q14
    }

    for (ii = 0; ii < usFftBin; ii++)
    {
        fSpeechPow = SEVC_COMPLEX_POW_SQ(pBeamSpeech[ii], 28) + 1;             //W32Q14
        fRatioTmp = (fSpeechPow >= pfSigPowMean[ii] ? 2097152
                     : MULT32_32(fSpeechPow, 2097152) / pfSigPowMean[ii]);  //W32Q21
        pfRatio[ii] = (fRatioTmp > fTrigTmp ? 0 : 2097152 - fRatioTmp);
        pfSpeechPow[ii] = fSpeechPow;
    }

    for (ii = 0; ii < 16; ii++)
    {
        fSpeechPow = pfSpeechPow[ii];
        fNoisePow = SEVC_COMPLEX_POW_SQ(pBeamNoise[ii], 28) + 1;               //W32Q14
        fRatioTmp = (fSpeechPow > fNoisePow ? 2097152
                     : MULT32_32(2097152, fSpeechPow) / fNoisePow);         //W32Q21
        pfLamda[ii] = (fRatioTmp < pstSevcCfg->fBfRatioTrig2 ?
                       MULT32_32_SQ(pfRatio[ii], pstSevcCfg->fBfMu, 21)
                       : MULT32_32_SQ(pfRatio[ii], fMuRatio2, 21));           //W32Q24
    }

    for (ii = 16; ii < 128; ii++)
    {
        fSpeechPow = pfSpeechPow[ii];
        fNoisePow = SEVC_COMPLEX_POW_SQ(pBeamNoise[ii], 28) + 1;               //W32Q14
        fRatioTmp = (fSpeechPow > fNoisePow ? 2097152
                     : MULT32_32(2097152, fSpeechPow) / fNoisePow);         //W32Q21
        pfLamda[ii] = (fRatioTmp < pstSevcCfg->fBfRatioTrig3 ?
                       MULT32_32_SQ(pfRatio[ii], pstSevcCfg->fBfMu, 21)
                       : MULT32_32_SQ(pfRatio[ii], fMuRatio2, 21));           //W32Q24
    }

    for (ii = 128; ii < usFftBin; ii++)
    {
        //0.1*ratio1*mu
        pfLamda[ii] = MULT32_32_SQ(pfRatio[ii], fMuRatio2, 21);                //W32Q24
    }

    sevcBfOutCalc(pstBfEng);
    sevcBfWUpdate(pstBfEng);
}
#endif
#endif
/************************************************************
  Function   : SEVC_BF_ShMemSizeGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/02, chunsheng.he create
************************************************************/
U32 SEVC_BF_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
#if 0
    U32 uiSize = 0;
#if SEVC_SWITCH_BF_SPR
    //pfPsd
    uiSize = pstSevcCfg->ucBfBeamCount * pstSevcCfg->usFftBin * sizeof(aisp_s32_t);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    //pfPsdSum
    uiSize = pstSevcCfg->usFftBin * sizeof(aisp_s32_t);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    //pfGain
    //uiSize = pstSevcCfg->usFftBin*sizeof(aisp_s32_t);
    //uiTotalMemSize += SIZE_ALIGN(uiSize);
#else
    //pfRatio
    uiSize = pstSevcCfg->usFftBin * sizeof(aisp_s32_t);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    //pfLamda
    uiSize = pstSevcCfg->usFftBin * sizeof(aisp_s32_t);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    //pEst
    uiSize = pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
#endif
#endif
    //MSG_DBG("%s [%d] uiTotalMemSize=%d\r\n", __func__, __LINE__, uiTotalMemSize);
    return uiTotalMemSize;
}

/************************************************************
  Function   : SEVC_BF_LocMemSizeGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/02, chunsheng.he create
************************************************************/
U32 SEVC_BF_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    U32 uiSize;
    //pstBfEng
    uiSize = sizeof(SEVC_BF_S);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    uiSize = pstSevcCfg->ucBfBeamCount * (sizeof(VOID *));
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    //pOut
    uiSize = pstSevcCfg->ucBfBeamCount * pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
#if 0
#if SEVC_SWITCH_BF_SPR == 0
    //ppX
    uiSize = pstSevcCfg->uiBfTaps * sizeof(VOID *);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    uiSize = pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t);
    uiTotalMemSize += pstSevcCfg->uiBfTaps * SIZE_ALIGN(uiSize);
    //ppfXPow
    uiSize = pstSevcCfg->uiBfTaps * sizeof(VOID *);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    uiSize = pstSevcCfg->usFftBin * sizeof(aisp_s32_t);
    uiTotalMemSize += pstSevcCfg->uiBfTaps * SIZE_ALIGN(uiSize);
    //pfPowSum
    uiSize = pstSevcCfg->usFftBin * sizeof(aisp_s64_t);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    //ppWGsc
    uiSize = pstSevcCfg->uiBfTaps * sizeof(VOID *);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    uiSize = pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t);
    uiTotalMemSize += pstSevcCfg->uiBfTaps * SIZE_ALIGN(uiSize);
#endif
#endif
#ifndef USE_SHARE_MEM
    uiTotalMemSize += SEVC_BF_ShMemSizeGet(pstSevcCfg);
#endif
#if MEM_BUFF_DBG
    MSG_DBG("%s uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}

/************************************************************
  Function   : SEVC_BF_New()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/02, chunsheng.he create
************************************************************/
SEVC_BF_S *SEVC_BF_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator)
{
    SEVC_BF_S *pstBfEng = NULL;
    U16 usSize;
#if SEVC_MIC_CHAN == 2
    U16 usMicChan = pstSevcCfg->micChan;
#else
    U16 usMicChan = 2;
#endif
    S32 ii;
    aisp_cpx_s32_t *pBfBeam;

    if (pstSevcCfg->fs == 8000)
    {
        //MSG_DBG("BF resource is %s\r\n", g_pcNbResource);
        pBfBeam = (aisp_cpx_s32_t *)g_iNbBfBeam;
    }
    else
    {
        //MSG_DBG("BF resource is %s\r\n", g_pcResource);
        pBfBeam = (aisp_cpx_s32_t *)g_iBfBeam;
    }

    MEM_LOCATOR_SHOW(pstMemLocator);
    usSize = sizeof(SEVC_BF_S);
    pstBfEng = (SEVC_BF_S *)memLocatorGet(pstMemLocator, usSize, AISP_ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstBfEng, FAIL);
    pstBfEng->pstSevcCfg = pstSevcCfg;
    BUFF_POINT_SET_ALIGN(pstBfEng->ppW, (pstSevcCfg->ucBfBeamCount * sizeof(VOID *)), pstMemLocator, FAIL);
    pstBfEng->ppW[0] = pBfBeam;

    for (ii = 1; ii < pstSevcCfg->ucBfBeamCount; ii++)
    {
        pstBfEng->ppW[ii] = pBfBeam + ii * usMicChan * pstSevcCfg->usFftBin;
    }

    //pOut
    usSize = pstSevcCfg->ucBfBeamCount * pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t);
    BUFF_POINT_SET_ALIGN(pstBfEng->pOut, usSize, pstMemLocator, FAIL);
#if 0
#if SEVC_SWITCH_BF_SPR == 0
    pstBfEng->fMuRatio2 = MULT32_32_SQ(209715, pstSevcCfg->fBfMu, 21);                    //W32Q24
    //ppX
    usSize = pstSevcCfg->uiBfTaps * sizeof(VOID *);
    BUFF_POINT_SET_ALIGN(pstBfEng->ppX, usSize, pstMemLocator, FAIL);
    usSize = pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t);

    for (ii = 0; ii < pstSevcCfg->uiBfTaps; ii++)
    {
        BUFF_POINT_SET_ALIGN(pstBfEng->ppX[ii], usSize, pstMemLocator, FAIL);
    }

    //ppfXPow
    usSize = pstSevcCfg->uiBfTaps * sizeof(VOID *);
    BUFF_POINT_SET_ALIGN(pstBfEng->ppfXPow, usSize, pstMemLocator, FAIL);
    usSize = pstSevcCfg->usFftBin * sizeof(aisp_s32_t);

    for (ii = 0; ii < pstSevcCfg->uiBfTaps; ii++)
    {
        BUFF_POINT_SET_ALIGN(pstBfEng->ppfXPow[ii], usSize, pstMemLocator, FAIL);
    }

    //pfPowSum
    usSize = pstSevcCfg->usFftBin * sizeof(aisp_s64_t);
    BUFF_POINT_SET_ALIGN(pstBfEng->pfPowSum, usSize, pstMemLocator, FAIL);

    for (ii = 0; ii < pstSevcCfg->usFftBin; ii++)
    {
        pstBfEng->pfPowSum[ii] = pstSevcCfg->fBfDelta;
    }

    //ppWGsc
    usSize = pstSevcCfg->uiBfTaps * sizeof(VOID *);
    BUFF_POINT_SET_ALIGN(pstBfEng->ppWGsc, usSize, pstMemLocator, FAIL);
    usSize = pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t);

    for (ii = 0; ii < pstSevcCfg->uiBfTaps; ii++)
    {
        BUFF_POINT_SET_ALIGN(pstBfEng->ppWGsc[ii], usSize, pstMemLocator, FAIL);
    }

#endif
    //shared mem
#if SEVC_SWITCH_BF_SPR
    //pfPsdSum
    usSize = pstSevcCfg->usFftBin * sizeof(aisp_s32_t);
    SH_BUFF_POINT_SET_ALIGN(pstBfEng->pfPsdSum, usSize, pstMemLocator, FAIL);
    //pfPsd
    usSize = pstSevcCfg->ucBfBeamCount * pstSevcCfg->usFftBin * sizeof(aisp_s32_t);
    SH_BUFF_POINT_SET_ALIGN(pstBfEng->pfPsd, usSize, pstMemLocator, FAIL);
#else
    //pfRatio
    usSize = pstSevcCfg->usFftBin * sizeof(aisp_s32_t);
    SH_BUFF_POINT_SET_ALIGN(pstBfEng->pfRatio, usSize, pstMemLocator, FAIL);
    //pfLamda
    usSize = pstSevcCfg->usFftBin * sizeof(aisp_s32_t);
    SH_BUFF_POINT_SET_ALIGN(pstBfEng->pfLamda, usSize, pstMemLocator, FAIL);
    //pEst
    usSize = pstSevcCfg->usFftBin * sizeof(aisp_cpx_s32_t);
    SH_BUFF_POINT_SET_ALIGN(pstBfEng->pEst, usSize, pstMemLocator, FAIL);
#endif
#endif
    MEM_LOCATOR_SHOW(pstMemLocator);
    return pstBfEng;
FAIL:
    MSG_DBG("%s [%d] fail\r\n", __func__, __LINE__);
    return NULL;
}

VOID SEVC_BF_CallBackFuncRegister(SEVC_BF_S *pstBfEng, VOID *pCBFunc, VOID *pUsrData)
{
    if (NULL == pstBfEng || NULL == pCBFunc)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return ;
    }

    pstBfEng->pCallBackFunc = (CallBackFunc)pCBFunc;
    pstBfEng->pUsrData = pUsrData;
}

/************************************************************
  Function   : SEVC_BF_Feed()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/02, chunsheng.he create
************************************************************/
S32 SEVC_BF_Feed(SEVC_BF_S *pstBfEng, aisp_cpx_s32_t *pFreqFrame)
{
    SEVC_CONFIG_S *pstSevcCfg = pstBfEng->pstSevcCfg;
    U16 usFftBins = pstSevcCfg->usFftBin;
    U32 uiSize;
    sevcBfBeamProcess(pstBfEng, pFreqFrame);

    if (NULL != pstBfEng->pCallBackFunc)
    {
        uiSize = pstSevcCfg->ucBfBeamCount * usFftBins * sizeof(aisp_cpx_s32_t);
        pstBfEng->pCallBackFunc((U8 *)pstBfEng->pOut, uiSize, pstBfEng->pUsrData);
    }

    pstBfEng->uiFrameCount++;
    return 0;
}


/************************************************************
  Function   : SEVC_BF_ResourceInfoGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/02, chunsheng.he create
************************************************************/
S8 *SEVC_BF_ResourceInfoGet(SEVC_BF_S *pstBfEng)
{
    return (S8 *)(pstBfEng->pstSevcCfg->fs == 8000 ? g_pcNbResource : g_pcResource);
}

/************************************************************
  Function   : SEVC_BF_Reset()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/02, chunsheng.he create
************************************************************/
VOID SEVC_BF_Reset(SEVC_BF_S *pstBfEng)
{
}

/************************************************************
  Function   : SEVC_BF_Delete()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/02, chunsheng.he create
************************************************************/
VOID SEVC_BF_Delete(SEVC_BF_S *pstBfEng)
{
}

#endif

