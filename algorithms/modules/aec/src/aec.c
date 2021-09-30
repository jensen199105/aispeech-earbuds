/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : aec.c
  Project    :
  Module     :
  Version    :
  Date       : 2016/08/06
  Author     : ziyuan.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  ziyuan.Jiang  2016/03/06         1.00              Create

************************************************************/
#include "aec.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include "naes.h"
#include "AISP_TSL_base.h"
#include "aisp_memmng.h"
#include "AISP_TSL_complex.h"
#include "AISP_TSL_sqrt.h"
#include "AISP_TSL_str.h"
#include "fft_wrapper.h"
#include "aec_base_fixed.h"
#include "aec_window.h"




static const short deltaFloor_aec[] =
{
    964,    962,    960,    958,    956,    955,    953,    951,    949,    947,    945,    943,    941,    940,    938,    936,
    934,    932,    930,    928,    926,    925,    923,    921,    919,    917,    915,    913,    911,    909,    908,    906,
    904,    902,    900,    898,    896,    894,    893,    891,    889,    887,    885,    883,    881,    879,    878,    876,
    874,    872,    870,    868,    866,    864,    863,    861,    859,    857,    855,    853,    851,    849,    847,    846,
    844,    842,    840,    838,    836,    834,    832,    831,    829,    827,    825,    823,    821,    819,    817,    816,
    814,    812,    810,    808,    806,    804,    802,    800,    799,    797,    795,    793,    791,    789,    787,    785,
    784,    782,    780,    778,    776,    774,    772,    770,    769,    767,    765,    763,    761,    759,    757,    755,
    754,    752,    750,    748,    746,    744,    742,    740,    738,    737,    735,    733,    731,    729,    727,    725,
    723,    722,    720,    718,    716,    714,    712,    710,    708,    707,    705,    703,    701,    699,    697,    695,
    693,    691,    690,    688,    686,    684,    682,    680,    678,    676,    675,    673,    671,    669,    667,    665,
    663,    661,    660,    658,    656,    654,    652,    650,    648,    646,    645,    643,    641,    639,    637,    635,
    633,    631,    629,    628,    626,    624,    622,    620,    618,    616,    614,    613,    611,    609,    607,    605,
    603,    601,    599,    598,    596,    594,    592,    590,    588,    586,    584,    582,    581,    579,    577,    575,
    573,    571,    569,    567,    566,    564,    562,    560,    558,    556,    554,    552,    551,    549,    547,    545,
    543,    541,    539,    537,    536,    534,    532,    530,    528,    526,    524,    522,    520,    519,    517,    515,
    513,    511,    509,    507,    505,    504,    502,    500,    498,    496,    494,    492,    490,    489,    487,    485,
    483,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,
    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,
    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,
    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,
    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,
    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,
    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,
    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,
    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,
    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,
    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,
    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,
    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,
    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,
    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,
    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,    482,
    482,
};

/************************************************************
  Function   : aec_emdMatrixInv()

  Description: Calls      : Called By  : Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create
    2017/03/03, Youhai.Jiang modified

************************************************************/
//static inline
void aec_emdMatrixInv(aec_core_T *pstAec)
{
    int i           = 0;
    int Bins_off_1  = 0;
    int iFftBins    = 0;
    int iRefNum = 0;
    aisp_s32_t tR11        = 0;
    aisp_s32_t tR22        = 0;
    aisp_s32_t normR       = 0;
    aisp_cpx_s32_t tR12    = {0, 0};
    aisp_cpx_s32_t *aREF   = NULL;
    aisp_s64_t mean_tmp    = 0;
    aisp_s64_t normR_tmp   = 0;
    aisp_s64_t R11_tmp     = 0;
    aisp_s64_t R22_tmp     = 0;
    aisp_s64_t R12_r_tmp   = 0;
    aisp_s64_t R12_i_tmp   = 0;
    aisp_s64_t R_thresh    = (aisp_s64_t)1 << 42;
    pstAec->meanR11 = 0;
    pstAec->meanR22 = 0;
    aREF = pstAec->puX_REF;
    iFftBins = pstAec->fftBins;
    iRefNum = pstAec->config->refNum;

    /* compute value for Rxx, and use tRxx to store their sum */
    switch (iRefNum)
    {
        case 2:
            {
                for (i = 0; i < iFftBins; ++i)
                {
                    /* 4032 = 63 * 2^6   63/64 means lambda ;          */
                    /* 6 means 2^6 = 64 , den of lembda (lembda=63/64) */
                    Bins_off_1 = i + iFftBins;
#ifdef USE_HIFI4_OPTIMIZE
                    int shift = 6 + pstAec->aR_Qbase[i];
                    aisp_s64_t tmp[4] = {0, 0, 0, 0};
                    ae_int64 *pRlt = (ae_int64 *)tmp;
                    ae_int32x2 *pRef1 = (ae_int32x2 *)&aREF[i];
                    ae_int32x2 *pRef2 = (ae_int32x2 *)&aREF[Bins_off_1];
                    AE_MULAAD32_HH_LL(pRlt[0], *pRef2, *pRef2);
                    AE_MULAAD32_HH_LL(pRlt[1], *pRef1, *pRef1);
                    AE_MULAAD32_HH_LL(pRlt[2], *pRef1, *pRef2);
                    aREF[i].imag = -aREF[i].imag;
                    AE_MULAAD32_HL_LH(pRlt[3], *pRef1, *pRef2);
                    aREF[i].imag = -aREF[i].imag;
                    R22_tmp   = SHL((aisp_s64_t)pstAec->aR22[i] * 63, shift) + tmp[0];
                    R11_tmp   = SHL((aisp_s64_t)pstAec->aR11[i] * 63, shift) + tmp[1];
                    R12_r_tmp = SHL((aisp_s64_t)pstAec->aR12[i].real * 63, shift) + tmp[2];
                    R12_i_tmp = SHL((aisp_s64_t)pstAec->aR12[i].imag * 63, shift) + tmp[3];
#else
                    R22_tmp   = SHL((aisp_s64_t)pstAec->aR22[i] * 63, 6 + pstAec->aR_Qbase[i]) +
                                (aisp_s64_t)aREF[Bins_off_1].real * aREF[Bins_off_1].real + (aisp_s64_t)aREF[Bins_off_1].imag * aREF[Bins_off_1].imag;
                    R11_tmp   = SHL((aisp_s64_t)pstAec->aR11[i] * 63, 6 + pstAec->aR_Qbase[i]) +
                                (aisp_s64_t)aREF[i].real * aREF[i].real + (aisp_s64_t)aREF[i].imag * aREF[i].imag;
                    R12_r_tmp = SHL((aisp_s64_t)pstAec->aR12[i].real * 63, 6 + pstAec->aR_Qbase[i]) +
                                (aisp_s64_t)aREF[i].real * aREF[Bins_off_1].real + (aisp_s64_t)aREF[i].imag * aREF[Bins_off_1].imag;
                    R12_i_tmp = SHL((aisp_s64_t)pstAec->aR12[i].imag * 63, 6 + pstAec->aR_Qbase[i]) +
                                (aisp_s64_t)aREF[i].real * aREF[Bins_off_1].imag - (aisp_s64_t)aREF[i].imag * aREF[Bins_off_1].real;
#endif

                    if ((R11_tmp > R_thresh) || (R22_tmp > R_thresh) || (R12_r_tmp > R_thresh) || (R12_i_tmp > R_thresh))
                    {
                        pstAec->aR11[i] = PSHR64(R11_tmp, 24);
                        pstAec->aR22[i] = PSHR64(R22_tmp, 24);
                        pstAec->aR12[i].real = PSHR64(R12_r_tmp, 24);
                        pstAec->aR12[i].imag = PSHR64(R12_i_tmp, 24);
                        pstAec->aR_Qbase[i]  = 12;
                        pstAec->meanR11 += SHL64(pstAec->aR11[i], 12);
                        pstAec->meanR22 += SHL64(pstAec->aR22[i], 12);
                    }
                    else
                    {
                        pstAec->aR11[i] = PSHR64(R11_tmp, 12);
                        pstAec->aR22[i] = PSHR64(R22_tmp, 12);
                        pstAec->aR12[i].real = PSHR64(R12_r_tmp, 12);
                        pstAec->aR12[i].imag = PSHR64(R12_i_tmp, 12);
                        pstAec->aR_Qbase[i]  = 0;
                        pstAec->meanR11 += pstAec->aR11[i];
                        pstAec->meanR22 += pstAec->aR22[i];
                    }
                }

                break;
            }

        case 1:
        default:
            {
                for (i = 0; i < iFftBins; ++i)
                {
                    pstAec->aR11[i] = PSHR((aisp_s64_t)pstAec->aR11[i] *  2064384 +
                                           (aisp_s64_t)aREF[i].real * aREF[i].real + (aisp_s64_t)aREF[i].imag * aREF[i].imag, 21);
                    pstAec->meanR11 += pstAec->aR11[i];
                }

                break;
            }
    }

    /* computing the inverse matrix */
    switch (iRefNum)
    {
        case 2:
            {
                /* 9 : 2^9 = 512 , fftBins */
                pstAec->meanDiag = pstAec->meanR22 + pstAec->meanR11;
                pstAec->confirmUpdate = pstAec->meanDiag  > SHL64(AEC_TAPS * iRefNum  * pstAec->config->sgmS2thr, 18);

                if (pstAec->confirmUpdate)
                {
                    for (i = 0; i < iFftBins; ++i)
                    {
                        /* 9 : 2^9 = 512 , fftBins */
                        mean_tmp  = PSHR64(pstAec->meanR11, pstAec->aR_Qbase[i]);
                        tR11      = PDIV64(mean_tmp + mean_tmp + mean_tmp, SHL(20, 9)) + pstAec->aR11[i];
                        mean_tmp  = PSHR64(pstAec->meanR22, pstAec->aR_Qbase[i]);
                        tR22      = PDIV64(mean_tmp + mean_tmp + mean_tmp, SHL(20, 9)) + pstAec->aR22[i];
                        tR12.real = PSHR64((aisp_s64_t)pstAec->aR12[i].real * pstAec->config->lembda_mu, AISP_TSL_Q15_SHIFT);
                        tR12.imag = PSHR64((aisp_s64_t)pstAec->aR12[i].imag * pstAec->config->lembda_mu, AISP_TSL_Q15_SHIFT);
                        normR_tmp = (aisp_s64_t)tR11 * tR22 - (aisp_s64_t)tR12.real * tR12.real - (aisp_s64_t)tR12.imag * tR12.imag;

                        if (normR_tmp >= SHL64(1, 55))
                        {
                            normR = PSHR64(normR_tmp, 32);

                            if (pstAec->aR_Qbase[i] == 0)
                            {
                                pstAec->aIR11[i] = PDIV64(SHL64(tR22, 7), normR);
                                pstAec->aIR22[i] = PDIV64(SHL64(tR11, 7), normR);
                                pstAec->aIR12[i].real = PDIV64(SHL64(-tR12.real, 7), normR);
                                pstAec->aIR12[i].imag = PDIV64(SHL64(-tR12.imag, 7), normR);
                            }
                            else
                            {
                                pstAec->aIR11[i] = PDIV64(PSHR(tR22, 5), normR);
                                pstAec->aIR22[i] = PDIV64(PSHR(tR11, 5), normR);
                                pstAec->aIR12[i].real = PDIV64(PSHR(-tR12.real, 5), normR);
                                pstAec->aIR12[i].imag = PDIV64(PSHR(-tR12.imag, 5), normR);
                            }
                        }
                        else
                        {
                            normR = PSHR64(normR_tmp, 24);

                            if (normR == 0)
                            {
                                if (tR22 > SHL32(1, 9))
                                {
                                    pstAec->aIR11[i] = 0x7fffffff;
                                }
                                else
                                {
                                    pstAec->aIR11[i] = SHL32(tR22, 16);
                                }

                                if (tR11 > SHL32(1, 9))
                                {
                                    pstAec->aIR22[i] = 0x7fffffff;
                                }
                                else
                                {
                                    pstAec->aIR22[i] = SHL32(tR11, 16);
                                }

                                if (tR12.real > SHL32(1, 9))
                                {
                                    pstAec->aIR12[i].real = 0x7fffffff;
                                }
                                else
                                {
                                    pstAec->aIR12[i].real = SHL32(-tR12.real, 16);
                                }

                                if (tR12.imag > SHL32(1, 9))
                                {
                                    pstAec->aIR12[i].imag = 0x7fffffff;
                                }
                                else
                                {
                                    pstAec->aIR12[i].imag = SHL32(-tR12.imag, 16);
                                }
                            }
                            else
                            {
                                pstAec->aIR11[i]      = PDIV64(SHL64(tR22, 15 - pstAec->aR_Qbase[i]), normR);
                                pstAec->aIR22[i]      = PDIV64(SHL64(tR11, 15 - pstAec->aR_Qbase[i]), normR);
                                pstAec->aIR12[i].real = PDIV64(SHL64(-tR12.real, 15 - pstAec->aR_Qbase[i]), normR);
                                pstAec->aIR12[i].imag = PDIV64(SHL64(-tR12.imag, 15 - pstAec->aR_Qbase[i]), normR);
                            }
                        }
                    }
                }

                break;
            }

        case 1:
        default:
            {
                pstAec->meanDiag = pstAec->meanR11;
                pstAec->confirmUpdate = pstAec->meanDiag  > SHL64(AEC_TAPS * iRefNum  * pstAec->config->sgmS2thr, 9);

                if (pstAec->confirmUpdate)
                {
                    for (i = 0; i < iFftBins; ++i)
                    {
                        pstAec->aIR11[i] = PDIV32_32_P15(AISP_TSL_Q15_ONE_MINI, (pstAec->aR11[i] + 3));
                    }

                    break;
                }
            }
    }
}

