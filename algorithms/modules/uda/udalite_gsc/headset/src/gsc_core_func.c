#include <stdio.h>

#include "AISP_TSL_base.h"
#include "AISP_TSL_common.h"
#include "AISP_TSL_math.h"

#include "gsc_core_types.h"
#include "gsc_api.h"

/************************************************************
  Function   : fftInit()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/11/15, Youhai.Jiang create

************************************************************/
S32 fftInit(AISP_TSL_FFTENG_S *pstFftEng, S32 fftSize)
{
    AISP_TSL_fft_alloc(pstFftEng, fftSize);
    return 0;
}

/************************************************************
  Function   : fftDelete()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/11/15, Youhai.Jiang create

************************************************************/
VOID fftDelete(AISP_TSL_FFTENG_S *pstFftEng)
{
    AISP_PTR_FREE(pstFftEng->fftCfg);
    AISP_PTR_FREE(pstFftEng->ifftCfg);
}


/************************************************************
  Function   : fftProcess()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/11/15, Youhai.Jiang create

************************************************************/
S32 fftProcess(GSC_Api_S *pstGscApi, S8 *pcData)
{
    GSC_S32_T      aiRawData[GSC_FRMLEN * GSC_MICS];
    GSC_COMPLEX_T   acpxXFFT[GSC_FRMLEN * GSC_MICS];
    S16            *psRaw        = (S16 *)pcData;
    GSC_S32_T      *piLeftRaw    = (GSC_S32_T *)pstGscApi->pstGscInstance->piRaw;
    GSC_S32_T      *psHanningWin = (GSC_S32_T *)pstGscApi->psHanningWind;
    GSC_COMPLEX_T  *pcpxXFFT     = (GSC_COMPLEX_T *)pstGscApi->pstGscInstance->pcpxXFFT;
    S32 iRet  = 0;
    S16 sLoop = 0;
#if 0

    for (sLoop = 0; sLoop < GSC_FRMINC; sLoop++)
    {
        printf("psRaw[%3d] = %x\n", sLoop, psRaw[2 * sLoop]);
    }

#endif

    /* First input is only a half frame */
    if (pstGscApi->pstGscCfg->iInitFlag)
    {
        switch (GSC_MICS)
        {
            case 2:
                {
                    GSC_S32_T *pMic1LeftRaw     = piLeftRaw;
                    GSC_S32_T *pMic2LeftRaw     = piLeftRaw + GSC_FRMINC;
                    GSC_S32_T *pMic1Raw         = aiRawData;
                    GSC_S32_T *pMic2Raw         = aiRawData + GSC_FRMLEN;
                    GSC_S32_T *pHanningBtm      = psHanningWin + GSC_FRMINC;
                    GSC_COMPLEX_T *pcpxMic1XFFT = pcpxXFFT;
                    GSC_COMPLEX_T *pcpxMic2XFFT = pcpxXFFT + GSC_FFTBINS;
                    GSC_COMPLEX_T *pcpxMic1FFT  = acpxXFFT;
                    GSC_COMPLEX_T *pcpxMic2FFT  = acpxXFFT + GSC_FRMLEN;

                    for (sLoop = 0; sLoop < GSC_FRMINC; sLoop++, psRaw += GSC_IN_CHAN)
                    {
                        *pMic1Raw = *pMic1LeftRaw++;
                        *pMic2Raw = *pMic2LeftRaw++;
#ifdef GSC_FIXED
#ifdef RDMA_FFT_512   /* raw data is 32Q24 for 512 points fft */
                        *(pMic1Raw + GSC_FRMINC) = PSHR((long long)(*pHanningBtm) * (*psRaw),
                                                        AISP_TSL_Q22_SHIFT);/* W32Q24 = Q31xQ15 >> 22 */
                        *(pMic2Raw + GSC_FRMINC) = PSHR((long long)(*pHanningBtm) * (*(psRaw + 1)), AISP_TSL_Q22_SHIFT);
#elif defined RDMA_FFT_256  /* raw data is 32Q23 for 256 points fft */
                        *(pMic1Raw + GSC_FRMINC) = PSHR((long long)(*pHanningBtm) * (*psRaw),
                                                        AISP_TSL_Q23_SHIFT);/* W32Q23 = Q31xQ15 >> 23 */
                        *(pMic2Raw + GSC_FRMINC) = PSHR((long long)(*pHanningBtm) * (*(psRaw + 1)), AISP_TSL_Q23_SHIFT);
#endif
#else
                        *(pMic1Raw + GSC_FRMINC) = *pHanningBtm * *psRaw * GSC_Q15_SCALAR;
                        *(pMic2Raw + GSC_FRMINC) = *pHanningBtm * *(psRaw + 1) * GSC_Q15_SCALAR;
#endif
                        pHanningBtm++;
                        pMic1Raw++;
                        pMic2Raw++;
                    }

#if 0
                    /* Dump the data after add window */
                    pMic1Raw = aiRawData;

                    for (sLoop = 0; sLoop < GSC_FRMLEN; sLoop++)
                    {
                        //printf("pMic1Raw[%3d] = %f\n", sLoop, *pMic1Raw++);
                        printf("%d\n", *pMic1Raw++);
                    }

#endif
#ifdef GSC_FIXED
#ifdef STATIC_MEM_FIXED_FFT
                    /* input 32Q24 output 32Q15 */
                    AISP_TSL_FFT(pstGscApi->stFFTEng, aiRawData, pcpxMic1FFT);
                    AISP_TSL_FFT(pstGscApi->stFFTEng, aiRawData + GSC_FRMLEN, pcpxMic2FFT);
#else
                    float in1[GSC_FFTLEN];
                    float in2[GSC_FFTLEN];
                    float out1[GSC_FFTBINS << 1];
                    float out2[GSC_FFTBINS << 1];
#ifdef RDMA_FFT_512
                    float scalar = 1.0 / AISP_TSL_Q24_ONE;
#elif defined RDMA_FFT_256
                    float scalar = 1.0 / AISP_TSL_Q23_ONE;
#endif

                    for (sLoop = 0; sLoop < GSC_FFTLEN; sLoop++)
                    {
                        in1[sLoop] = (float)aiRawData[sLoop] * scalar;
                        in2[sLoop] = (float)aiRawData[sLoop + GSC_FRMLEN] * scalar;
                    }

                    AISP_TSL_FFT(pstGscApi->stFFTEng, in1, out1);
                    AISP_TSL_FFT(pstGscApi->stFFTEng, in2, out2);

                    for (sLoop = 0; sLoop < GSC_FFTBINS; sLoop++)
                    {
                        pcpxMic1FFT[sLoop].real = round(out1[2 * sLoop]  * AISP_TSL_Q15_ONE);
                        pcpxMic1FFT[sLoop].imag = round(out1[2 * sLoop + 1] * AISP_TSL_Q15_ONE);
                        pcpxMic2FFT[sLoop].real = round(out2[2 * sLoop]  * AISP_TSL_Q15_ONE);
                        pcpxMic2FFT[sLoop].imag = round(out2[2 * sLoop + 1] * AISP_TSL_Q15_ONE);
                    }

#endif
#else
                    AISP_TSL_FFT(pstGscApi->stFFTEng, aiRawData, pcpxMic1FFT);
                    AISP_TSL_FFT(pstGscApi->stFFTEng, aiRawData + GSC_FRMLEN, pcpxMic2FFT);
#endif

                    /* aXFFT stores Frequency data */
                    for (sLoop = 0; sLoop < GSC_FFTBINS; sLoop++)
                    {
                        *pcpxMic1XFFT++ = *pcpxMic1FFT++;
                        *pcpxMic2XFFT++ = *pcpxMic2FFT++;
                    }

                    break;
                }

            default:
                {
#ifdef AISP_TSL_INFO
                    printf("Not support. Please check the configuration.\n");
#endif
                    break;
                }
        }
    }
    else
    {
        pstGscApi->pstGscCfg->iInitFlag = 1;
        iRet = 1;
    }

    psRaw = (S16 *)pcData;

    switch (GSC_MICS)
    {
        case 2:
            {
                GSC_S32_T *pMic1LeftRaw = piLeftRaw;
                GSC_S32_T *pMic2LeftRaw = piLeftRaw + GSC_FRMINC;

                for (sLoop = 0; sLoop < GSC_FRMINC; sLoop++, psRaw += GSC_IN_CHAN)
                {
#ifdef GSC_FIXED
#ifdef RDMA_FFT_512   /* raw data is 32Q24 for 512 points fft */
                    *pMic1LeftRaw++ = PSHR((long long)(*psHanningWin) * (*psRaw),     AISP_TSL_Q22_SHIFT);/* W32Q24 = Q31xQ15 >> 22 */
                    *pMic2LeftRaw++ = PSHR((long long)(*psHanningWin) * (*(psRaw + 1)), AISP_TSL_Q22_SHIFT);
#elif defined RDMA_FFT_256  /* raw data is 32Q23 for 256 points fft */
                    *pMic1LeftRaw++ = PSHR((long long)(*psHanningWin) * (*psRaw),     AISP_TSL_Q23_SHIFT);/* W32Q23 = Q31xQ15 >> 23 */
                    *pMic2LeftRaw++ = PSHR((long long)(*psHanningWin) * (*(psRaw + 1)), AISP_TSL_Q23_SHIFT);
#endif
                    psHanningWin++;
#else
                    *pMic1LeftRaw++ = *psHanningWin * *psRaw * GSC_Q15_SCALAR;
                    *pMic2LeftRaw++ = *psHanningWin * *(psRaw + 1) * GSC_Q15_SCALAR;
                    psHanningWin++;
#endif
                }

                break;
            }

        default:
            {
#ifdef AISP_TSL_INFO
                printf("Not support. Please check the configuration.\n");
#endif
                break;
            }
    }

    return iRet;
}

