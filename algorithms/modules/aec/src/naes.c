/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : naes.c
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
  ziyuan.Jiang  2016/08/06         1.00              Create

************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "naes.h"
#include "AISP_TSL_base.h"
#include "AISP_TSL_sqrt.h"
#include "AISP_TSL_common.h"
#include "aec_base_fixed.h"


#ifdef AISPEECH_FIXED
#define PRIOR_MIN           (104)   /* (0.0032) */
#else
#define PRIOR_MIN           (0.0032)
#endif
static aisp_s32_t table[] =
{
#ifdef AISPEECH_FIXED
    26921, 33429, 39473, 45067, 50254, 55080,
    59594, 63836, 67842, 71643, 75264, 78727,
    82048, 85244, 88326, 91307, 94194, 96998,
    99724, 102377, 104964
#else
    0.82157, 1.02017, 1.20461, 1.37534,
    1.53363, 1.68092, 1.81865, 1.94811,
    2.07038, 2.18638, 2.29688, 2.40255,
    2.50391, 2.60144, 2.69551, 2.78647,
    2.87458, 2.96015, 3.04333, 3.12431,
    3.20326
#endif
};
/************************************************************
  Function   : naes_hyperGeomGain()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2017/03/06, Youhai.Jiang create

************************************************************/
//static inline
aisp_s32_t naes_hyperGeomGain(aisp_s32_t x)
{
    aisp_s32_t sqrt_result;
    aisp_s32_t frac = x * 2;
#ifdef AISPEECH_FIXED
    int   integer = frac >> 15;
#else
    int   integer = floor(frac);
#endif
#ifdef AISPEECH_FIXED
    frac = frac - (integer << 15);   //why +1 ?
#else
    frac = frac - integer;
#endif

    if (integer < -1)
    {
        return AISP_TSL_Q15_ONE_MINI;
    }

    if (integer  > 18)
    {
#ifdef AISPEECH_FIXED
        return AISP_TSL_Q15_ONE_MINI + (139165696 / x); //PDIV64(139165696, x);
#else
        return 1 + 0.1296f / x;
#endif
    }

#ifdef AISPEECH_FIXED
    sqrt_result = AISP_TSL_sqrt32(x + 3);
    return PDIV64((aisp_s64_t)(AISP_TSL_Q15_ONE_MINI - frac) * table[integer] + (aisp_s64_t)frac * table[integer + 1] ,
                  sqrt_result);
#else
    return ((1 - frac) * table[integer] + frac * table[integer + 1]) / sqrtf(x + 0.0001f);
#endif
}