/************************************************************
  Function   : aec_vadAndFft()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create
    2017/03/03, Youhai.Jiang modified

************************************************************/

void aec_vadAndFft(aec_core_T *pstAec, aisp_s32_t mean)
{
    int i = 0;
    int j = 0;
    int iRefNum = pstAec->config->refNum;
    int iFrmSize = pstAec->config->frameSize;
    aisp_s32_t refBufMean = 0;
    aisp_s32_t VAD_amp    = 0;
    aisp_s32_t refNewSum  = 0;
    aisp_cpx_s32_t *pXTmp = NULL;

    if (pstAec->idx == 0)
    {
        if (pstAec->config->vad)
        {
            refBufMean = PDIV32_32(mean , iFrmSize);

            for (i = 0; i < iFrmSize; ++i)
            {
                refNewSum = 0;

                for (j = 0; j < iRefNum; ++j)
                {
                    refNewSum += AISP_TSL_ABS(pstAec->aRefFrameOld[j][i]);
                }

                VAD_amp += AISP_TSL_ABS(refNewSum - refBufMean);
            }

            if (pstAec->vadResult == 0)
            {
                if (VAD_amp > pstAec->config->amth * iFrmSize)
                {
                    pstAec->specnt++;

                    if (pstAec->specnt >= pstAec->config->sil2spe)
                    {
                        pstAec->vadResult = 1;
                        pstAec->specnt    = 0;
                    }
                }
                else
                {
                    pstAec->specnt = 0;
                }
            }
            else
            {
                if (VAD_amp < pstAec->config->amth * iFrmSize)
                {
                    pstAec->silcnt = pstAec->silcnt + 1;

                    if (pstAec->silcnt >=  pstAec->config->spe2sil)
                    {
                        pstAec->vadResult = 0;
                        pstAec->silcnt    = 0;
                    }
                }
                else
                {
                    pstAec->silcnt = 0;
                }
            }
        }
        else
        {
            pstAec->vadResult = 1;
        }

        /* when voice detect, convert ref to REF */
        if (pstAec->vadResult != 0)
        {
            pXTmp = pstAec->puX_REF;
            pstAec->puX_REF = pstAec->pX0;
            pstAec->pX0 = pstAec->pX1;
            pstAec->pX1 = pstAec->pX2;
            pstAec->pX2 = pXTmp;

            for (j = 0; j < iRefNum; ++j)
            {
                /* fft out len : 513*complex ; if len > 513 then REF2 will be destroy!!!*/
                //aec_fft(pstAec->fftEngine, pstAec->aRefFrameWide[j], pstAec->puX_REF+j*pstAec->fftBins);
                AISP_TSL_fftr32(pstAec->fftEngine, pstAec->aRefFrameWide[j], pstAec->puX_REF + j * pstAec->fftBins);
            }

            if (pstAec->config->emd)
            {
                /*  calc the inverse matrix here
                 *  and  share the result between different mics.
                 */
                aec_emdMatrixInv(pstAec);
            }
        }
    }
}

/************************************************************
  Function   : aec_preProcess()

  Description: VAD check
               time domain to Freqent domain
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create
    2017/03/03, Youhai.Jiang modified

************************************************************/
void aec_preProcess(aec_core_T *pstAec, const short *micFrame, const short *refFrame0,
                    const short *refFrame1, const short *refFrame2)
{
    int i = 0;
    int j = 0;
    int iRefNum  = pstAec->config->refNum;
    int iFrmSize = pstAec->config->frameSize;
    const short *pRefFrame[3];
    unsigned int ulOff     = 0;
    aisp_s16_t clipth      = 0;
#ifdef USE_HIFI4_OPTIMIZE
    aisp_s32_t tmp         = 0;
    aisp_s32_t maxTmp      = 0;
#endif
    aisp_s16_t tmp32       = 0;
    aisp_s16_t tmp45       = 0;
    aisp_s16_t micNormliz  = 0;
    aisp_s16_t refNormaliz = 0;
    aisp_s32_t maxabsMic   = 0;
    aisp_s32_t mean        = 0;    // 32bit Q15
    aisp_s16_t preemph     = pstAec->config->preemph;

    /* test_aec called this */
    if (pstAec->config->micNum == 1)
    {
        pstAec->maxabsRef = 0;
    }

    clipth = 32766;
    pRefFrame[0] = refFrame0;
    pRefFrame[1] = refFrame1;
    pRefFrame[2] = refFrame2;

    /* find the maximum abs value in the frame */
    for (i = 0; i < iFrmSize; ++i)
    {
        /* normalize the raw data */
        micNormliz = micFrame[i];
#ifndef USE_HIFI4_OPTIMIZE

        if (maxabsMic < abs(micNormliz))
        {
            maxabsMic = abs(micNormliz);
        }

#endif
#ifdef USE_HIFI4_OPTIMIZE
        tmp32 = PSHR32((aisp_s32_t)preemph * pstAec->memX, 15);
        tmp32 = micNormliz - tmp32;
        pstAec->memX = micNormliz;
        /* the top half */
        pstAec->aMicFrameWide[i] = PSHR32((aisp_s32_t)pstAec->hanningWindowSqrt[i] * pstAec->aMicFrameOld[i], 15);
        /* the bottom half */
        ulOff = i + iFrmSize;
        pstAec->aMicFrameOld[i] = tmp32;
        pstAec->aMicFrameWide[ulOff] = PSHR32((aisp_s32_t)pstAec->hanningWindowSqrt[ulOff] * pstAec->aMicFrameOld[i], 15);
#else
        tmp32 = micNormliz - MULT16_16_P15(preemph, pstAec->memX);
        pstAec->memX = micNormliz;
        /* the top half */
        pstAec->aMicFrameWide[i] = MULT16_16_P15(pstAec->hanningWindowSqrt[i], pstAec->aMicFrameOld[i]);
        /* the bottom half */
        ulOff = i + iFrmSize;
        pstAec->aMicFrameOld[i] = tmp32;
        pstAec->aMicFrameWide[ulOff] = MULT16_16_P15(pstAec->hanningWindowSqrt[ulOff], pstAec->aMicFrameOld[i]);
#endif
    }

#ifdef USE_HIFI4_OPTIMIZE
    tmp = vec_min16x16_fast(micFrame, iFrmSize);
    maxabsMic = vec_max16x16_fast(micFrame, iFrmSize);

    if (maxabsMic < AISP_TSL_ABS(tmp))
    {
        maxabsMic = AISP_TSL_ABS(tmp);
    }

#endif

    if (pstAec->config->emd == 0)
    {
        /* emd computation do not need the frequency domain MIC signal */
        AISP_TSL_fftr32(pstAec->fftEngine, pstAec->aMicFrameWide, pstAec->aMIC);
    }

    if (pstAec->idx == 0)
    {
        pstAec->aRefFrameWide[0] = (aisp_s16_t *)pstAec->aExtdw;
        pstAec->aRefFrameWide[1] = (aisp_s16_t *)(pstAec->aExtdW0 + (iFrmSize << 1));

        for (j = 0; j < iRefNum; ++j)
        {
            for (i = 0; i < iFrmSize; ++i)
            {
                refNormaliz = pRefFrame[j][i];
#ifndef USE_HIFI4_OPTIMIZE

                if (pstAec->maxabsRef < AISP_TSL_ABS(refNormaliz))
                {
                    pstAec->maxabsRef = AISP_TSL_ABS(refNormaliz);
                }

#endif
#ifdef USE_HIFI4_OPTIMIZE
                tmp45 = PSHR32((aisp_s32_t)preemph * pstAec->memD[j], 15);
                tmp45 = refNormaliz - tmp45;
#else
                tmp45 = refNormaliz - MULT16_16_P15(preemph, pstAec->memD[j]);
#endif
                pstAec->memD[j] = refNormaliz;
                /* emd is enable, no need to add hanning window */
                pstAec->aRefFrameWide[j][i] = pstAec->aRefFrameOld[j][i]; /* checked 9 */
                pstAec->aRefFrameOld[j][i]  = tmp45;
                pstAec->aRefFrameWide[j][i + iFrmSize] = tmp45; /* checked 10 */
#ifndef USE_HIFI4_OPTIMIZE

                if (pstAec->config->vad)
                {
                    mean += AISP_TSL_ABS(tmp45);
                }

#endif
            }

#ifdef USE_HIFI4_OPTIMIZE
            tmp = vec_min16x16_fast(&pRefFrame[j][0], iFrmSize);
            maxTmp = vec_max16x16_fast(&pRefFrame[j][0], iFrmSize);

            if (pstAec->maxabsRef < AISP_TSL_ABS(tmp))
            {
                pstAec->maxabsRef = AISP_TSL_ABS(tmp);
            }

            if (pstAec->maxabsRef < maxTmp)
            {
                pstAec->maxabsRef = maxTmp;
            }

#endif
        }

        if (pstAec->config->vad)
        {
            for (j = 0; j < iRefNum; ++j)
            {
                for (i = 0; i < iFrmSize; i += 4)
                {
                    mean += AISP_TSL_ABS(pstAec->aRefFrameOld[j][i]) + AISP_TSL_ABS(pstAec->aRefFrameOld[j][i + 1]) +
                            AISP_TSL_ABS(pstAec->aRefFrameOld[j][i + 2]) + AISP_TSL_ABS(pstAec->aRefFrameOld[j][i + 3]);
                }
            }
        }

        aec_vadAndFft(pstAec, mean);
    }

    if (maxabsMic >= clipth || pstAec->maxabsRef >= clipth)
    {
        pstAec->clip = 1;
    }
    else
    {
        pstAec->clip = 0;
    }

    return ;
}

/************************************************************
  Function   : aec_emdUpdateWeight()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create
    2017/03/03, Youhai.Jiang modified

************************************************************/
extern void AISP_TSL_addWithGain_vec_32x32(int* piSrcA, int* piDst, int iLen, int iShift, int iGain);