/************************************************************
  Function   : fftProcessParallel()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/11/15, Youhai.Jiang create

************************************************************/
S32 fftProcessParallel(GSC_Api_S *pstGscApi, S8 *pcChan1, S8 *pcChan2)
{
    GSC_S32_T      aiRawData[GSC_FRMLEN * GSC_MICS];
    GSC_COMPLEX_T   acpxXFFT[GSC_FRMLEN * GSC_MICS];
    S16            *psRaw1       = (S16 *)pcChan1;
    S16            *psRaw2       = (S16 *)pcChan2;
    GSC_S32_T      *piLeftRaw    = (GSC_S32_T *)pstGscApi->pstGscInstance->piRaw;
    GSC_S32_T      *psHanningWin = (GSC_S32_T *)pstGscApi->psHanningWind;
    GSC_COMPLEX_T  *pcpxXFFT     = (GSC_COMPLEX_T *)pstGscApi->pstGscInstance->pcpxXFFT;
    S32 iRet = 0;
    S16 sLoop;

    /* First input is only a half frame */
    if (pstGscApi->pstGscCfg->iInitFlag)
    {
        switch (GSC_MICS)
        {
            case 2:
                {
                    GSC_S32_T *pMic1LeftRaw     = piLeftRaw;
                    GSC_S32_T *pMic2LeftRaw     = piLeftRaw + GSC_FRMINC;
                    GSC_S32_T *pMic1Raw         = aiRawData;
                    GSC_S32_T *pMic2Raw         = aiRawData + GSC_FRMLEN;
                    GSC_S32_T *pHanningBtm      = psHanningWin + GSC_FRMINC;
                    GSC_COMPLEX_T *pcpxMic1XFFT = pcpxXFFT;
                    GSC_COMPLEX_T *pcpxMic2XFFT = pcpxXFFT + GSC_FFTBINS;
                    GSC_COMPLEX_T *pcpxMic1FFT  = acpxXFFT;
                    GSC_COMPLEX_T *pcpxMic2FFT  = acpxXFFT + GSC_FRMLEN;

                    for (sLoop = 0; sLoop < GSC_FRMINC; sLoop++, psRaw1++, psRaw2++)
                    {
                        *pMic1Raw = *pMic1LeftRaw++;
                        *pMic2Raw = *pMic2LeftRaw++;
#ifdef GSC_FIXED
#ifdef RDMA_FFT_512   /* raw data is 32Q24 for 512 points fft */
                        *(pMic1Raw + GSC_FRMINC) = PSHR((long long)(*pHanningBtm) * (*psRaw1), AISP_TSL_Q22_SHIFT);/* W32Q24 = Q31xQ15 >> 22 */
                        *(pMic2Raw + GSC_FRMINC) = PSHR((long long)(*pHanningBtm) * (*psRaw2), AISP_TSL_Q22_SHIFT);
#elif defined RDMA_FFT_256  /* raw data is 32Q23 for 256 points fft */
                        /* raw data is 32Q23 for 256 points fft */
                        *(pMic1Raw + GSC_FRMINC) = PSHR((long long)(*pHanningBtm) * (*psRaw1), AISP_TSL_Q23_SHIFT);
                        *(pMic2Raw + GSC_FRMINC) = PSHR((long long)(*pHanningBtm) * (*psRaw2), AISP_TSL_Q23_SHIFT);
#endif
#else
                        *(pMic1Raw + GSC_FRMINC) = *pHanningBtm * *psRaw1 * GSC_Q15_SCALAR;
                        *(pMic2Raw + GSC_FRMINC) = *pHanningBtm * *psRaw2 * GSC_Q15_SCALAR;
#endif
                        pHanningBtm++;
                        pMic1Raw++;
                        pMic2Raw++;
                    }

#ifdef GSC_FIXED
#ifdef STATIC_MEM_FIXED_FFT
                    /* input 32Q24 output 32Q15 */
                    AISP_TSL_FFT(pstGscApi->stFFTEng, aiRawData, pcpxMic1FFT);
                    AISP_TSL_FFT(pstGscApi->stFFTEng, aiRawData + GSC_FRMLEN, pcpxMic2FFT);
#else
                    float in1[GSC_FFTLEN];
                    float in2[GSC_FFTLEN];
                    float out1[GSC_FFTBINS << 1];
                    float out2[GSC_FFTBINS << 1];
#ifdef RDMA_FFT_512
                    float scalar = 1.0 / AISP_TSL_Q24_ONE;
#elif defined RDMA_FFT_256
                    float scalar = 1.0 / AISP_TSL_Q23_ONE;
#endif

                    for (sLoop = 0; sLoop < GSC_FFTLEN; sLoop++)
                    {
                        in1[sLoop] = (float)aiRawData[sLoop] * scalar;
                        in2[sLoop] = (float)aiRawData[sLoop + GSC_FRMLEN] * scalar;
                    }

                    AISP_TSL_FFT(pstGscApi->stFFTEng, in1, out1);
                    AISP_TSL_FFT(pstGscApi->stFFTEng, in2, out2);

                    for (sLoop = 0; sLoop < GSC_FFTBINS; sLoop++)
                    {
                        pcpxMic1FFT[sLoop].real = round(out1[2 * sLoop]  * AISP_TSL_Q15_ONE);
                        pcpxMic1FFT[sLoop].imag = round(out1[2 * sLoop + 1] * AISP_TSL_Q15_ONE);
                        pcpxMic2FFT[sLoop].real = round(out2[2 * sLoop]  * AISP_TSL_Q15_ONE);
                        pcpxMic2FFT[sLoop].imag = round(out2[2 * sLoop + 1] * AISP_TSL_Q15_ONE);
                    }

#endif
#else
                    AISP_TSL_FFT(pstGscApi->stFFTEng, aiRawData, pcpxMic1FFT);
                    AISP_TSL_FFT(pstGscApi->stFFTEng, aiRawData + GSC_FRMLEN, pcpxMic2FFT);
#endif

                    /* aXFFT stores Frequency data */
                    for (sLoop = 0; sLoop < GSC_FFTBINS; sLoop++)
                    {
                        *pcpxMic1XFFT++ = *pcpxMic1FFT++;
                        *pcpxMic2XFFT++ = *pcpxMic2FFT++;
                    }

                    break;
                }

            default:
                {
#ifdef AISP_TSL_INFO
                    printf("Not support. Please check the configuration.\n");
#endif
                    break;
                }
        }
    }
    else
    {
        pstGscApi->pstGscCfg->iInitFlag = 1;
        iRet = 1;
    }

    psRaw1       = (S16 *)pcChan1;
    psRaw2       = (S16 *)pcChan2;

    switch (GSC_MICS)
    {
        case 2:
            {
                GSC_S32_T *pMic1LeftRaw = piLeftRaw;
                GSC_S32_T *pMic2LeftRaw = piLeftRaw + GSC_FRMINC;

                for (sLoop = 0; sLoop < GSC_FRMINC; sLoop++, psRaw1++, psRaw2++)
                {
#ifdef GSC_FIXED
#ifdef RDMA_FFT_512
                    *pMic1LeftRaw++ = PSHR((long long)(*psHanningWin) * (*psRaw1), AISP_TSL_Q22_SHIFT);
                    *pMic2LeftRaw++ = PSHR((long long)(*psHanningWin) * (*psRaw2), AISP_TSL_Q22_SHIFT);
#elif defined RDMA_FFT_256
                    *pMic1LeftRaw++ = PSHR((long long)(*psHanningWin) * (*psRaw1), AISP_TSL_Q23_SHIFT);
                    *pMic2LeftRaw++ = PSHR((long long)(*psHanningWin) * (*psRaw2), AISP_TSL_Q23_SHIFT);
#endif
                    psHanningWin++;
#else
                    *pMic1LeftRaw++ = *psHanningWin * *psRaw1 * GSC_Q15_SCALAR;
                    *pMic2LeftRaw++ = *psHanningWin * *psRaw2 * GSC_Q15_SCALAR;
                    psHanningWin++;
#endif
                }

                break;
            }

        default:
            {
#ifdef AISP_TSL_INFO
                printf("Not support. Please check the configuration.\n");
#endif
                break;
            }
    }

    return iRet;
}


