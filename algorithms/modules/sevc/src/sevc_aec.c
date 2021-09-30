#include<stdio.h>
#include "AISP_TSL_base.h"
#include "AISP_TSL_str.h"
#include "AISP_TSL_common.h"
#include "AISP_TSL_complex.h"
#include "fft_wrapper.h"
#include "sevc_config.h"
#include "sevc_types.h"
#include "sevc_util.h"
#include "sevc_aec.h"

extern FILE *fpYyPcm;
#define TALK_MIC_AEC_OFF 1
#define AEC_NARROWBAND   1

#define AEC_SCALOR      1
#if SEVC_SWITCH_AEC
static inline VOID sevcAecFrameNorm(SEVC_CONFIG_S *pstSevcCfg, aisp_s32_t *pfFramePreemph,
                                    aisp_s16_t *pfFrame, aisp_s32_t *pfDataX)
{
    U16 usLoop;
    U16 usFrameShift = pstSevcCfg->frameShift;
#if SEVC_SWITCH_AEC_PREEMPH
    aisp_s16_t fAecPreemph = pstSevcCfg->fAecPreemph;
    aisp_s32_t fDataXTmp = 0;
    aisp_s16_t fNormliz;
    fDataXTmp = *pfDataX;

    for (usLoop = 0; usLoop < usFrameShift; ++usLoop)
    {
        fNormliz = pfFrame[usLoop];
        pfFramePreemph[usLoop] = fNormliz - MULT16_32_P15(fAecPreemph, fDataXTmp);
        fDataXTmp = fNormliz;
    }

    *pfDataX = fDataXTmp;
#else

    for (usLoop = 0; usLoop < usFrameShift; ++usLoop)
    {
        /*normalize the original audio data*/
        pfFramePreemph[usLoop] = pfFrame[usLoop];
    }

#endif
}