//static inline
void aec_emdUpdateWeight(aec_core_T *pstAec, int tap_index)
{
    int i = 0;
    int iLoopCnt = pstAec->config->refNum * pstAec->fftBins;
    int iFirstLoop = iLoopCnt - (iLoopCnt & 0x3);
    aisp_cpx_s32_t *pcpxW = pstAec->aW[tap_index];
    aisp_cpx_s32_t *pcpxExtdW0 = pstAec->aExtdW0;
    aisp_s16_t miuQuote = pstAec->MIUQUOTE;

#if 0
    for (i = 0; i < iFirstLoop; i += 4)
    {
        COMPLEX_MUL_FLOAT(pcpxExtdW0[i], pcpxExtdW0[i], miuQuote);
        COMPLEX_MUL_FLOAT(pcpxExtdW0[i + 1], pcpxExtdW0[i + 1], miuQuote);
        COMPLEX_ADD(pcpxW[i], pcpxW[i], pcpxExtdW0[i]);
        COMPLEX_MUL_FLOAT(pcpxExtdW0[i + 2], pcpxExtdW0[i + 2], miuQuote);
        COMPLEX_ADD(pcpxW[i + 1], pcpxW[i + 1], pcpxExtdW0[i + 1]);
        COMPLEX_MUL_FLOAT(pcpxExtdW0[i + 3], pcpxExtdW0[i + 3], miuQuote);
        COMPLEX_ADD(pcpxW[i + 2], pcpxW[i + 2], pcpxExtdW0[i + 2]);
        COMPLEX_ADD(pcpxW[i + 3], pcpxW[i + 3], pcpxExtdW0[i + 3]);
    }

    for (; i < iLoopCnt; ++i)
    {
        COMPLEX_MUL_FLOAT(pcpxExtdW0[i], pcpxExtdW0[i], miuQuote);
        COMPLEX_ADD(pcpxW[i], pcpxW[i], pcpxExtdW0[i]);
    }
#else
    AISP_TSL_addWithGain_vec_32x32(pcpxExtdW0,pcpxW,iLoopCnt * 2,15,miuQuote);
#endif

    return ;
}


/************************************************************
  Function   : aec_emdConstrainStoreDw()

  Description: update dW
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create
    2017/03/03, Youhai.Jiang modified

************************************************************/
//static inline
void aec_emdConstrainStoreDw(aec_core_T *pstAec, int i, int refIndex, aisp_cpx_s32_t dW)
{
    int fftSize = (pstAec->config->frameSize << 1);
    int offset  = refIndex * fftSize;
    int subOff  = 0;

    if (i == 0)
    {
        pstAec->aExtdW0[offset] = dW;
    }
    else
    {
        subOff = fftSize - i;
        COMPLEX_CONJ(pstAec->aExtdW0[offset + subOff], dW);
        pstAec->aExtdW0[offset + i] = dW;
    }
}

/************************************************************
  Function   : aec_emdConstrainUpdateWeight()

  Description: update weight using dw
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create
    2017/03/03, Youhai.Jiang modified

************************************************************/
void aec_emdConstrainUpdateWeight(aec_core_T *pstAec, int tap_index)
{
    int i = 0;
    int iFrmSize = pstAec->config->frameSize;
    int iFftBins = pstAec->fftBins;
    int fftSize = (pstAec->config->frameSize << 1);
    aisp_s16_t shift = 1;  //when shift=1,aec_iff32 will scan all output

    switch (pstAec->config->refNum)
    {
        case 2:
            {
                shift = AISP_TSL_ifftr32(pstAec->fftEngine, (pstAec->aExtdW0 + fftSize), pstAec->aExtdw);
#ifdef USE_HIFI4_OPTIMIZE
                AIPS_TSL_memset(&pstAec->aExtdw[iFrmSize], 0, (iFrmSize << 1));
#else

                for (i = 0; i < iFrmSize; ++i)
                {
                    pstAec->aExtdw[iFrmSize + i] = 0;
                }

#endif
                //FFT
                AISP_TSL_fftr32(pstAec->fftEngine, (pstAec->aExtdw), (pstAec->aExtdW0 + iFftBins));

                if (shift > 0)
                {
                    int loopCnt = iFftBins - (iFftBins & 3);
                    aisp_cpx_s32_t *pExtDw0 = &pstAec->aExtdW0[iFftBins];

                    for (i = 0; i < loopCnt; i += 4)
                    {
                        pExtDw0[i].real <<= shift;
                        pExtDw0[i].imag <<= shift;
                        pExtDw0[i + 1].real <<= shift;
                        pExtDw0[i + 1].imag <<= shift;
                        pExtDw0[i + 2].real <<= shift;
                        pExtDw0[i + 2].imag <<= shift;
                        pExtDw0[i + 3].real <<= shift;
                        pExtDw0[i + 3].imag <<= shift;
                    }

                    for (; i < iFftBins; ++i)
                    {
                        pExtDw0[i].real <<= shift;
                        pExtDw0[i].imag <<= shift;
                    }
                }

                shift = 1;
            }

        case 1:
        default:
            {
                //IFFT
                shift = AISP_TSL_ifftr32(pstAec->fftEngine, (pstAec->aExtdW0), (pstAec->aExtdw));
#ifdef USE_HIFI4_OPTIMIZE
                AISP_TSL_memset(&pstAec->aExtdw[iFrmSize], 0, (iFrmSize << 1));
#else

                for (i = 0; i < iFrmSize; ++i)
                {
                    pstAec->aExtdw[iFrmSize + i] = 0;
                }

#endif
                //FFT
                AISP_TSL_fftr32(pstAec->fftEngine, (pstAec->aExtdw), (pstAec->aExtdW0));

                if (shift > 0)
                {
                    int loopCnt = iFftBins - (iFftBins & 3);
                    aisp_cpx_s32_t *pExtDw0 = &pstAec->aExtdW0[0];

                    for (i = 0; i < loopCnt; i += 4)
                    {
                        pExtDw0[i].real <<= shift;
                        pExtDw0[i].imag <<= shift;
                        pExtDw0[i + 1].real <<= shift;
                        pExtDw0[i + 1].imag <<= shift;
                        pExtDw0[i + 2].real <<= shift;
                        pExtDw0[i + 2].imag <<= shift;
                        pExtDw0[i + 3].real <<= shift;
                        pExtDw0[i + 3].imag <<= shift;
                    }

                    for (; i < iFftBins; ++i)
                    {
                        pExtDw0[i].real <<= shift;
                        pExtDw0[i].imag <<= shift;
                    }
                }

                if (pstAec->clip == 0)
                {
                    aec_emdUpdateWeight(pstAec, tap_index);
                }

                break;
            }
    }
}

extern VOID AISP_TSL_cmac_vec_32x32(S32 *piSrcA, S32 *piSrcB, S32 *piDst, S32 len, S32 shift);

/************************************************************
  Function   : aec_emdProcess()

  Description: Support up to 3 ref.
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create
    2017/03/03, Youhai.Jiang modified

************************************************************/
aisp_cpx_s32_t EST[513];