/************************************************************
  Function   : gscUpdateWindNoiseState()

  Description: Calculate the wind noise state every frame.
  Calls      :
  Called By  : gscProcess(...)
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/10/12, Chao.Xu create

************************************************************/
VOID gscUpdateWindNoiseState(GSC_Api_S *pstGscApi, GSC_COMPLEX_T *pcpxY)
{
    GSC_CoreCfg_S *pstGscCfg    = pstGscApi->pstGscCfg;
    /* Zone for gsc struct */
    GSC_COMPLEX_T *pcpxMic1Data = NULL;
    /* Variables for coefficients */
    GSC_S32_T iSc_alpha         = pstGscCfg->iSc_alpha;
#ifdef GSC_FIXED
    GSC_S32_T iSc_alphaMinusOne = AISP_TSL_Q15_ONE - iSc_alpha;
#else
    GSC_S32_T iSc_alphaMinusOne = 1.0 - iSc_alpha;
#endif
    S32 iEndBins  = pstGscCfg->iEndBins;
    /* Zone for temporary variables */
    GSC_S64_T llPowerTmp = 0;
    /* Zone for index */
    S32 iBinIdx = 0;
    pcpxMic1Data = pcpxY + 1;

    for (iBinIdx = 1; iBinIdx < iEndBins; iBinIdx++)
    {
#ifdef GSC_FIXED
        llPowerTmp += PSHR((long long)pcpxMic1Data->real * pcpxMic1Data->real + (long long)pcpxMic1Data->imag *
                           pcpxMic1Data->imag, AISP_TSL_Q15_SHIFT);
#else
        llPowerTmp += pcpxMic1Data->real * pcpxMic1Data->real + pcpxMic1Data->imag * pcpxMic1Data->imag;
#endif
        pcpxMic1Data++;
    }

#ifdef GSC_FIXED
    pstGscCfg->iPSD_Y = PSHR((long long)iSc_alpha * pstGscCfg->iPSD_Y + (long long)iSc_alphaMinusOne * llPowerTmp,
                             AISP_TSL_Q15_SHIFT); /* W64Q15 */
#else
    pstGscCfg->iPSD_Y = iSc_alpha * pstGscCfg->iPSD_Y + iSc_alphaMinusOne * llPowerTmp;
#endif

    /* Wind Noise status detected */
    if (pstGscCfg->iPSD_Y > pstGscCfg->iSc_thr * (iEndBins - 1))
    {
        if (!pstGscCfg->iNoiseSta)
        {
            pstGscCfg->iNoiseSta = 1;

            if (pstGscApi->pfunWindCb)
            {
                (pstGscApi->pfunWindCb)(pstGscApi->pParm, WIND_IN, NULL, 0);
            }

            //printf("noise 0->1\n");
        }
    }
    else
    {
        if (pstGscCfg->iNoiseSta)
        {
            pstGscCfg->iNoiseSta = 0;

            if (pstGscApi->pfunWindCb)
            {
                (pstGscApi->pfunWindCb)(pstGscApi->pParm, WIND_OUT, NULL, 0);
            }

            //printf("noise 1->0\n");
        }
    }

#if 0
#ifdef GSC_FIXED
    printf("llPowerTmp = %lld iPSD_Y = %lld\n", llPowerTmp, pstGscCfg->iPSD_Y);
    printf("iNoiseSta = %d\n", pstGscCfg->iNoiseSta);
#else
    printf("llPowerTmp = %f iPSD_Y = %f\n", llPowerTmp, pstGscCfg->iPSD_Y);
    printf("iNoiseSta = %d\n", pstGscCfg->iNoiseSta);
#endif
#endif
}