/************************************************************
  Function   : sevcSevcPreProcess()

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
static inline VOID sevcAecInstancePreProcess(SEVC_AEC_INSTANCE_S *pstAecInstance,
        aisp_s16_t *pfMicFrame)
{
    U16 usFrameShift = pstAecInstance->pstSevcCfg->frameShift;
    aisp_s32_t fDataX = pstAecInstance->fDataX;
    aisp_s32_t *pfFramePreemph = pstAecInstance->pfMicFramePreemph;
    sevcAecFrameNorm(pstAecInstance->pstSevcCfg,
                     pfFramePreemph + usFrameShift,
                     pfMicFrame, &fDataX);
    pstAecInstance->fDataX = fDataX;
}

/************************************************************
  Function   : sevcSevcErrCalc()

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
static inline VOID sevcAecErrCalc(SEVC_AEC_S *pstAecEng, aisp_cpx_s32_t *pstMicFrame, U16 usMicIdx)
{
    SEVC_CONFIG_S *pstSevcCfg = pstAecEng->pstSevcCfg;
    SEVC_AEC_INSTANCE_S *pstAecInstance = pstAecEng->ppstAecInstance[usMicIdx];
    S16 sLoop2;
    U16 usAecTaps = pstSevcCfg->usAecTaps;
    U16 usRefNum = pstSevcCfg->refChan;
    U16 usFftBin = pstSevcCfg->usFftBin;
    U16 usRefIdx;
    // aisp_cpx_s32_t *pEstBin = pstAecInstance->pOutEst;
    aisp_cpx_s32_t ***pppRefX, **ppRefX;
    aisp_cpx_s32_t *pRefXRow;
    //weight
    aisp_cpx_s32_t **ppWF;
    aisp_cpx_s32_t *pWRow;
    aisp_cpx_s32_t *pErrBin;
    //EST = sum(X.*W,2);
    pppRefX = pstAecEng->pppRefX;
    // error buffer
    pErrBin = pstAecEng->pErrOut + usMicIdx * usFftBin;
#if TALK_MIC_AEC_OFF

    if (usMicIdx != 0)
    {
#endif
        //clean est
        AISP_TSL_memset(pErrBin, 0, usFftBin * sizeof(aisp_cpx_s32_t));

        for (sLoop2 = 0; sLoop2 < usAecTaps; sLoop2++)
        {
            ppWF = pstAecInstance->pppWFDomain[sLoop2];
            ppRefX = pppRefX[sLoop2];

            for (usRefIdx = 0; usRefIdx < usRefNum; usRefIdx++)
            {
                pWRow = ppWF[usRefIdx];
                pRefXRow = ppRefX[usRefIdx];
#if AEC_NARROWBAND == 1

                if (usMicIdx == 2)
                {
                    sevcVecMLA_Complex(pErrBin, pWRow, pRefXRow, 128, 24); //W32Q21
                }
                else
#endif
                {
                    sevcVecMLA_Complex(pErrBin, pWRow, pRefXRow, usFftBin, 24); //W32Q21
                }
            }
        }

        sevcVecSub_Complex(pErrBin, pstMicFrame, pErrBin, usFftBin, 0);
#if TALK_MIC_AEC_OFF
    }
    else
    {
        pErrBin = pstAecEng->pErrOut + usMicIdx * usFftBin;

        for (sLoop2 = 0; sLoop2 < usFftBin; sLoop2++)
        {
            pErrBin[sLoop2].real = pstMicFrame[sLoop2].real;
            pErrBin[sLoop2].imag = pstMicFrame[sLoop2].imag;
        }
    }

#endif
}

/************************************************************
  Function   : sevcAecWeightUpdate()

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
static inline VOID sevcAecWeightUpdate(SEVC_AEC_S *pstAecEng, U16 usMicIdx)
{
    U16 usTapIdx, usRefIdx, usBinIdx;
    SEVC_CONFIG_S *pstSevcCfg = pstAecEng->pstSevcCfg;
    U16 usAecTaps = pstSevcCfg->usAecTaps;
    U16 usRefNum = pstSevcCfg->refChan;
    U16 usFftBin = pstSevcCfg->usFftBin;
    aisp_s32_t fMiu = pstSevcCfg->fAecMu;
    SEVC_AEC_INSTANCE_S *pstAecInstance = pstAecEng->ppstAecInstance[usMicIdx];
    aisp_cpx_s32_t **ppWF, *pWRow;
    aisp_cpx_s32_t *pErrNorm = pstAecEng->pErrNorm;
    aisp_cpx_s32_t **ppRefX, *pRefXRow;
    aisp_s64_t *pfDataTmp = pstAecEng->pfDataTmp;
    aisp_cpx_s32_t *pErrBin = pstAecEng->pErrOut + usMicIdx * usFftBin;
    aisp_s32_t fAecDelta = pstAecEng->pstSevcCfg->fAecDelta;
    aisp_s32_t *pfRefPowerSum = pstAecEng->pfRefPowerSum;
    aisp_s32_t *pfDiag = pstAecEng->pfDiag;
    aisp_s64_t *pfErrPower = pstAecEng->pfErrPower + usMicIdx * usFftBin;
    aisp_s64_t errPowerTmp;
#if TALK_MIC_AEC_OFF

    if (0 == usMicIdx)
    {
        return;
    }

#endif

    // add error power to normalization factor
    if (pstAecEng->refVad)
    {
#if AEC_NARROWBAND == 1

        if (usMicIdx == 2)
        {
            for (usBinIdx = 0; usBinIdx < 128; usBinIdx++)
            {
                *pfDataTmp++ = (*pfErrPower++) + (*pfRefPowerSum++) + (*pfDiag++) + fAecDelta;
            }
        }
        else
#endif
        {
            for (usBinIdx = 0; usBinIdx < usFftBin; usBinIdx++)
            {
                *pfDataTmp++ = (*pfErrPower++) + (*pfRefPowerSum++) + (*pfDiag++) + fAecDelta;
            }
        }

        pfDataTmp = pstAecEng->pfDataTmp;
        pErrBin = pstAecEng->pErrOut + usMicIdx * usFftBin;
        pfErrPower = pstAecEng->pfErrPower + usMicIdx * usFftBin;
#if AEC_NARROWBAND == 1

        if (usMicIdx == 2)
        {
            for (usBinIdx = 0; usBinIdx < 128; usBinIdx++)
            {
#ifdef FREQ_BIN_Q21
                pErrNorm->real = PSHR((MULT32_32(pErrBin->real, fMiu)) / pfDataTmp[usBinIdx], 5); //W32Q18
                pErrNorm->imag = PSHR((MULT32_32(pErrBin->imag, fMiu)) / pfDataTmp[usBinIdx], 5); //W32Q18
#else
                pErrNorm->real = ((aisp_s64_t)SHL(MULT32_32(pErrBin->real, fMiu), 6)) / pfDataTmp[usBinIdx]; //W32Q23
                pErrNorm->imag = ((aisp_s64_t)SHL(MULT32_32(pErrBin->imag, fMiu), 6)) / pfDataTmp[usBinIdx]; //W32Q23
#endif
                pErrBin++;
                pErrNorm++;
            }
        }
        else
#endif
        {
            for (usBinIdx = 0; usBinIdx < usFftBin; usBinIdx++)
            {
#ifdef FREQ_BIN_Q21
                pErrNorm->real = PSHR((MULT32_32(pErrBin->real, fMiu)) / pfDataTmp[usBinIdx], 5); //W32Q18
                pErrNorm->imag = PSHR((MULT32_32(pErrBin->imag, fMiu)) / pfDataTmp[usBinIdx], 5); //W32Q18
#else
                pErrNorm->real = ((aisp_s64_t)SHL(MULT32_32(pErrBin->real, fMiu), 6)) / pfDataTmp[usBinIdx]; //W32Q23
                pErrNorm->imag = ((aisp_s64_t)SHL(MULT32_32(pErrBin->imag, fMiu), 6)) / pfDataTmp[usBinIdx]; //W32Q23
#endif
                pErrBin++;
                pErrNorm++;
            }
        }

        pErrNorm = pstAecEng->pErrNorm;

        for (usTapIdx = 0; usTapIdx < usAecTaps; usTapIdx++)
        {
            ppWF = pstAecInstance->pppWFDomain[usTapIdx];
            ppRefX = pstAecEng->pppRefX[usTapIdx];

            for (usRefIdx = 0; usRefIdx < usRefNum; usRefIdx++)
            {
                pWRow = ppWF[usRefIdx];
                pRefXRow = ppRefX[usRefIdx];
#if AEC_NARROWBAND == 1

                if (usMicIdx == 2)
                {
#ifdef FREQ_BIN_Q21
                    sevcVecConjMLA_Complex(pWRow, pErrNorm, pRefXRow, 128, 15); //W32Q24
#else
                    sevcVecConjMLA_Complex(pWRow, pErrNorm, pRefXRow, 128, 14); //W32Q24
#endif
                }
                else
#endif
                {
#ifdef FREQ_BIN_Q21
                    sevcVecConjMLA_Complex(pWRow, pErrNorm, pRefXRow, usFftBin, 15); //W32Q24
#else
                    sevcVecConjMLA_Complex(pWRow, pErrNorm, pRefXRow, usFftBin, 14); //W32Q24
#endif
                }

#if AEC_NARROWBAND == 1

                if (usMicIdx == 2)
                {
                    for (usBinIdx = 0; usBinIdx < 128; usBinIdx++)
                    {
                        if (AISP_TSL_ABS(pWRow[usBinIdx].real) > 335544320 || AISP_TSL_ABS(pWRow[usBinIdx].imag) > 335544320)
                        {
                            //335544320 = 20*2^24
                            pWRow[usBinIdx].real = 0;
                            pWRow[usBinIdx].imag = 0;
                        }
                    }
                }
                else
#endif
                {
                    for (usBinIdx = 0; usBinIdx < usFftBin; usBinIdx++)
                    {
                        if (AISP_TSL_ABS(pWRow[usBinIdx].real) > 335544320 || AISP_TSL_ABS(pWRow[usBinIdx].imag) > 335544320)
                        {
                            //335544320 = 20*2^24
                            pWRow[usBinIdx].real = 0;
                            pWRow[usBinIdx].imag = 0;
                        }
                    }
                }
            }
        }
    }

    // load errPower for next frame
    pErrBin = pstAecEng->pErrOut + usMicIdx * usFftBin;
    pfErrPower = pstAecEng->pfErrPower + usMicIdx * usFftBin;
#if AEC_NARROWBAND == 1

    if (usMicIdx == 2)
    {
        for (usBinIdx = 0; usBinIdx < 128; ++usBinIdx)
        {
            errPowerTmp = (aisp_s64_t)pErrBin->real * pErrBin->real + (aisp_s64_t)pErrBin->imag * pErrBin->imag;
            *pfErrPower = PSHR(errPowerTmp, 26); // Q16
            pfErrPower++;
            pErrBin++;
        }
    }
    else
#endif
    {
        for (usBinIdx = 0; usBinIdx < usFftBin; ++usBinIdx)
        {
            errPowerTmp = (aisp_s64_t)pErrBin->real * pErrBin->real + (aisp_s64_t)pErrBin->imag * pErrBin->imag;
            *pfErrPower = PSHR(errPowerTmp, 26); // Q16
            pfErrPower++;
            pErrBin++;
        }
    }
}

static inline VOID sevcAecErrUpdate(SEVC_AEC_S *pstAecEng, U16 usMicIdx)
{
    SEVC_CONFIG_S *pstSevcCfg = pstAecEng->pstSevcCfg;
    SEVC_AEC_INSTANCE_S *pstAecInstance = pstAecEng->ppstAecInstance[usMicIdx];
    aisp_cpx_s32_t *pErrBin = pstAecInstance->pErr;
    aisp_cpx_s32_t *pErrNorm = pstAecEng->pErrNorm;
    aisp_s32_t *pfRefPowerSum = pstAecEng->pfRefPowerSum;
    S32 ii;
    U16 usFftBin = pstSevcCfg->usFftBin;

    for (ii = 0; ii < usFftBin; ii++)
    {
        pErrBin[ii].real -= MULT32_32_SQ(pErrNorm[ii].real, pfRefPowerSum[ii], 18);
        pErrBin[ii].imag -= MULT32_32_SQ(pErrNorm[ii].imag, pfRefPowerSum[ii], 18);
    }
}

/************************************************************
  Function   : sevcAecErrPop()

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
static inline VOID sevcAecErrPop(SEVC_AEC_S *pstAecEng, aisp_cpx_s32_t *micFrame)
{
    U16 usMicNum = pstAecEng->pstSevcCfg->micChan;
    U16 usFftBin = pstAecEng->pstSevcCfg->usFftBin;
    aisp_cpx_s32_t *pErr;
    aisp_cpx_s32_t *pMic;
    U16 usSizeComp = sizeof(aisp_cpx_s32_t);
    S32 iMicIdx = 0, iIdx;
    aisp_cpx_s32_t *pErrOut = pstAecEng->pErrOut;
    U32 uiSize = 0;
    aisp_s32_t aecGain = pstAecEng->pstSevcCfg->aecGain;
    aisp_s64_t dataTmp;

    if (NULL != pstAecEng->pCallBackFunc)
    {
        for (iMicIdx = 0; iMicIdx < usMicNum; iMicIdx++)
        {
            if (aecGain != 32768)
            {
                for (iIdx = 0; iIdx < usFftBin; iIdx++)
                {
                    // COMPLEX_MUL_FLOAT(*pErrOut, *pErrOut, aecGain);
                    dataTmp = (aisp_s64_t)pErrOut->real * aecGain;
                    pErrOut->real = PSHR(dataTmp, 15); // Gain Q15
                    dataTmp = (aisp_s64_t)pErrOut->imag * aecGain;
                    pErrOut->imag = PSHR(dataTmp, 15); // Gain Q15
                    pErrOut++;
                }
            }
        }

        // place the first and last echo into buffer
        uiSize = usMicNum * usFftBin * usSizeComp;
        pErrOut = pstAecEng->pErrOut + usMicNum * usFftBin;
        // first error buffer
        pErr = pstAecEng->pErrOut;
        // first mic buffer
        pMic = micFrame;
        iMicIdx = 0;
        {
            // pErrBin = pstAecEng->ppstAecInstance[iMicIdx]->pOutEst;
            for (iIdx = 0; iIdx < usFftBin; iIdx++)
            {
                pErrOut->real = pMic->real - pErr->real;
                pErrOut->imag = pMic->imag - pErr->imag;
                pErrOut++;
                pMic++;
                pErr++;
            }
        }
        uiSize += usFftBin * usSizeComp;
#if SEVC_MIC_CHAN == 3
        // third mic buffer
        pMic = micFrame + (usMicNum - 1) * usFftBin;
        // third error buffer
        pErr = pstAecEng->pErrOut + (usMicNum - 1) * usFftBin;

        for (iIdx = 0; iIdx < usFftBin; iIdx++)
        {
            pErrOut->real = pMic->real - pErr->real;
            pErrOut->imag = pMic->imag - pErr->imag;
            pErrOut++;
            pMic++;
            pErr++;
        }

        uiSize += usFftBin * usSizeComp;
#endif
        pErrOut = pstAecEng->pErrOut;
        pstAecEng->pCallBackFunc((U8 *)pErrOut, uiSize, pstAecEng->pUsrData);
    }
}

/************************************************************
  Function   : sevcAecMicProcess()

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
static inline VOID sevcAecMicProcess(SEVC_AEC_S *pstAecEng, aisp_cpx_s32_t *pstMicFrame)
{
    SEVC_CONFIG_S *pstSevcCfg = pstAecEng->pstSevcCfg;
    U16 usMicIdx = 0;
    U16 usMicNum = pstSevcCfg->micChan;
    aisp_cpx_s32_t *pstNewFrame = pstMicFrame;

    for (usMicIdx = 0; usMicIdx < usMicNum; usMicIdx++)
    {
        sevcAecErrCalc(pstAecEng, pstNewFrame, usMicIdx);
        sevcAecWeightUpdate(pstAecEng, usMicIdx);
        pstNewFrame += pstSevcCfg->usFftBin;
    }
}

/************************************************************
  Function   : sevcAecRefProcess()

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
static inline VOID sevcAecRefProcess(SEVC_AEC_S *pstAecEng, aisp_cpx_s32_t *pstRefFrame)
{
    S16 sLoop;
    U16 usRefIdx;
    U16 usAecTaps = pstAecEng->pstSevcCfg->usAecTaps;
    U16 usRefNum = pstAecEng->pstSevcCfg->refChan;
    U16 usFftBin = pstAecEng->pstSevcCfg->usFftBin;
    aisp_cpx_s32_t **ppRefBins = pstAecEng->pppRefX[usAecTaps];
    aisp_cpx_s32_t *pRefBins;
    aisp_s32_t **ppRefPower = pstAecEng->pppfRefPower[usAecTaps];
    aisp_s32_t *pRefPower;
    aisp_s32_t *pfDiag;
    aisp_s32_t fAecLamdaUp = pstAecEng->pstSevcCfg->fAecLamdaUp; //W32Q20
    aisp_s32_t fAecLamdaUpResidual = 1048576 - fAecLamdaUp;      //W32Q20
    aisp_s32_t fAecLamdaDown = pstAecEng->pstSevcCfg->fAecLamdaDown; //W32Q20
    aisp_s32_t fAecLamdaDownResidual = 1048576 - fAecLamdaDown; //W32Q20
    aisp_s32_t **ppRefPowerOld, *pRefPowerOld;
    aisp_s32_t *pfRefPowerSum = pstAecEng->pfRefPowerSum;

    for (usRefIdx = 0; usRefIdx < usRefNum; usRefIdx++)
    {
        pRefBins = ppRefBins[usRefIdx];
        pRefPower = ppRefPower[usRefIdx];

        for (sLoop = 0; sLoop < usFftBin; sLoop++)
        {
            pRefBins[sLoop].real = pstRefFrame[sLoop].real;
            pRefBins[sLoop].imag = pstRefFrame[sLoop].imag;
        }

#ifdef FREQ_BIN_Q21
        sevcVecPowY32X32_Complex(pRefPower, pRefBins, usFftBin, 26); //W32Q16
#else
        sevcVecPowY32X32_Complex(pRefPower, pRefBins, usFftBin, 14); //W32Q16
#endif
    }

    for (sLoop = usAecTaps; sLoop > 0; sLoop--)
    {
        pstAecEng->pppRefX[sLoop] = pstAecEng->pppRefX[sLoop - 1];
        pstAecEng->pppfRefPower[sLoop] = pstAecEng->pppfRefPower[sLoop - 1];
    }

    pstAecEng->pppRefX[0] = ppRefBins;
    pstAecEng->pppfRefPower[0] = ppRefPower;
    pfDiag = pstAecEng->pfDiag;

    if (pstAecEng->refVad)
    {
        if (usAecTaps == 1)
        {
            ppRefPower = pstAecEng->pppfRefPower[0];
            pRefPower = ppRefPower[0];

            for (sLoop = 0; sLoop < usFftBin; sLoop++)
            {
                pfRefPowerSum[sLoop] = pRefPower[sLoop];
            }

            for (usRefIdx = 1; usRefIdx < usRefNum; usRefIdx++)
            {
                pRefPower = ppRefPower[usRefIdx];
                sevcVec32Add32_Real(pfRefPowerSum, pfRefPowerSum, pRefPower, usFftBin, 0); //W32Q16
            }
        }
        else
        {
            //sum
            ppRefPowerOld = pstAecEng->pppfRefPower[usAecTaps];
            ppRefPower = pstAecEng->pppfRefPower[0];

            for (usRefIdx = 0; usRefIdx < usRefNum; usRefIdx++)
            {
                pRefPowerOld = ppRefPowerOld[usRefIdx];
                pRefPower = ppRefPower[usRefIdx];
                sevcVec32Add32_Real(pfRefPowerSum, pfRefPowerSum, pRefPower, usFftBin, 0);
                sevcVec32Sub32_Real(pfRefPowerSum, pfRefPowerSum, pRefPowerOld, usFftBin, 0);
            }
        }

        for (sLoop = 0; sLoop < usFftBin; sLoop++)
        {
            pfDiag[sLoop] = (pfDiag[sLoop] < pfRefPowerSum[sLoop] ?
                             MULT32_32_SQ(pfDiag[sLoop], fAecLamdaUpResidual, 20) + MULT32_32_SQ(pfRefPowerSum[sLoop], fAecLamdaUp, 20)
                             : MULT32_32_SQ(pfDiag[sLoop], fAecLamdaDownResidual, 20) + MULT32_32_SQ(pfRefPowerSum[sLoop], fAecLamdaDown, 20));
        }
    }
}

/************************************************************
  Function   : sevcAecInstanceReset()

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
static inline VOID sevcAecInstanceReset(SEVC_AEC_INSTANCE_S *pstAecInstance)
{
    SEVC_CONFIG_S *pstSevcCfg = pstAecInstance->pstSevcCfg;
    U16 usTapIdx, usRefIdx, usBinIdx;
    U16 usAecTaps = pstSevcCfg->usAecTaps;
    U16 usRefNum = pstSevcCfg->refChan;
    U16 usFftBins = pstSevcCfg->usFftBin;
    // U16 usFrameShift = pstSevcCfg->frameShift;
    aisp_cpx_s32_t **ppWF;
    aisp_cpx_s32_t *pWFRow;
    // aisp_s32_t *pfMicFramePreemph = pstAecInstance->pfMicFramePreemph;

    // for (usBinIdx = 0; usBinIdx < usFrameShift; usBinIdx++)
    // {
    //     pfMicFramePreemph[usBinIdx] = 0;
    // }

    for (usTapIdx = 0; usTapIdx < usAecTaps; usTapIdx++)
    {
        ppWF = pstAecInstance->pppWFDomain[usTapIdx];

        for (usRefIdx = 0; usRefIdx < usRefNum; usRefIdx++)
        {
            pWFRow = ppWF[usRefIdx];

            for (usBinIdx = 0; usBinIdx < usFftBins; usBinIdx++)
            {
                COMPLEX_ZERO(pWFRow[usBinIdx]);
            }
        }
    }
}

/************************************************************
  Function   : sevcAecInstanceLocalMemSizeGet()

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
static U32 sevcAecInstanceLocalMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    U16 usSizeComp = sizeof(aisp_cpx_s32_t);
    U16 usRefNum = pstSevcCfg->refChan;
    // U16 usFrameSize = pstSevcCfg->usFftFrameSize;
    U16 usAecTaps = pstSevcCfg->usAecTaps;
    uiTotalMemSize += SIZE_ALIGN(sizeof(SEVC_AEC_INSTANCE_S));
    //pfMicFramePreemph;
    // uiTotalMemSize += SIZE_ALIGN(usFrameSize * sizeof(aisp_s32_t));
    //pppWFDomain
    uiTotalMemSize += SIZE_ALIGN(usAecTaps * sizeof(VOID *));
    uiTotalMemSize += usAecTaps * SIZE_ALIGN(usRefNum * sizeof(VOID *));
    uiTotalMemSize += usAecTaps * usRefNum * SIZE_ALIGN(pstSevcCfg->usFftBin * usSizeComp);
    //pOutEst;
    // uiTotalMemSize += SIZE_ALIGN(pstSevcCfg->usFftBin * usSizeComp);
//#endif
#if MEM_BUFF_DBG
    MSG_DBG("%s uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}

/************************************************************
  Function   : SEVC_AEC_ShMemSizeGet()

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
U32 SEVC_AEC_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    U16 usSizeComp = sizeof(aisp_cpx_s32_t);
    //U16 usFrameSize = pstSevcCfg->usFftFrameSize;
    U16 usFftBin = pstSevcCfg->usFftBin;
    //pErrNorm
    uiTotalMemSize += SIZE_ALIGN((usFftBin * usSizeComp));
    //pfDataTmp
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s64_t));
    //pfRefPowerSum;
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s32_t));
    //MSG_DBG("%s [%d] uiTotalMemSize=%d\r\n", __func__, __LINE__, uiTotalMemSize);
    return uiTotalMemSize;
}

/************************************************************
  Function   : SEVC_AEC_LocMemSizeGet()

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
U32 SEVC_AEC_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiTotalMemSize = 0;
    U16 usSizeComp = sizeof(aisp_cpx_s32_t);
    //U16 usFftBin = pstSevcCfg->usFftBin;
    U16 usMicNum = pstSevcCfg->micChan;
    U16 usRefNum = pstSevcCfg->refChan;
    U16 usAecTaps = pstSevcCfg->usAecTaps;
    U16 usFftBin = pstSevcCfg->usFftBin;
    uiTotalMemSize += SIZE_ALIGN(sizeof(SEVC_AEC_S));
    //pppRefX;
    uiTotalMemSize += (usAecTaps + 1) * SIZE_ALIGN(sizeof(VOID *));
    uiTotalMemSize += (usAecTaps + 1) * usRefNum * SIZE_ALIGN(sizeof(VOID *));
    uiTotalMemSize += (usAecTaps + 1) * usRefNum * SIZE_ALIGN(usFftBin * usSizeComp);
    //pppfRefPower;
    uiTotalMemSize += (usAecTaps + 1) * SIZE_ALIGN(sizeof(VOID *));
    uiTotalMemSize += (usAecTaps + 1) * usRefNum * SIZE_ALIGN(sizeof(VOID *));
    uiTotalMemSize += (usAecTaps + 1) * usRefNum * SIZE_ALIGN(usFftBin * sizeof(aisp_s32_t));
    //pfDiag
    uiTotalMemSize += SIZE_ALIGN(usFftBin * sizeof(aisp_s32_t));
    //ppstAecInstance; //
    uiTotalMemSize += SIZE_ALIGN(usMicNum * sizeof(VOID *));
    uiTotalMemSize += usMicNum * sevcAecInstanceLocalMemSizeGet(pstSevcCfg);
    {
        //pErrOut + pEstOut
        uiTotalMemSize += SIZE_ALIGN(usMicNum * usFftBin * usSizeComp);

        if (usMicNum == 3)
        {
            uiTotalMemSize += SIZE_ALIGN(2 * usFftBin * usSizeComp); // export two linear echo when micNum equals 3
        }
        else
        {
            uiTotalMemSize += SIZE_ALIGN(usFftBin * usSizeComp);
        }
    }
    // errPower
    uiTotalMemSize += usMicNum * usFftBin * sizeof(aisp_s64_t);
#ifndef USE_SHARE_MEM
    //share mem vars
    uiTotalMemSize += SEVC_AEC_ShMemSizeGet(pstSevcCfg);
#endif
#if MEM_BUFF_DBG
    MSG_DBG("%s uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}

/************************************************************
  Function   : sevcAecInstanceNew()

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
static SEVC_AEC_INSTANCE_S *sevcAecInstanceNew(SEVC_CONFIG_S *pstSevcCfg,
        MEM_LOCATOR_S *pstMemLocator)
{
    U32 uiSize, uiSize2;
    U16 usTapIdx, usRefIdx;
    U16 usAecTaps = pstSevcCfg->usAecTaps;
    U16 usRefNum = pstSevcCfg->refChan;
    // U16 usFrameSize = pstSevcCfg->usFftFrameSize;
    U16 usSizeComp = sizeof(aisp_cpx_s32_t);
    SEVC_AEC_INSTANCE_S *pstAecInstance;
    MEM_LOCATOR_SHOW(pstMemLocator);
    uiSize = sizeof(SEVC_AEC_INSTANCE_S);
    pstAecInstance = (SEVC_AEC_INSTANCE_S *)memLocatorGet(pstMemLocator,
                     uiSize, AISP_ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstAecInstance, FAIL);
    pstAecInstance->pstSevcCfg = pstSevcCfg;
    //pfMicFramePreemph
    // BUFF_POINT_SET_ALIGN(pstAecInstance->pfMicFramePreemph,
    //                      (usFrameSize * sizeof(aisp_s32_t)),
    //                      pstMemLocator, FAIL);
    // adaptor weight init
    //pppFDomainW
    uiSize = usAecTaps * sizeof(VOID *);
    BUFF_POINT_SET_ALIGN(pstAecInstance->pppWFDomain,
                         uiSize, pstMemLocator, FAIL);
    uiSize = usRefNum * sizeof(VOID *);

    for (usTapIdx = 0; usTapIdx < usAecTaps; usTapIdx++)
    {
        BUFF_POINT_SET_ALIGN(pstAecInstance->pppWFDomain[usTapIdx],
                             uiSize, pstMemLocator, FAIL);

        for (usRefIdx = 0; usRefIdx < usRefNum; usRefIdx++)
        {
            uiSize2 = pstSevcCfg->usFftBin * usSizeComp;
            BUFF_POINT_SET_ALIGN(pstAecInstance->pppWFDomain[usTapIdx][usRefIdx],
                                 uiSize2, pstMemLocator, FAIL);
        }
    }

    //pfErrFrame;
    // BUFF_POINT_SET_ALIGN(pstAecInstance->pfErrFrame, (usFrameSize * sizeof(aisp_s32_t)),
    //                      pstMemLocator, FAIL);
    //pOutEst;
    // uiSize2 = pstSevcCfg->usFftBin * usSizeComp;
    // BUFF_POINT_SET_ALIGN(pstAecInstance->pOutEst, uiSize2,
    //                      pstMemLocator, FAIL);
    MEM_LOCATOR_SHOW(pstMemLocator);
    return pstAecInstance;
FAIL:
    MSG_DBG("%s mem fail\r\n", __func__);
    return NULL;
}

/************************************************************
  Function   : SEVC_AEC_New()

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
SEVC_AEC_S *SEVC_AEC_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator)
{
    U32 uiSize;
    U16 usIdx, usRefIdx;
    U16 usSizeComp = sizeof(aisp_cpx_s32_t);
    U16 usAecTaps = pstSevcCfg->usAecTaps;
    U16 usMicNum = pstSevcCfg->micChan;
    U16 usRefNum = pstSevcCfg->refChan;
    //U16 usFrameSize = pstSevcCfg->usFftFrameSize;
    U16 usFftBin = pstSevcCfg->usFftBin;
    SEVC_AEC_S *pstAecEng = NULL;
#if SEVC_SWITCH_AEC_EMD
    aisp_s32_t fDeltaFloorMin;
    aisp_s32_t fDeltaFloorTmp;
    aisp_s32_t fDataTmp;
    aisp_s32_t *pfDeltaFloor;
#endif

    if (NULL == pstSevcCfg || NULL == pstMemLocator)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return NULL;
    }

    MEM_LOCATOR_SHOW(pstMemLocator);
    uiSize = sizeof(SEVC_AEC_S); //112
    pstAecEng = (SEVC_AEC_S *)memLocatorGet(pstMemLocator, uiSize, AISP_ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstAecEng, FAIL);
    //MEM_LOCATOR_SHOW(pstMemLocator);
    pstAecEng->pstSevcCfg = pstSevcCfg;
#ifdef USE_MULT_THREAD
    //pstAecEng->pstFftEng = AISP_TSL_FFT_init(pstSevcCfg->usFrameSize);
    pstAecEng->pstFftEng = pstSevcCfg->ppstFftEng[usMicIdx];
#else
    pstAecEng->pstFftEng = &pstSevcCfg->stFFTEng;
#endif
#if SEVC_SWITCH_VAD == 0
    pstAecEng->refVad = 1;
#endif
    pstAecEng->uiFrameCount = 0;
    //pppRefX
    BUFF_POINT_SET_ALIGN(pstAecEng->pppRefX,
                         ((usAecTaps + 1)*sizeof(VOID *)), pstMemLocator, FAIL);

    for (usIdx = 0; usIdx <= usAecTaps; usIdx++)
    {
        BUFF_POINT_SET_ALIGN(pstAecEng->pppRefX[usIdx],
                             (usRefNum * sizeof(VOID *)), pstMemLocator, FAIL);

        for (usRefIdx = 0; usRefIdx < usRefNum; usRefIdx++)
        {
            BUFF_POINT_SET_ALIGN(pstAecEng->pppRefX[usIdx][usRefIdx],
                                 (usFftBin * usSizeComp), pstMemLocator, FAIL);
        }
    }

    //pppfRefPower
    BUFF_POINT_SET_ALIGN(pstAecEng->pppfRefPower, ((usAecTaps + 1)*sizeof(VOID *)),
                         pstMemLocator, FAIL);

    for (usIdx = 0; usIdx <= usAecTaps; usIdx++)
    {
        BUFF_POINT_SET_ALIGN(pstAecEng->pppfRefPower[usIdx], (usRefNum * sizeof(VOID *)),
                             pstMemLocator, FAIL);

        for (usRefIdx = 0; usRefIdx < usRefNum; usRefIdx++)
        {
            BUFF_POINT_SET_ALIGN(pstAecEng->pppfRefPower[usIdx][usRefIdx],
                                 (usFftBin * sizeof(aisp_s32_t)), pstMemLocator, FAIL);
        }
    }

    //pfDiag
    BUFF_POINT_SET_ALIGN(pstAecEng->pfDiag, (usFftBin * sizeof(aisp_s32_t)),
                         pstMemLocator, FAIL);
    //ppstAecInstance
    BUFF_POINT_SET_ALIGN(pstAecEng->ppstAecInstance, (usMicNum * sizeof(VOID *)),
                         pstMemLocator, FAIL);

    for (usIdx = 0; usIdx < usMicNum; usIdx++)
    {
        pstAecEng->ppstAecInstance[usIdx] = sevcAecInstanceNew(pstSevcCfg, pstMemLocator);
        AISP_TSL_PTR_CHECK(pstAecEng->ppstAecInstance[usIdx], FAIL);
    }

    // pErrOut + pEstOut
    uiSize = usMicNum * usFftBin * usSizeComp;

    // export two linear echo when micNum equals 3
    if (usMicNum == 3)
    {
        uiSize += 2 * usFftBin * usSizeComp;
    }
    else
    {
        uiSize += usFftBin * usSizeComp;
    }

    BUFF_POINT_SET_ALIGN(pstAecEng->pErrOut, uiSize, pstMemLocator, FAIL);
    // pfErrPower
    uiSize = usMicNum * usFftBin * sizeof(aisp_s64_t);
    BUFF_POINT_SET_ALIGN(pstAecEng->pfErrPower, uiSize, pstMemLocator, FAIL);
    //shared mem
    SH_BUFF_POINT_SET_ALIGN(pstAecEng->pfDataTmp, usFftBin * sizeof(aisp_s64_t), pstMemLocator, FAIL);
    //pErrNorm
    SH_BUFF_POINT_SET_ALIGN(pstAecEng->pErrNorm,
                            (usFftBin * usSizeComp),
                            pstMemLocator, FAIL);
    //pfRefPowerSum
    SH_BUFF_POINT_SET_ALIGN(pstAecEng->pfRefPowerSum, (usFftBin * sizeof(aisp_s32_t)),
                            pstMemLocator, FAIL);
    MEM_LOCATOR_SHOW(pstMemLocator);
    return pstAecEng;
FAIL:
    MSG_DBG("%s [%d] fail\r\n", __func__, __LINE__);
    return NULL;
}

/************************************************************
  Function   : SEVC_AEC_CallBackFuncRegister()

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
VOID SEVC_AEC_CallBackFuncRegister(SEVC_AEC_S *pstAecEng,
                                   VOID *pCBFunc, VOID *pUsrData)
{
    if (NULL == pstAecEng || NULL == pCBFunc)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return ;
    }

    pstAecEng->pCallBackFunc = (CallBackFunc)pCBFunc;
    pstAecEng->pUsrData = pUsrData;
}


/************************************************************
  Function   : SEVC_AEC_Feed()

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
VOID SEVC_AEC_Feed(SEVC_AEC_S *pstAecEng, aisp_cpx_s32_t *pstMicFrame, aisp_cpx_s32_t *pstRefFrame)
{
    sevcAecRefProcess(pstAecEng, pstRefFrame);
    sevcAecMicProcess(pstAecEng, pstMicFrame);
    sevcAecErrPop(pstAecEng, pstMicFrame);
    pstAecEng->uiFrameCount++;
}

/************************************************************
  Function   : SEVC_AEC_Reset()

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
VOID SEVC_AEC_Reset(SEVC_AEC_S *pstAecEng)
{
    U16 usMicIdx;
    U16 usMicNum = pstAecEng->pstSevcCfg->micChan;
    pstAecEng->uiFrameCount = 0;

    for (usMicIdx = 0; usMicIdx < usMicNum; usMicIdx++)
    {
        sevcAecInstanceReset(pstAecEng->ppstAecInstance[usMicIdx]);
    }
}

/************************************************************
  Function   : SEVC_AEC_Delete()

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
VOID SEVC_AEC_Delete(SEVC_AEC_S *pstAecEng)
{
    return ;
}

#endif