void aec_emdProcess(aec_core_T *pstAec)
{
    int errSum   = 0;
    int micSum   = 0;
    int i        = 0;
    int j        = 0;
    int offset   = 0;
    int indexTmp = 0;
    int iRefNum  = 0;
    int iFftBins = 0;
    int iFrameSize = 0;
    aisp_s32_t floatTmp = 0;
    aisp_s32_t REFPower = 0;
    aisp_cpx_s32_t *pX3 = NULL;
    //aisp_cpx_s32_t EST;
    aisp_cpx_s32_t xe_0 = {0, 0};
    aisp_cpx_s32_t xe_1 = {0, 0};
    aisp_cpx_s32_t cpxTmp = {0, 0};
    aisp_cpx_s32_t dW[4][3];                    /* pseudo alloc maybe used here*/
    aisp_cpx_s32_t aERRPreNorm = {0, 0};
    aisp_cpx_s32_t aERRPre     = {0, 0};
    aisp_cpx_s32_t *aX[4];
    //aisp_cpx_s32_t acpxTmp[4] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
    short reset = 0;
    pX3 = pstAec->puX_REF;
    iRefNum = pstAec->config->refNum;
    iFftBins = pstAec->fftBins;
    iFrameSize = pstAec->config->frameSize;
    aisp_s32_t *aXPower = *pstAec->aXPower;
#if 1
    for (i = 0; i < iFftBins; ++i)
    {
        EST[i].imag = EST[i].real =0;
    }
    /*default oen ref*/
    AISP_TSL_cmac_vec_32x32((S32 *)pstAec->pX0,(S32 *)pstAec->aW[0], (S32*)EST, iFftBins , AISP_TSL_Q15_SHIFT);
    AISP_TSL_cmac_vec_32x32((S32 *)pstAec->pX1,(S32 *)pstAec->aW[1], (S32*)EST, iFftBins , AISP_TSL_Q15_SHIFT);
    AISP_TSL_cmac_vec_32x32((S32 *)pstAec->pX2,(S32 *)pstAec->aW[2], (S32*)EST, iFftBins , AISP_TSL_Q15_SHIFT);
    AISP_TSL_cmac_vec_32x32((S32 *)pX3,(S32 *)pstAec->aW[3], (S32*)EST, iFftBins, AISP_TSL_Q15_SHIFT);
#endif

    for (i = 0; i < iFftBins; ++i)
    {
#if 0
        EST.real = 0;
        EST.imag = 0;
#endif
        /*
         * Compute frequency domain EST
         * by adding the product of weight and history reference signal
         */
#ifdef USE_HIFI4_OPTIMIZE
        offset = i;

        for (j = 0; j < iRefNum; ++j)
        {
            /*
             * filter:
             * EST = sum(X.*W,2);
             */
            ae_f32x2  c[4];
            ae_f32x2  tw[4];
            ae_f32x2 *X[8];
            aisp_cpx_s32_t tmp[4] MEM_ALIGN(8);
            X[0] = (ae_f32x2 *)&pstAec->aW[0][offset];
            X[1] = (ae_f32x2 *)&pstAec->aW[1][offset];
            X[2] = (ae_f32x2 *)&pstAec->aW[2][offset];
            X[3] = (ae_f32x2 *)&pstAec->aW[3][offset];
            X[4] = (ae_f32x2 *)&pstAec->pX0[offset];
            X[5] = (ae_f32x2 *)&pstAec->pX1[offset];
            X[6] = (ae_f32x2 *)&pstAec->pX2[offset];
            X[7] = (ae_f32x2 *)&pX3[offset];
            c[0] = AE_F32X2_SLAIS(ae_f32x2_loadi(X[0], 0), 8);
            c[1] = AE_F32X2_SLAIS(ae_f32x2_loadi(X[1], 0), 8);
            c[2] = AE_F32X2_SLAIS(ae_f32x2_loadi(X[2], 0), 8);
            c[3] = AE_F32X2_SLAIS(ae_f32x2_loadi(X[3], 0), 8);
            tw[0] = AE_F32X2_SLAIS(ae_f32x2_loadi(X[4], 0), 8);
            tw[1] = AE_F32X2_SLAIS(ae_f32x2_loadi(X[5], 0), 8);
            tw[2] = AE_F32X2_SLAIS(ae_f32x2_loadi(X[6], 0), 8);
            tw[3] = AE_F32X2_SLAIS(ae_f32x2_loadi(X[7], 0), 8);
            c[0] = AE_MULFC32RAS(c[0], tw[0]);
            c[1] = AE_MULFC32RAS(c[1], tw[1]);
            c[2] = AE_MULFC32RAS(c[2], tw[2]);
            c[3] = AE_MULFC32RAS(c[3], tw[3]);
            AE_S32X2_I(c[0], (ae_int32x2 *)&tmp[0], 0);
            AE_S32X2_I(c[1], (ae_int32x2 *)&tmp[1], 0);
            AE_S32X2_I(c[2], (ae_int32x2 *)&tmp[2], 0);
            AE_S32X2_I(c[3], (ae_int32x2 *)&tmp[3], 0);
            EST.real += tmp[0].real + tmp[1].real + tmp[2].real + tmp[3].real;
            EST.imag += tmp[0].imag + tmp[1].imag + tmp[2].imag + tmp[3].imag;
            offset += iFftBins;
        }

#else
        offset = i;

#if 0
        for (j = 0; j < iRefNum; ++j)
        {
            /*
             * filter:
             * EST = sum(X.*W,2);
             */
            acpxTmp[0].real += PSHR((aisp_s64_t)pstAec->pX0[offset].real * pstAec->aW[0][offset].real
                                    - (aisp_s64_t)pstAec->pX0[offset].imag * pstAec->aW[0][offset].imag, AISP_TSL_Q15_SHIFT);
            acpxTmp[0].imag += PSHR((aisp_s64_t)pstAec->pX0[offset].real * pstAec->aW[0][offset].imag
                                    + (aisp_s64_t)pstAec->pX0[offset].imag * pstAec->aW[0][offset].real, AISP_TSL_Q15_SHIFT);
            acpxTmp[1].real += PSHR((aisp_s64_t)pstAec->pX1[offset].real * pstAec->aW[1][offset].real
                                    - (aisp_s64_t)pstAec->pX1[offset].imag * pstAec->aW[1][offset].imag, AISP_TSL_Q15_SHIFT);
            acpxTmp[1].imag += PSHR((aisp_s64_t)pstAec->pX1[offset].real * pstAec->aW[1][offset].imag
                                    + (aisp_s64_t)pstAec->pX1[offset].imag * pstAec->aW[1][offset].real, AISP_TSL_Q15_SHIFT);
            acpxTmp[2].real += PSHR((aisp_s64_t)pstAec->pX2[offset].real * pstAec->aW[2][offset].real
                                    - (aisp_s64_t)pstAec->pX2[offset].imag * pstAec->aW[2][offset].imag, AISP_TSL_Q15_SHIFT);
            acpxTmp[2].imag += PSHR((aisp_s64_t)pstAec->pX2[offset].real * pstAec->aW[2][offset].imag
                                    + (aisp_s64_t)pstAec->pX2[offset].imag * pstAec->aW[2][offset].real, AISP_TSL_Q15_SHIFT);
            acpxTmp[3].real += PSHR((aisp_s64_t)pX3[offset].real * pstAec->aW[3][offset].real
                                    - (aisp_s64_t)pX3[offset].imag * pstAec->aW[3][offset].imag, AISP_TSL_Q15_SHIFT);
            acpxTmp[3].imag += PSHR((aisp_s64_t)pX3[offset].real * pstAec->aW[3][offset].imag
                                    + (aisp_s64_t)pX3[offset].imag * pstAec->aW[3][offset].real, AISP_TSL_Q15_SHIFT);
            offset += iFftBins;
        }

        EST.real += acpxTmp[0].real + acpxTmp[1].real + acpxTmp[2].real + acpxTmp[3].real;
        EST.imag += acpxTmp[0].imag + acpxTmp[1].imag + acpxTmp[2].imag + acpxTmp[3].imag;
        acpxTmp[0].real = 0;
        acpxTmp[0].imag = 0;
        acpxTmp[1].real = 0;
        acpxTmp[1].imag = 0;
        acpxTmp[2].real = 0;
        acpxTmp[2].imag = 0;
        acpxTmp[3].real = 0;
        acpxTmp[3].imag = 0;
#endif

#endif

        if (i == 0)
        {
            pstAec->aMIC[0].real = EST[i].real;
            pstAec->aMIC[0].imag = EST[i].imag;
        }
        else
        {
            indexTmp = (iFrameSize << 1) - i;
            COMPLEX_CONJ(pstAec->aMIC[indexTmp], EST[i]);
            pstAec->aMIC[i].real = EST[i].real;
            pstAec->aMIC[i].imag = EST[i].imag;
        }
    }

    /* time domain EST, reuse MIC and MICFrameWide to save memory
     * since in emd mode we do not need frequency domain mic signal
     */
    //aec_ifft32(pstAec->fftEngine, pstAec->aMIC, pstAec->aEstFrameWide,&shift);
    AISP_TSL_ifftr32(pstAec->fftEngine, pstAec->aMIC, pstAec->aEstFrameWide);
#ifdef USE_HIFI4_OPTIMIZE

    for (i = 0; i < iFrameSize; i += 4)
    {
        aisp_s16_t tmp0[4];
        aisp_s16_t tmp1[4];
        aisp_s16_t *pErrFrameWide = &pstAec->aErrFrameWide[iFrameSize];
        aisp_s16_t *pEstFrameWide = &pstAec->aEstFrameWide[iFrameSize];
        ae_f16x4   *pUphalfErrFrmWide;
        pUphalfErrFrmWide = (ae_f16x4 *)&pstAec->aErrFrameWide[i];
        *(ae_int16x4 *)pUphalfErrFrmWide = AE_ZERO16();
        *((ae_int16x4 *)&pErrFrameWide[i]) =  AE_SUB16(*((ae_int16x4 *)&pstAec->aMicFrameOld[i]),
                                              *((ae_int16x4 *)&pEstFrameWide[i]));
        {
            ae_f16x4 *pTmp0 = (ae_f16x4 *)tmp0;
            ae_f16x4 *pTmp1 = (ae_f16x4 *)tmp1;
            ae_f16x4 *pErrFrmWide = (ae_f16x4 *)&pErrFrameWide[i];
            ae_f16x4 *pMicFrameOld = (ae_f16x4 *)&pstAec->aMicFrameOld[i];
            *pTmp0 = AE_ABS16S(*pErrFrmWide);
            *pTmp1 = AE_ABS16S(*pMicFrameOld);
            errSum += tmp0[0] + tmp0[1] + tmp0[2] + tmp0[3];
            micSum += tmp1[0] + tmp1[1] + tmp1[2] + tmp1[3];
        }
    }

#else

    for (i = 0; i < iFrameSize; ++i)
    {
        int secondHalfOffset = i + iFrameSize;
        pstAec->aErrFrameWide[i] = 0;
        pstAec->aErrFrameWide[secondHalfOffset] = pstAec->aMicFrameOld[i] - pstAec->aEstFrameWide[secondHalfOffset];
        errSum += AISP_TSL_ABS(pstAec->aErrFrameWide[secondHalfOffset]);
        micSum += AISP_TSL_ABS(pstAec->aMicFrameOld[i]);
    }

#endif
#ifdef USE_HIFI4_OPTIMIZE
    {
        aisp_s16_t *pEst16 = &pstAec->aEstFrameWide[iFrameSize];
        aisp_s16_t *pHanning = &pstAec->hanningWindowSqrt[iFrameSize];
        aisp_s32_t tmp[4];
        aisp_s32_t tmp1[4];
        ae_int32x4 *pAeTmp1 = (ae_int32x4 *)tmp;
        ae_int32x4 *pAeTmp2 = (ae_int32x4 *)tmp1;
        ae_int16x4 *pAePtr1;
        ae_int16x4 *pAePtr2;
        ae_int16x4 *pAePtr3;
        ae_int16x4 *pAePtr4;
        ae_int16x4 *pAePtr5;
        ae_f16x4  *pAeX1;
        ae_f32x2  *pAeY1 = (ae_f32x2 *)&tmp[0];
        ae_f32x2  *pAeY2 = (ae_f32x2 *)&tmp[2];
        ae_f32x2  *pAeY3 = (ae_f32x2 *)&tmp1[0];
        ae_f32x2  *pAeY4 = (ae_f32x2 *)&tmp1[2];

        for (i = 0; i < iFrameSize; i += 4)
        {
            pAePtr1 = (ae_int16x4 *) &pstAec->hanningWindowSqrt[i];
            pAePtr2 = (ae_int16x4 *) &pstAec->aEstOld[i];
            pAePtr3 = (ae_int16x4 *) &pHanning[i];
            pAePtr4 = (ae_int16x4 *) &pEst16[i];
            pAePtr5 = (ae_int16x4 *) &pstAec->aEstOld[i];
            pAeX1 = (ae_f16x4 *)&pstAec->aEstFrameWide[i];
            *pAeTmp1 = AE_MUL16X4_vector(*pAePtr1, *pAePtr2);
            /* update aErrFrameWide */
            *pAeX1 = AE_ROUND16X4F32SASYM(*pAeY1, *pAeY2);
            *pAeX1 = AE_SLAA16S(*pAeX1, 1);
            /* update aErrPreOld */
            AE_S16X4_I(*pAePtr4, pAePtr2, 0);
            *pAeTmp2 = AE_MUL16X4_vector(*pAePtr3, *pAePtr4);
            /* update pErr16 */
            *(ae_f16x4 *)pAePtr4 = AE_ROUND16X4F32SASYM(*pAeY3, *pAeY4);
            *(ae_f16x4 *)pAePtr4 = AE_SLAA16S(*(ae_f16x4 *)pAePtr4, 1);
        }
    }
#else
    offset = iFrameSize;

    for (i = 0; i < iFrameSize; ++i)
    {
        pstAec->aEstFrameWide[i] = PSHR((aisp_s32_t)pstAec->hanningWindowSqrt[i] * pstAec->aEstOld[i], AISP_TSL_Q15_SHIFT);
        pstAec->aEstOld[i] = pstAec->aEstFrameWide[offset];
        pstAec->aEstFrameWide[offset] = PSHR((aisp_s32_t)pstAec->hanningWindowSqrt[offset] * pstAec->aEstOld[i],
                                             AISP_TSL_Q15_SHIFT);
        offset++;
    }

#endif

    /* errSum > 1.5 * micSum */
    if ((errSum << 1) > ((micSum << 1) + micSum))
    {
        pstAec->resetCount++;
    }
    else
    {
        pstAec->resetCount = 0;
    }

    if (pstAec->resetCount > 150)
    {
        reset = 1;
        pstAec->resetCount = 0;
    }

    /* aMIC currently holds the data of ERR */
    //aec_fft(pstAec->fftEngine, pstAec->aErrFrameWide, (aisp_s32_t *)pstAec->aERR);
    AISP_TSL_fftr32(pstAec->fftEngine, pstAec->aErrFrameWide, pstAec->aERR);

    /* update W filter */
    /* mean(diagR)/PK > sgmS2thr */
    if (pstAec->confirmUpdate)
    {
        aX[0] = pstAec->pX0;
        aX[1] = pstAec->pX1;
        aX[2] = pstAec->pX2;
        aX[3] = pX3;

        switch (iRefNum)
        {
            case 2:
                {
#ifdef USE_HIFI4_OPTIMIZE
                    aisp_cpx_s32_t  tmp[2];
                    aisp_s64_t  tmp1, tmp2, tmp3, tmp4;
                    aisp_cpx_s32_t cpxTmp1, cpxTmp2;
                    ae_int64   aeRlt1, aeRlt2, aeRlt3, aeRlt4;

                    for (j = 0; j < AEC_TAPS; ++j)
                    {
                        for (i = 0; i < iFftBins; ++i)
                        {
                            COMPLEX_CONJ_MUL_HIFI4(xe_0, pstAec->aERR[i], aX[j][i]);
                            COMPLEX_CONJ_MUL_HIFI4(xe_1, pstAec->aERR[i], aX[j][i + iFftBins]);
                            COMPLEX_MUL_HIFI4(cpxTmp1, xe_1, pstAec->pIR12[i]);
                            COMPLEX_CONJ_MUL_HIFI4(cpxTmp2, xe_0, pstAec->pIR12[i]);
                            COMPLEX_MUL_FLOAT(tmp[0], xe_0, pstAec->pIR11[i]);
                            COMPLEX_MUL_FLOAT(tmp[1], xe_1, pstAec->pIR22[i]);
                            *(ae_int32x2 *)&dW[j][0] = AE_ADD32(*(ae_int32x2 *)&tmp[0], *(ae_int32x2 *)&cpxTmp1);
                            *(ae_int32x2 *)&dW[j][1] = AE_ADD32(*(ae_int32x2 *)&tmp[1], *(ae_int32x2 *)&cpxTmp2);
                            aec_emdConstrainStoreDw(pstAec, i, 0, dW[j][0]);
                            aec_emdConstrainStoreDw(pstAec, i, 1, dW[j][1]);
                        }

                        if (pstAec->config->constrain)
                        {
                            aec_emdConstrainUpdateWeight(pstAec, j);
                        }
                        else
                        {
                            aec_emdUpdateWeight(pstAec, j);
                        }
                    }

#else

                    for (j = 0; j < AEC_TAPS; ++j)
                    {
                        for (i = 0; i < iFftBins; ++i)
                        {
                            xe_0.real     = PSHR((aisp_s64_t)pstAec->aERR[i].real * aX[j][i].real + (aisp_s64_t)pstAec->aERR[i].imag *
                                                 aX[j][i].imag, AISP_TSL_Q15_SHIFT);
                            xe_0.imag     = PSHR((aisp_s64_t)pstAec->aERR[i].imag * aX[j][i].real - (aisp_s64_t)pstAec->aERR[i].real *
                                                 aX[j][i].imag, AISP_TSL_Q15_SHIFT);
                            indexTmp      = i + iFftBins;
                            xe_1.real     = PSHR((aisp_s64_t)pstAec->aERR[i].real * aX[j][indexTmp].real +
                                                 (aisp_s64_t)pstAec->aERR[i].imag * aX[j][indexTmp].imag, AISP_TSL_Q15_SHIFT);
                            xe_1.imag     = PSHR((aisp_s64_t)pstAec->aERR[i].imag * aX[j][indexTmp].real -
                                                 (aisp_s64_t)pstAec->aERR[i].real * aX[j][indexTmp].imag, AISP_TSL_Q15_SHIFT);
                            dW[j][0].real = PSHR((aisp_s64_t)xe_0.real * pstAec->pIR11[i] + (aisp_s64_t)xe_1.real * pstAec->pIR12[i].real -
                                                 (aisp_s64_t)xe_1.imag * pstAec->pIR12[i].imag,  AISP_TSL_Q15_SHIFT);
                            dW[j][0].imag = PSHR((aisp_s64_t)xe_0.imag * pstAec->pIR11[i] + (aisp_s64_t)xe_1.real * pstAec->pIR12[i].imag +
                                                 (aisp_s64_t)xe_1.imag * pstAec->pIR12[i].real,  AISP_TSL_Q15_SHIFT);
                            dW[j][1].real = PSHR((aisp_s64_t)xe_0.real * pstAec->pIR12[i].real + (aisp_s64_t)xe_0.imag * pstAec->pIR12[i].imag +
                                                 (aisp_s64_t)xe_1.real * pstAec->pIR22[i] , AISP_TSL_Q15_SHIFT);
                            dW[j][1].imag = PSHR((aisp_s64_t)xe_0.imag * pstAec->pIR12[i].real - (aisp_s64_t)xe_0.real * pstAec->pIR12[i].imag +
                                                 (aisp_s64_t)xe_1.imag * pstAec->pIR22[i] , AISP_TSL_Q15_SHIFT);
                            aec_emdConstrainStoreDw(pstAec, i, 0, dW[j][0]);
                            aec_emdConstrainStoreDw(pstAec, i, 1, dW[j][1]);
                        }

                        if (pstAec->config->constrain)
                        {
                            aec_emdConstrainUpdateWeight(pstAec, j);
                        }
                        else
                        {
                            aec_emdUpdateWeight(pstAec, j);
                        }
                    }

#endif
                    break;
                }

            case 1:
            default:
                {
                    for (j = 0; j < AEC_TAPS; ++j)
                    {
                        for (i = 0; i < iFftBins; ++i)
                        {
#ifdef USE_HIFI4_OPTIMIZE
                            COMPLEX_CONJ_MUL_HIFI4(xe_0, pstAec->aERR[i], aX[j][i]);
                            MULT32_32_P15_OPT(dW[j][0].real, xe_0.real, pstAec->pIR11[i]);
                            MULT32_32_P15_OPT(dW[j][0].imag, xe_0.imag, pstAec->pIR11[i]);
#else
                            COMPLEX_CONJ_MUL(xe_0, pstAec->aERR[i], aX[j][i]);
                            dW[j][0].real = MULT32_32_P15(xe_0.real, pstAec->pIR11[i]);
                            dW[j][0].imag = MULT32_32_P15(xe_0.imag, pstAec->pIR11[i]);
#endif
                            aec_emdConstrainStoreDw(pstAec, i, 0, dW[j][0]);
                        }

                        if (pstAec->config->constrain)
                        {
                            aec_emdConstrainUpdateWeight(pstAec, j);
                        }
                        else
                        {
                            aec_emdUpdateWeight(pstAec, j);
                        }
                    }

                    break;
                }
        }
    }

    /* overlap and window the err and est */
#ifdef USE_HIFI4_OPTIMIZE
    aisp_s16_t *pErr16 = &pstAec->aErrFrameWide[iFrameSize];
    aisp_s16_t *pHanning = &pstAec->hanningWindowSqrt[iFrameSize];
    aisp_s32_t tmp[4];
    aisp_s32_t tmp1[4];
    ae_int32x4 *pAeTmp1 = (ae_int32x4 *)tmp;
    ae_int32x4 *pAeTmp2 = (ae_int32x4 *)tmp1;
    ae_int16x4 *pAePtr1;
    ae_int16x4 *pAePtr2;
    ae_int16x4 *pAePtr3;
    ae_int16x4 *pAePtr4;
    ae_int16x4 *pAePtr5;
    ae_f16x4  *pAeX1;
    ae_f32x2  *pAeY1 = (ae_f32x2 *)&tmp[0];
    ae_f32x2  *pAeY2 = (ae_f32x2 *)&tmp[2];
    ae_f32x2  *pAeY3 = (ae_f32x2 *)&tmp1[0];
    ae_f32x2  *pAeY4 = (ae_f32x2 *)&tmp1[2];

    for (i = 0; i < iFrameSize; i += 4)
    {
        pAePtr1 = (ae_int16x4 *) &pstAec->hanningWindowSqrt[i];
        pAePtr2 = (ae_int16x4 *) &pstAec->aErrPreOld[i];
        pAePtr3 = (ae_int16x4 *) &pHanning[i];
        pAePtr4 = (ae_int16x4 *) &pErr16[i];
        pAePtr5 = (ae_int16x4 *) &pstAec->aErrPreOld[i];
        pAeX1 = (ae_f16x4 *)&pstAec->aErrFrameWide[i];
        *pAeTmp1 = AE_MUL16X4_vector(*pAePtr1, *pAePtr2);
        /* update aErrFrameWide */
        *pAeX1 = AE_ROUND16X4F32SASYM(*pAeY1, *pAeY2);
        *pAeX1 = AE_SLAA16S(*pAeX1, 1);
        /* update aErrPreOld */
        AE_S16X4_I(*pAePtr4, pAePtr2, 0);
        *pAeTmp2 = AE_MUL16X4_vector(*pAePtr3, *pAePtr4);
        /* update pErr16 */
        *(ae_f16x4 *)pAePtr4 = AE_ROUND16X4F32SASYM(*pAeY3, *pAeY4);
        *(ae_f16x4 *)pAePtr4 = AE_SLAA16S(*(ae_f16x4 *)pAePtr4, 1);
    }

#else
    offset = iFrameSize;

    for (i = 0; i < iFrameSize; ++i)
    {
        pstAec->aErrFrameWide[i] = PSHR((aisp_s32_t)pstAec->hanningWindowSqrt[i] * pstAec->aErrPreOld[i], AISP_TSL_Q15_SHIFT);
        pstAec->aErrPreOld[i] = pstAec->aErrFrameWide[offset];
        pstAec->aErrFrameWide[offset] = PSHR((aisp_s32_t)pstAec->hanningWindowSqrt[offset] * pstAec->aErrPreOld[i],
                                             AISP_TSL_Q15_SHIFT);
        offset++;
    }

#endif
    /* aMIC hold the value of ERR */
    //aec_fft(pstAec->fftEngine, pstAec->aErrFrameWide, pstAec->aMIC);
    AISP_TSL_fftr32(pstAec->fftEngine, pstAec->aErrFrameWide, pstAec->aMIC);

    /* compute the power spectrum for est and err */
    for (i = 0; i < iFftBins; ++i)
    {
        /* Update Delta */
        pstAec->aDelta[i] = (pstAec->aERRPower[i] + pstAec->aXPowerSumSmooth[i]) * DELTA_GAIN;

        /* restrict Delta */

        if (pstAec->aDelta[i] <= pstAec->aDeltaFloor[i])
        {
            pstAec->aDelta[i] = pstAec->aDeltaFloor[i];
        }
        else
            if (pstAec->aDelta[i] > pstAec->MaxDelta)
            {
                pstAec->aDelta[i] = pstAec->MaxDelta;
            }

#ifdef USE_HIFI4_OPTIMIZE

        if (pstAec->idx == 0)
        {
            for (j = 0; j < iRefNum; ++j)
            {
                offset = j * iFftBins + i;
                /* calc X_POWER_SUM */
                pstAec->aXPowerSum[i] -= pstAec->aXPower[offset];
                aisp_s32_t tmp[2];
                ae_int64 aeResult1 = AE_ZERO64();
                AE_MULAAD32_HH_LL(aeResult1, *(ae_int32x2 *)&pX3[offset], *(ae_int32x2 *)&pX3[offset]);
                aeResult1 = AE_SLAA64(aeResult1, AISP_Q17_SHIFT);
                *(ae_f32x2 *)tmp = AE_ROUND32F64SASYM(aeResult1);
                REFPower = tmp[0];
                pstAec->aXPower[offset] = PSHR((aisp_s64_t)(AISP_TSL_Q15_ONE_MINI - pstAec->config->lamda_xPower) *
                                               pstAec->aXPower[offset] + (aisp_s64_t)pstAec->config->lamda_xPower * REFPower, AISP_TSL_Q15_SHIFT);
                pstAec->aXPowerSum[i] += pstAec->aXPower[offset];
            }

            if (pstAec->aXPowerSum[i] > pstAec->aXPowerSumSmooth[i])
            {
                pstAec->aXPowerSumSmooth[i] = PSHR(((aisp_s64_t)pstAec->aXPowerSumSmooth[i] * (AISP_TSL_Q15_ONE_MINI -
                                                    pstAec->config->upStep))
                                                   + ((aisp_s64_t)pstAec->aXPowerSum[i] * pstAec->config->upStep), AISP_TSL_Q15_SHIFT);
            }
            else
            {
                pstAec->aXPowerSumSmooth[i] = PSHR(((aisp_s64_t)pstAec->aXPowerSumSmooth[i] * (AISP_TSL_Q15_ONE_MINI -
                                                    pstAec->config->downStep))
                                                   + ((aisp_s64_t)pstAec->aXPowerSum[i] * pstAec->config->downStep), AISP_TSL_Q15_SHIFT);
            }
        }

#else

        /* smooth X_POWER_SUM */
        if (pstAec->idx == 0)
        {
            offset = i;

            for (j = 0; j < iRefNum; ++j)
            {
                /* calc X_POWER_SUM */
                pstAec->aXPowerSum[i] -= aXPower[offset];
                REFPower = PSHR((aisp_s64_t)pX3[offset].real * pX3[offset].real + (aisp_s64_t)pX3[offset].imag * pX3[offset].imag,
                                AISP_TSL_Q15_SHIFT);
                aXPower[offset] = PSHR((aisp_s64_t)(AISP_TSL_Q15_ONE_MINI - pstAec->config->lamda_xPower) *
                                               aXPower[offset] + (aisp_s64_t)pstAec->config->lamda_xPower * REFPower, AISP_TSL_Q15_SHIFT);
                pstAec->aXPowerSum[i] += aXPower[offset];
                offset += iFftBins;
            }

            if (pstAec->aXPowerSum[i] > pstAec->aXPowerSumSmooth[i])
            {
                pstAec->aXPowerSumSmooth[i] = PSHR(((aisp_s64_t)pstAec->aXPowerSumSmooth[i] * (AISP_TSL_Q15_ONE_MINI -
                                                    pstAec->config->upStep))
                                                   + ((aisp_s64_t)pstAec->aXPowerSum[i] * pstAec->config->upStep), AISP_TSL_Q15_SHIFT);
            }
            else
            {
                pstAec->aXPowerSumSmooth[i] = PSHR(((aisp_s64_t)pstAec->aXPowerSumSmooth[i] * (AISP_TSL_Q15_ONE_MINI -
                                                    pstAec->config->downStep))
                                                   + ((aisp_s64_t)pstAec->aXPowerSum[i] * pstAec->config->downStep), AISP_TSL_Q15_SHIFT);
            }
        }

#endif
    }

    /* + aXPower fifo. Chengfei.Song, 2020/03/17 15:34:43*/
    if (pstAec->idx == 0)
    {
        for(i = 0 ; i < AEC_TAPS - 1; i++)
        {
            pstAec->aXPower[i] = pstAec->aXPower[i + 1];
        }
        pstAec->aXPower[i] = aXPower;
    }
    /* - aXPower fifo. Chengfei.Song, 2020/03/17 15:34:43*/

#ifdef USE_HIFI4_OPTIMIZE

    if (pstAec->config->nlp == 1)
    {
        for (i = 0; i < iFftBins; ++i)
        {
            aERRPre.real = pstAec->aMIC[i].real;
            aERRPre.imag = pstAec->aMIC[i].imag;
            floatTmp = pstAec->aXPowerSum[i] + pstAec->aDelta[i];
            aERRPreNorm.real = PDIV64(SHL64(aERRPre.real, 15), floatTmp);
            aERRPreNorm.imag = PDIV64(SHL64(aERRPre.imag, 15), floatTmp);
            COMPLEX_MUL_FLOAT(cpxTmp, aERRPreNorm, (pstAec->aXPowerSum[i] >> 1));
            COMPLEX_SUB(pstAec->aERR[i], aERRPre, cpxTmp);
        }

        AISP_TSL_cpxVecPower(pstAec->aERRPower, pstAec->aERR, iFftBins);
        AISP_TSL_fftr32(pstAec->fftEngine, pstAec->aEstFrameWide, pstAec->aEST);
    }
    else   //when nlp=0,the EST hold the ERRPre.
    {
        for (i = 0; i < iFftBins; ++i)
        {
            pstAec->aEST[i].real = pstAec->aMIC[i].real;
            pstAec->aEST[i].imag = pstAec->aMIC[i].imag;
        }
    }

#else

    if (pstAec->config->nlp == 1)
    {
        for (i = 0; i < iFftBins; ++i)
        {
            aERRPre.real = pstAec->aMIC[i].real;
            aERRPre.imag = pstAec->aMIC[i].imag;
            floatTmp = pstAec->aXPowerSum[i] + pstAec->aDelta[i];
            aERRPreNorm.real = PDIV64(SHL64(aERRPre.real, 15), floatTmp);
            aERRPreNorm.imag = PDIV64(SHL64(aERRPre.imag, 15), floatTmp);
            COMPLEX_MUL_FLOAT(cpxTmp, aERRPreNorm, pstAec->aXPowerSum[i]);
            COMPLEX_MUL_FLOAT(cpxTmp, cpxTmp, GAMMA_PRE);
            COMPLEX_SUB(pstAec->aERR[i], aERRPre, cpxTmp);
            pstAec->aERRPower[i] = PSHR((aisp_s64_t)pstAec->aERR[i].real * pstAec->aERR[i].real
                                        + (aisp_s64_t)pstAec->aERR[i].imag * pstAec->aERR[i].imag, AISP_TSL_Q15_SHIFT);
        }

        AISP_TSL_fftr32(pstAec->fftEngine, pstAec->aEstFrameWide, pstAec->aEST);
    }
    else   //when nlp=0,the EST hold the ERRPre.
    {
        for (i = 0; i < iFftBins; ++i)
        {
            pstAec->aEST[i].real = pstAec->aMIC[i].real;
            pstAec->aEST[i].imag = pstAec->aMIC[i].imag;
        }
    }

#endif

    if (reset)
    {
        //aec_reset(pstAec);
        AISP_TSL_memset(pstAec->aW[0], 0, (iFftBins * iRefNum * sizeof(aisp_cpx_s32_t)));
        AISP_TSL_memset(pstAec->aW[1], 0, (iFftBins * iRefNum * sizeof(aisp_cpx_s32_t)));
        AISP_TSL_memset(pstAec->aW[2], 0, (iFftBins * iRefNum * sizeof(aisp_cpx_s32_t)));
        AISP_TSL_memset(pstAec->aW[3], 0, (iFftBins * iRefNum * sizeof(aisp_cpx_s32_t)));

        if (pstAec->idx == 0)
        {
            switch (iRefNum)
            {
                case 2:
                    {
                        AISP_TSL_memset(pstAec->aR12, 0, iFftBins * sizeof(aisp_cpx_s32_t));
                        AISP_TSL_memset(pstAec->aR22, 0, iFftBins * sizeof(aisp_s32_t));
                    }

                case 1:
                default:
                    {
                        AISP_TSL_memset(pstAec->aR11, 0, iFftBins * sizeof(aisp_s32_t));
                    }
            }
        }
    }
}

