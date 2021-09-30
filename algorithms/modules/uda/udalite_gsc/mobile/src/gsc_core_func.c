#include "AISP_TSL_base.h"
#include "AISP_TSL_common.h"
#include "AISP_TSL_math.h"

#include "gsc_core_types.h"

#ifdef USE_CM4_OPTIMIZE
#define USE_CM4_OPTIMIZE_ENHANCE
#include "arm_math.h"

//extern void print_log(const char *fmt, ...);
#endif

#ifndef GSC_ENABLE_HALF_VAD
static VOID rdmaLinearConv(GSC_S32_T *piX, GSC_S32_T *piFilter, GSC_S32_T *piY, S32 iXLen, S32 iFilterLen, S32 iYLen);
#endif

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
                    /* input 32Q24 output 32Q15 */
                    AISP_TSL_FFT(pstGscApi->stFFTEng, aiRawData, pcpxMic1FFT);
                    AISP_TSL_FFT(pstGscApi->stFFTEng, aiRawData + GSC_FRMLEN, pcpxMic2FFT);
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
                    *pMic1LeftRaw++ = PSHR((long long)(*psHanningWin) * (*psRaw),     AISP_TSL_Q22_SHIFT);/* W32Q24 = Q31xQ15 >> 22 */
                    *pMic2LeftRaw++ = PSHR((long long)(*psHanningWin) * (*(psRaw + 1)), AISP_TSL_Q22_SHIFT);
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
                    /* input 32Q24 output 32Q15 */
                    AISP_TSL_FFT(pstGscApi->stFFTEng, aiRawData, pcpxMic1FFT);
                    AISP_TSL_FFT(pstGscApi->stFFTEng, aiRawData + GSC_FRMLEN, pcpxMic2FFT);
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
  Function   : gscUpdateVAD()

  Description: Switch the VAD status
  Calls      :
  Called By  :
  Input      : pY stores as [2 x 257] complex num
  Output     :
  Return     :
  Others     :
               1.Shared memory is used as follow:
                           pcpxMic1 ------------ pcpxMIC/pllMic1Power
                                    | 257 Real |
                                    ------------
                                    | 257 Imag |
                           pcpxMic2 ------------ pcpxEST/pllMic2Power
                                    | 257 Real |
                                    ------------
                                    | 257 Imag |
                           pcpxRef1 ------------ pcpxREF/pllMicAbsBin
                                    | 257 Real |
                                    ------------
                                    | 257 Imag |
                           pcpxRef2 ------------ pcpxErrPreNorm/pllRefNewSum
                                    | 257 Real |
                                    ------------
                                    | 257 Imag |
                                    ------------
               2.Shared memory size in function 'gscUpdateVAD', be careful with type 'GSC_S64_T':
                   GSC_S64_T allMic1Power[GSC_FFTBINS] = {0};
                   GSC_S64_T allMic2Power[GSC_FFTBINS] = {0};
                   GSC_S64_T allMicAbsBin[GSC_FFTBINS] = {0};
                   GSC_S64_T allRefNewSum[GSC_FFTBINS] = {0};

  History    :
    2019/04/08, Chao.Xu create
    2019/05/07, Chao.Xu finish fixed-point version, bit-exact with Mics_ULA2micMobile_20190325_fixed