/************************************************************
  Function   : gscProcess()

  Description: gsc process only for headset
  Calls      :
  Called By  :
  Input      : pY stores as [2 x 129] complex num
  Output     :
  Return     :
  Others     : 1.Shared memory is used as follow:
                           pcpxXFFT ------------ pcpxOut
                                    | 129 Real |
                                    ------------
                                    | 129 Imag |
                                    ------------
                                    | 129 Real |
                                    ------------
                                    | 129 Imag |
                           pcpxMic1 ------------ pcpxMIC(pcpxREF)(pcpxErrPreNorm)
                                    | 129 Real |
                                    ------------
                                    | 129 Imag |
                           pcpxMic2 ------------ pcpxEST
                                    | 129 Real |
                                    ------------
                                    | 129 Imag |
  History    :
    2018/11/09, Youhai.Jiang create
    2019/07/02, Donglei.Xu   modify
************************************************************/
VOID gscProcess(GSC_Api_S *pstGscApi, GSC_COMPLEX_T *pcpxY)
{
    GSC_Core_S       *pstGscInst    = pstGscApi->pstGscInstance;
    GSC_CoreCfg_S    *pstGscCfg     = pstGscApi->pstGscCfg;
    /* Zone for gsc struct */
    GSC_COMPLEX_T   *pcpxTargetX    = NULL;
    GSC_COMPLEX_T   *pcpxTargetW    = NULL;
    GSC_COMPLEX64_T *pcpxErrPreNorm = NULL;
    GSC_COMPLEX_T   *pcpxMIC        = NULL;
    GSC_COMPLEX_T   *pcpxREF        = NULL;
    GSC_COMPLEX_T   *pcpxMic1Data   = NULL;
    GSC_COMPLEX_T   *pcpxMic2Data   = NULL;
    GSC_COMPLEX_T   *pcpxMic1       = NULL;
    GSC_COMPLEX_T   *pcpxMic2       = NULL;
    GSC_COMPLEX_T   *pcpxRef1       = NULL;
    GSC_COMPLEX_T   *pcpxRef2       = NULL;
    GSC_COMPLEX_T   *pcpxEST        = NULL;
    GSC_COMPLEX_T   *pcpxGscX       = (GSC_COMPLEX_T *)pstGscInst->pcpxGscX;
    GSC_COMPLEX_T   *pcpxWeight     = (GSC_COMPLEX_T *)pstGscInst->pcpxGscWeight;
    GSC_COMPLEX_T   *pcpxOut        = (GSC_COMPLEX_T *)pstGscInst->pcpxXFFT;
    GSC_COMPLEX_T   *pcpxTmp        = pcpxOut;
    GSC_S64_T       *pllXPowerSum   = (GSC_S64_T *)pstGscInst->pGscXPowerSum;
    /* Variables for W&B matrix */
    GSC_COMPLEX_T   *pcpx32B        = (GSC_COMPLEX_T *)pstGscInst->pcpxGscB;
    GSC_COMPLEX_T   *pcpx32Wq       = (GSC_COMPLEX_T *)pstGscInst->pcpxGscWq;
    GSC_COMPLEX_T   *pcpx32Mic1Wq   = pcpx32Wq;
    GSC_COMPLEX_T   *pcpx32Mic2Wq   = pcpx32Wq + GSC_FFTBINS;
    GSC_COMPLEX_T   *pcpx32Mic1B    = pcpx32B;
    GSC_COMPLEX_T   *pcpx32Mic2B    = pcpx32B  + GSC_FFTBINS;
    /* Variables for coefficients */
    GSC_S32_T        iMu             = pstGscCfg->iMu;
    S32              iTapCounter     = pstGscCfg->iGscCurxIdx;
    S32              iGscNum         = pstGscCfg->iGsc;
    S32              iGscTaps        = pstGscCfg->iTaps;
    S32              iWnorm_switch   = pstGscCfg->iWnorm_switch;
    S32              iTa_switch      = pstGscCfg->iTa_switch;
    S32              iWnorm_thre     = pstGscCfg->iWnorm_thre;
    S32              iWnorm_sqrtthre = pstGscCfg->iWnorm_sqrtthre;
    S32              iTa_thre        = pstGscCfg->iTa_thre;
    S32              iTa_val         = pstGscCfg->iTa_val;
    S32              iErrGain        = pstGscCfg->iErrGain;
#ifndef GSC_FIXED
    GSC_S32_T        iGsc_alpha     = (GSC_S32_T)(iGscTaps - 1) / iGscTaps;
#else
    GSC_S32_T        iGsc_alpha     = (((iGscTaps - 1) << 15) + (iGscTaps >> 1)) / iGscTaps; //Q15
#endif
    /* Zone for temporary variables */
#ifdef GSC_FIXED
    GSC_COMPLEX_T    icpxEST_U         = {0, 0};
#endif
    GSC_S64_T        llERR_U_POWER     = 0;
    GSC_S64_T        llERR_POWER       = 0;
    GSC_S64_T        llTmp             = 0;
#ifdef GSC_FIXED
    GSC_S64_T        iTmp              = 0;
#endif
    S8               asTa[GSC_FFTBINS] = {0};
    /* Zone for index */
    S32 iFilterLen                  = GSC_FFTBINS * iGscTaps;
    S32 iBinIdx  = 0;
    S32 iBeamIdx = 0;
    S32 iTapsIdx = 0;
#if 0
    /* Dump the mic1/mic2 data in frequency domain. */
    S32 iLoop;
    pcpxMic1Data = pcpxY;
    pcpxMic2Data = pcpxY + GSC_FFTBINS;

    for (iLoop = 0; iLoop < GSC_FFTBINS; iLoop++)
    {
#ifdef GSC_FIXED
        //printf("%d %d\n", pcpxMic2Data->real, pcpxMic2Data->imag);
#else
        //printf("%.15f %.15f\n", pcpxMic2Data->real, pcpxMic2Data->imag);
#endif
        pcpxMic1Data++;
        pcpxMic2Data++;
    }

#endif

    /* Update wind noise status. */
    if (pstGscCfg->iNoiseDec)
    {
        gscUpdateWindNoiseState(pstGscApi, pcpxY);
    }

    for (iBeamIdx = 0; iBeamIdx < iGscNum; iBeamIdx++)
    {
        /*
         * REF=sum(B(:,:).'.*Y(:,:,kk),2);
        */
        pcpxMic1Data = pcpxY;
        pcpxMic2Data = pcpxY + GSC_FFTBINS;
        pcpxRef1     = (GSC_COMPLEX_T *)pstGscInst->pcpxMic1;
        pcpxRef2     = (GSC_COMPLEX_T *)pstGscInst->pcpxMic2;

        for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
        {
            /*
            * target signal pass through W matrix and noise through Block matrix
            */
#ifdef GSC_FIXED
            /* noise */
            pcpxRef1->real = PSHR((long long)pcpx32Mic1B->real * pcpxMic1Data->real
                                  - (long long)pcpx32Mic1B->imag * pcpxMic1Data->imag, AISP_TSL_Q24_SHIFT);
            pcpxRef1->imag = PSHR((long long)pcpx32Mic1B->real * pcpxMic1Data->imag
                                  + (long long)pcpx32Mic1B->imag * pcpxMic1Data->real, AISP_TSL_Q24_SHIFT);
            pcpxRef2->real = PSHR((long long)pcpx32Mic2B->real * pcpxMic2Data->real
                                  - (long long)pcpx32Mic2B->imag * pcpxMic2Data->imag, AISP_TSL_Q24_SHIFT);
            pcpxRef2->imag = PSHR((long long)pcpx32Mic2B->real * pcpxMic2Data->imag
                                  + (long long)pcpx32Mic2B->imag * pcpxMic2Data->real, AISP_TSL_Q24_SHIFT);
#else
            /* Noise */
            pcpxRef1->real = pcpx32Mic1B->real * pcpxMic1Data->real - pcpx32Mic1B->imag * pcpxMic1Data->imag;
            pcpxRef1->imag = pcpx32Mic1B->real * pcpxMic1Data->imag + pcpx32Mic1B->imag * pcpxMic1Data->real;
            pcpxRef2->real = pcpx32Mic2B->real * pcpxMic2Data->real - pcpx32Mic2B->imag * pcpxMic2Data->imag;
            pcpxRef2->imag = pcpx32Mic2B->real * pcpxMic2Data->imag + pcpx32Mic2B->imag * pcpxMic2Data->real;
#endif
            pcpxRef1++;
            pcpxRef2++;
            pcpx32Mic1B++;
            pcpx32Mic2B++;
            pcpxMic1Data++;
            pcpxMic2Data++;
        }

        /* Sum REF */
        pcpxREF  = (GSC_COMPLEX_T *)pstGscInst->pcpxMic1;
        pcpxRef2 = (GSC_COMPLEX_T *)pstGscInst->pcpxMic2;

        for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
        {
            pcpxREF->real += pcpxRef2->real;
            pcpxREF->imag += pcpxRef2->imag;
            pcpxREF++;
            pcpxRef2++;
        }

        /* Updata X */
        pcpxREF     = (GSC_COMPLEX_T *)pstGscInst->pcpxMic1;
        pcpxTargetX = pcpxGscX + GSC_FFTBINS * iTapCounter;

        for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
        {
            /*  Agsc.X(:,1) = REF */
            pcpxTargetX->real = pcpxREF->real;
            pcpxTargetX->imag = pcpxREF->imag;
            /* The variable llERR_U_POWER is used temporarily, please ignore the name. */
#ifdef GSC_FIXED
            llERR_U_POWER = (long long)pcpxREF->real * pcpxREF->real + (long long)pcpxREF->imag *
                            pcpxREF->imag; /* W64Q30 = Q15*Q15 */
            *pllXPowerSum = PSHR((long long)iGsc_alpha * (*pllXPowerSum), 15) + llERR_U_POWER; /* W64Q30 = W64Q15*W64Q30/Q15 */
#else
            llERR_U_POWER = pcpxREF->real * pcpxREF->real + pcpxREF->imag * pcpxREF->imag;
            *pllXPowerSum = iGsc_alpha * (*pllXPowerSum) + llERR_U_POWER;
#endif
            pcpxTargetX++;
            pcpxREF++;
            pllXPowerSum++;
        }

        /*
         * MIC=sum(Wq(:,:).'.*Y(:,:,kk),2);
        */
        pcpxMic1Data = pcpxY;
        pcpxMic2Data = pcpxY + GSC_FFTBINS;
        pcpxMic1     = (GSC_COMPLEX_T *)pstGscInst->pcpxMic1;
        pcpxMic2     = (GSC_COMPLEX_T *)pstGscInst->pcpxMic2;

        for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
        {
            /*
            * target signal pass through W matrix and noise through Block matrix
            */
#ifdef GSC_FIXED
            pcpxMic1->real = PSHR((long long)pcpx32Mic1Wq->real * pcpxMic1Data->real
                                  - (long long)pcpx32Mic1Wq->imag * pcpxMic1Data->imag, AISP_TSL_Q24_SHIFT);/* W32Q15=W32Q24*W32Q15>>Q24 */
            pcpxMic1->imag = PSHR((long long)pcpx32Mic1Wq->real * pcpxMic1Data->imag
                                  + (long long)pcpx32Mic1Wq->imag * pcpxMic1Data->real, AISP_TSL_Q24_SHIFT);
            pcpxMic2->real = PSHR((long long)pcpx32Mic2Wq->real * pcpxMic2Data->real
                                  - (long long)pcpx32Mic2Wq->imag * pcpxMic2Data->imag, AISP_TSL_Q24_SHIFT);
            pcpxMic2->imag = PSHR((long long)pcpx32Mic2Wq->real * pcpxMic2Data->imag
                                  + (long long)pcpx32Mic2Wq->imag * pcpxMic2Data->real, AISP_TSL_Q24_SHIFT);
#else
            /* Target signal */
            pcpxMic1->real = pcpx32Mic1Wq->real * pcpxMic1Data->real - pcpx32Mic1Wq->imag * pcpxMic1Data->imag;
            pcpxMic1->imag = pcpx32Mic1Wq->real * pcpxMic1Data->imag + pcpx32Mic1Wq->imag * pcpxMic1Data->real;
            pcpxMic2->real = pcpx32Mic2Wq->real * pcpxMic2Data->real - pcpx32Mic2Wq->imag * pcpxMic2Data->imag;
            pcpxMic2->imag = pcpx32Mic2Wq->real * pcpxMic2Data->imag + pcpx32Mic2Wq->imag * pcpxMic2Data->real;
#endif
            pcpxMic1++;
            pcpxMic2++;
            pcpx32Mic1Wq++;
            pcpx32Mic2Wq++;
            pcpxMic1Data++;
            pcpxMic2Data++;
        }

        /* Switch W&B to next beam */
        pcpx32Mic1Wq += GSC_FFTBINS;
        pcpx32Mic2Wq += GSC_FFTBINS;
        pcpx32Mic1B  += GSC_FFTBINS;
        pcpx32Mic2B  += GSC_FFTBINS;
        /* Sum MIC */
        pcpxMIC  = (GSC_COMPLEX_T *)pstGscInst->pcpxMic1;
        pcpxMic2 = (GSC_COMPLEX_T *)pstGscInst->pcpxMic2;

        for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
        {
            pcpxMIC->real += pcpxMic2->real;
            pcpxMIC->imag += pcpxMic2->imag;
            pcpxMIC++;
            pcpxMic2++;
        }

        /* Reset EST */
        pcpxEST     = (GSC_COMPLEX_T *)pstGscInst->pcpxMic2;

        for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
        {
            pcpxEST->real = 0;
            pcpxEST->imag = 0;
            pcpxEST++;
        }

        /* calc EST */
        pcpxTargetW = pcpxWeight;

        for (iTapsIdx = 0; iTapsIdx < iGscTaps; iTapsIdx++)
        {
            pcpxTargetX  = pcpxGscX + ((iGscTaps + iTapCounter - iTapsIdx) % iGscTaps) * GSC_FFTBINS;
            pcpxEST      = (GSC_COMPLEX_T *)pstGscInst->pcpxMic2;

            for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
            {
#ifdef GSC_FIXED
                /* W32Q15=W32Q15*W32Q24 >>Q24 */
                pcpxEST->real += PSHR((long long)pcpxTargetX->real * pcpxTargetW->real - (long long)pcpxTargetX->imag *
                                      pcpxTargetW->imag, 24);
                pcpxEST->imag += PSHR((long long)pcpxTargetX->real * pcpxTargetW->imag + (long long)pcpxTargetX->imag *
                                      pcpxTargetW->real, 24);
#else
                pcpxEST->real += pcpxTargetX->real * pcpxTargetW->real - pcpxTargetX->imag * pcpxTargetW->imag;
                pcpxEST->imag += pcpxTargetX->real * pcpxTargetW->imag + pcpxTargetX->imag * pcpxTargetW->real;
#endif
                pcpxTargetW++;
                pcpxTargetX++;
                pcpxEST++;
            }
        }

        /* calc ta */
        pcpxMic1Data = pcpxY;
        pcpxMic2Data = pcpxY + GSC_FFTBINS;

        for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
        {
#ifdef GSC_FIXED
            llTmp = ((long long)pcpxMic1Data->real * pcpxMic1Data->real + (long long)pcpxMic1Data->imag * pcpxMic1Data->imag)
                    - iTa_thre * ((long long)pcpxMic2Data->real * pcpxMic2Data->real + (long long)pcpxMic2Data->imag * pcpxMic2Data->imag);
#else
            llTmp = (pcpxMic1Data->real * pcpxMic1Data->real + pcpxMic1Data->imag * pcpxMic1Data->imag)
                    - iTa_thre * (pcpxMic2Data->real * pcpxMic2Data->real + pcpxMic2Data->imag * pcpxMic2Data->imag);
#endif

            if (iTa_switch == 0)
            {
                asTa[iBinIdx] = 1;
            }
            else
            {
                if (llTmp > 0)
                {
                    asTa[iBinIdx] = iTa_val;
                }
                else
                {
                    asTa[iBinIdx] = 1;
                }
            }

            pcpxMic1Data++;
            pcpxMic2Data++;
        }

        /* calc block out = MIC - EST */
        pcpxMIC  = (GSC_COMPLEX_T *)pstGscInst->pcpxMic1;
        pcpxEST  = (GSC_COMPLEX_T *)pstGscInst->pcpxMic2;

        for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
        {
            pcpxTmp->real = pcpxMIC->real - pcpxEST->real;
            pcpxTmp->imag = pcpxMIC->imag - pcpxEST->imag;
            pcpxTmp++;
            pcpxMIC++;
            pcpxEST++;
        }

        /* Update ERR_PRE_NORM */
        pcpxTmp     = pcpxOut;
        pllXPowerSum = (GSC_S64_T *)pstGscInst->pGscXPowerSum;
        pcpxErrPreNorm = (GSC_COMPLEX64_T *)pstGscInst->pcpxMic1;

        for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
        {
#ifdef GSC_FIXED
            llERR_POWER = (long long)pcpxTmp->real * pcpxTmp->real + (long long)pcpxTmp->imag * pcpxTmp->imag;/* W64Q30=Q15*Q15 */
            llTmp = (*pllXPowerSum) + (long long)iErrGain * llERR_POWER;/* W64Q30 */
#else
            llERR_POWER = pcpxTmp->real * pcpxTmp->real + pcpxTmp->imag * pcpxTmp->imag;
            llTmp = (*pllXPowerSum) + iErrGain * llERR_POWER;
#endif
#ifdef GSC_FIXED
            pcpxErrPreNorm->real = PDIV64(((long long)pcpxTmp->real << 39), llTmp + 1); /* Q24 = (Q15 << 39)/Q30 */
            pcpxErrPreNorm->imag = PDIV64(((long long)pcpxTmp->imag << 39), llTmp + 1);
#else
            pcpxErrPreNorm->real = pcpxTmp->real / llTmp;
            pcpxErrPreNorm->imag = pcpxTmp->imag / llTmp;
#endif
            pcpxTmp++;
            pllXPowerSum++;
            pcpxErrPreNorm++;
        }

        /* Update weight */
        pcpxTargetW = pcpxWeight;

        for (iTapsIdx = 0; iTapsIdx < iGscTaps; iTapsIdx++)
        {
            pcpxErrPreNorm = (GSC_COMPLEX64_T *)pstGscInst->pcpxMic1;
            /* pcpxTargetX points at the head one */
            pcpxTargetX  = pcpxGscX + ((iGscTaps + iTapCounter - iTapsIdx) % iGscTaps) * GSC_FFTBINS;

            for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
            {
#ifdef GSC_FIXED
                icpxEST_U.real = PSHR((long long)pcpxErrPreNorm->real * pcpxTargetX->real + (long long)pcpxErrPreNorm->imag *
                                      pcpxTargetX->imag, 15);/* W32Q24 = Q24xQ15 >> 15 */
                icpxEST_U.imag = PSHR((long long)pcpxErrPreNorm->imag * pcpxTargetX->real - (long long)pcpxErrPreNorm->real *
                                      pcpxTargetX->imag, 15);
                pcpxTargetW->real += PSHR((long long)iMu * asTa[iBinIdx] * icpxEST_U.real, 24);/* W32Q24 = Q24xQ24 >> 24 */
                pcpxTargetW->imag += PSHR((long long)iMu * asTa[iBinIdx] * icpxEST_U.imag, 24);
#else
                pcpxTargetW->real += iMu * asTa[iBinIdx] * (pcpxErrPreNorm->real * pcpxTargetX->real + pcpxErrPreNorm->imag *
                                     pcpxTargetX->imag);
                pcpxTargetW->imag += iMu * asTa[iBinIdx] * (pcpxErrPreNorm->imag * pcpxTargetX->real - pcpxErrPreNorm->real *
                                     pcpxTargetX->imag);
#endif
                pcpxTargetW++;
                pcpxTargetX++;
                pcpxErrPreNorm++;
            }
        }

        /* if Wnorm_switch == 1 */
        if (iWnorm_switch == 1)
        {
            pcpxTargetW = pcpxWeight;

            for (iTapsIdx = 0; iTapsIdx < iGscTaps; iTapsIdx++)
            {
                for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
                {
#ifdef GSC_FIXED
                    iTmp = PSHR((long long)pcpxTargetW->real * pcpxTargetW->real + (long long)pcpxTargetW->imag * pcpxTargetW->imag,
                                24); //Q24 = Q24xQ24 >> 24
#else
                    llTmp = pcpxTargetW->real * pcpxTargetW->real + pcpxTargetW->imag * pcpxTargetW->imag;
#endif
#ifdef GSC_FIXED

                    if (iTmp >= iWnorm_thre)
                    {
                        pcpxTargetW->real = PDIV64((long long)pcpxTargetW->real * iWnorm_sqrtthre, iTmp + 1) ;
                        pcpxTargetW->imag = PDIV64((long long)pcpxTargetW->imag * iWnorm_sqrtthre, iTmp + 1) ;
                    }

#else

                    if (llTmp >= iWnorm_thre)
                    {
                        pcpxTargetW->real = pcpxTargetW->real / llTmp * iWnorm_sqrtthre;
                        pcpxTargetW->imag = pcpxTargetW->imag / llTmp * iWnorm_sqrtthre;
                    }

#endif
                    pcpxTargetW++;
                }
            }
        }

        /* switch to next W and X. */
        pcpxWeight += iFilterLen;
        pcpxGscX   += iFilterLen;
    }

    /* Update the tap index. */
    pstGscCfg->iGscCurxIdx = ++iTapCounter % iGscTaps;
    return;
}