/************************************************************
  Function   : aec_muteProcess()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create
    2017/03/03, Youhai.Jiang create

************************************************************/
void aec_muteProcess(aec_core_T *pstAec, short *pErr)
{
    /*nothing to do*/
}

/************************************************************
  Function   : aec_updateErr()

  Description: Update frequency ERR signal.
               Call nlp process.
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create
    2017/03/03, Youhai.Jiang modified

************************************************************/
void aec_updateErr(aec_core_T *pstAec)
{
    int i              = 0;
    int mute_time_ms   = pstAec->config->mute_time_ms;
    int fftBins        = pstAec->fftBins;
    int sampleRate     = pstAec->config->fs;
    int mute_frame_num = 0;
    /*
     * If vad detection is turned on and there is no vadResult,
     * use frequency domain Micphone signal to replace ERR.
     * In the condition of emd, We do not need the frequency domain
     * mic signal in the first MDF process. So we need to do
     * the fft here.
     */

    if (pstAec->config->vad && (pstAec->vadResult == 0))
    {
        if (pstAec->config->emd)
        {
            AISP_TSL_fftr32(pstAec->fftEngine, pstAec->aMicFrameWide, pstAec->aMIC);
        }

        for (i = 0; i < fftBins; ++i)
        {
            pstAec->aERR[i].real = pstAec->aMIC[i].real;
            pstAec->aERR[i].imag = pstAec->aMIC[i].imag;
        }

        return ;
    }

    if (pstAec->config->mute)
    {
        mute_frame_num = mute_time_ms * sampleRate / (1000 * fftBins);

        if (pstAec->mute_Frames < mute_frame_num)
        {
            pstAec->mute_Frames ++;
            return ;
        }
    }

    if (pstAec->config->nlp)
    {
        /* call nlp function to compute the gain and pframe. */
        naes_feed(pstAec);
    }
    else
    {
        /* nlp is turned off */
        for (i = 0; i < fftBins; ++i)
        {
            /*the EST holds the ERRPre*/
            pstAec->aERR[i].real = pstAec->aEST[i].real;
            pstAec->aERR[i].imag = pstAec->aEST[i].imag;
        }
    }

    return ;
}