************************************************************/
#ifndef GSC_ENABLE_HALF_VAD
S32 gscUpdateVAD(GSC_Api_S *pstGscApi, GSC_COMPLEX_T *pcpxY)
{
    GSC_S32_T aiGscPH1_f[288] = {0}; /* fftBins + size(hanning_15) - 1, and aligned 8 bytes */
    GSC_Core_S   *pstGscInst = pstGscApi->pstGscInstance;
    GSC_CoreCfg_S *pstGscCfg = pstGscApi->pstGscCfg;
    /* Zone for configuration. */
    GSC_S32_T iLogGLRFact    = pstGscCfg->iLogGLRFact;
    GSC_S32_T iGLRexp        = pstGscCfg->iGLRexp;
#ifndef GSC_FIXED
    GSC_S32_T iPriorFact     = pstGscCfg->iPriorFact;
#endif
    GSC_S32_T iAlphaPH1mean  = pstGscCfg->iAlphaPH1mean;
    GSC_S32_T iAlphaPSD      = pstGscCfg->iAlphaPSD;
    /* Zone for variable. */
    GSC_S64_T *pllMic1Power  = (GSC_S64_T *)pstGscInst->pcpxMic1;
#ifndef FULLZEROJUDGEWAY
    GSC_S64_T *pllMic2Power  = (GSC_S64_T *)pstGscInst->pcpxMic2;
    GSC_S32_T *pllMicAbsBin  = (GSC_S32_T *)pstGscInst->pcpxRef1;
#endif
    GSC_S64_T *pllRefNewSum  = (GSC_S64_T *)
                               pstGscInst->pcpxRef2;/* Shared with pcpxMic1/pcpxMic2/pcpxRef1/pcpxRef2, be careful. */
    GSC_COMPLEX_T *pcpxMic1  = pcpxY;
    GSC_COMPLEX_T *pcpxMic2  = pcpxY + GSC_FFTBINS; /* Mic1 and mic2 data in frequency domain */
    GSC_S64_T *pllGscNoisePSD = pstGscInst->pGscNoisePSD;
    GSC_S32_T *piGscPH1      = pstGscInst->pGscPH1;
    GSC_S32_T *piGscPH1_f    = aiGscPH1_f;
    GSC_S32_T *piPH1mean     = pstGscInst->pGscPH1mean;
    /* Zone for immediate variable. */
    GSC_S64_T llMicAbsSum    = 0;
    GSC_S64_T llTmp          = 0;
    GSC_S64_T llSnrPost1     = 0;
    GSC_S32_T iTmp           = 0;
#ifndef GSC_FIXED
    GSC_S32_T iGLR           = 0;
#endif
    S32 iBins                = 0;
    //S32 iLoop                = 0;
#ifndef FULLZEROJUDGEWAY
    S32 iFlgAnyGTthresh      = 0;
#endif
#ifdef FULLZEROJUDGEWAY
    S32 iFlgNonFullZero      = 0;
#endif
#if 0//def USE_CM4_OPTIMIZE_ENHANCE
    float pfMic1Dst[GSC_FFTBINS];
    float pfMic2Dst[GSC_FFTBINS];
    float pfTmpMic1[GSC_FFTBINS << 1];
    float pfTmpMic2[GSC_FFTBINS << 1];
#endif
    /* Record the index of the executed times */
    pstGscCfg->iIdxExeFrm++;
    /* Calculate the power for each mic. */
#if 0//def USE_CM4_OPTIMIZE_ENHANCE

    for (iBins = 0, iLoop = 0; iBins < GSC_FFTBINS; iBins++, iLoop += 2)
    {
        pfTmpMic1[iLoop] = pcpxMic1[iBins].real;
        pfTmpMic1[iLoop + 1] = pcpxMic1[iBins].imag;
        pfTmpMic2[iLoop] = pcpxMic2[iBins].real;
        pfTmpMic2[iLoop + 1] = pcpxMic2[iBins].imag;
    }

    arm_cmplx_mag_squared_f32(pfTmpMic1, pfMic1Dst, GSC_FFTBINS);
    arm_cmplx_mag_squared_f32(pfTmpMic2, pfMic2Dst, GSC_FFTBINS);

    /* update power of mic1 and mic2 */
    for (iBins = 0; iBins < GSC_FFTBINS; iBins++)
    {
        /* power is Q24 */
        pllMic1Power[iBins] = PSHR_POSITIVE((GSC_S64_T)(pfMic1Dst[iBins] + 0.5), AISP_TSL_Q6_SHIFT);
        pllMic2Power[iBins] = PSHR_POSITIVE((GSC_S64_T)(pfMic2Dst[iBins] + 0.5), AISP_TSL_Q6_SHIFT);
        arm_sqrt_f32(pllMic1Power[iBins], &pfMic1Dst[iBins]);
        arm_sqrt_f32(pllMic2Power[iBins], &pfMic2Dst[iBins]);
    }

    //arm_cmplx_mag_f32 (pfTmpMic1, pfMic1Dst, GSC_FFTBINS);
    //arm_cmplx_mag_f32 (pfTmpMic2, pfMic2Dst, GSC_FFTBINS);
    //arm_vsqrt_f32 (pfMic1Dst, pfMic1Dst, GSC_FFTBINS);
    //arm_vsqrt_f32 (pfMic2Dst, pfMic2Dst, GSC_FFTBINS);
    /* sum */
    arm_add_f32(pfMic1Dst, pfMic2Dst, pfMic1Dst, GSC_FFTBINS);

    for (iBins = 0; iBins < GSC_FFTBINS; iBins++)
    {
        pllMicAbsBin[iBins] = (int)(pfMic1Dst[iBins] + 0.5);
        llMicAbsSum += pllMicAbsBin[iBins];
    }

#else

    for (iBins = 0; iBins < GSC_FFTBINS; iBins++)
    {
#ifdef GSC_FIXED
        *pllMic1Power = (long long)pcpxMic1->real * pcpxMic1->real + (long long)pcpxMic1->imag *
                        pcpxMic1->imag;/* Fixed: W64Q30 = Q15xQ15 */
#ifndef FULLZEROJUDGEWAY
        *pllMic2Power = (long long)pcpxMic2->real * pcpxMic2->real + (long long)pcpxMic2->imag * pcpxMic2->imag;
        *pllMicAbsBin = AISP_TSL_sqrt_xW64_yW32(*pllMic1Power) + AISP_TSL_sqrt_xW64_yW32(
                            *pllMic2Power);/* Fixed: Q15 = sqrt(Q30) */
#endif
        *pllMic1Power = PSHR_POSITIVE(*pllMic1Power, AISP_TSL_Q6_SHIFT);/* Fixed: Q24 = Q30/Q6 */
#ifndef FULLZEROJUDGEWAY
        *pllMic2Power = PSHR_POSITIVE(*pllMic2Power, AISP_TSL_Q6_SHIFT);/* Fixed: Q24 = Q30/Q6 */
#endif
#else
        *pllMic1Power = pcpxMic1->real * pcpxMic1->real + pcpxMic1->imag * pcpxMic1->imag;
#ifndef FULLZEROJUDGEWAY
        *pllMic2Power = pcpxMic2->real * pcpxMic2->real + pcpxMic2->imag * pcpxMic2->imag;
        *pllMicAbsBin = sqrt(*pllMic1Power) + sqrt(*pllMic2Power);
#endif
#endif
#ifndef FULLZEROJUDGEWAY
        llMicAbsSum  += *pllMicAbsBin;/* Fixed: W64Q15 += W32Q15 */
        pcpxMic2++;
        pllMic2Power++;
        pllMicAbsBin++;
#endif
        pcpxMic1++;
        pllMic1Power++;
    }

#endif
#ifndef FULLZEROJUDGEWAY
    pllMicAbsBin = (GSC_S32_T *)pstGscInst->pcpxRef1;

    for (iBins = 0; iBins < GSC_FFTBINS; iBins++)
    {
        if (*pllMicAbsBin > GSC_ZERO_THRESH)
        {
            iFlgAnyGTthresh = 1;
            break;
        }

        pllMicAbsBin++;
    }

#else

    for (iBins = 0; iBins < GSC_FFTBINS; iBins++)
    {
        if (pcpxMic1->real != 0 || pcpxMic1->imag != 0 || pcpxMic2->real != 0 || pcpxMic2->imag != 0)
        {
            iFlgNonFullZero = 1;
            break;
        }

        pcpxMic1++;
        pcpxMic2++;
    }

#endif
#ifndef FULLZEROJUDGEWAY

    if (((pstGscCfg->iFlgFstFrm_nonzero == 0) && (llMicAbsSum > GSC_ZERO_THRESH)) || ((pstGscCfg->iFlgFstFrm_nonzero == 1)
            && (iFlgAnyGTthresh == 1)))
#else
    if (((pstGscCfg->iFlgFstFrm_nonzero == 0) && (iFlgNonFullZero == 1)) || ((pstGscCfg->iFlgFstFrm_nonzero == 1)
            && (iFlgNonFullZero == 1)))
#endif
    {
        pstGscCfg->iFlgFstFrm_nonzero = 1;

        /* init noise PSD in 5 frame */
        if (pstGscCfg->iIdxExeFrm <= (5 + pstGscCfg->iIdxFstFrm_nonzero))
        {
            pllMic1Power = (GSC_S64_T *)pstGscInst->pcpxMic1;

            for (iBins = 0; iBins < GSC_FFTBINS; iBins++)
            {
#ifdef GSC_FIXED
                *pllGscNoisePSD += PSHR_POSITIVE((long long)3355443 * (*pllMic1Power), 24);/* Fixed: W64Q24 += Q24xQ30/Q30 */
#else
                *pllGscNoisePSD += 0.2 * (*pllMic1Power);
#endif
                pllGscNoisePSD++;
                pllMic1Power++;
            }
        }
        else
        {
            pstGscCfg->iIdxExeFrm = 6 + pstGscCfg->iIdxFstFrm_nonzero; /* Do not update noise_psd any more. */
        }

        pllMic1Power  = (GSC_S64_T *)pstGscInst->pcpxMic1;
        pllGscNoisePSD = pstGscInst->pGscNoisePSD;
        piPH1mean     = pstGscInst->pGscPH1mean;

        for (iBins = 0; iBins < GSC_FFTBINS; iBins++)
        {
            /* Attention: when pllGscNoisePSD and Mic1Power are both small, there is a big error on snrPost1. */
#ifdef GSC_FIXED
            if (pstGscCfg->iIdxExeFrm == pstGscCfg->iIdxFstFrm_nonzero)
            {
                llSnrPost1 = 83886080;/* Fixed: Q24(5) */
            }
            else
            {
                llSnrPost1 = ((*pllMic1Power << 24) + (*pllGscNoisePSD >> 1)) / (*pllGscNoisePSD +
                             1); /* Fixed: Q24 = Q48/Q24, avoid div0 2019.06.24 */
            }

            llSnrPost1 = AISP_TSL_MIN(llSnrPost1, 16777216 * 100); /* Be sure snr will not overflow. */
            llTmp      = iLogGLRFact + PSHR_POSITIVE((long long)iGLRexp * llSnrPost1, 24);/* Fixed: Q24 = Q24 + Q24xQ24/Q24 */
            iTmp       = AISP_TSL_MIN(llTmp, 20 * 16777216 - 1);
            *piGscPH1  = AISP_TSL_sigmoid_xW32Q24_yW32Q24(iTmp);
#else
            llSnrPost1  = *pllMic1Power / *pllGscNoisePSD;
            iTmp       = iLogGLRFact + iGLRexp * llSnrPost1;
            iGLR       = iPriorFact * exp(AISP_TSL_MIN(iTmp, 80.0)); /* Attention: exp will expand error, 80 by default */
            *piGscPH1  = iGLR / (1.0 + iGLR);
#endif
#ifdef GSC_FIXED
            *piPH1mean = PSHR_POSITIVE((long long)iAlphaPH1mean * (*piPH1mean) + (long long)(16777216 - iAlphaPH1mean) *
                                       (*piGscPH1), 24);/* Fixed: Q24 */

            if (*piPH1mean > 16609444)
            {
                *piGscPH1 = AISP_TSL_MIN(*piGscPH1, 16609444);
            }

            llTmp           = PSHR_POSITIVE((long long)(*piGscPH1) * (*pllGscNoisePSD) + (long long)(16777216 - *piGscPH1) *
                                            (*pllMic1Power), 24);/* W64Q24 */
            *pllGscNoisePSD = PSHR_POSITIVE((long long)iAlphaPSD * (*pllGscNoisePSD) + (long long)(16777216 - iAlphaPSD) * llTmp,
                                            24);/* W64Q24 */
#else
            *piPH1mean = iAlphaPH1mean * (*piPH1mean) + (1.0 - iAlphaPH1mean) * (*piGscPH1);

            if (*piPH1mean > 0.99)
            {
                *piGscPH1 = AISP_TSL_MIN(*piGscPH1, 0.99);
            }

            iTmp           = (*piGscPH1) * (*pllGscNoisePSD) + (1.0 - *piGscPH1) * (*pllMic1Power);
            *pllGscNoisePSD = iAlphaPSD * (*pllGscNoisePSD) + (1.0 - iAlphaPSD) * iTmp;
#endif
            pllMic1Power++;
            pllGscNoisePSD++;
            piGscPH1++;
            piPH1mean++;
        }

        /* PH1_f =  conv(fd.hanning_15,PH1);PH1_f = PH1_f(16:272); */
        rdmaLinearConv(pstGscApi->pstGscInstance->pGscPH1, pstGscApi->pstGscInstance->pHanning_15, aiGscPH1_f, GSC_FFTBINS, 31,
                       GSC_FFTBINS + 31 - 1);
        piGscPH1     = pstGscInst->pGscPH1 + 5;
        pllRefNewSum = ((GSC_S64_T *)pstGscInst->pcpxRef2) + 5;
        piGscPH1_f   = aiGscPH1_f + 15 + 5;
        llMicAbsSum  = 0;
        pllMic1Power = ((GSC_S64_T *)pstGscInst->pcpxMic1) + 5;

        for (iBins = 0; iBins < (GSC_FFTBINS - (5 + 1)); iBins++)
        {
#ifdef GSC_FIXED
            iTmp          = PSHR_POSITIVE((long long)(*piGscPH1) * (*piGscPH1_f), 24);
            *pllRefNewSum = AISP_TSL_sqrt_xW64_yW32((unsigned long long)iTmp * (*pllMic1Power));/* Fixed: Q24 = sqrt(Q48) */
#else
            *pllRefNewSum = sqrt((*piGscPH1) * (*piGscPH1_f) * (*pllMic1Power));
#endif
            llMicAbsSum  += *pllRefNewSum;
            pllRefNewSum++;
            piGscPH1++;
            piGscPH1_f++;
            pllMic1Power++;
        }

#ifdef GSC_FIXED
        llMicAbsSum  = (llMicAbsSum + ((GSC_FFTBINS - (5 + 1)) >> 1)) / (GSC_FFTBINS - (5 + 1));
#else
        llMicAbsSum /= (GSC_FFTBINS - (5 + 1));
#endif
        /* Calculate VAD_amp. */
        llTmp = 0;
        pllRefNewSum = ((GSC_S64_T *)pstGscInst->pcpxRef2) + 5;

        for (iBins = 0; iBins < (GSC_FFTBINS - (5 + 1)); iBins++)
        {
#ifdef GSC_FIXED
            llTmp += AISP_TSL_ABS(*pllRefNewSum - llMicAbsSum);
#else
            llTmp += fabs(*pllRefNewSum - llMicAbsSum);
#endif
            pllRefNewSum++;
        }

#ifdef GSC_FIXED
        llTmp  = (llTmp + ((GSC_FFTBINS - (5 + 1)) >> 1)) / (GSC_FFTBINS - (5 + 1));
#else
        llTmp /= (GSC_FFTBINS - (5 + 1));
#endif

        /* Update the state of vad. */
        if (pstGscCfg->iVAD == 0)
        {
            if (llTmp > pstGscCfg->iAmth)
            {
                pstGscCfg->iSpeCnt++;

                if (pstGscCfg->iSpeCnt >= pstGscCfg->iSil2Spe)
                {
                    pstGscCfg->iVAD    = 1;
                    pstGscCfg->iSpeCnt = 0;
                }
            }
            else
            {
                pstGscCfg->iSpeCnt = 0;
            }
        }
        else
        {
            if (llTmp < pstGscCfg->iAmth)
            {
                pstGscCfg->iSilCnt++;

                if (pstGscCfg->iSilCnt >= pstGscCfg->iSpe2Sil)
                {
                    pstGscCfg->iVAD    = 0;
                    pstGscCfg->iSilCnt = 0;
                }
            }
            else
            {
                pstGscCfg->iSilCnt = 0;
            }
        }

        return 0; /* VAD is on and normal process */
    }
    else
    {
        if (pstGscCfg->iFlgFstFrm_nonzero == 0)
        {
            pstGscCfg->iIdxFstFrm_nonzero++;
        }

        return 1; /* VAD is off and ent_block is zero */
    }
}
#else
S32 gscUpdateHalfVAD(GSC_Api_S *pstGscApi, GSC_COMPLEX_T *pcpxY)
{
    GSC_Core_S   *pstGscInst = pstGscApi->pstGscInstance;
    GSC_CoreCfg_S *pstGscCfg = pstGscApi->pstGscCfg;
    /* Zone for configuration. */
    GSC_S32_T iLogGLRFact    = pstGscCfg->iLogGLRFact;
    GSC_S32_T iGLRexp        = pstGscCfg->iGLRexp;
#ifndef GSC_FIXED
    GSC_S32_T iPriorFact     = pstGscCfg->iPriorFact;
#endif
    GSC_S32_T iAlphaPH1mean  = pstGscCfg->iAlphaPH1mean;
    GSC_S32_T iAlphaPSD      = pstGscCfg->iAlphaPSD;
    /* Zone for variable. */
    GSC_S64_T *pllMic1Power  = (GSC_S64_T *)pstGscInst->pcpxMic1;
#ifndef FULLZEROJUDGEWAY
    GSC_S64_T *pllMic2Power  = (GSC_S64_T *)pstGscInst->pcpxMic2;
    GSC_S32_T *pllMicAbsBin  = (GSC_S32_T *)pstGscInst->pcpxRef1;
#endif
    GSC_S64_T *pllRefNewSum  = (GSC_S64_T *)
                               pstGscInst->pcpxRef2;/* Shared with pcpxMic1/pcpxMic2/pcpxRef1/pcpxRef2, be careful. */
    GSC_COMPLEX_T *pcpxMic1  = pcpxY;
    GSC_COMPLEX_T *pcpxMic2  = pcpxY + GSC_FFTBINS; /* Mic1 and mic2 data in frequency domain */
    GSC_S64_T *pllGscNoisePSD = pstGscInst->pGscNoisePSD;
    GSC_S32_T *piGscPH1      = pstGscInst->pGscPH1;
    GSC_S32_T *piPH1mean     = pstGscInst->pGscPH1mean;
    /* Zone for immediate variable. */
    GSC_S64_T llMicAbsSum    = 0;
    GSC_S64_T llTmp          = 0;
    GSC_S64_T llSnrPost1     = 0;
    GSC_S32_T iTmp           = 0;
#ifndef GSC_FIXED
    GSC_S32_T iGLR           = 0;
#endif
    S32 iBins                = 0;
#ifdef FULLZEROJUDGEWAY
    S32 iFlgNonFullZero      = 0;
#else
    S32 iFlgAnyGTthresh      = 0;
#endif
    /* Record the index of the executed times */
    pstGscCfg->iIdxExeFrm++;

    /* Calculate the power for each mic. */
    for (iBins = 0; iBins < GSC_FFTBINS; iBins++)
    {
#ifdef GSC_FIXED
        *pllMic1Power = (long long)pcpxMic1->real * pcpxMic1->real + (long long)pcpxMic1->imag *
                        pcpxMic1->imag;/* Fixed: W64Q30 = Q15xQ15 */
#ifndef FULLZEROJUDGEWAY
        *pllMic2Power = (long long)pcpxMic2->real * pcpxMic2->real + (long long)pcpxMic2->imag * pcpxMic2->imag;
        *pllMicAbsBin = AISP_TSL_sqrt_xW64_yW32(*pllMic1Power) + AISP_TSL_sqrt_xW64_yW32(
                            *pllMic2Power);/* Fixed: Q15 = sqrt(Q30) */
#endif
        *pllMic1Power = PSHR_POSITIVE(*pllMic1Power, AISP_TSL_Q6_SHIFT);/* Fixed: Q24 = Q30/Q6 */
#ifndef FULLZEROJUDGEWAY
        *pllMic2Power = PSHR_POSITIVE(*pllMic2Power, AISP_TSL_Q6_SHIFT);/* Fixed: Q24 = Q30/Q6 */
#endif
#else
        *pllMic1Power = pcpxMic1->real * pcpxMic1->real + pcpxMic1->imag * pcpxMic1->imag;
#ifndef FULLZEROJUDGEWAY
        *pllMic2Power = pcpxMic2->real * pcpxMic2->real + pcpxMic2->imag * pcpxMic2->imag;
        *pllMicAbsBin = sqrt(*pllMic1Power) + sqrt(*pllMic2Power);
#endif
#endif
#ifndef FULLZEROJUDGEWAY
        llMicAbsSum  += *pllMicAbsBin;/* Fixed: W64Q15 += W32Q15 */
        pcpxMic2++;
        pllMic2Power++;
        pllMicAbsBin++;
#endif
        pcpxMic1++;
        pllMic1Power++;
    }

#ifndef FULLZEROJUDGEWAY
    pllMicAbsBin = (GSC_S32_T *)pstGscInst->pcpxRef1;

    for (iBins = 0; iBins < GSC_FFTBINS; iBins++)
    {
        if (*pllMicAbsBin > GSC_ZERO_THRESH)
        {
            iFlgAnyGTthresh = 1;
            break;
        }

        pllMicAbsBin++;
    }

#else

    for (iBins = 0; iBins < GSC_FFTBINS; iBins++)
    {
        if (pcpxMic1->real != 0 || pcpxMic1->imag != 0 || pcpxMic2->real != 0 || pcpxMic2->imag != 0)
        {
            iFlgNonFullZero = 1;
            break;
        }

        pcpxMic1++;
        pcpxMic2++;
    }

#endif
#ifndef FULLZEROJUDGEWAY

    if (((pstGscCfg->iFlgFstFrm_nonzero == 0) && (llMicAbsSum > GSC_ZERO_THRESH)) || ((pstGscCfg->iFlgFstFrm_nonzero == 1)
            && (iFlgAnyGTthresh == 1)))
#else
    if (((pstGscCfg->iFlgFstFrm_nonzero == 0) && (iFlgNonFullZero == 1)) || ((pstGscCfg->iFlgFstFrm_nonzero == 1)
            && (iFlgNonFullZero == 1)))
#endif
    {
        pstGscCfg->iFlgFstFrm_nonzero = 1;

        /* init noise PSD in 5 frame */
        if (pstGscCfg->iIdxExeFrm <= (5 + pstGscCfg->iIdxFstFrm_nonzero))
        {
            pllMic1Power = (GSC_S64_T *)pstGscInst->pcpxMic1;

            for (iBins = 15; iBins < GSC_FFTBINS - 4; iBins += 4)
            {
#ifdef GSC_FIXED
                *pllGscNoisePSD += PSHR_POSITIVE((long long)3355443 * (pllMic1Power[iBins]), 24);/* Fixed: W64Q24 += Q24xQ30/Q30 */
#else
                *pllGscNoisePSD += 0.2 * (pllMic1Power[iBins]);
#endif
                pllGscNoisePSD++;
            }
        }
        else
        {
            pstGscCfg->iIdxExeFrm = 6 + pstGscCfg->iIdxFstFrm_nonzero; /* Do not update noise_psd any more. */
        }

        pllMic1Power  = (GSC_S64_T *)pstGscInst->pcpxMic1;
        pllGscNoisePSD = pstGscInst->pGscNoisePSD;
        piPH1mean     = pstGscInst->pGscPH1mean;

        for (iBins = 15; iBins < GSC_FFTBINS - 4; iBins = iBins + 4)
        {
            /* Attention: when pllGscNoisePSD and Mic1Power are both small, there is a big error on snrPost1. */
#ifdef GSC_FIXED
            if (pstGscCfg->iIdxExeFrm == pstGscCfg->iIdxFstFrm_nonzero)
            {
                llSnrPost1 = 83886080;/* Fixed: Q24(5) */
            }
            else
            {
                llSnrPost1 = ((pllMic1Power[iBins] << 24) + (*pllGscNoisePSD >> 1)) / (*pllGscNoisePSD +
                             1); /* Fixed: Q24 = Q48/Q24, avoid div0 2019.06.24 */
            }

            llSnrPost1 = AISP_TSL_MIN(llSnrPost1, 16777216 * 100); /* Be sure snr will not overflow. */
            llTmp      = iLogGLRFact + PSHR_POSITIVE((long long)iGLRexp * llSnrPost1, 24);/* Fixed: Q24 = Q24 + Q24xQ24/Q24 */
            iTmp       = AISP_TSL_MIN(llTmp, 20 * 16777216 - 1);
            *piGscPH1  = AISP_TSL_sigmoid_xW32Q24_yW32Q24(iTmp);
#else
            llSnrPost1  = pllMic1Power[iBins] / *pllGscNoisePSD;
            iTmp       = iLogGLRFact + iGLRexp * llSnrPost1;
            iGLR       = iPriorFact * exp(AISP_TSL_MIN(iTmp, 80.0)); /* Attention: exp will expand error, 80 by default */
            *piGscPH1  = iGLR / (1.0 + iGLR);
#endif
#ifdef GSC_FIXED
            *piPH1mean = PSHR_POSITIVE((long long)iAlphaPH1mean * (*piPH1mean) + (long long)(16777216 - iAlphaPH1mean) *
                                       (*piGscPH1), 24);/* Fixed: Q24 */

            if (*piPH1mean > 16609444)
            {
                *piGscPH1 = AISP_TSL_MIN(*piGscPH1, 16609444);
            }

            llTmp           = PSHR_POSITIVE((long long)(*piGscPH1) * (*pllGscNoisePSD) + (long long)(16777216 - *piGscPH1) *
                                            (pllMic1Power[iBins]), 24);/* W64Q24 */
            *pllGscNoisePSD = PSHR_POSITIVE((long long)iAlphaPSD * (*pllGscNoisePSD) + (long long)(16777216 - iAlphaPSD) * llTmp,
                                            24);/* W64Q24 */
#else
            *piPH1mean = iAlphaPH1mean * (*piPH1mean) + (1.0 - iAlphaPH1mean) * (*piGscPH1);

            if (*piPH1mean > 0.99)
            {
                *piGscPH1 = AISP_TSL_MIN(*piGscPH1, 0.99);
            }

            iTmp           = (*piGscPH1) * (*pllGscNoisePSD) + (1.0 - *piGscPH1) * (pllMic1Power[iBins]);
            *pllGscNoisePSD = iAlphaPSD * (*pllGscNoisePSD) + (1.0 - iAlphaPSD) * iTmp;
#endif
            pllGscNoisePSD++;
            piGscPH1++;
            piPH1mean++;
        }

        piGscPH1     = pstGscInst->pGscPH1;
        pllRefNewSum = ((GSC_S64_T *)pstGscInst->pcpxRef2);
        llMicAbsSum  = 0;
        pllMic1Power = ((GSC_S64_T *)pstGscInst->pcpxMic1);

        for (iBins = 15; iBins < GSC_FFTBINS - 4; iBins = iBins + 4)
        {
#ifdef GSC_FIXED
            iTmp          = PSHR_POSITIVE((long long)(*piGscPH1) * (*piGscPH1), 24);
            *pllRefNewSum = AISP_TSL_sqrt_xW64_yW32((unsigned long long)iTmp * (pllMic1Power[iBins]));/* Fixed: Q24 = sqrt(Q48) */
#else
            *pllRefNewSum = (*piGscPH1) * sqrt(pllMic1Power[iBins]);
#endif
            llMicAbsSum  += *pllRefNewSum;
            pllRefNewSum++;
            piGscPH1++;
        }

#ifdef GSC_FIXED
        llMicAbsSum  = (llMicAbsSum + (GSC_EVENFFTBINS >> 1)) / GSC_EVENFFTBINS;
#else
        llMicAbsSum /= GSC_EVENFFTBINS;
#endif
        /* Calculate VAD_amp. */
        llTmp = 0;
        pllRefNewSum = ((GSC_S64_T *)pstGscInst->pcpxRef2);

        for (iBins = 0; iBins < GSC_EVENFFTBINS; iBins++)
        {
#ifdef GSC_FIXED
            llTmp += AISP_TSL_ABS(*pllRefNewSum - llMicAbsSum);
#else
            llTmp += fabs(*pllRefNewSum - llMicAbsSum);
#endif
            pllRefNewSum++;
        }

#ifdef GSC_FIXED
        llTmp  = (llTmp + (GSC_EVENFFTBINS >> 1)) / GSC_EVENFFTBINS;
#else
        llTmp /= GSC_EVENFFTBINS;
#endif

        /* Update the state of vad. */
        if (pstGscCfg->iVAD == 0)
        {
            if (llTmp > pstGscCfg->iAmth)
            {
                pstGscCfg->iSpeCnt++;

                if (pstGscCfg->iSpeCnt >= pstGscCfg->iSil2Spe)
                {
                    pstGscCfg->iVAD    = 1;
                    pstGscCfg->iSpeCnt = 0;
                }
            }
            else
            {
                pstGscCfg->iSpeCnt = 0;
            }
        }
        else
        {
            if (llTmp < pstGscCfg->iAmth)
            {
                pstGscCfg->iSilCnt++;

                if (pstGscCfg->iSilCnt >= pstGscCfg->iSpe2Sil)
                {
                    pstGscCfg->iVAD    = 0;
                    pstGscCfg->iSilCnt = 0;
                }
            }
            else
            {
                pstGscCfg->iSilCnt = 0;
            }
        }

        return 0; /* VAD is on and normal process */
    }
    else
    {
        if (pstGscCfg->iFlgFstFrm_nonzero == 0)
        {
            pstGscCfg->iIdxFstFrm_nonzero++;
        }

        return 1; /* VAD is off and ent_block is zero */
    }
}
#endif
/************************************************************
  Function   : gscProcessMobile()

  Description: gsc process only for mobile
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
    2019/04/24, Chao.Xu create
    2019/05/07, Chao.Xu finish fixed-point version, bit-exact with Mics_ULA2micMobile_20190325_fixed

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
    GSC_COMPLEX_T   *pcpxOut        = (GSC_COMPLEX_T *)pstGscInst->pcpxBlockOut;
    GSC_COMPLEX_T   *pcpxTmp        = pcpxOut;
    GSC_COMPLEX_T   *pcpxH          = (GSC_COMPLEX_T *)pstGscInst->pcpxGscH;
    GSC_S64_T       *pllXPowerSum   = (GSC_S64_T *)pstGscInst->pGscXPowerSum;
    GSC_S64_T       *pllYPowerSum   = (GSC_S64_T *)pstGscInst->pGscYPowerSum;
    /* Variables for W&B matrix */
    GSC_COMPLEX_T   *pcpx32B        = (GSC_COMPLEX_T *)pstGscInst->pcpxGscB;
    GSC_COMPLEX_T   *pcpx32Wq       = (GSC_COMPLEX_T *)pstGscInst->pcpxGscWq;
    GSC_COMPLEX_T   *pcpx32Mic1Wq   = pcpx32Wq;
    GSC_COMPLEX_T   *pcpx32Mic2Wq   = pcpx32Wq + GSC_FFTBINS;
    GSC_COMPLEX_T   *pcpx32Mic1B    = pcpx32B;
    GSC_COMPLEX_T   *pcpx32Mic2B    = pcpx32B  + GSC_FFTBINS;
    /* Variables for coefficients */
    GSC_S32_T        iTapsPSD       = pstGscCfg->iTapsPSD;
    GSC_S32_T        iTapsDalta     = pstGscCfg->iTapsDalta;
    GSC_S32_T        iMu            = pstGscCfg->iMu;
    S32              iTapCounter    = pstGscCfg->iGscCurxIdx;
    S32              iGscNum        = pstGscCfg->iGsc;
    S32              iGscTaps       = pstGscCfg->iTaps;
    /* Zone for temporary variables */
    GSC_COMPLEX_T    icpxU          = {0, 0};
    GSC_COMPLEX_T    icpxEST_U      = {0, 0};
    GSC_COMPLEX64_T  llcpxERR_U_PRE_NORM = {0, 0};
    GSC_S64_T        llERR_U_POWER  = 0;
    GSC_S64_T        llERR_POWER    = 0;
    GSC_S64_T        llTmp          = 0;
    /* Zone for index */
    S32 iFilterLen                  = GSC_FFTBINS * iGscTaps;
    S32 iBinIdx  = 0;
    S32 iBeamIdx = 0;
    S32 iTapsIdx = 0;