/************************************************************
  Function   : dataPop()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/11/13, Youhai.Jiang create

************************************************************/
#ifdef LUDA_SUPPORT_FREQOUT_DIRECT
VOID dataPop(GSC_Api_S *pstGscApi, VOID *pvData)
{
    S32 iLoop                   = 0;
    S32 iGscNum                 = pstGscApi->pstGscCfg->iGsc;
    GSC_COMPLEX_T *pcpxEntBlock = NULL;

    for (iLoop = 0; iLoop < iGscNum; iLoop++)
    {
        pcpxEntBlock = ((GSC_COMPLEX_T *)pvData) + iLoop * GSC_FFTBINS;

        if (pstGscApi->pfuncGscOutputHook)
        {
            pstGscApi->pfuncGscOutputHook(pstGscApi->pvUsrData, iLoop, (S8 *)pcpxEntBlock, GSC_FFTBINS * sizeof(GSC_COMPLEX_T));
        }
    }
}
#else
VOID dataPop(GSC_Api_S *pstGscApi, VOID *pvData)
{
    GSC_S32_T     *psHannWindow  = NULL;
    GSC_S32_T     *piOut         = NULL;
    GSC_S16_T     *psDataLeft    = (GSC_S16_T *)pstGscApi->pstGscInstance->psDataLeft;
    GSC_COMPLEX_T *pcpxEntBlock  = NULL;
    GSC_COMPLEX_T *pcpxEntOff    = NULL;
    GSC_COMPLEX_T *pcpxTop       = NULL;
    GSC_COMPLEX_T *pcpxBtm       = NULL;
    GSC_COMPLEX_T acpxSe[GSC_FRMLEN];
    GSC_S32_T     aiOut[GSC_FRMLEN];
    S32 iGscNum = pstGscApi->pstGscCfg->iGsc;
    S16 aFixedOut[GSC_FRMLEN];
    S32 iOffset = GSC_FFTBINS - 1;
    S32 iLoop;
    S8  cGscIdx;

    for (cGscIdx = 0; cGscIdx < iGscNum; cGscIdx++)
    {
        piOut        = aiOut;
        pcpxTop      = &acpxSe[1];
        pcpxBtm      = &acpxSe[GSC_FFTBINS];
        psHannWindow  = pstGscApi->psHanningWind;
        pcpxEntBlock = ((GSC_COMPLEX_T *)pvData) + GSC_FFTBINS * cGscIdx;
        pcpxEntOff   = pcpxEntBlock + GSC_FFTBINS - 2;
        /* DC overwrite */
        acpxSe[0].real = pcpxEntBlock[0].real;
        acpxSe[0].imag = 0;
        acpxSe[iOffset].real = pcpxEntBlock[iOffset].real;
        acpxSe[iOffset].imag = 0;
        /* pcpxEntBlock points at entBlock[1] */
        pcpxEntBlock++;

        for (iLoop = 1; iLoop < iOffset; iLoop++, pcpxEntOff--)
        {
            pcpxTop->real = pcpxEntBlock->real;
            pcpxTop->imag = pcpxEntBlock->imag;
            pcpxBtm->real =  pcpxEntOff->real;
            pcpxBtm->imag = -pcpxEntOff->imag;
            pcpxEntBlock++;
            pcpxTop++;
            pcpxBtm++;
        }

#if 0

        for (iLoop = 0; iLoop < GSC_FFTLEN; iLoop++)
        {
            printf("acpxSe[%3d] = %f + %f\n", iLoop, acpxSe[iLoop].real, acpxSe[iLoop].imag);
        }

#endif
#ifdef GSC_FIXED
#ifdef STATIC_MEM_FIXED_FFT
        /* do iFFT */
        AISP_TSL_IFFT(pstGscApi->stFFTEng, acpxSe, piOut);
#else
        float in[GSC_FFTLEN << 1];
        float out[GSC_FFTLEN];

        for (iLoop = 0; iLoop < GSC_FRMLEN; iLoop++)
        {
            in[2 * iLoop] = (float)acpxSe[iLoop].real / AISP_TSL_Q15_ONE;
            in[2 * iLoop + 1] = (float)acpxSe[iLoop].imag / AISP_TSL_Q15_ONE;
        }

        AISP_TSL_IFFT(pstGscApi->stFFTEng, in, out);

        for (iLoop = 0; iLoop < GSC_FRMLEN; iLoop++)
        {
            piOut[iLoop] = round(out[iLoop] * (1 << (AISP_TSL_Q15_SHIFT - 9)));
        }

#endif
#else
        /* do iFFT */
        AISP_TSL_IFFT(pstGscApi->stFFTEng, acpxSe, piOut);
#endif

        /* fftScalar is the amplification factor of ifft */
        for (iLoop = 0; iLoop < GSC_FRMLEN; iLoop++)
        {
#ifdef GSC_FIXED
            *piOut = PSHR((long long) * piOut * *psHannWindow, AISP_TSL_Q31_SHIFT);
            piOut++;
            psHannWindow++;
#else
            //printf("out[%d]: %.8f\n", iLoop, (float)piOut[iLoop]/512);
            *piOut++ *= *psHannWindow++ / GSC_FFTLEN;
#endif
        }

        /* output data overlap */
        piOut = aiOut + GSC_FRMINC;

        for (iLoop = 0; iLoop < GSC_FRMINC; iLoop++)
        {
            /* output data GSC_FRMINC points per time, without overflow protection by default. */
            aFixedOut[iLoop] = (aiOut[iLoop] + *psDataLeft) * GSC_S16_SCALAR;
            /* store bottom half data in */
            *psDataLeft++ = *piOut++;
        }

        /* pDataLeft points at next beam */
        if (pstGscApi->pfuncGscOutputHook)
        {
            pstGscApi->pfuncGscOutputHook(pstGscApi->pvUsrData, cGscIdx, (S8 *)aFixedOut, GSC_FRMINC << 1);
        }
    }
}
#endif