AEC_VISIBLE_ATTR void aec_api_paramSet(aec_coreConf_T *config, short argc)
{
    config->frameSize      = 512;
    config->fs             = 16000;
    config->refNum         = argc - 3;
    config->index          = 0;
    config->vad            = 1;
    config->hdr            = 0;
    config->mute           = 0;
    config->nlp            = 1;
    config->constrain      = 1;
    config->mute_time_ms   = 300;
    config->sil2spe        = 1;
    config->spe2sil        = 500;
    // config->deltafloorflag = 1;
    config->xCoherence     = 1;
    config->gainFlag       = 0;
    config->DcRemoveFlag   = 0;
    config->lembda         = 32112; /* AISP_FIXED16(0.98) */
    config->deltamax       = 4915;  /* AISP_FIXED16(0.15) */
    config->Mu             = 13107; /* AISP_FIXED16(0.4) */
    config->Noise_floor    = 0;     /* AISP_FIXED32(0.0000001) */
    config->amth           = 15;
    config->preemph        = 29491; // round(0.9*32768)
    config->lembda_mu      = 32440; // round(0.99*32768)
    config->miu            = AISP_TSL_Q15_ONE_MINI;
    config->upStep         = 524;   /* AISP_FIXED16(0.5 * config->frameSize / config->fs) */
    config->downStep       = 3;     /* AISP_FIXED16(0.0031 * config->frameSize / config->fs) */
    config->scaft          = 16384; /* AISP_FIXED16(0.5) */
    config->lamda_x        = 29490; /* AISP_FIXED16(0.9) */
    config->sgmS2thr       = 13;    /* AISP_FIXED16(0.0004) */
    config->lamda_xPower   = 9830;  /* AISP_FIXED16(0.3) */
    config->outGain        = 1;

    if (config->refNum > 1)
    {
        config->emd = 1;
    }
    else
    {
        config->emd = 0;
    }
}

AEC_VISIBLE_ATTR void aec_api_paramReset(aec_coreConf_T *config, aec_coreConfReset_T *newconfig)
{
    config->vad = newconfig->vad;
    config->wavChan = newconfig->wavChan;
    config->micNum = newconfig->micNum;
    config->refNum = newconfig->refNum;
    config->hdr = newconfig->hdr;
    config->nlp = newconfig->nlp;
    config->emd = newconfig->emd;
    config->freqOut = newconfig->freqOut;
    config->DcRemoveFlag = newconfig->DcRemoveFlag;
    config->gainFlag = newconfig->gainFlag;
    config->outGain = newconfig->outGain;
}