#ifdef GSC_FIXED
    S32 iBinsOffset = GSC_FFTBINS << 1;
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
#ifdef GSC_FIXED
        AISP_TSL_cmul_vec_32_32((S32 *)pcpx32Mic1Wq, (S32 *)pcpxMic1Data, (S32 *)pcpxMic1, GSC_FFTBINS, AISP_TSL_Q24_SHIFT);
        AISP_TSL_cmul_vec_32_32((S32 *)pcpx32Mic2Wq, (S32 *)pcpxMic2Data, (S32 *)pcpxMic2, GSC_FFTBINS, AISP_TSL_Q24_SHIFT);
        AISP_TSL_cmul_vec_32_32((S32 *)pcpx32Mic1B, (S32 *)pcpxMic1Data, (S32 *)pcpxRef1, GSC_FFTBINS, AISP_TSL_Q24_SHIFT);
        AISP_TSL_cmul_vec_32_32((S32 *)pcpx32Mic2B, (S32 *)pcpxMic2Data, (S32 *)pcpxRef2, GSC_FFTBINS, AISP_TSL_Q24_SHIFT);
        /* Switch W&B to next beam */
        pcpx32Mic1Wq += iBinsOffset;
        pcpx32Mic2Wq += iBinsOffset;
        pcpx32Mic1B  += iBinsOffset;
        pcpx32Mic2B  += iBinsOffset;
