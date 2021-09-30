#include "AISP_TSL_base.h"
#include "AISP_TSL_common.h"
#include "AISP_TSL_math.h"

#include "gsc_core_types.h"

#ifdef USE_CM4_OPTIMIZE
#define USE_CM4_OPTIMIZE_ENHANCE
#include "arm_math.h"

//extern void print_log(const char *fmt, ...);
#endif

#include "stdio.h"

#ifdef JIELI_BF_OPTIMIZATION_FFT
    /* +- 1-bit error */
    extern void jl_fft_512(int *in , int *out);
    extern void jl_ifft_512(int *in , int *out);
#endif // JIELI_BF_OPTIMIZATION_FFT

#define JIELI_BF_OPTIMIZATION_CMUL /* bit-exact */
#define JIELI_BF_OPTIMIZATION_CMAC /* bit-exact */
#define JIELI_BF_OPTIMIZATION_CMACWITHGAIN /* +- 1-bit error because of AISP_TSL_cmacWithGain_vec_32x32_2 version*/
#define JIELI_BF_OPTIMIZATION_BITTRUE
#define JIELI_BF_OPTIMIZATION_DIV64 /* +- 1-bit error */

#ifdef JIELI_BF_OPTIMIZATION_CMUL
    extern VOID AISP_TSL_cmul_vec_32x32(S32 *piSrcA, S32 *piSrcB, S32 *piDst, S32 len, S32 shift);
#endif // JIELI_BF_OPTIMIZATION_CMUL
#ifdef JIELI_BF_OPTIMIZATION_CMAC
    extern VOID AISP_TSL_cmac_vec_32x32(S32* piSrcA, S32* piSrcB, S32* piDst, S32 len, S32 shift);
#endif // JIELI_BF_OPTIMIZATION_CMAC
#ifdef JIELI_BF_OPTIMIZATION_CMACWITHGAIN
    extern VOID AISP_TSL_cmacWithGain_vec_32x32_2(S32* piSrcA, S32* piSrcB, S32* piDst, S32 iLen, S32 iShiftFst, S32 iShiftSnd, S32 iGain);