/************************************************************
  Function   : aec_new()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create
    2017/03/03, youhai.Jiang modified

************************************************************/
AEC_VISIBLE_ATTR void *aec_new(void *filename)
{
    aec_coreConf_T *config = (aec_coreConf_T *)filename;
    int i      = 0;
    int nfft   = config->frameSize << 1;
    int refNum = config->refNum;
    size_t sizeComplexFftbins;
    size_t sizeFloatFftbins;
    size_t sizeFloatFrame;
    size_t sizeComplexWide;
    size_t sizeFloatWide;
    aec_core_T *pstAec   = (aec_core_T *)AISP_MEM_CALLOC(1, sizeof(aec_core_T));
    naes_core_T *pstNAes = (naes_core_T *)AISP_MEM_CALLOC(1, sizeof(naes_core_T));
    pstAec->pstNAes      = pstNAes;
    pstNAes->frameSize   = config->frameSize;
    pstNAes->sampleRate  = config->fs;
    pstNAes->fftBins     = pstNAes->frameSize + 1;
    pstNAes->Noise_floor = config->Noise_floor;

    if (NULL == pstAec)
    {
        goto FAIL;
    }

    pstAec->config         = config;
    pstAec->idx            = config->index;
    pstAec->fftBins        = config->frameSize + 1;
    pstAec->MIUQUOTE       = PSHR_POSITIVE(((aisp_s32_t)pstAec->config->miu * (AISP_TSL_Q15_ONE_MINI -
                                            pstAec->config->lembda)),
                                           AISP_TSL_Q15_SHIFT);
    pstAec->hanningWindowSqrt = AISP_TSL_hanningWindowWithSqrt(nfft);

    if (NULL == pstAec->hanningWindowSqrt)
    {
        goto HANNING_WIN_CREATE_FAIL;
    }

    if (pstAec->idx == 0)
    {
#ifndef JL_AEC_OPTIMIZATION_FFT
        pstAec->fftEngine = AISP_TSL_fft32_init(nfft);

        if (NULL == pstAec->fftEngine)
        {
            goto FFT_ALLOC_FAIL;
        }
#endif // JL_AEC_OPTIMIZATION_FFT
    }

    pstAec->fftScalar    = nfft;
    pstAec->mute_Frames  = 0;

    if (pstAec->config->hdr == 0)
    {
        pstAec->maxVal = 1;
    }
    else
    {
        pstAec->maxVal = 32767 << 6;  /*  normalized */
    }

    pstAec->MinDelta = 241;         //32bit Q15
    pstAec->MaxDelta = 68717380;    //32bit Q15
    // pstAec->deltafloorSwitch = 1;
    pstAec->resetCount = 0;
    pstAec->aDeltaFloor = (aisp_s16_t *)deltaFloor_aec;

    /*
     * 20170109 bug fixed: convergence is too slow
     * memXold is used to store the last point of the frame before
     */
    for (i = 0; i < pstAec->config->micNum; i++)
    {
        pstAec->memXold[i] = 0; /* AISP_FIXED16(0.0f) */
        pstAec->memEold[i] = 0; /* AISP_FIXED16(0.0f) */
    }

    /*
     * memX is used to store the mic value of the current frame
     * memD is used to store the ref value of the current frame
     */
    pstAec->memX = 0;   /* AISP_FIXED16(0.0f) */
    pstAec->memE = 0;   /* AISP_FIXED16(0.0f) */
    pstAec->memD[0] = 0;    /* AISP_FIXED16(0.0f) */
    pstAec->memD[1] = 0;    /* AISP_FIXED16(0.0f) */
    pstAec->memDold[0] = 0; /* AISP_FIXED16(0.0f) */
    pstAec->memDold[1] = 0; /* AISP_FIXED16(0.0f) */
    sizeComplexFftbins = pstAec->fftBins * sizeof(aisp_cpx_s32_t);
    sizeFloatFftbins   = pstAec->fftBins * sizeof(aisp_s32_t);
    sizeFloatFrame     = pstAec->config->frameSize * sizeof(aisp_s16_t);
    sizeComplexWide    = (pstAec->config->frameSize << 1) * sizeof(aisp_cpx_s32_t);
    sizeFloatWide      = (pstAec->config->frameSize << 1) * sizeof(aisp_s16_t);
#ifndef AISP_SHARE_MEM
//    AISP_TSL_memAllocator_T * pstAllocator = NULL;
//    pstAllocator = AISP_TSL_mem_allocatorInit();
    pstNAes->pPs_old    = AISP_MEM_CALLOC(1, sizeFloatFftbins);
    pstNAes->PPyy       = AISP_MEM_CALLOC(1, sizeFloatFftbins);
    pstNAes->PPee       = AISP_MEM_CALLOC(1, sizeFloatFftbins);
    pstNAes->PPey       = AISP_MEM_CALLOC(1, sizeComplexFftbins);
    pstAec->aERRPower   = AISP_MEM_CALLOC(1, sizeFloatFftbins);
    pstAec->aW[0]      = AISP_MEM_CALLOC(1, sizeComplexFftbins * refNum);
    pstAec->aW[1]      = AISP_MEM_CALLOC(1, sizeComplexFftbins * refNum);
    pstAec->aW[2]      = AISP_MEM_CALLOC(1, sizeComplexFftbins * refNum);
    pstAec->aW[3]      = AISP_MEM_CALLOC(1, sizeComplexFftbins * refNum);
    pstAec->aMicFrameOld  = AISP_MEM_CALLOC(1, sizeFloatFrame);
    pstAec->aERR          = AISP_MEM_CALLOC(1, sizeComplexFftbins);
    pstAec->aMicFrameWide = AISP_MEM_CALLOC(1, sizeFloatWide);

    /* idx used to indicate whether to alloc space */
    if (pstAec->idx == 0)
    {
        pstAec->clip = 0;
        pstAec->maxabsRef = 0;  /* AISP_FIXED16(0.0f) */
        pstAec->aEstFrameWide = AISP_MEM_CALLOC(1, sizeFloatWide);
        pstAec->aErrFrameWide = AISP_MEM_CALLOC(1, sizeFloatWide);
        // pstAec->aDelta = pstAec->pstNAes->LEAK2;
        pstAec->aX[0]       = AISP_MEM_CALLOC(1, sizeComplexFftbins * refNum);
        pstAec->aX[1]       = AISP_MEM_CALLOC(1, sizeComplexFftbins * refNum);
        pstAec->aX[2]       = AISP_MEM_CALLOC(1, sizeComplexFftbins * refNum);
        pstAec->aX[3]       = AISP_MEM_CALLOC(1, sizeComplexFftbins * refNum);
        for(i = 0; i < AEC_TAPS; i++)
        {
            pstAec->aXPower[i]     = AISP_MEM_CALLOC(1, sizeFloatFftbins * refNum);    
        }
        

        if (pstAec->config->emd && pstAec->config->constrain)
        {
            pstAec->aExtdW0       = AISP_MEM_CALLOC(1, sizeComplexWide * pstAec->config->refNum);
            //time domain
            pstAec->aExtdw        = AISP_MEM_CALLOC(1, sizeFloatWide);
        }

        pstAec->aXPowerSum        = AISP_MEM_CALLOC(1, sizeFloatFftbins);
        pstAec->aXPowerSumSmooth  = AISP_MEM_CALLOC(1, sizeFloatFftbins);

        for (i = 0; i < pstAec->config->refNum ; ++i)
        {
            pstAec->aRefFrameOld[i]  = AISP_MEM_CALLOC(1, sizeFloatFrame);
        }

        if (pstAec->config->emd != 0)
        {
            switch (pstAec->config->refNum)
            {
                case 2:
                    {
                        pstAec->aR12  = AISP_MEM_CALLOC(1, sizeComplexFftbins);
                        pstAec->aR22  = AISP_MEM_CALLOC(1, sizeFloatFftbins);
                        pstAec->aIR12 = AISP_MEM_CALLOC(1, sizeComplexFftbins);
                        pstAec->aIR22 = AISP_MEM_CALLOC(1, sizeFloatFftbins);
                    }

                case 1:
                default:
                    {
                        pstAec->aR11 = AISP_MEM_CALLOC(1, sizeFloatFftbins);
                        pstAec->aR_Qbase = AISP_MEM_CALLOC(1, pstAec->fftBins);
                        pstAec->aIR11 = AISP_MEM_CALLOC(1, sizeFloatFftbins);
                    }
            }
        }

        // aispeech_mem_allocatorRegister(pstAllocator, (void **)(&pstAec->aErrFrameWide), sizeFloatWide, 1);
    }

    if (pstAec->config->emd)
    {
        pstAec->aErrPreOld = AISP_MEM_CALLOC(1, sizeFloatFrame);
        pstAec->aEstOld    = AISP_MEM_CALLOC(1, sizeFloatFrame);
    }

    pstAec->aErrOld = AISP_MEM_CALLOC(1, sizeFloatFrame);
#endif
    pstAec->pIR11   = pstAec->aIR11;
    pstAec->pIR12   = pstAec->aIR12;
    pstAec->pIR22   = pstAec->aIR22;
    pstAec->pX0 = &pstAec->aX[0][0];
    pstAec->pX1 = &pstAec->aX[1][0];
    pstAec->pX2 = &pstAec->aX[2][0];
    pstAec->puX_REF = &pstAec->aX[3][0];
    pstAec->aMIC = pstAec->aExtdW0;
    pstAec->aEST = pstAec->aExtdW0;
    pstAec->pstNAes->LEAK2 = (aisp_s32_t *)(pstAec->aExtdW0 + pstAec->fftBins);
    pstAec->aDelta = pstAec->pstNAes->LEAK2;
    return pstAec;
FFT_ALLOC_FAIL:
#ifndef JL_AEC_OPTIMIZATION_FFT
    AISP_TSL_fft_destroy(pstAec->fftEngine);
#endif // JL_AEC_OPTIMIZATION_FFT
HANNING_WIN_CREATE_FAIL:

    if (pstAec->hanningWindowSqrt)
    {
        AISP_MEM_FREE(pstAec->hanningWindowSqrt);
    }

    if (pstAec->pstNAes)
    {
        AISP_MEM_FREE(pstAec->pstNAes);
    }

    if (pstAec)
    {
        AISP_MEM_FREE(pstAec);
    }

FAIL:
    return NULL;
}

/************************************************************
  Function   : aec_feed()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create
    2017/03/03, youhai.Jiang modified

************************************************************/
AEC_VISIBLE_ATTR void aec_feed(aec_core_T *pstAec, const short *micFrame, const short *refFrame0,
                               const short *refFrame1, const short *refFrame2)
{
    aec_preProcess(pstAec, micFrame, refFrame0, refFrame1, refFrame2);
    return ;
}

/************************************************************
  Function   : aec_feedSharedRef()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create
    2017/03/03, youhai.Jiang modified

************************************************************/
AEC_VISIBLE_ATTR void aec_feedSharedRef(aec_core_T *pstAec, const short *micFrame, const aec_core_T *pRefSrcAec)
{
    pstAec->aEstFrameWide = pRefSrcAec->aEstFrameWide;
    pstAec->fftEngine = pRefSrcAec->fftEngine;
    aec_preProcess(pstAec, micFrame, NULL, NULL, NULL);
    pstAec->aErrFrameWide = pRefSrcAec->aErrFrameWide;
    pstAec->vadResult = pRefSrcAec->vadResult;
    pstAec->confirmUpdate  = pRefSrcAec->confirmUpdate;
    pstAec->aDeltaFloor = pRefSrcAec->aDeltaFloor;
    pstAec->pIR11 = pRefSrcAec->aIR11;
    pstAec->pIR12 = pRefSrcAec->aIR12;
    pstAec->pIR22 = pRefSrcAec->aIR22;
    pstAec->pX0     = pRefSrcAec->pX0;
    pstAec->pX1     = pRefSrcAec->pX1;
    pstAec->pX2     = pRefSrcAec->pX2;
    pstAec->puX_REF = pRefSrcAec->puX_REF;
    pstAec->aXPowerSum       = pRefSrcAec->aXPowerSum;
    pstAec->aXPowerSumSmooth = pRefSrcAec->aXPowerSumSmooth;
    pstAec->aExtdW0          = pRefSrcAec->aExtdW0;
    pstAec->aExtdw           = pRefSrcAec->aExtdw;
    pstAec->aMIC             = pstAec->aExtdW0;
    pstAec->aDelta           = pRefSrcAec->pstNAes->LEAK2;
    pstAec->aEST             = pRefSrcAec->aExtdW0;
    // pstAec->aERR             = pRefSrcAec->aERR;
    pstAec->pstNAes->aPost        = (aisp_s32_t *)pRefSrcAec->aExtdW0;
    pstAec->pstNAes->LEAK2        = pRefSrcAec->pstNAes->LEAK2;
    pstAec->pstNAes->residualEcho = pstAec->pstNAes->LEAK2;
    return ;
}

/************************************************************
  Function   : aec_run()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create
    2017/03/03, youhai.Jiang modified

************************************************************/
AEC_VISIBLE_ATTR void aec_run(aec_core_T *pstAec)
{
    if ((pstAec->config->vad == 0) || (pstAec->vadResult == 1))
    {
        aec_emdProcess(pstAec);
        aec_updateErr(pstAec);
    }

    return ;
}

/************************************************************
  Function   : aec_popOverlappedSpectrum()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create
    2017/03/03, youhai.Jiang modified

************************************************************/
AEC_VISIBLE_ATTR void aec_popOverlappedSpectrum(aec_core_T *pstAec, aisp_cpx_s32_t *pERR)
{
    memcpy(pERR, pstAec->aERR, pstAec->fftBins * sizeof(aisp_cpx_s32_t));
    return ;
}

/************************************************************
  Function   : aec_popTimeDomainPcm()

  Description: Get time domain err signal and do ifft
               and overlapped.
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create
    2017/03/03, youhai.Jiang modified

************************************************************/
AEC_VISIBLE_ATTR void aec_popTimeDomainPcm(aec_core_T *pstAec, short *pErr)
{
    /*In order to save memory footprint*/
    /*overwrite aMIC to compute time domain Err*/
    int i              = 0;
    int mute_time_ms   = pstAec->config->mute_time_ms;
    int sampleRate     = pstAec->config->fs;
    int fftBins        = pstAec->fftBins;
    int frameSize      = pstAec->config->frameSize;
    int mute_frame_num = 0;
    aisp_s32_t tmp_out = 0;
    aisp_s32_t preemph = (aisp_s32_t)pstAec->config->preemph;

    /* do the ifft if there is voice activity or voice activity detection is turned off */
    if ((pstAec->config->vad == 0) || (pstAec->vadResult != 0))
    {
        if (pstAec->config->mute)
        {
            mute_frame_num = mute_time_ms * sampleRate / (1000 * fftBins);

            if (pstAec->mute_Frames < mute_frame_num)
            {
                aec_muteProcess(pstAec, pErr);
                return;
            }
        }

        /*
         * fill the last half of spectrum in order to do ifft
         */
        pstAec->aMIC[0].real = pstAec->aERR[0].real;
        pstAec->aMIC[0].imag = pstAec->aERR[0].imag;

        for (i = 1; i < fftBins; ++i)
        {
            COMPLEX_CONJ(pstAec->aMIC[(frameSize << 1) - i], pstAec->aERR[i]);
            pstAec->aMIC[i].real = pstAec->aERR[i].real;
            pstAec->aMIC[i].imag = pstAec->aERR[i].imag;
        }

        AISP_TSL_ifftr32(pstAec->fftEngine, pstAec->aMIC, pstAec->aErrFrameWide);

        /*overlapped add*/
        for (i = 0; i < frameSize; ++i)
        {
            pErr[i] = MULT16_16_P15(pstAec->aErrFrameWide[i], pstAec->hanningWindowSqrt[i]) + pstAec->aErrOld[i];
            pstAec->aErrOld[i] = MULT16_16_P15(pstAec->aErrFrameWide[i + frameSize],
                                               pstAec->hanningWindowSqrt[i + frameSize]);
        }
    }
    else
    {
        /* overlapped */
        for (i = 0; i < frameSize; ++i)
        {
            pErr[i] = MULT16_16_P15(pstAec->aMicFrameWide[i], pstAec->hanningWindowSqrt[i]) + pstAec->aErrOld[i];
            pstAec->aErrOld[i] = MULT16_16_P15(pstAec->aMicFrameWide[i + frameSize],
                                               pstAec->hanningWindowSqrt[i + frameSize]);
        }
    }

    /* de-emphasis */
    if (pstAec->config->gainFlag == 0)
    {
        for (i = 0; i < frameSize; ++i)
        {
            tmp_out = pErr[i];
            tmp_out = tmp_out + PSHR((aisp_s32_t)preemph * pstAec->memE, AISP_TSL_Q15_SHIFT);

            if (tmp_out >= 32767)
            {
                pErr[i] = 32767;
                pstAec->memE = 32767;
            }
            else
                if (tmp_out <= -32768)
                {
                    pErr[i] = -32768;
                    pstAec->memE = -32768;
                }
                else
                {
                    pErr[i] = tmp_out;
                    pstAec->memE = tmp_out;
                }
        }
    }
    else
    {
        for (i = 0; i < frameSize; ++i)
        {
            tmp_out = pErr[i];
            tmp_out = tmp_out + PSHR((aisp_s32_t)preemph * pstAec->memE, AISP_TSL_Q15_SHIFT);
            pErr[i] = tmp_out * pstAec->config->outGain;
            pstAec->memE = tmp_out;
        }
    }

    return ;
}