#else

        for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
        {
            /*
             * target signal pass through W matrix and noise through Block matrix
             */
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
#endif
        /* Sum mic1 and mic2 */
        pcpxMIC  = (GSC_COMPLEX_T *)pstGscInst->pcpxMic1;
        pcpxMic2 = (GSC_COMPLEX_T *)pstGscInst->pcpxMic2;
        pcpxREF  = (GSC_COMPLEX_T *)pstGscInst->pcpxRef1;
        pcpxRef2 = (GSC_COMPLEX_T *)pstGscInst->pcpxRef2;
#ifdef GSC_FIXED
        AISP_TSL_add_vec_32((S32 *)pcpxMIC, (S32 *)pcpxMic2, (S32 *)pcpxMIC, iBinsOffset);
        AISP_TSL_add_vec_32((S32 *)pcpxREF, (S32 *)pcpxRef2, (S32 *)pcpxREF, iBinsOffset);
#else

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

#endif

        if (pstGscCfg->iAdaBM)
        {
            pcpxMIC     = (GSC_COMPLEX_T *)pstGscInst->pcpxMic1;
            pcpxREF     = (GSC_COMPLEX_T *)pstGscInst->pcpxRef1;
            pcpxEST     = (GSC_COMPLEX_T *)pstGscInst->pcpxMic2;
            pcpxTargetX = pcpxGscX + GSC_FFTBINS * iTapCounter; /* iTapCounter: 0 0 0 1 1 1 2 2 2 3 3 3 0 0 0 1 1 1 ...... */

            for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
            {
                /* Adaptive block matrix. */
#ifdef GSC_FIXED
                llTmp         = (long long)pcpxMIC->real * pcpxMIC->real + (long long)pcpxMIC->imag *
                                pcpxMIC->imag;             /* W64Q30 = Q15xQ15 */
                *pllYPowerSum = PSHR_POSITIVE((long long)iTapsPSD * (*pllYPowerSum) + (long long)(AISP_TSL_Q15_ONE - iTapsPSD) * llTmp,
                                              15);/* W64Q15 = Q15xQ15/Q15 */
#else
                llTmp         = pcpxMIC->real * pcpxMIC->real + pcpxMIC->imag * pcpxMIC->imag;
                *pllYPowerSum = iTapsPSD * (*pllYPowerSum) + (1.0 - iTapsPSD) * llTmp;
#endif
                /* Filter, EST_U is zero at startup. */
#ifdef GSC_FIXED
                icpxEST_U.real = PSHR((long long)pcpxMIC->real * pcpxH->real - (long long)pcpxMIC->imag * pcpxH->imag,
                                      24);/* Fixed: Q15 = Q15xQ24/Q24 */
                icpxEST_U.imag = PSHR((long long)pcpxMIC->real * pcpxH->imag + (long long)pcpxMIC->imag * pcpxH->real, 24);
#else
                icpxEST_U.real = pcpxMIC->real * pcpxH->real - pcpxMIC->imag * pcpxH->imag;
                icpxEST_U.imag = pcpxMIC->real * pcpxH->imag + pcpxMIC->imag * pcpxH->real;
#endif
                /* Error */
                icpxU.real  = pcpxREF->real - icpxEST_U.real;
                icpxU.imag  = pcpxREF->imag - icpxEST_U.imag;
                /* Error power */
#ifdef GSC_FIXED
                llERR_U_POWER = (long long)icpxU.real * icpxU.real + (long long)icpxU.imag * icpxU.imag;/* W64Q30 = Q15xQ15 */
#else
                llERR_U_POWER = icpxU.real * icpxU.real + icpxU.imag * icpxU.imag;
#endif

                if (pstGscCfg->iVAD == 1)
                {
#ifdef GSC_FIXED
                    llTmp = *pllYPowerSum + iTapsDalta * llERR_U_POWER;/* Attention: iTapsDalat is 4(Q0) by default! */
                    llTmp = AISP_TSL_Q39_ONE / (llTmp + 1);
                    llcpxERR_U_PRE_NORM.real = (S64)icpxU.real * llTmp;
                    llcpxERR_U_PRE_NORM.imag = (S64)icpxU.imag * llTmp;
                    //llcpxERR_U_PRE_NORM.real = PDIV64(((long long)icpxU.real << 39), (llTmp + 1));/* Q24 = (Q15 << 39)/Q30 */
                    //llcpxERR_U_PRE_NORM.imag = PDIV64(((long long)icpxU.imag << 39), (llTmp + 1));/* Same with matlab */
#else
                    llcpxERR_U_PRE_NORM.real = icpxU.real / (*pllYPowerSum + iTapsDalta * llERR_U_POWER);
                    llcpxERR_U_PRE_NORM.imag = icpxU.imag / (*pllYPowerSum + iTapsDalta * llERR_U_POWER);
#endif
                    /* The variable 'icpxEST_U' is used temporarily, please ignore the name. */
#ifdef GSC_FIXED
                    icpxEST_U.real = PSHR((long long)llcpxERR_U_PRE_NORM.real * pcpxMIC->real + (long long)llcpxERR_U_PRE_NORM.imag *
                                          pcpxMIC->imag, 15);
                    icpxEST_U.imag = PSHR((long long)llcpxERR_U_PRE_NORM.imag * pcpxMIC->real - (long long)llcpxERR_U_PRE_NORM.real *
                                          pcpxMIC->imag, 15);
#else
                    icpxEST_U.real = llcpxERR_U_PRE_NORM.real * pcpxMIC->real + llcpxERR_U_PRE_NORM.imag * pcpxMIC->imag;
                    icpxEST_U.imag = llcpxERR_U_PRE_NORM.imag * pcpxMIC->real - llcpxERR_U_PRE_NORM.real * pcpxMIC->imag;
#endif
#ifdef GSC_FIXED
                    pcpxH->real += PSHR((long long)iMu * icpxEST_U.real, 24);
                    pcpxH->imag += PSHR((long long)iMu * icpxEST_U.imag, 24);
#else
                    pcpxH->real += iMu * icpxEST_U.real;
                    pcpxH->imag += iMu * icpxEST_U.imag;
#endif
                }/* if (pstGscCfg->iVAD == 1) */

                pcpxTargetX->real = icpxU.real;
                pcpxTargetX->imag = icpxU.imag;
                /* X_POWER_SUM */
#ifdef GSC_FIXED
                *pllXPowerSum = PSHR_POSITIVE((long long)iTapsPSD * (*pllXPowerSum), 15) + llERR_U_POWER;
#else
                *pllXPowerSum = iTapsPSD * (*pllXPowerSum) + llERR_U_POWER;
//              *pllXPowerSum = iTapsPSD * (*pllXPowerSum) + (1.0 - iTapsPSD) * llERR_U_POWER;/* Guess (1.0-iTapPSD) will lead to 'PRE_NORM' divergence. */
#endif
                /* Reset EST */
                pcpxEST->real = 0;
                pcpxEST->imag = 0;
                pcpxMIC++;
                pcpxREF++;
                pllYPowerSum++;
                pllXPowerSum++;
                pcpxH++;
                pcpxEST++;
                pcpxTargetX++;
            }
        }
        else
        {
            pcpxREF     = (GSC_COMPLEX_T *)pstGscInst->pcpxRef1;
            pcpxEST     = (GSC_COMPLEX_T *)pstGscInst->pcpxMic2;
            pcpxTargetX = pcpxGscX + GSC_FFTBINS * iTapCounter;

            for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
            {
                pcpxTargetX->real = pcpxREF->real;
                pcpxTargetX->imag = pcpxREF->imag;
                /* The variable llERR_U_POWER is used temporarily, please ignore the name. */
#ifdef GSC_FIXED
                llERR_U_POWER = (long long)pcpxREF->real * pcpxREF->real + (long long)pcpxREF->imag * pcpxREF->imag;
                *pllXPowerSum = PSHR_POSITIVE((long long)iTapsPSD * (*pllXPowerSum), 15) + llERR_U_POWER;
#else
                llERR_U_POWER = pcpxREF->real * pcpxREF->real + pcpxREF->imag * pcpxREF->imag;
                *pllXPowerSum = iTapsPSD * (*pllXPowerSum) + llERR_U_POWER;
//              *pllXPowerSum = iTapsPSD * (*pllXPowerSum) + (1.0 - iTapsPSD) * llERR_U_POWER;
#endif
                /* Reset EST */
                pcpxEST->real = 0;
                pcpxEST->imag = 0;
                pcpxTargetX++;
                pcpxREF++;
                pcpxEST++;
                pllXPowerSum++;
            }
        }/* if (LUDA_ADABM) */

        /* calc EST */
        pcpxTargetW = pcpxWeight;

        for (iTapsIdx = 0; iTapsIdx < iGscTaps; iTapsIdx++)
        {
            //pcpxTargetX  = pcpxGscX + ((iGscTaps + iTapCounter - iTapsIdx)%iGscTaps)*GSC_FFTBINS;
            pcpxTargetX  = pcpxGscX + ((iGscTaps + iTapCounter - iTapsIdx) & (iGscTaps - 1)) * GSC_FFTBINS;
            pcpxEST      = (GSC_COMPLEX_T *)pstGscInst->pcpxMic2;
#ifdef GSC_FIXED
            AISP_TSL_cmul_sum_vec_32_32((S32 *)pcpxTargetX, (S32 *)pcpxTargetW, (S32 *)pcpxEST, GSC_FFTBINS, 24);
            pcpxTargetW += GSC_FFTBINS;
#else

            for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
            {
                pcpxEST->real += pcpxTargetX->real * pcpxTargetW->real - pcpxTargetX->imag * pcpxTargetW->imag;
                pcpxEST->imag += pcpxTargetX->real * pcpxTargetW->imag + pcpxTargetX->imag * pcpxTargetW->real;
                pcpxTargetW++;
                pcpxTargetX++;
                pcpxEST++;
            }

#endif
        }

        /* calc block out = MIC - EST */
        pcpxMIC  = (GSC_COMPLEX_T *)pstGscInst->pcpxMic1;
        pcpxEST  = (GSC_COMPLEX_T *)pstGscInst->pcpxMic2;
#ifdef GSC_FIXED
        AISP_TSL_sub_vec_32((S32 *)pcpxMIC, (S32 *)pcpxEST, (S32 *)pcpxTmp, iBinsOffset);
        pcpxTmp += GSC_FFTBINS;
        pcpxMIC += GSC_FFTBINS;
        pcpxEST += GSC_FFTBINS;
#else

        for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
        {
            pcpxTmp->real = pcpxMIC->real - pcpxEST->real;
            pcpxTmp->imag = pcpxMIC->imag - pcpxEST->imag;
            pcpxTmp++;
            pcpxMIC++;
            pcpxEST++;
        }

#endif
        /* Matrix W and X is 257 x Taps x beams, so switch to next W and X. */
        pcpxWeight += iFilterLen;
        pcpxGscX   += iFilterLen;
    }/* for (iBeamIdx=0; iBeamIdx < iGscNum; iBeamIdx++) */

    if (pstGscCfg->iVAD == 0)
    {
        pcpxTmp      = pcpxOut; /* acpxBlockOut */
        pllXPowerSum = (GSC_S64_T *)pstGscInst->pGscXPowerSum;
        pcpxWeight   = (GSC_COMPLEX_T *)pstGscInst->pcpxGscWeight;
        pcpxGscX     = (GSC_COMPLEX_T *)pstGscInst->pcpxGscX;

        for (iBeamIdx = 0; iBeamIdx < iGscNum; iBeamIdx++)
        {
            pcpxErrPreNorm = (GSC_COMPLEX64_T *)pstGscInst->pcpxRef1;

            for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
            {
#ifdef GSC_FIXED
                llERR_POWER = (long long)pcpxTmp->real * pcpxTmp->real + (long long)pcpxTmp->imag * pcpxTmp->imag;
                llTmp = (*pllXPowerSum) + (long long)iTapsDalta * llERR_POWER;/* W64Q30 */
#else
                llERR_POWER = pcpxTmp->real * pcpxTmp->real + pcpxTmp->imag * pcpxTmp->imag;
                llTmp = (*pllXPowerSum) + iTapsDalta * llERR_POWER;
#endif
#ifdef GSC_FIXED
                /* Fix Bug 20190822: The reciprocal should be with high precision, here is Q30. */
                llTmp = (((long long)1 << 60) + ((llTmp + 1) >> 1)) / (llTmp + 1); /* Q30 = (Q60 + Q30/2)/Q30 */
                pcpxErrPreNorm->real = PSHR((long long)pcpxTmp->real * llTmp, 21); /* Q24 = Q15 * Q30 >> 21 */
                pcpxErrPreNorm->imag = PSHR((long long)pcpxTmp->imag * llTmp, 21);
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
                pcpxErrPreNorm = (GSC_COMPLEX64_T *)pstGscInst->pcpxRef1;
                /* pcpxTargetX points at the head one */
                //pcpxTargetX  = pcpxGscX + ((iGscTaps + iTapCounter - iTapsIdx)%iGscTaps)*GSC_FFTBINS;
                pcpxTargetX  = pcpxGscX + ((iGscTaps + iTapCounter - iTapsIdx) & (iGscTaps - 1)) * GSC_FFTBINS;
#ifdef GSC_FIXED
                AISP_TSL_cmulj_sum_vec_with_mu_32_32((S64 *)pcpxErrPreNorm, (S32 *)pcpxTargetX, (S32 *)pcpxTargetW, iMu, GSC_FFTBINS,
                                                     15, 24);
                pcpxTargetW += GSC_FFTBINS;
#else

                for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
                {
                    pcpxTargetW->real += iMu * (pcpxErrPreNorm->real * pcpxTargetX->real
                                                + pcpxErrPreNorm->imag * pcpxTargetX->imag);
                    pcpxTargetW->imag += iMu * (pcpxErrPreNorm->imag * pcpxTargetX->real
                                                - pcpxErrPreNorm->real * pcpxTargetX->imag);
                    pcpxTargetW++;
                    pcpxTargetX++;
                    pcpxErrPreNorm++;
                }

#endif
            }

            /* Matrix W and X is 257 x Taps x beams, so switch to next W and X. */
            pcpxWeight += iFilterLen;
            pcpxGscX   += iFilterLen;
        }
    }/* if (pstGscCfg->iVAD == 0) */

    /* Update the tap index. */
    //pstGscCfg->iGscCurxIdx = ++iTapCounter % iGscTaps;
    pstGscCfg->iGscCurxIdx = ++iTapCounter & (iGscTaps - 1);
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