#endif // JIELI_BF_OPTIMIZATION_CMACWITHGAIN
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

    /* First input is only a half frame */
    if (pstGscApi->pstGscCfg->iInitFlag)
    {
#ifndef JIELI_BF_OPTIMIZATION_BITTRUE
        switch (GSC_MICS)
        {
            case 2:
                {
#endif
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
                        /* raw data is 32Q24 for 512 points fft */
                        *(pMic1Raw + GSC_FRMINC) = PSHR((long long)(*pHanningBtm) * (*psRaw),
                                                        AISP_TSL_Q22_SHIFT);/* W32Q24 = Q31xQ15 >> 22 */
                        *(pMic2Raw + GSC_FRMINC) = PSHR((long long)(*pHanningBtm) * (*(psRaw + 1)), AISP_TSL_Q22_SHIFT);
#else
                        *(pMic1Raw + GSC_FRMINC) = *pHanningBtm * *psRaw * GSC_Q15_SCALAR;
                        *(pMic2Raw + GSC_FRMINC) = *pHanningBtm * *(psRaw + 1) * GSC_Q15_SCALAR;
#endif
                        pHanningBtm++;
                        pMic1Raw++;
                        pMic2Raw++;
                    }

#ifdef GSC_FIXED
#ifdef STATIC_MEM_FIXED_FFT
    #ifdef JIELI_BF_OPTIMIZATION_FFT
                    /* Not bit-exact. */
                    jl_fft_512(aiRawData,              (int *)pcpxMic1FFT);
                    jl_fft_512(aiRawData + GSC_FRMLEN, (int *)pcpxMic2FFT);
    #else
                    /* input 32Q24 output 32Q15 */
                    AISP_TSL_FFT(pstGscApi->stFFTEng, aiRawData, pcpxMic1FFT);
                    AISP_TSL_FFT(pstGscApi->stFFTEng, aiRawData + GSC_FRMLEN, pcpxMic2FFT);
    #endif // JIELI_BF_OPTIMIZATION_FFT
#else
                    float in1[GSC_FFTLEN];
                    float in2[GSC_FFTLEN];
                    float out1[GSC_FFTBINS << 1];
                    float out2[GSC_FFTBINS << 1];
                    float scalar = 1.0 / AISP_TSL_Q24_ONE;

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
    #ifdef JIELI_BF_OPTIMIZATION_FFT
                    /* Q24 ==> Q15, aXFFT stores Frequency data */
                    for (sLoop = 0; sLoop < GSC_FFTBINS; sLoop++)
                    {
                        pcpxMic1XFFT->real = PSHR(pcpxMic1FFT->real, 9);
                        pcpxMic1XFFT->imag = PSHR(pcpxMic1FFT->imag, 9);
                        pcpxMic2XFFT->real = PSHR(pcpxMic2FFT->real, 9);
                        pcpxMic2XFFT->imag = PSHR(pcpxMic2FFT->imag, 9);

                        pcpxMic1FFT++;  pcpxMic2FFT++;
                        pcpxMic1XFFT++; pcpxMic2XFFT++;
                    }
    #else
                    /* aXFFT stores Frequency data */
                    for (sLoop = 0; sLoop < GSC_FFTBINS; sLoop++)
                    {
                        *pcpxMic1XFFT++ = *pcpxMic1FFT++;
                        *pcpxMic2XFFT++ = *pcpxMic2FFT++;
                    }
    #endif // JIELI_BF_OPTIMIZATION_FFT
#ifndef JIELI_BF_OPTIMIZATION_BITTRUE
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
#endif
    }
    else
    {
        pstGscApi->pstGscCfg->iInitFlag = 1;
        iRet = 1;
    }

    psRaw = (S16 *)pcData;
#ifndef JIELI_BF_OPTIMIZATION_BITTRUE
    switch (GSC_MICS)
    {
        case 2:
            {
#endif
                GSC_S32_T *pMic1LeftRaw = piLeftRaw;
                GSC_S32_T *pMic2LeftRaw = piLeftRaw + GSC_FRMINC;

                for (sLoop = 0; sLoop < GSC_FRMINC; sLoop++, psRaw += GSC_IN_CHAN)
                {
#ifdef GSC_FIXED
                    *pMic1LeftRaw++ = PSHR((long long)(*psHanningWin) * (*psRaw),     AISP_TSL_Q22_SHIFT);/* W32Q24 = Q31xQ15 >> 22 */
                    *pMic2LeftRaw++ = PSHR((long long)(*psHanningWin) * (*(psRaw + 1)), AISP_TSL_Q22_SHIFT);
                    psHanningWin++;
#else
                    *pMic1LeftRaw++ = *psHanningWin * *psRaw * GSC_Q15_SCALAR;
                    *pMic2LeftRaw++ = *psHanningWin * *(psRaw + 1) * GSC_Q15_SCALAR;
                    psHanningWin++;
#endif
                }
#ifndef JIELI_BF_OPTIMIZATION_BITTRUE
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
#endif
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
                        /* raw data is 32Q24 for 512 points fft */
                        *(pMic1Raw + GSC_FRMINC) = PSHR((long long)(*pHanningBtm) * (*psRaw1), AISP_TSL_Q22_SHIFT);
                        *(pMic2Raw + GSC_FRMINC) = PSHR((long long)(*pHanningBtm) * (*psRaw2), AISP_TSL_Q22_SHIFT);
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
    #ifdef JIELI_BF_OPTIMIZATION_FFT
                    /* Not bit-exact. */
                    jl_fft_512(aiRawData,              (int *)pcpxMic1FFT);
                    jl_fft_512(aiRawData + GSC_FRMLEN, (int *)pcpxMic2FFT);
    #else
                    /* input 32Q24 output 32Q15 */
                    AISP_TSL_FFT(pstGscApi->stFFTEng, aiRawData, pcpxMic1FFT);
                    AISP_TSL_FFT(pstGscApi->stFFTEng, aiRawData + GSC_FRMLEN, pcpxMic2FFT);
    #endif
#else
                    float in1[GSC_FFTLEN];
                    float in2[GSC_FFTLEN];
                    float out1[GSC_FFTBINS << 1];
                    float out2[GSC_FFTBINS << 1];
                    float scalar = 1.0 / AISP_TSL_Q24_ONE;

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
    #ifdef JIELI_BF_OPTIMIZATION_FFT
                    /* Q24 ==> Q15, aXFFT stores Frequency data */
                    for (sLoop = 0; sLoop < GSC_FFTBINS; sLoop++)
                    {
                        pcpxMic1XFFT->real = PSHR(pcpxMic1FFT->real, 9);
                        pcpxMic1XFFT->imag = PSHR(pcpxMic1FFT->imag, 9);
                        pcpxMic2XFFT->real = PSHR(pcpxMic2FFT->real, 9);
                        pcpxMic2XFFT->imag = PSHR(pcpxMic2FFT->imag, 9);

                        pcpxMic1FFT++;  pcpxMic2FFT++;
                        pcpxMic1XFFT++; pcpxMic2XFFT++;
                    }
    #else
                    /* aXFFT stores Frequency data */
                    for (sLoop = 0; sLoop < GSC_FFTBINS; sLoop++)
                    {
                        *pcpxMic1XFFT++ = *pcpxMic1FFT++;
                        *pcpxMic2XFFT++ = *pcpxMic2FFT++;
                    }
    #endif
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
                    *pMic1LeftRaw++ = PSHR((long long)(*psHanningWin) * (*psRaw1), AISP_TSL_Q22_SHIFT);
                    *pMic2LeftRaw++ = PSHR((long long)(*psHanningWin) * (*psRaw2), AISP_TSL_Q22_SHIFT);
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
  Function   : gscProcessHome()

  Description: gsc process only for home
  Calls      :
  Called By  :
  Input      : pY stores as [2 x 257] complex num
  Output     : acpxBlockOut stores as [3 x 257] complex num
  Return     :
  Others     :
               1.Shared memory is used as follow:
                           pcpxMic1 ------------ pcpxMIC
                                    | 257 Real |
                                    ------------
                                    | 257 Imag |
                           pcpxMic2 ------------ pcpxEST
                                    | 257 Real |
                                    ------------
                                    | 257 Imag |
                           pcpxRef1 ------------ pcpxREF(pcpxErrPreNorm)
                                    | 257 Real |
                                    ------------
                                    | 257 Imag |
                           pcpxRef2 ------------
                                    | 257 Real |
                                    ------------
                                    | 257 Imag |
                                    ------------
  History    :
    2019/09/11, donglei.Xu create
************************************************************/
VOID gscProcess(GSC_Api_S *pstGscApi, GSC_COMPLEX_T *pcpxY)
{
    GSC_Core_S       *pstGscInst    = pstGscApi->pstGscInstance;
    GSC_CoreCfg_S    *pstGscCfg     = pstGscApi->pstGscCfg;
    /* Zone for gsc struct */
    GSC_COMPLEX_T   *pcpxTargetX    = NULL;
    GSC_COMPLEX_T   *pcpxTargetW    = NULL;
    GSC_COMPLEX_T   *pcpxErrPreNorm = NULL;
    GSC_COMPLEX_T   *pcpxMIC        = NULL;
    GSC_COMPLEX_T   *pcpxREF        = NULL;
    GSC_COMPLEX_T   *pcpxEST        = NULL;
    GSC_COMPLEX_T   *pcpxMic1Data   = NULL;
    GSC_COMPLEX_T   *pcpxMic2Data   = NULL;
    GSC_COMPLEX_T   *pcpxMic1       = NULL;
    GSC_COMPLEX_T   *pcpxMic2       = NULL;
    GSC_COMPLEX_T   *pcpxRef1       = NULL;
    GSC_COMPLEX_T   *pcpxRef2       = NULL;
    GSC_COMPLEX_T   *pcpxERR        = (GSC_COMPLEX_T *)pstGscInst->pcpxBlockOut;
    GSC_COMPLEX_T   *pcpxGscX       = (GSC_COMPLEX_T *)pstGscInst->pcpxGscX;
    GSC_COMPLEX_T   *pcpxWeight     = (GSC_COMPLEX_T *)pstGscInst->pcpxGscWeight;
    GSC_S64_T       *pllXPowerSum   = (GSC_S64_T *)pstGscInst->pGscXPowerSum;
    GSC_S64_T       *pllXPowerSumSmooth = (GSC_S64_T *)pstGscInst->pGscXPowerSumSmooth;
    GSC_S64_T       *pllGscXPower   = (GSC_S64_T *)pstGscInst->pGscXPower;
    GSC_S64_T       *pllTargetXPwr  = NULL;
    GSC_S64_T       *pllDeltaFloor  = (GSC_S64_T *)pstGscInst->pDeltaFloor;
    GSC_S64_T       *pllErrPower    = (GSC_S64_T *)pstGscInst->pErrPower;
    /* Variables for W&B matrix */
    GSC_COMPLEX_T   *pcpx32B        = (GSC_COMPLEX_T *)pstGscInst->pcpxGscB;
    GSC_COMPLEX_T   *pcpx32Wq       = (GSC_COMPLEX_T *)pstGscInst->pcpxGscWq;
    GSC_COMPLEX_T   *pcpx32Mic1Wq   = pcpx32Wq;
    GSC_COMPLEX_T   *pcpx32Mic2Wq   = pcpx32Wq + GSC_FFTBINS;
    GSC_COMPLEX_T   *pcpx32Mic1B    = pcpx32B;
    GSC_COMPLEX_T   *pcpx32Mic2B    = pcpx32B  + GSC_FFTBINS;
    /* Variables for coefficients */
    S32              iTapCounter    = pstGscCfg->iGscCurxIdx;
    S32              iGscNum        = pstGscCfg->iGsc;
    S32              iGscTaps       = pstGscCfg->iTaps;
    S32              iDeltaGain     = pstGscCfg->iDeltaGain;
    GSC_S64_T        iMaxDelta      = pstGscCfg->iMaxDelta;
    GSC_S32_T        iupStep        = pstGscCfg->iupStep;
    GSC_S32_T        idownStep      = pstGscCfg->idownStep;
    GSC_S32_T        iGamma_pre     = pstGscCfg->iGamma_pre;
    GSC_S32_T        iMu            = pstGscCfg->iMu;
    /* Zone for temporary variables */
    GSC_COMPLEX64_T  llcpxERR = {0, 0};
#ifdef JIELI_BF_OPTIMIZATION_DIV64
    GSC_S32_T        iTmp            = 0;
    GSC_S64_T        llTmpPwr        = 0;
#endif // 1
    GSC_S64_T        llTmp           = 0;
    GSC_S64_T        llDelta         = 0;
#ifdef GSC_FIXED
    GSC_COMPLEX_T    icpxPreNomaMuTa = {0, 0};
#endif
    /* Zone for index */
    S32 iFilterLen                  = GSC_FFTBINS * iGscTaps;
    S32 iBinIdx  = 0;
    S32 iBeamIdx = 0;
    S32 iTapsIdx = 0;
#if 0
    /*Dump the mic1/mic2 data in frequency domain. */
    S32 iLoop;
    pcpxMic1Data = pcpxY;
    pcpxMic2Data = pcpxY + GSC_FFTBINS;

    for (iLoop = 0; iLoop < GSC_FFTBINS; iLoop++)
    {
#ifdef GSC_FIXED
        printf("%d %d\n", pcpxMic1Data->real, pcpxMic1Data->imag);
        printf("%d %d\n", pcpxMic2Data->real, pcpxMic2Data->imag);
#else
        printf("%.15f %.15f\n", pcpxMic2Data->real, pcpxMic2Data->imag);
#endif
        pcpxMic1Data++;
        pcpxMic2Data++;
    }

#endif

    for (iBeamIdx = 0; iBeamIdx < iGscNum; iBeamIdx++)
    {
        /*
         * MIC = sum(Wq(:,:,beamnum).'.*Y(:,:,kk),2); % MIC: 257x1
         * REF = sum( B(:,:,beamnum).'.*Y(:,:,kk),2); % REF: 257x1
         */
        pcpxMic1Data = pcpxY;
        pcpxMic2Data = pcpxY + GSC_FFTBINS;
        pcpxMic1     = (GSC_COMPLEX_T *)pstGscInst->pcpxMic1;
        pcpxMic2     = (GSC_COMPLEX_T *)pstGscInst->pcpxMic2;
        pcpxRef1     = (GSC_COMPLEX_T *)pstGscInst->pcpxRef1;
        pcpxRef2     = (GSC_COMPLEX_T *)pstGscInst->pcpxRef2;
#ifdef JIELI_BF_OPTIMIZATION_CMUL
    #ifdef JIELI_BF_OPTIMIZATION_CMAC
        AISP_TSL_cmul_vec_32x32((S32 *)pcpx32Mic1Wq, (S32 *)pcpxMic1Data, (S32 *)pcpxMic1, GSC_FFTBINS, AISP_TSL_Q24_SHIFT);
        AISP_TSL_cmac_vec_32x32((S32 *)pcpx32Mic2Wq, (S32 *)pcpxMic2Data, (S32 *)pcpxMic1, GSC_FFTBINS, AISP_TSL_Q24_SHIFT);
        AISP_TSL_cmul_vec_32x32((S32 *)pcpx32Mic1B,  (S32 *)pcpxMic1Data, (S32 *)pcpxRef1, GSC_FFTBINS, AISP_TSL_Q24_SHIFT);
        AISP_TSL_cmac_vec_32x32((S32 *)pcpx32Mic2B,  (S32 *)pcpxMic2Data, (S32 *)pcpxRef1, GSC_FFTBINS, AISP_TSL_Q24_SHIFT);
    #else
        AISP_TSL_cmul_vec_32x32((S32 *)pcpx32Mic1Wq, (S32 *)pcpxMic1Data, (S32 *)pcpxMic1, GSC_FFTBINS, AISP_TSL_Q24_SHIFT);
        AISP_TSL_cmul_vec_32x32((S32 *)pcpx32Mic2Wq, (S32 *)pcpxMic2Data, (S32 *)pcpxMic2, GSC_FFTBINS, AISP_TSL_Q24_SHIFT);
        AISP_TSL_cmul_vec_32x32((S32 *)pcpx32Mic1B,  (S32 *)pcpxMic1Data, (S32 *)pcpxRef1, GSC_FFTBINS, AISP_TSL_Q24_SHIFT);
        AISP_TSL_cmul_vec_32x32((S32 *)pcpx32Mic2B,  (S32 *)pcpxMic2Data, (S32 *)pcpxRef2, GSC_FFTBINS, AISP_TSL_Q24_SHIFT);
    #endif
        /* Switch W&B to next beam */
        pcpx32Mic1Wq += 514;
        pcpx32Mic2Wq += 514;
        pcpx32Mic1B  += 514;
        pcpx32Mic2B  += 514;
#else
        for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
        {
            /*
             * target signal pass through W matrix and noise through Block matrix
             */
#ifdef GSC_FIXED
            pcpxMic1->real = PSHR((long long)pcpx32Mic1Wq->real * pcpxMic1Data->real
                                  - (long long)pcpx32Mic1Wq->imag * pcpxMic1Data->imag, AISP_TSL_Q24_SHIFT);
            pcpxMic1->imag = PSHR((long long)pcpx32Mic1Wq->real * pcpxMic1Data->imag
                                  + (long long)pcpx32Mic1Wq->imag * pcpxMic1Data->real, AISP_TSL_Q24_SHIFT);
            pcpxMic2->real = PSHR((long long)pcpx32Mic2Wq->real * pcpxMic2Data->real
                                  - (long long)pcpx32Mic2Wq->imag * pcpxMic2Data->imag, AISP_TSL_Q24_SHIFT);
            pcpxMic2->imag = PSHR((long long)pcpx32Mic2Wq->real * pcpxMic2Data->imag
                                  + (long long)pcpx32Mic2Wq->imag * pcpxMic2Data->real, AISP_TSL_Q24_SHIFT);
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
            /* Target signal */
            pcpxMic1->real = pcpx32Mic1Wq->real * pcpxMic1Data->real - pcpx32Mic1Wq->imag * pcpxMic1Data->imag;
            pcpxMic1->imag = pcpx32Mic1Wq->real * pcpxMic1Data->imag + pcpx32Mic1Wq->imag * pcpxMic1Data->real;
            pcpxMic2->real = pcpx32Mic2Wq->real * pcpxMic2Data->real - pcpx32Mic2Wq->imag * pcpxMic2Data->imag;
            pcpxMic2->imag = pcpx32Mic2Wq->real * pcpxMic2Data->imag + pcpx32Mic2Wq->imag * pcpxMic2Data->real;
            /* Noise */
            pcpxRef1->real = pcpx32Mic1B->real * pcpxMic1Data->real - pcpx32Mic1B->imag * pcpxMic1Data->imag;
            pcpxRef1->imag = pcpx32Mic1B->real * pcpxMic1Data->imag + pcpx32Mic1B->imag * pcpxMic1Data->real;
            pcpxRef2->real = pcpx32Mic2B->real * pcpxMic2Data->real - pcpx32Mic2B->imag * pcpxMic2Data->imag;
            pcpxRef2->imag = pcpx32Mic2B->real * pcpxMic2Data->imag + pcpx32Mic2B->imag * pcpxMic2Data->real;
#endif
            pcpxMic1++;
            pcpxMic2++;
            pcpxRef1++;
            pcpxRef2++;
            pcpx32Mic1Wq++;
            pcpx32Mic2Wq++;
            pcpx32Mic1B++;
            pcpx32Mic2B++;
            pcpxMic1Data++;
            pcpxMic2Data++;
        }

        /* Switch W&B to next beam */
        pcpx32Mic1Wq += GSC_FFTBINS;
        pcpx32Mic2Wq += GSC_FFTBINS;
        pcpx32Mic1B  += GSC_FFTBINS;
        pcpx32Mic2B  += GSC_FFTBINS;
#endif // JIELI_BF_OPTIMIZATION_CMUL

#ifndef JIELI_BF_OPTIMIZATION_CMAC
        /* Sum mic1 and mic2 */
        pcpxMIC  = (GSC_COMPLEX_T *)pstGscInst->pcpxMic1;
        pcpxMic2 = (GSC_COMPLEX_T *)pstGscInst->pcpxMic2;
        pcpxREF  = (GSC_COMPLEX_T *)pstGscInst->pcpxRef1;
        pcpxRef2 = (GSC_COMPLEX_T *)pstGscInst->pcpxRef2;

        for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
        {
            /* Notice: assumes only two mics */
            pcpxMIC->real += pcpxMic2->real;
            pcpxMIC->imag += pcpxMic2->imag;
            pcpxREF->real += pcpxRef2->real;
            pcpxREF->imag += pcpxRef2->imag;
            pcpxMIC++;
            pcpxMic2++;
            pcpxREF++;
            pcpxRef2++;
        }
#endif // JIELI_BF_OPTIMIZATION_CMAC

        /* Updata X, and save the ref spec into buffer */
        pcpxTargetX    = (GSC_COMPLEX_T *)pcpxGscX + GSC_FFTBINS * iTapCounter;
        pcpxREF        = (GSC_COMPLEX_T *)pstGscInst->pcpxRef1;
        pcpxEST        = (GSC_COMPLEX_T *)pstGscInst->pcpxMic2;
        pllTargetXPwr  = (GSC_S64_T *)pllGscXPower + GSC_FFTBINS * iTapCounter;
        pllXPowerSum  = (GSC_S64_T *)pstGscInst->pGscXPowerSum;

        for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
        {
            /* Updata X, and save the ref spec into buffer */
            pcpxTargetX->real = pcpxREF->real;
            pcpxTargetX->imag = pcpxREF->imag;
            /* Update X_POWER, and save the ref spec power into buffer */
#ifdef GSC_FIXED
            *pllTargetXPwr = PSHR_POSITIVE((long long)pcpxREF->real * pcpxREF->real
                                           + (long long)pcpxREF->imag * pcpxREF->imag, AISP_TSL_Q15_SHIFT);
#else
            *pllTargetXPwr = pcpxREF->real * pcpxREF->real + pcpxREF->imag * pcpxREF->imag;
#endif
            pcpxEST->real = 0;
            pcpxEST->imag = 0;
            *pllXPowerSum = 0;
            pcpxTargetX++;
            pllTargetXPwr++;
            pcpxREF++;
            pcpxEST++;
            pllXPowerSum++;
        }

        /* Calc XPowerSum and EST */
        pcpxTargetW   = (GSC_COMPLEX_T *)pcpxWeight;

        for (iTapsIdx = 0; iTapsIdx < iGscTaps; iTapsIdx++)
        {
            pcpxTargetX   = pcpxGscX + ((iGscTaps + iTapCounter - iTapsIdx) % iGscTaps) * GSC_FFTBINS;
            pllTargetXPwr = pllGscXPower + ((iGscTaps + iTapCounter - iTapsIdx) % iGscTaps) * GSC_FFTBINS;
            pcpxEST       = (GSC_COMPLEX_T *)pstGscInst->pcpxMic2;
            pllXPowerSum  = (GSC_S64_T *)pstGscInst->pGscXPowerSum;
#ifdef JIELI_BF_OPTIMIZATION_CMAC
            for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
            {
                *pllXPowerSum += *pllTargetXPwr;
                pllXPowerSum++; pllTargetXPwr++;
            }
            AISP_TSL_cmac_vec_32x32((S32 *)pcpxTargetW, (S32 *)pcpxTargetX, (S32 *)pcpxEST, GSC_FFTBINS, AISP_TSL_Q24_SHIFT);
            pcpxTargetW += GSC_FFTBINS;
#else
            for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
            {
#ifdef GSC_FIXED
                pcpxEST->real += PSHR((long long)pcpxTargetX->real * pcpxTargetW->real
                                      - (long long)pcpxTargetX->imag * pcpxTargetW->imag, AISP_TSL_Q24_SHIFT);
                pcpxEST->imag += PSHR((long long)pcpxTargetX->real * pcpxTargetW->imag
                                      + (long long)pcpxTargetX->imag * pcpxTargetW->real, AISP_TSL_Q24_SHIFT);
#else
                pcpxEST->real += pcpxTargetX->real * pcpxTargetW->real - pcpxTargetX->imag * pcpxTargetW->imag;
                pcpxEST->imag += pcpxTargetX->real * pcpxTargetW->imag + pcpxTargetX->imag * pcpxTargetW->real;
#endif
                *pllXPowerSum += *pllTargetXPwr;
                pcpxEST++;
                pcpxTargetX++;
                pcpxTargetW++;
                pllXPowerSum++;
                pllTargetXPwr++;
            }
#endif // JIELI_BF_OPTIMIZATION_CMAC
        }

        /* Calc block out = MIC - EST */
        pcpxMIC        = (GSC_COMPLEX_T *)pstGscInst->pcpxMic1;
        pcpxEST        = (GSC_COMPLEX_T *)pstGscInst->pcpxMic2;
        pcpxErrPreNorm = (GSC_COMPLEX_T *)pstGscInst->pcpxRef1;
        pllXPowerSum   = (GSC_S64_T *)pstGscInst->pGscXPowerSum;
        pllDeltaFloor  = (GSC_S64_T *)pstGscInst->pDeltaFloor;

        for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
        {
            pcpxERR->real = pcpxMIC->real - pcpxEST->real;
            pcpxERR->imag = pcpxMIC->imag - pcpxEST->imag;
            llTmp = (*pllErrPower * 16 + *pllXPowerSumSmooth) * iDeltaGain;
            llDelta  = AISP_TSL_MIN(AISP_TSL_MAX(llTmp, *pllDeltaFloor), iMaxDelta);

            /* smooth X_POWER_SUM */
            if (*pllXPowerSum > *pllXPowerSumSmooth)
            {
#ifdef GSC_FIXED
    #if 0
                *pllXPowerSumSmooth = PSHR((long long)(*pllXPowerSumSmooth) * 2113123910 + (long long)(
                                               *pllXPowerSum) * 34359738, AISP_TSL_Q31_SHIFT);
    #else
                *pllXPowerSumSmooth = PSHR((long long)(*pllXPowerSumSmooth) * (AISP_TSL_Q31_ONE - iupStep) + (long long)(
                                               *pllXPowerSum) * iupStep, AISP_TSL_Q31_SHIFT);
    #endif
#else
                *pllXPowerSumSmooth = (*pllXPowerSumSmooth) * (1 - iupStep) + (*pllXPowerSum) * iupStep;
#endif
            }
            else
            {
#ifdef GSC_FIXED
    #if 0
                *pllXPowerSumSmooth = PSHR((long long)(*pllXPowerSumSmooth) * 2147270618 + (long long)(
                                               *pllXPowerSum) * 213030, AISP_TSL_Q31_SHIFT);
    #else
                *pllXPowerSumSmooth = PSHR((long long)(*pllXPowerSumSmooth) * (AISP_TSL_Q31_ONE - idownStep) + (long long)(
                                               *pllXPowerSum) * idownStep, AISP_TSL_Q31_SHIFT);
    #endif
#else
                *pllXPowerSumSmooth = (*pllXPowerSumSmooth) * (1 - idownStep) + (*pllXPowerSum) * idownStep;
#endif
            }

            /* normalize error spec */
#ifdef GSC_FIXED
#ifdef JIELI_BF_OPTIMIZATION_DIV64
    #if 1
            llTmpPwr = *pllXPowerSum + llDelta;
            if (llTmpPwr > 2147483647)
            {
                llTmp    = ((long long)1073741824 + (llTmpPwr >> 1)) / llTmpPwr;/* Q30/Q15 */
            }
            else
            {
                iTmp     = (int)(llTmpPwr);
                llTmpPwr = (long long)1073741824 + (iTmp >> 1);
                llTmp    = llTmpPwr / iTmp;/* Q30/Q15 */
            }
    #else
            llTmpPwr = *pllXPowerSum + llDelta;
            llTmp    = ((long long)1073741824 + (llTmpPwr >> 1)) / llTmpPwr;/* Q30/Q15 */
    #endif
            pcpxErrPreNorm->real = PSHR((long long)pcpxERR->real * llTmp, AISP_TSL_Q15_SHIFT);
            pcpxErrPreNorm->imag = PSHR((long long)pcpxERR->imag * llTmp, AISP_TSL_Q15_SHIFT);
#else
            pcpxErrPreNorm->real = PDIV64((long long)pcpxERR->real << 15, *pllXPowerSum + llDelta);
            pcpxErrPreNorm->imag = PDIV64((long long)pcpxERR->imag << 15, *pllXPowerSum + llDelta);
#endif
#else
            llTmp = 1.0 / (*pllXPowerSum + llDelta);
            pcpxErrPreNorm->real = pcpxERR->real * llTmp;
            pcpxErrPreNorm->imag = pcpxERR->imag * llTmp;
#endif
            /* update posteriori error to ERR */
#ifdef GSC_FIXED
            llcpxERR.real = pcpxERR->real - PSHR((long long)PSHR_POSITIVE((long long)iGamma_pre * (*pllXPowerSum),
                                                 AISP_TSL_Q15_SHIFT) * pcpxErrPreNorm->real, AISP_TSL_Q15_SHIFT);
            llcpxERR.imag = pcpxERR->imag - PSHR((long long)PSHR_POSITIVE((long long)iGamma_pre * (*pllXPowerSum),
                                                 AISP_TSL_Q15_SHIFT) * pcpxErrPreNorm->imag, AISP_TSL_Q15_SHIFT);
            *pllErrPower = PSHR_POSITIVE((long long)llcpxERR.real * llcpxERR.real + (long long)llcpxERR.imag * llcpxERR.imag,
                                         AISP_TSL_Q15_SHIFT);
#else
            llcpxERR.real = pcpxERR->real - iGamma_pre * (*pllXPowerSum) * pcpxErrPreNorm->real;
            llcpxERR.imag = pcpxERR->imag - iGamma_pre * (*pllXPowerSum) * pcpxErrPreNorm->imag;
            *pllErrPower = llcpxERR.real * llcpxERR.real + llcpxERR.imag * llcpxERR.imag;
#endif
            pcpxERR++;
            pcpxMIC++;
            pcpxEST++;
            pllXPowerSumSmooth++;
            pllErrPower++;
            pllDeltaFloor++;
            pllXPowerSum++;
            pcpxErrPreNorm++;
        }

        /* Update W */
        pcpxTargetW = (GSC_COMPLEX_T *)pcpxWeight;

        for (iTapsIdx = 0; iTapsIdx < iGscTaps; iTapsIdx++)
        {
            pcpxErrPreNorm = (GSC_COMPLEX_T *)pstGscInst->pcpxRef1;
            /* pcpxTargetX points at the head one */
            pcpxTargetX  = pcpxGscX + ((iGscTaps + iTapCounter - iTapsIdx) % iGscTaps) * GSC_FFTBINS;
#ifdef JIELI_BF_OPTIMIZATION_CMACWITHGAIN
            AISP_TSL_cmacWithGain_vec_32x32_2(pcpxErrPreNorm, pcpxTargetX, pcpxTargetW, GSC_FFTBINS, 15, 6, iMu);
            pcpxTargetW += GSC_FFTBINS;
#else

            for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
            {
#ifdef GSC_FIXED
                icpxPreNomaMuTa.real = PSHR((long long)iMu * pcpxErrPreNorm->real, AISP_TSL_Q15_SHIFT);
                icpxPreNomaMuTa.imag = PSHR((long long)iMu * pcpxErrPreNorm->imag, AISP_TSL_Q15_SHIFT);
                pcpxTargetW->real += PSHR((long long)icpxPreNomaMuTa.real * pcpxTargetX->real
                                          + (long long)icpxPreNomaMuTa.imag * pcpxTargetX->imag , 6);
                pcpxTargetW->imag += PSHR((long long)icpxPreNomaMuTa.imag * pcpxTargetX->real
                                          - (long long)icpxPreNomaMuTa.real * pcpxTargetX->imag , 6);
#else
                pcpxTargetW->real += iMu * (pcpxErrPreNorm->real * pcpxTargetX->real
                                            + pcpxErrPreNorm->imag * pcpxTargetX->imag);
                pcpxTargetW->imag += iMu * (pcpxErrPreNorm->imag * pcpxTargetX->real
                                            - pcpxErrPreNorm->real * pcpxTargetX->imag);
#endif
                pcpxTargetW++;
                pcpxTargetX++;
                pcpxErrPreNorm++;
            }
#endif // JIELI_BF_OPTIMIZATION_CMACWITHGAIN
        }

        /* Matrix W and X is 257 x Taps x beams, so switch to next W and X. */
        pcpxWeight += iFilterLen;
        pcpxGscX   += iFilterLen;
        pllGscXPower += iFilterLen;
    }/* for (iBeamIdx=0; iBeamIdx < iGscNum; iBeamIdx++) */

    /* Update the tap index. */
    pstGscCfg->iGscCurxIdx = ++iTapCounter % iGscTaps;
    return;
}

/*calc power*/
void gsc_energySum(GSC_Api_S *pstGscApi,GSC_COMPLEX_T *freq,S8 cGscIdx)
{
    GSC_ENERGY_S *pstGscEnergy = pstGscApi->pstGscInstance->pstGscEnergy;
    GSC_S64_T *pEng = pstGscEnergy->pBeamPow + cGscIdx * GSC_ENERGY_FRMLEN;
    GSC_S64_T tmp = 0;
    int i;
    for(i = GSC_ENERGY_START ; i < GSC_ENERGY_END; i++ )
    {
        tmp += (GSC_S64_T)freq[i].imag * freq[i].imag + (GSC_S64_T)freq[i].real * freq[i].real;
    }
    pEng[pstGscEnergy->ucIndex] = tmp;
    if(cGscIdx == (pstGscApi->pstGscCfg->iGsc - 1))
    {
        pstGscEnergy->ucIndex++;
        if(pstGscEnergy->ucIndex == GSC_ENERGY_FRMLEN)
        {
            pstGscEnergy->ucIndex = 0;
        }
    }

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
    GSC_S16_T     *psDataLeft    = NULL;
    GSC_COMPLEX_T *pcpxEntBlock  = NULL;
    GSC_COMPLEX_T *pcpxEntOff    = NULL;
    GSC_COMPLEX_T *pcpxTop       = NULL;
    GSC_COMPLEX_T *pcpxBtm       = NULL;
    GSC_COMPLEX_T acpxSe[GSC_FRMLEN];
    GSC_S32_T     aiOut[GSC_FRMLEN];
    S32 iGscNum = 0;
    S16 aFixedOut[GSC_FRMLEN];
    S32 iOffset = GSC_FFTBINS - 1;
    S32 iLoop;
    S8  cGscIdx;
    iGscNum    = pstGscApi->pstGscCfg->iGsc;
    psDataLeft = (GSC_S16_T *)pstGscApi->pstGscInstance->psDataLeft;

    for (cGscIdx = 0; cGscIdx < iGscNum; cGscIdx++)
    {
        piOut        = aiOut;
        pcpxTop      = &acpxSe[1];
        pcpxBtm      = &acpxSe[GSC_FFTBINS];
        psHannWindow  = pstGscApi->psHanningWind;
        pcpxEntBlock = ((GSC_COMPLEX_T *)pvData) + GSC_FFTBINS * cGscIdx;
        pcpxEntOff   = pcpxEntBlock + GSC_FFTBINS - 2;

        gsc_energySum(pstGscApi,pcpxEntBlock,cGscIdx);
        
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

#ifdef GSC_FIXED
#ifdef STATIC_MEM_FIXED_FFT
        /* do iFFT */
    #ifdef JIELI_BF_OPTIMIZATION_FFT
        /* Not bit-exact. */
        jl_ifft_512((int *)acpxSe, piOut);
    #else
        AISP_TSL_IFFT(pstGscApi->stFFTEng, acpxSe, piOut);
    #endif
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