/************************************************************
  Function   : aec_reset()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create
    2017/03/03, youhai.Jiang modified

************************************************************/
AEC_VISIBLE_ATTR void aec_reset(aec_core_T *pstAec)
{
    int i = 0;
    size_t sizeComplexFftbins;
    size_t sizeFloatFftbins;
    size_t sizeFloatFrame;
    size_t sizeComplexWide;
    size_t sizeFloatWide;
    naes_reset(pstAec->pstNAes);
    /********************************add by zq********************************************/
    /*
     * 20170109 bug fixed: convergence is too slow
     * memXold is used to store the last point of the frame before
     */
    sizeComplexFftbins = pstAec->fftBins * sizeof(aisp_cpx_s32_t);
    sizeFloatFftbins   = pstAec->fftBins * sizeof(aisp_s32_t);
    sizeFloatFrame     = pstAec->config->frameSize * sizeof(aisp_s16_t);
    sizeComplexWide    = (pstAec->config->frameSize << 1) * sizeof(aisp_cpx_s32_t);
    sizeFloatWide      = (pstAec->config->frameSize << 1) * sizeof(aisp_s16_t);

    for (i = 0; i < pstAec->config->micNum; i++)
    {
        pstAec->memXold[i] = 0; /* AISP_FIXED16(0.0f) */
        pstAec->memEold[i] = 0; /* AISP_FIXED16(0.0f) */
    }

    /*
     * memX is used to store the mic value of the current frame
     * memD is used to store the ref value of the current frame
     */
    pstAec->memX = 0;   /* AISP_FIXED16(0.0f) */
    pstAec->memE = 0;   /* AISP_FIXED16(0.0f) */
    pstAec->memD[0] = 0;    /* AISP_FIXED16(0.0f) */
    pstAec->memD[1] = 0;    /* AISP_FIXED16(0.0f) */
    pstAec->memDold[0] = 0; /* AISP_FIXED16(0.0f) */
    pstAec->memDold[1] = 0; /* AISP_FIXED16(0.0f) */
//    AISP_TSL_memset(pstAec->aDelta, 0, (pstAec->fftBins * sizeof(aisp_s32_t)));
    AISP_TSL_memset(pstAec->pstNAes->pPs_old, 0, pstAec->fftBins * sizeof(aisp_s32_t));
    AISP_TSL_memset(pstAec->pstNAes->PPyy, 0, pstAec->fftBins * sizeof(aisp_s32_t));
    AISP_TSL_memset(pstAec->pstNAes->PPee, 0, pstAec->fftBins * sizeof(aisp_s32_t));
    AISP_TSL_memset(pstAec->pstNAes->PPey, 0, pstAec->fftBins * sizeof(aisp_cpx_s32_t));
    AISP_TSL_memset(pstAec->aERR, 0, pstAec->fftBins * sizeof(aisp_cpx_s32_t));
    AISP_TSL_memset(pstAec->aMicFrameWide, 0, (pstAec->config->frameSize << 1) * sizeof(aisp_s16_t));
    /********************************add by zq********************************************/
    AISP_TSL_memset(pstAec->aERRPower, 0, (pstAec->fftBins * sizeof(aisp_s32_t)));
    AISP_TSL_memset(pstAec->aW[0], 0, (pstAec->fftBins * pstAec->config->refNum * sizeof(aisp_cpx_s32_t)));
    AISP_TSL_memset(pstAec->aW[1], 0, (pstAec->fftBins * pstAec->config->refNum * sizeof(aisp_cpx_s32_t)));
    AISP_TSL_memset(pstAec->aW[2], 0, (pstAec->fftBins * pstAec->config->refNum * sizeof(aisp_cpx_s32_t)));
    AISP_TSL_memset(pstAec->aW[3], 0, (pstAec->fftBins * pstAec->config->refNum * sizeof(aisp_cpx_s32_t)));
    AISP_TSL_memset(pstAec->aMicFrameOld, 0, pstAec->config->frameSize * sizeof(aisp_s16_t));

    if (pstAec->idx == 0)
    {
        /********************************add by zq**********************************************/
        pstAec->clip = 0;
        pstAec->maxabsRef = 0;  /* AISP_FIXED16(0.0f) */
        AISP_TSL_memset(pstAec->aEstFrameWide, 0, sizeFloatWide);
        AISP_TSL_memset(pstAec->aErrFrameWide, 0, sizeFloatWide);
        AISP_TSL_memset(pstAec->aX[0], 0, sizeComplexFftbins * pstAec->config->refNum);
        AISP_TSL_memset(pstAec->aX[1], 0, sizeComplexFftbins * pstAec->config->refNum);
        AISP_TSL_memset(pstAec->aX[2], 0, sizeComplexFftbins * pstAec->config->refNum);
        AISP_TSL_memset(pstAec->aX[3], 0, sizeComplexFftbins * pstAec->config->refNum);
        //AISP_TSL_memset(pstAec->aXPower, 0, sizeFloatFftbins * pstAec->config->refNum );
        for(i = 0; i < AEC_TAPS; i++)
        {
            AISP_TSL_memset(pstAec->aXPower[i], 0, sizeFloatFftbins * pstAec->config->refNum);
        }

        if (pstAec->config->emd && pstAec->config->constrain)
        {
            AISP_TSL_memset(pstAec->aExtdW0, 0, sizeComplexWide * pstAec->config->refNum);
            //time domain
            AISP_TSL_memset(pstAec->aExtdw , 0, sizeFloatWide);
        }

        AISP_TSL_memset(pstAec->aXPowerSum , 0, sizeFloatFftbins);
        AISP_TSL_memset(pstAec->aXPowerSumSmooth , 0, sizeFloatFftbins);

        /********************************add by zq***********************************************/
        for (i = 0; i < pstAec->config->refNum ; ++i)
        {
            AISP_TSL_memset(pstAec->aRefFrameOld[i], 0, pstAec->config->frameSize * sizeof(aisp_s16_t));
        }

        if (pstAec->config->emd != 0)
        {
            switch (pstAec->config->refNum)
            {
                case 2:
                    {
                        AISP_TSL_memset(pstAec->aR12, 0, sizeComplexFftbins);
                        AISP_TSL_memset(pstAec->aR22, 0, sizeFloatFftbins);
                        /********************************add by zq***********************************************/
                        AISP_TSL_memset(pstAec->aIR12, 0, sizeComplexFftbins);
                        AISP_TSL_memset(pstAec->aIR22, 0, sizeFloatFftbins);
                        /********************************add by zq***********************************************/
                    }

                case 1:
                default:
                    {
                        AISP_TSL_memset(pstAec->aR11, 0, sizeFloatFftbins);
                        AISP_TSL_memset(pstAec->aR_Qbase, 0, pstAec->fftBins);
                        AISP_TSL_memset(pstAec->aIR11, 0, sizeFloatFftbins);
                    }
            }
        }
    }

    if (pstAec->config->emd)
    {
        AISP_TSL_memset(pstAec->aErrPreOld, 0, sizeFloatFrame);
        AISP_TSL_memset(pstAec->aEstOld, 0, sizeFloatFrame);
    }

    AISP_TSL_memset(pstAec->aErrOld, 0, sizeFloatFrame);
///////////////////////////////////////////////////////////////////////////////////////////////
    pstAec->pIR11   = pstAec->aIR11;
    pstAec->pIR12   = pstAec->aIR12;
    pstAec->pIR22   = pstAec->aIR22;
    pstAec->pX0 = &pstAec->aX[0][0];
    pstAec->pX1 = &pstAec->aX[1][0];
    pstAec->pX2 = &pstAec->aX[2][0];
    pstAec->puX_REF = &pstAec->aX[3][0];
    pstAec->aMIC = pstAec->aExtdW0;
    pstAec->aEST = pstAec->aExtdW0;
    pstAec->pstNAes->LEAK2 = (aisp_s32_t *)(pstAec->aExtdW0 + (pstAec->config->frameSize << 1) + pstAec->config->frameSize);
    pstAec->aDelta = pstAec->pstNAes->LEAK2;
    return ;
}

/************************************************************
  Function   : aec_delete()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create
    2017/03/03, youhai.Jiang modified
************************************************************/
AEC_VISIBLE_ATTR void aec_delete(aec_core_T *pstAec)
{
    int i = 0;

    if (pstAec)
    {

        if (pstAec->idx == 0)
        {
#ifndef JL_AEC_OPTIMIZATION_FFT
            AISP_TSL_fft_destroy(pstAec->fftEngine);
#endif
        }
        if (pstAec->pstNAes)
        {
            AISP_MEM_FREE(pstAec->pstNAes->pPs_old);
            AISP_MEM_FREE(pstAec->pstNAes->PPyy);
            AISP_MEM_FREE(pstAec->pstNAes->PPee);
            AISP_MEM_FREE(pstAec->pstNAes->PPey);
            AISP_MEM_FREE(pstAec->pstNAes);
        }

        pstAec->pstNAes = NULL;

        if (pstAec->memoryPool)
        {
            AISP_MEM_FREE(pstAec->memoryPool);
        }

        if (pstAec->idx == 0)
        {
            AISP_MEM_FREE(pstAec->aEstFrameWide);
            AISP_MEM_FREE(pstAec->aErrFrameWide);
            // pstAec->aDelta = pstAec->pstNAes->LEAK2;
            AISP_MEM_FREE(pstAec->aX[0]);
            AISP_MEM_FREE(pstAec->aX[1]);
            AISP_MEM_FREE(pstAec->aX[2]);
            AISP_MEM_FREE(pstAec->aX[3]);
            //AISP_MEM_FREE(pstAec->aXPower);
            for(i = 0; i < AEC_TAPS; i++)
            {
                AISP_MEM_FREE(pstAec->aXPower[i]);
            }
            AISP_MEM_FREE(pstAec->aExtdW0);
            AISP_MEM_FREE(pstAec->aExtdw);
            AISP_MEM_FREE(pstAec->aXPowerSum);
            AISP_MEM_FREE(pstAec->aXPowerSumSmooth);

            if (pstAec->config->emd != 0)
            {
                switch (pstAec->config->refNum)
                {
                    case 2:
                        {
                            AISP_MEM_FREE(pstAec->aR12);
                            AISP_MEM_FREE(pstAec->aR22);
                            AISP_MEM_FREE(pstAec->aIR12);
                            AISP_MEM_FREE(pstAec->aIR22);
                        }

                    case 1:
                    default:
                        {
                            AISP_MEM_FREE(pstAec->aR11);
                            AISP_MEM_FREE(pstAec->aR_Qbase);
                            AISP_MEM_FREE(pstAec->aIR11);
                        }
                }
            }

            for (i = 0; i < pstAec->config->refNum ; ++i)
            {
                AISP_MEM_FREE(pstAec->aRefFrameOld[i]);
            }
        }

        if (pstAec->config->emd)
        {
            AISP_MEM_FREE(pstAec->aErrPreOld);
            AISP_MEM_FREE(pstAec->aEstOld);
        }

        AISP_MEM_FREE(pstAec->aErrOld);
        AISP_MEM_FREE(pstAec->aERRPower);
        AISP_MEM_FREE(pstAec->aMicFrameOld);
        AISP_MEM_FREE(pstAec->aERR);
        AISP_MEM_FREE(pstAec->aMicFrameWide);
        AISP_MEM_FREE(pstAec->aW[0]);
        AISP_MEM_FREE(pstAec->aW[1]);
        AISP_MEM_FREE(pstAec->aW[2]);
        AISP_MEM_FREE(pstAec->aW[3]);
        AISP_MEM_FREE(pstAec);
    }

    return ;
}