#ifndef GSC_ENABLE_HALF_VAD
/************************************************************
  Function   : rdmaLinearConv()

  Description: Y = X convMultipy Filter
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/09, Chao.Xu create

************************************************************/
static VOID rdmaLinearConv(GSC_S32_T *piX, GSC_S32_T *piFilter, GSC_S32_T *piY, S32 iXLen, S32 iFilterLen, S32 iYLen)
{
    S32 iLoop1, iLoop2;
#ifdef GSC_FIXED
    S64 llYSum = 0;
    S32 iLoopStar = 0;

    for (iLoop1 = 0; iLoop1 < iYLen; iLoop1++)
    {
        iLoopStar = iLoop1 - iFilterLen + 1;

        for (iLoop2 = iLoopStar; iLoop2 <= iLoop1; iLoop2++)
        {
            llYSum += (S64)piFilter[iLoop1 - iLoop2] * piX[iLoop2];/* Fixed: W32Q24 += Q31*Q24/Q24 */
        }

        *piY++ = (llYSum + (1 << 30)) >> 31;/* Attention: If FilterLen is too long, llYSum will be overflow! */
        llYSum = 0;
    }

#else

    for (iLoop1 = 0; iLoop1 < iYLen; iLoop1++)
    {
        *piY = 0;

        for (iLoop2 = 0; iLoop2 < iXLen; iLoop2++)
        {
            if ((iLoop1 - iLoop2) >= 0 && (iLoop1 - iLoop2) < iFilterLen)
            {
                *piY += piFilter[iLoop1 - iLoop2] * piX[iLoop2];
            }
        }

        piY++;
    }

#endif
}
#endif