/************************************************************
  Function   : naes_calcGain()

  Description: compute the gain and speech presence probability
               The algorithm compute the gain in mel domain
               and converted back to linear domain
               and updated the gain again
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/06, ziyuan.Jiang create

************************************************************/
void naes_calcGain(aec_core_T *pAEC_State)
{
    int  i                = 0;
    int fftBins           = 0;
    aisp_s32_t Gain       = 0;
    aisp_s32_t tmp        = 0;
    aisp_s32_t *presidualEcho = NULL;
    aisp_s32_t *p_aPost       = NULL;
    aisp_s32_t aEstPower  = 0;
    aisp_s32_t priorRatio = 0;
    aisp_s32_t theta      = 0;
    aisp_s32_t MM         = 0;
#ifdef USE_HIFI4_OPTIMIZE
    //aisp_s32_t* pLeak2        = NULL;
    aisp_s32_t lamda_y    = 0;
    aisp_s32_t tmp0[2];
    aisp_s32_t tmp1[2];
    aisp_s32_t tmp2[2];
    aisp_s32_t tmp3[2];
    aisp_s64_t tmp4;
    aisp_s64_t tmp5;
    ae_int64 aeResult;
    ae_int64 aeResult1;
    ae_int64 aeResult2;
    ae_int64 aeResult3;
    aisp_s64_t tmp6;
    aisp_s64_t tmp7;
    ae_int64   aeTmp64[7];
#endif
    aisp_s16_t lamda_x     = 0;
    aisp_s16_t lamda_y1     = 0;
    aisp_s32_t aPrior      = 0;
    aisp_s64_t HighQualityPPyy;
    aisp_s64_t HighQualityPPee;
    aisp_s64_t tmpHigh;
    aisp_s32_t tmpW32;
    aisp_s32_t sn1;
    aisp_s32_t aErrPower;
    static long long frameCount = 0;
    aisp_s32_t  *pErrPower = NULL;
    naes_core_T *pstNAes   = pAEC_State->pstNAes;
    /* update pointer */
    pstNAes->residualEcho  = pstNAes->LEAK2;
    fftBins                = pstNAes->fftBins;
    //leftCnt                = fftBins - 4;
    pErrPower              = (aisp_s32_t *)pstNAes->pErrPower;
    //pPPee                  = pstNAes->PPee;
    //pPPyy                  = pstNAes->PPyy;
    //pLeak2                 = pstNAes->LEAK2;
    presidualEcho          = pstNAes->residualEcho;
    p_aPost                = pstNAes->aPost;

    if (!pstNAes->xcoherenceInit)
    {
        lamda_x = 0;
        pstNAes->xcoherenceInit = 1;
#ifdef USE_HIFI4_OPTIMIZE
        lamda_y = AISP_TSL_Q15_ONE_MINI;
#endif
    }
    else
    {
        lamda_x = pAEC_State->config->lamda_x;
#ifdef USE_HIFI4_OPTIMIZE
        lamda_y = AISP_TSL_Q15_ONE_MINI - lamda_x;
#endif
    }

    lamda_y1 = AISP_TSL_Q15_ONE_MINI - lamda_x;

    if (pAEC_State->idx == 0)
    {
        frameCount++;
    }

    for (i = 0; i < fftBins; ++i)
    {
#ifdef USE_HIFI4_OPTIMIZE
        ae_StateEstPower = AE_MULZAAD32_HH_LL(ae_StateEST[i], ae_StateEST[i]);
        ae_StateEstPower = AE_ADD64(ae_StateEstPower, ae_factor);
        ae_StateEstPower = AE_SLAI64(ae_StateEstPower, 22);
        tmpHigh = AE_TRUNCA32Q64(ae_StateEstPower);
        aEstPower = XT_MINU(tmpHigh, AISP_Q31_ONE);
#else
        //Q20
        tmpHigh = PSHR64((aisp_s64_t)pAEC_State->aEST[i].real * pAEC_State->aEST[i].real
                         + (aisp_s64_t)pAEC_State->aEST[i].imag * pAEC_State->aEST[i].imag, AISP_TSL_Q10_SHIFT);

        if (tmpHigh > AISP_Q31_ONE)
        {
            aEstPower = AISP_Q31_ONE;
        }
        else
        {
            aEstPower = tmpHigh;
        }

#endif
        // add pErrPower  Q20
        aErrPower = PSHR64((aisp_s64_t)pAEC_State->aERR[i].real * pAEC_State->aERR[i].real
                           + (aisp_s64_t)pAEC_State->aERR[i].imag * pAEC_State->aERR[i].imag, AISP_TSL_Q10_SHIFT);
        HighQualityPPyy = (aisp_s64_t)lamda_x * pstNAes->PPyy[i] + (aisp_s64_t)(lamda_y1) *
                          aEstPower; // Q30  ----niukun 20180724   Q35  ----niukun 20180807
        //pstNAes->PPyy[i] = PSHR64((aisp_s64_t)lamda_x * pstNAes->PPyy[i] + (aisp_s64_t)(AISP_TSL_Q15_ONE_MINI - lamda_x) * aEstPower, AISP_TSL_Q15_SHIFT);  //Q20
        pstNAes->PPyy[i] = PSHR64(HighQualityPPyy, AISP_Q15_SHIFT);  //Q20
        /* Pee=lamda_x*Pee+(1-lamda_x)*ERR.*conj(ERR) */
        HighQualityPPee = (aisp_s64_t)lamda_x * pstNAes->PPee[i] + (aisp_s64_t)(lamda_y1) *
                          aErrPower; // Q30  ----niukun 20180724  Q35  ----niukun 20180807
        //pstNAes->PPee[i] = PSHR64((aisp_s64_t)lamda_x * pstNAes->PPee[i] + (aisp_s64_t)(AISP_TSL_Q15_ONE_MINI - lamda_x) * aErrPower, AISP_TSL_Q15_SHIFT);  //Q20
        /*Pey*/ // ----niukun 20180724
        pstNAes->PPee[i] = PSHR64(HighQualityPPee, AISP_Q15_SHIFT);  //Q20
        pstNAes->PPey[i].real = PSHR64((aisp_s64_t)lamda_x * pstNAes->PPey[i].real + (aisp_s64_t)(lamda_y1) * PSHR64((
                                           aisp_s64_t)pAEC_State->aEST[i].real * pAEC_State->aERR[i].real + (aisp_s64_t)pAEC_State->aEST[i].imag *
                                       pAEC_State->aERR[i].imag , AISP_TSL_Q5_SHIFT), AISP_TSL_Q15_SHIFT); //Q25
        pstNAes->PPey[i].imag = PSHR64((aisp_s64_t)lamda_x * pstNAes->PPey[i].imag + (aisp_s64_t)(lamda_y1) * PSHR64((
                                           aisp_s64_t)pAEC_State->aEST[i].imag * pAEC_State->aERR[i].real - (aisp_s64_t)pAEC_State->aEST[i].real *
                                       pAEC_State->aERR[i].imag , AISP_TSL_Q5_SHIFT), AISP_TSL_Q15_SHIFT); //Q25

        if (HighQualityPPyy == 0 || HighQualityPPee == 0)
        {
            pstNAes->LEAK2[i] = AISP_TSL_Q15_ONE_MINI;
        }
        else
            if (HighQualityPPyy <= HighQualityPPee && (pstNAes->PPyy[i] == 0))
            {
                tmpHigh = (aisp_s64_t)pstNAes->PPey[i].real * pstNAes->PPey[i].real + (aisp_s64_t)pstNAes->PPey[i].imag *
                          pstNAes->PPey[i].imag; // Q30  //Q50 --niukun0807
                // tmpW32 = tmpHigh * AISP_Q15_ONE / (HighQualityPPyy * HighQualityPPee); //Q15
                tmpW32 = tmpHigh * AISP_TSL_Q20_MINI / PSHR(HighQualityPPyy * HighQualityPPee, 15); //Q15
                pstNAes->LEAK2[i] = PSHR64((aisp_s64_t)pAEC_State->config->scaft * AISP_TSL_sqrt32(tmpW32), 15);

                if (pstNAes->LEAK2[i] > AISP_TSL_Q15_ONE_MINI)
                {
                    pstNAes->LEAK2[i] = AISP_TSL_Q15_ONE_MINI;
                }
            }
            else
                if (pstNAes->PPyy[i] == 0 || pstNAes->PPee[i] == 0)
                {
                    tmpHigh = (aisp_s64_t)pstNAes->PPey[i].real * pstNAes->PPey[i].real + (aisp_s64_t)pstNAes->PPey[i].imag *
                              pstNAes->PPey[i].imag; //Q30   //Q50 --niukun0807
                    sn1 = HighQualityPPee * pAEC_State->config->scaft / HighQualityPPyy ; // Q15
                    tmpW32 = tmpHigh * AISP_TSL_Q20_MINI / PSHR64(HighQualityPPyy * HighQualityPPee , 15); //Q15
                    pstNAes->LEAK2[i] = PSHR64((aisp_s64_t)pAEC_State->config->scaft * AISP_TSL_sqrt32(tmpW32), 15);

                    if (pstNAes->LEAK2[i] > AISP_TSL_Q15_ONE_MINI)
                    {
                        pstNAes->LEAK2[i] = AISP_TSL_Q15_ONE_MINI;
                    }
                }
                else
                    if ((pstNAes->PPyy[i] <= pstNAes->PPee[i]) || (pstNAes->PPee[i] < 0)) //|| (pstNAes->PPee[i] < 0)
                    {
                        tmpHigh = (aisp_s64_t)pstNAes->PPey[i].real * pstNAes->PPey[i].real + (aisp_s64_t)pstNAes->PPey[i].imag *
                                  pstNAes->PPey[i].imag; //Q50

                        if (pstNAes->PPee[i] < 0)
                        {
                            pstNAes->PPee[i] = AISP_TSL_Q31_ONE_MINI;
                        }

                        if (tmpHigh < 297528130221121700LL)
                        {
                            tmpW32 = ((aisp_s64_t)tmpHigh * AISP_TSL_Q5_ONE_MINI) / ((aisp_s64_t)pstNAes->PPee[i] * pstNAes->PPyy[i]); //Q15
                        }
                        else
                        {
                            tmpW32 = AISP_TSL_Q63_ONE / ((aisp_s64_t)pstNAes->PPee[i] * pstNAes->PPyy[i]); //Q15
                        }

                        pstNAes->LEAK2[i] = PSHR64((aisp_s64_t)pAEC_State->config->scaft * AISP_TSL_sqrt32(tmpW32), 15);

                        if (pstNAes->LEAK2[i] > AISP_TSL_Q15_ONE_MINI)
                        {
                            pstNAes->LEAK2[i] = AISP_TSL_Q15_ONE_MINI;
                        }
                    }
                    else
                    {
                        tmpHigh = (aisp_s64_t)pstNAes->PPey[i].real * pstNAes->PPey[i].real + (aisp_s64_t)pstNAes->PPey[i].imag *
                                  pstNAes->PPey[i].imag; //Q50
                        sn1 = ((aisp_s64_t)pAEC_State->config->scaft * pstNAes->PPee[i] / pstNAes->PPyy[i]); //Q15

                        if (tmpHigh < 297528130221121700LL)
                        {
                            tmpW32 = ((aisp_s64_t)tmpHigh * AISP_TSL_Q5_ONE_MINI) / ((aisp_s64_t)pstNAes->PPee[i] * pstNAes->PPyy[i]); //Q15
                        }
                        else
                        {
                            tmpW32 = AISP_TSL_Q63_ONE / ((aisp_s64_t)pstNAes->PPee[i] * pstNAes->PPyy[i]); //Q15
                        }

                        pstNAes->LEAK2[i] = PSHR64((aisp_s64_t)sn1 * AISP_TSL_sqrt32(tmpW32), 15);

                        if (pstNAes->LEAK2[i] > AISP_TSL_Q15_ONE_MINI)
                        {
                            pstNAes->LEAK2[i] = AISP_TSL_Q15_ONE_MINI;
                        }
                    }

        pstNAes->residualEcho[i] = PSHR64((aisp_s64_t)aEstPower * pstNAes->LEAK2[i] , AISP_TSL_Q20_SHIFT);
    }

    for (i = 0; i < fftBins; ++i)
    {
        if ((presidualEcho[i] == 0) || (pErrPower[i] > presidualEcho[i] * 40))
        {
            p_aPost[i] = (aisp_s32_t)1310680; /* AISP_FIXED32(40); */
        }
        else
        {
            tmp = PDIV32_32_P15(pErrPower[i], presidualEcho[i]);
            p_aPost[i] = tmp < (aisp_s32_t)1310680 ? tmp : (aisp_s32_t)1310680;  /* AISP_FIXED32(40); */
        }
    }

    if (pstNAes->xcoherenceInit2 == 0)
    {
        pstNAes->xcoherenceInit2 = 1;
#ifdef USE_HIFI4_OPTIMIZE

        for (i = 0; i < fftBins ; i++)
        {
            tmp7 = p_aPost[i] > AISP_TSL_Q15_ONE_MINI ? (aisp_s64_t)(p_aPost[i] - AISP_TSL_Q15_ONE_MINI) * GAMMA : 0;
            aeTmp64[1] = int64_rtor_ae_int64(tmp7);
            aeResult = AE_SLAA64(aeTmp64[1], AISP_Q17_SHIFT);
            *(ae_f32x2 *)tmp0 = AE_ROUND32F64SASYM(aeResult);
            aPrior = tmp0[0] + (aisp_s32_t)(AISP_TSL_Q15_ONE_MINI - GAMMA);
            priorRatio = PDIV64(SHL64(aPrior, 15) , AISP_TSL_Q15_ONE_MINI + aPrior);
            tmp4 = (aisp_s64_t)priorRatio * p_aPost[i];
            aeTmp64[2] = int64_rtor_ae_int64(tmp4);
            aeResult1 = AE_SLAA64(aeTmp64[2], AISP_Q17_SHIFT);
            *(ae_f32x2 *)tmp1 = AE_ROUND32F64SASYM(aeResult1);
            theta = tmp1[0];
            MM   = naes_hyperGeomGain(theta);
            Gain = (PSHR64((aisp_s64_t)priorRatio * MM ,
                           AISP_TSL_Q15_SHIFT) > AISP_TSL_Q15_ONE_MINI ? AISP_TSL_Q15_ONE_MINI : PSHR64((aisp_s64_t)priorRatio * MM ,
                                   AISP_TSL_Q15_SHIFT));
            tmp5 = (aisp_s64_t)pAEC_State->aERR[i].real * Gain;
            aeTmp64[3] = int64_rtor_ae_int64(tmp5);
            aeResult2 = AE_SLAA64(aeTmp64[3], AISP_Q17_SHIFT);
            *(ae_f32x2 *)tmp2 = AE_ROUND32F64SASYM(aeResult2);
            tmp6 = (aisp_s64_t)pAEC_State->aERR[i].imag * Gain;
            aeTmp64[0] = int64_rtor_ae_int64(tmp6);
            aeResult3 = AE_SLAA64(aeTmp64[0], AISP_Q17_SHIFT);
            *(ae_f32x2 *)tmp3 = AE_ROUND32F64SASYM(aeResult3);
            pAEC_State->aERR[i].real = tmp2[0];
            pAEC_State->aERR[i].imag = tmp3[0];
        }

        AISP_TSL_cpxVecPower(pstNAes->pPs_old, pAEC_State->aERR, fftBins);
#else

        for (i = 0; i < fftBins; i++)
        {
            aPrior = PSHR64((aisp_s64_t)(pstNAes->aPost[i] > AISP_TSL_Q15_ONE_MINI ? pstNAes->aPost[i] - AISP_TSL_Q15_ONE_MINI : 0)
                            * GAMMA , AISP_TSL_Q15_SHIFT) + (AISP_TSL_Q15_ONE_MINI - GAMMA);
            priorRatio = PDIV64(SHL64(aPrior, 15) , AISP_TSL_Q15_ONE_MINI + aPrior);
            theta = PSHR64((aisp_s64_t)priorRatio * pstNAes->aPost[i] , AISP_TSL_Q15_SHIFT);
            MM   = naes_hyperGeomGain(theta);
            Gain = (PSHR64((aisp_s64_t)priorRatio * MM ,
                           AISP_TSL_Q15_SHIFT) > AISP_TSL_Q15_ONE_MINI ? AISP_TSL_Q15_ONE_MINI : PSHR64((aisp_s64_t)priorRatio * MM ,
                                   AISP_TSL_Q15_SHIFT));
            pAEC_State->aERR[i].real = PSHR64((aisp_s64_t)pAEC_State->aERR[i].real * Gain, AISP_TSL_Q15_SHIFT);
            pAEC_State->aERR[i].imag = PSHR64((aisp_s64_t)pAEC_State->aERR[i].imag * Gain, AISP_TSL_Q15_SHIFT);
            pstNAes->pPs_old[i] = PSHR64((aisp_s64_t)pAEC_State->aERR[i].real * pAEC_State->aERR[i].real
                                         + (aisp_s64_t)pAEC_State->aERR[i].imag * pAEC_State->aERR[i].imag , AISP_TSL_Q15_SHIFT);
        }

#endif
    }
    else
    {
#ifdef USE_HIFI4_OPTIMIZE

        for (i = 0; i < fftBins; i++)
        {
            if ((presidualEcho[i]) == 0
                    || ((aisp_s64_t)pstNAes->pPs_old[i] * (AISP_TSL_Q15_ONE_MINI - GAMMA) > SHL64(presidualEcho[i] * 100, 15)))
            {
                aPrior = 3276800;
            }
            else
            {
                aPrior = PSHR64((aisp_s64_t)GAMMA * (pstNAes->aPost[i] > 32767 ? pstNAes->aPost[i] - 32767 : 0), AISP_TSL_Q15_SHIFT) +
                         PDIV64((aisp_s64_t)pstNAes->pPs_old[i] * (AISP_TSL_Q15_ONE_MINI - GAMMA) , pstNAes->residualEcho[i]);
                aPrior = (aPrior > PRIOR_MIN ? aPrior : PRIOR_MIN);
                aPrior = (aPrior > 3276800 ? 3276800 : aPrior);
            }

            priorRatio = PDIV32_32_P15(aPrior, AISP_TSL_Q15_ONE_MINI + aPrior);
            tmp4 = (aisp_s64_t)priorRatio * pstNAes->aPost[i];
            aeTmp64[4] = int64_rtor_ae_int64(tmp4);
            aeResult1 = AE_SLAA64(aeTmp64[4] , AISP_Q17_SHIFT);
            *(ae_f32x2 *)tmp1 = AE_ROUND32F64SASYM(aeResult1);
            theta = tmp1[0];
            MM = naes_hyperGeomGain(theta);
            Gain = PSHR64((aisp_s64_t)priorRatio * MM ,
                          AISP_TSL_Q15_SHIFT) > AISP_TSL_Q15_ONE_MINI ? AISP_TSL_Q15_ONE_MINI : PSHR64((aisp_s64_t)priorRatio * MM ,
                                  AISP_TSL_Q15_SHIFT);
            tmp5 = (aisp_s64_t)pAEC_State->aERR[i].real * Gain;
            aeTmp64[5] = int64_rtor_ae_int64(tmp5);
            aeResult2 = AE_SLAA64(aeTmp64[5] , AISP_Q17_SHIFT);
            *(ae_f32x2 *)tmp2 = AE_ROUND32F64SASYM(aeResult2);
            tmp6 = (aisp_s64_t)pAEC_State->aERR[i].imag * Gain;
            aeTmp64[6] = int64_rtor_ae_int64(tmp6);
            aeResult3 = AE_SLAA64(aeTmp64[6], AISP_Q17_SHIFT);
            *(ae_f32x2 *)tmp3 = AE_ROUND32F64SASYM(aeResult3);
            pAEC_State->aERR[i].real = tmp2[0];
            pAEC_State->aERR[i].imag = tmp3[0];
        }

        AISP_TSL_cpxVecPower(pstNAes->pPs_old, pAEC_State->aERR, fftBins);
#else

        for (i = 0; i < fftBins; i++)
        {
            if ((pstNAes->residualEcho[i]) == 0
                    || ((aisp_s64_t)pstNAes->pPs_old[i] * (AISP_TSL_Q15_ONE_MINI - GAMMA) > SHL64(pstNAes->residualEcho[i] * 100, 15)))
            {
                aPrior = 3276800;
            }
            else
            {
                aPrior = PSHR64((aisp_s64_t)GAMMA * (pstNAes->aPost[i] > 32767 ? pstNAes->aPost[i] - 32767 : 0), AISP_TSL_Q15_SHIFT) +
                         PDIV64((aisp_s64_t)pstNAes->pPs_old[i] * (AISP_TSL_Q15_ONE_MINI - GAMMA) , pstNAes->residualEcho[i]);
                aPrior = (aPrior > PRIOR_MIN ? aPrior : PRIOR_MIN);
                aPrior = (aPrior > 3276800 ? 3276800 : aPrior);
            }

            priorRatio = PDIV32_32_P15(aPrior , AISP_TSL_Q15_ONE_MINI + aPrior);
            theta = PSHR64((aisp_s64_t)priorRatio * pstNAes->aPost[i] , AISP_TSL_Q15_SHIFT);
            MM = naes_hyperGeomGain(theta);
            Gain = PSHR64((aisp_s64_t)priorRatio * MM ,
                          AISP_TSL_Q15_SHIFT) > AISP_TSL_Q15_ONE_MINI ? AISP_TSL_Q15_ONE_MINI : PSHR64((aisp_s64_t)priorRatio * MM ,
                                  AISP_TSL_Q15_SHIFT);
            pAEC_State->aERR[i].real = PSHR64((aisp_s64_t)pAEC_State->aERR[i].real * Gain, AISP_TSL_Q15_SHIFT);
            pAEC_State->aERR[i].imag = PSHR64((aisp_s64_t)pAEC_State->aERR[i].imag * Gain, AISP_TSL_Q15_SHIFT);
            pstNAes->pPs_old[i] = PSHR64((aisp_s64_t)pAEC_State->aERR[i].real * pAEC_State->aERR[i].real
                                         + (aisp_s64_t)pAEC_State->aERR[i].imag * pAEC_State->aERR[i].imag , AISP_TSL_Q15_SHIFT);
        }

#endif
    }
}

/************************************************************
  Function   : naes_feed()

  Description: data feed to naes
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/06, ziyuan.Jiang create

************************************************************/
void naes_feed(aec_core_T *pAEC_State)
{
    pAEC_State->pstNAes->pErrPower = pAEC_State->aERRPower;
    pAEC_State->pstNAes->aPost = (aisp_s32_t *)pAEC_State->aExtdW0;
    naes_calcGain(pAEC_State);
}

/************************************************************
  Function   : naes_reset()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/06, ziyuan.Jiang create

************************************************************/
void naes_reset(naes_core_T *pstNAes)
{
    pstNAes->xcoherenceInit = 0;
    pstNAes->xcoherenceInit2 = 0;
}

