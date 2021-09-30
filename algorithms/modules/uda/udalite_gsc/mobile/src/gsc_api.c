#include "stdio.h"

#include "AISP_TSL_types.h"
#include "AISP_TSL_common.h"
#include "AISP_TSL_str.h"

#include "gsc_api.h"
#include "gsc_core_config.h"
#include "gsc_core_tables.h"
#include "gsc_core_func.h"
#include "gsc_core_version.h"

/************************************************************
 *   Function   : GSC_CfgInit()
 *   Description:
 *   Calls      :
 *   Called By  :
 *   Input      :
 *   Output     :
 *   Return     :
 *   Others     : Used in UDA
 *   History    :
 *       2019/04/28, chao.xu create
 *
 *************************************************************/
S32 GSC_CfgInit(GSC_CoreCfg_S *pstGscCfg, const char *pcBinFn)
{
    S32 iRet = 0;
    iRet = gscDefaultCfgInit(pstGscCfg);

    if (iRet == -1)
    {
#ifdef AISP_TSL_INFO
        printf("gscDefaultCfg init error.\n");
#endif
        goto FAIL;
    }

#ifndef GSC_FIXED
    iRet = gscCfgUpdate(pstGscCfg, pcBinFn);

    if (iRet == -1)
    {
#ifdef AISP_TSL_INFO
        printf("gscCfg update error.\n");
#endif
        goto FAIL;
    }

#endif
    return 0;
FAIL:
    return -1;
}

/************************************************************
 *   Function   : GSC_CfgMemSizeGet()
 *   Description: Get the size of config memory
 *   Calls      :
 *   Called By  :
 *   Input      :
 *   Output     :
 *   Return     :
 *   Others     : Used in UDA
 *   History    :
 *       2019/04/28, chao.xu create
 *
 *************************************************************/
S32 GSC_CfgMemSizeGet(VOID)
{
    S32 iTotalMemSize = 0;
    iTotalMemSize  = AISP_ALIGN_SIZE(sizeof(GSC_CoreCfg_S), AISP_ALIGN_8_MASK);
    return iTotalMemSize;
}

/************************************************************
 *   Function   : GSC_SharedMemSizeGet()
 *   Description: Get the size of shared memory
 *   Calls      :
 *   Called By  :
 *   Input      :
 *   Output     :
 *   Return     :
 *   Others     : Used in UDA
 *   History    :
 *       2019/04/28, chao.xu create
 *
 *************************************************************/
S32 GSC_SharedMemSizeGet(GSC_CoreCfg_S *pstGscCfg)
{
    S32 iTotalMemSize = 0;
    /* pcpxXFFT buf size */
    iTotalMemSize += GSC_MICS * GSC_FFTBINS * sizeof(GSC_COMPLEX_T);
    iTotalMemSize  = AISP_ALIGN_SIZE(iTotalMemSize, AISP_ALIGN_8_MASK);
    /* pcpxMic1 buf size, shared with pcpxMIC */
    iTotalMemSize += GSC_FFTBINS * sizeof(GSC_COMPLEX_T);
    iTotalMemSize  = AISP_ALIGN_SIZE(iTotalMemSize, AISP_ALIGN_8_MASK);
    /* pcpxMic2 buf size, shared with pcpxEST */
    iTotalMemSize += GSC_FFTBINS * sizeof(GSC_COMPLEX_T);
    iTotalMemSize  = AISP_ALIGN_SIZE(iTotalMemSize, AISP_ALIGN_8_MASK);
    /* pcpxRef1 buf size, shared with pcpxREF */
    iTotalMemSize += GSC_FFTBINS * sizeof(GSC_COMPLEX_T);
    iTotalMemSize  = AISP_ALIGN_SIZE(iTotalMemSize, AISP_ALIGN_8_MASK);
    /* pcpxRef2 buf size, shared with pcpxErrPreNorm */
    iTotalMemSize += GSC_FFTBINS * sizeof(GSC_COMPLEX_T);
    iTotalMemSize  = AISP_ALIGN_SIZE(iTotalMemSize, AISP_ALIGN_8_MASK);
    return iTotalMemSize;
}

/************************************************************
 *   Function   : GSC_UnsharedMemSizeGet()
 *   Description:
 *   Calls      :
 *   Called By  :
 *   Input      :
 *   Output     :
 *   Return     :
 *   Others     : Used in UDA
 *   History    :
 *       2019/04/30, chao.xu create
 *
 *************************************************************/
S32 GSC_UnsharedMemSizeGet(GSC_CoreCfg_S *pstGscCfg)
{
    int iGscNum = 0;
    int iTaps   = 0;
    int iTmpLen = 0;
    S32 iTotalMemSize = 0;

    if (NULL == pstGscCfg)
    {
        iGscNum = 3;
#ifdef GSC_ENABLE_HALF_VAD
        iTaps   = 2;
#else
        iTaps   = 4;
#endif
    }
    else
    {
        iGscNum = pstGscCfg->iGsc;
        iTaps   = pstGscCfg->iTaps;
    }

    iTmpLen = iGscNum * GSC_FFTBINS * sizeof(GSC_COMPLEX_T);
    /* GSC Api, including fft engine. */
    iTotalMemSize += sizeof(GSC_Api_S);
    iTotalMemSize  = AISP_ALIGN_SIZE(iTotalMemSize, AISP_ALIGN_8_MASK);
    /* GSC_CoreCfg_S is alloced outside. */
    /* GSC instance */
    iTotalMemSize += sizeof(GSC_Core_S);
    iTotalMemSize  = AISP_ALIGN_SIZE(iTotalMemSize, AISP_ALIGN_8_MASK);
    /* pcpxGscX buf size */
    iTotalMemSize += iTmpLen * iTaps;
    iTotalMemSize  = AISP_ALIGN_SIZE(iTotalMemSize, AISP_ALIGN_8_MASK);
    /* pcpxGscWeight buf size */
    iTotalMemSize += iTmpLen * iTaps;
    iTotalMemSize  = AISP_ALIGN_SIZE(iTotalMemSize, AISP_ALIGN_8_MASK);
    /* pcpxBlockOut buf size */
    iTotalMemSize += iTmpLen;
    iTotalMemSize  = AISP_ALIGN_SIZE(iTotalMemSize, AISP_ALIGN_8_MASK);
    /* pcpxGscH buf size */
    iTotalMemSize += iTmpLen;
    iTotalMemSize  = AISP_ALIGN_SIZE(iTotalMemSize, AISP_ALIGN_8_MASK);
    /* pGscXPowerSum buf size */
    iTotalMemSize += iGscNum * GSC_FFTBINS * sizeof(GSC_S64_T);
    iTotalMemSize  = AISP_ALIGN_SIZE(iTotalMemSize, AISP_ALIGN_8_MASK);
    /* pGscYPowerSum buf size */
    iTotalMemSize += iGscNum * GSC_FFTBINS * sizeof(GSC_S64_T);
    iTotalMemSize  = AISP_ALIGN_SIZE(iTotalMemSize, AISP_ALIGN_8_MASK);
    /* pGscNoisePSD buf size */
    iTotalMemSize += GSC_FFTBINS * sizeof(GSC_S64_T);
    iTotalMemSize  = AISP_ALIGN_SIZE(iTotalMemSize, AISP_ALIGN_8_MASK);
    /* pGscPH1 buf size */
    iTotalMemSize += GSC_FFTBINS * sizeof(GSC_S32_T);
    iTotalMemSize  = AISP_ALIGN_SIZE(iTotalMemSize, AISP_ALIGN_8_MASK);
    /* pGscPH1mean buf size */
    iTotalMemSize += GSC_FFTBINS * sizeof(GSC_S32_T);
    iTotalMemSize  = AISP_ALIGN_SIZE(iTotalMemSize, AISP_ALIGN_8_MASK);
    /* piRaw buf size */
    iTotalMemSize += GSC_MICS * GSC_FRMINC * sizeof(GSC_S32_T);
    iTotalMemSize  = AISP_ALIGN_SIZE(iTotalMemSize, AISP_ALIGN_8_MASK);
    /* psDataLeft buf size */
    iTotalMemSize += iGscNum * GSC_FRMINC * sizeof(GSC_S16_T);
    iTotalMemSize  = AISP_ALIGN_SIZE(iTotalMemSize, AISP_ALIGN_8_MASK);
    return iTotalMemSize;
}

/************************************************************
 *   Function   : GSC_New()
 *   Description:
 *   Calls      :
 *   Called By  :
 *   Input      :
 *   Output     :
 *   Return     :
 *   Others     : Used in UDA
 *   History    :
 *       2019/04/30, chao.xu create
 *
 *************************************************************/
VOID *GSC_New(GSC_CoreCfg_S *pstGscCfg, S8 *pcSharedBuf, S8 *pcUnsharedBuf, GSC_FUNCHOOK_T pGscCallback,
              void *pvUsrData)
{
    S32 iPtrCnt =  0;
    S32 iBinIdx =  0;
    S32 iRet    = -1;
    GSC_Api_S *pstGscApi = NULL;

    if (pstGscCfg == NULL)
    {
        return NULL;
    }

    /****************************** Unshared Memory for UDA ************************************/
    if (pcUnsharedBuf == NULL)
    {
        return NULL;
    }

    iPtrCnt  = 0;
    AISP_TSL_memset(pcUnsharedBuf, 0, GSC_UnsharedMemSizeGet(pstGscCfg));
    /* Engine */
    pstGscApi = (GSC_Api_S *)pcUnsharedBuf;
    iPtrCnt += sizeof(GSC_Api_S);
    iPtrCnt  = AISP_ALIGN_SIZE(iPtrCnt, AISP_ALIGN_8_MASK);
    pstGscApi->pstGscInstance = (GSC_Core_S *)(AISP_MEM_ALIGN((pcUnsharedBuf + iPtrCnt), 8));
    iPtrCnt += sizeof(GSC_Core_S);
    iPtrCnt  = AISP_ALIGN_SIZE(iPtrCnt, AISP_ALIGN_8_MASK);
    /* Variables */
    pstGscApi->pstGscInstance->pcpxGscX      = (GSC_COMPLEX_T *)(AISP_MEM_ALIGN((pcUnsharedBuf + iPtrCnt), 8));
    iPtrCnt += pstGscCfg->iGsc * GSC_FFTBINS * pstGscCfg->iTaps * sizeof(GSC_COMPLEX_T);
    iPtrCnt  = AISP_ALIGN_SIZE(iPtrCnt, AISP_ALIGN_8_MASK);
    pstGscApi->pstGscInstance->pcpxGscWeight = (GSC_COMPLEX_T *)(AISP_MEM_ALIGN((pcUnsharedBuf + iPtrCnt), 8));
    iPtrCnt += pstGscCfg->iGsc * GSC_FFTBINS * pstGscCfg->iTaps * sizeof(GSC_COMPLEX_T);
    iPtrCnt  = AISP_ALIGN_SIZE(iPtrCnt, AISP_ALIGN_8_MASK);
    pstGscApi->pstGscInstance->pcpxBlockOut  = (GSC_COMPLEX_T *)(AISP_MEM_ALIGN((pcUnsharedBuf + iPtrCnt), 8));
    iPtrCnt += pstGscCfg->iGsc * GSC_FFTBINS * sizeof(GSC_COMPLEX_T);
    iPtrCnt  = AISP_ALIGN_SIZE(iPtrCnt, AISP_ALIGN_8_MASK);
    pstGscApi->pstGscInstance->pcpxGscH      = (GSC_COMPLEX_T *)(AISP_MEM_ALIGN((pcUnsharedBuf + iPtrCnt), 8));
    iPtrCnt += pstGscCfg->iGsc * GSC_FFTBINS * sizeof(GSC_COMPLEX_T);
    iPtrCnt  = AISP_ALIGN_SIZE(iPtrCnt, AISP_ALIGN_8_MASK);
    pstGscApi->pstGscInstance->pGscXPowerSum = (GSC_S64_T *)(AISP_MEM_ALIGN((pcUnsharedBuf + iPtrCnt), 8));
    iPtrCnt += pstGscCfg->iGsc * GSC_FFTBINS * sizeof(GSC_S64_T);
    iPtrCnt  = AISP_ALIGN_SIZE(iPtrCnt, AISP_ALIGN_8_MASK);
    pstGscApi->pstGscInstance->pGscYPowerSum = (GSC_S64_T *)(AISP_MEM_ALIGN((pcUnsharedBuf + iPtrCnt), 8));
    iPtrCnt += pstGscCfg->iGsc * GSC_FFTBINS * sizeof(GSC_S64_T);
    iPtrCnt  = AISP_ALIGN_SIZE(iPtrCnt, AISP_ALIGN_8_MASK);
    pstGscApi->pstGscInstance->pGscNoisePSD = (GSC_S64_T *)(AISP_MEM_ALIGN((pcUnsharedBuf + iPtrCnt), 8));
    iPtrCnt += GSC_FFTBINS * sizeof(GSC_S64_T);
    iPtrCnt  = AISP_ALIGN_SIZE(iPtrCnt, AISP_ALIGN_8_MASK);
    pstGscApi->pstGscInstance->pGscPH1 = (GSC_S32_T *)(AISP_MEM_ALIGN((pcUnsharedBuf + iPtrCnt), 8));
    iPtrCnt += GSC_FFTBINS * sizeof(GSC_S32_T);
    iPtrCnt  = AISP_ALIGN_SIZE(iPtrCnt, AISP_ALIGN_8_MASK);
    pstGscApi->pstGscInstance->pGscPH1mean = (GSC_S32_T *)(AISP_MEM_ALIGN((pcUnsharedBuf + iPtrCnt), 8));
    iPtrCnt += GSC_FFTBINS * sizeof(GSC_S32_T);
    iPtrCnt  = AISP_ALIGN_SIZE(iPtrCnt, AISP_ALIGN_8_MASK);
    pstGscApi->pstGscInstance->piRaw = (GSC_S32_T *)(AISP_MEM_ALIGN((pcUnsharedBuf + iPtrCnt), 8));
    iPtrCnt += GSC_MICS * GSC_FRMINC * sizeof(GSC_S32_T);
    iPtrCnt  = AISP_ALIGN_SIZE(iPtrCnt, AISP_ALIGN_8_MASK);
    pstGscApi->pstGscInstance->psDataLeft = (GSC_S16_T *)(AISP_MEM_ALIGN((pcUnsharedBuf + iPtrCnt), 8));
    iPtrCnt += pstGscCfg->iGsc * GSC_FRMINC * sizeof(GSC_S16_T);
    iPtrCnt  = AISP_ALIGN_SIZE(iPtrCnt, AISP_ALIGN_8_MASK);

    /****************************** Shared Memory for UDA ************************************/
    if (pcSharedBuf == NULL)
    {
        return NULL;
    }

    iPtrCnt  = 0;
    AISP_TSL_memset(pcSharedBuf, 0, GSC_SharedMemSizeGet(pstGscCfg));
    pstGscApi->pstGscInstance->pcpxXFFT = (GSC_COMPLEX_T *)(AISP_MEM_ALIGN((pcSharedBuf + iPtrCnt), 8));
    iPtrCnt += GSC_MICS * GSC_FFTBINS * sizeof(GSC_COMPLEX_T);
    iPtrCnt  = AISP_ALIGN_SIZE(iPtrCnt, AISP_ALIGN_8_MASK);
    pstGscApi->pstGscInstance->pcpxMic1 = (GSC_COMPLEX_T *)(AISP_MEM_ALIGN((pcSharedBuf + iPtrCnt), 8));
    iPtrCnt += GSC_FFTBINS * sizeof(GSC_COMPLEX_T);
    iPtrCnt  = AISP_ALIGN_SIZE(iPtrCnt, AISP_ALIGN_8_MASK);
    pstGscApi->pstGscInstance->pcpxMic2 = (GSC_COMPLEX_T *)(AISP_MEM_ALIGN((pcSharedBuf + iPtrCnt), 8));
    iPtrCnt += GSC_FFTBINS * sizeof(GSC_COMPLEX_T);
    iPtrCnt  = AISP_ALIGN_SIZE(iPtrCnt, AISP_ALIGN_8_MASK);
    pstGscApi->pstGscInstance->pcpxRef1 = (GSC_COMPLEX_T *)(AISP_MEM_ALIGN((pcSharedBuf + iPtrCnt), 8));
    iPtrCnt += GSC_FFTBINS * sizeof(GSC_COMPLEX_T);
    iPtrCnt  = AISP_ALIGN_SIZE(iPtrCnt, AISP_ALIGN_8_MASK);
    pstGscApi->pstGscInstance->pcpxRef2 = (GSC_COMPLEX_T *)(AISP_MEM_ALIGN((pcSharedBuf + iPtrCnt), 8));
    iPtrCnt += GSC_FFTBINS * sizeof(GSC_COMPLEX_T);
    iPtrCnt  = AISP_ALIGN_SIZE(iPtrCnt, AISP_ALIGN_8_MASK);
    /* Engine initialization */
    pstGscApi->pstGscCfg = pstGscCfg;
    pstGscApi->pstGscInstance->pHanning_15  = (GSC_S32_T *)g_aHanning_15Window31; /* Gsc linear convolution */
    pstGscApi->pstGscInstance->pcpxGscWq    = (GSC_COMPLEX_T *)g_asGscWq;
    pstGscApi->pstGscInstance->pcpxGscB     = (GSC_COMPLEX_T *)g_asGscB;
    pstGscApi->psHanningWind      = (GSC_S32_T *)g_aHanningWindow512; /* FFT hanning window */
    pstGscApi->pfuncGscOutputHook = (GSC_FUNCHOOK_T)pGscCallback;
    pstGscApi->pvUsrData          = pvUsrData;

    for (iBinIdx = 0; iBinIdx < GSC_FFTBINS; iBinIdx++)
    {
#ifdef GSC_FIXED
        pstGscApi->pstGscInstance->pGscPH1mean[iBinIdx] = 8388608;
#else
        pstGscApi->pstGscInstance->pGscPH1mean[iBinIdx] = 0.5;
#endif
    }

    /* Init fft engine, fftLen = 512 */
    iRet = fftInit(&pstGscApi->stFFTEng, GSC_FFTLEN);

    if (iRet < 0)
    {
#ifdef AISP_TSL_INFO
        printf("FFT engine int fail.\n");
#endif
        goto FAIL;
    }

    return pstGscApi;
FAIL:
    return NULL;
}

/************************************************************
  Function   : GSC_LenPerSnd()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/28, Chao.Xu create

************************************************************/
GSC_VISIBILITY_ATTR S32 GSC_LenPerSnd(GSC_Api_S *pstGscApi)
{
    return (GSC_FRMINC * GSC_IN_CHAN << 1);
}

/************************************************************
  Function   : GSC_Start()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/29, Chao.Xu modify

************************************************************/
GSC_VISIBILITY_ATTR S32 GSC_Start(GSC_Api_S *pstGscApi, void *pvHandler, void *pvData)
{
    return 0;
}

/************************************************************
  Function   : GSC_Feed()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/11/09, Youhai.Jiang create
    2019/04/29, Chao.Xu modify

************************************************************/
GSC_VISIBILITY_ATTR S32 GSC_Feed(GSC_Api_S *pstGscApi, S8 *pcData, S32 iLen)
{
#ifdef LUDA_SUPPORT_FREQOUT_DIRECT
    S32 aiZero[GSC_FFTBINS * 2] = {0};
#else
    S16 asZero[GSC_FRMINC] = {0};
#endif
    S32 iRet  = -1;
    S32 iRet1 = -1;
    S32 iIdx  = 0;

    if ((GSC_FRMINC * GSC_IN_CHAN << 1) != iLen)
    {
#ifdef AISP_TSL_INFO
        printf("Input Length is not supported.\n");
#endif
        return iRet;
    }

    iRet = fftProcess(pstGscApi, pcData);

    if (!iRet)
    {
        /* VAD update */
#ifdef GSC_ENABLE_HALF_VAD
        iRet1 = gscUpdateHalfVAD(pstGscApi, (GSC_COMPLEX_T *)pstGscApi->pstGscInstance->pcpxXFFT);
#else
        iRet1 = gscUpdateVAD(pstGscApi, (GSC_COMPLEX_T *)pstGscApi->pstGscInstance->pcpxXFFT);
#endif

        if (iRet1 == 0) /* Normal gsc process. */
        {
            /* gsc process */
            gscProcess(pstGscApi, (GSC_COMPLEX_T *)pstGscApi->pstGscInstance->pcpxXFFT);
            /* data output */
            dataPop(pstGscApi, pstGscApi->pstGscInstance->pcpxBlockOut);
        }
        else
        {
            /* fd.ent_block(:,kk,:)=0 */
            if (pstGscApi->pfuncGscOutputHook)
            {
                for (iIdx = 0; iIdx < pstGscApi->pstGscCfg->iGsc; iIdx++)
                {
#ifdef LUDA_SUPPORT_FREQOUT_DIRECT
                    pstGscApi->pfuncGscOutputHook(pstGscApi->pvUsrData, iIdx, (S8 *)aiZero, GSC_FFTBINS * sizeof(GSC_COMPLEX_T));
#else
                    pstGscApi->pfuncGscOutputHook(pstGscApi->pvUsrData, iIdx, (S8 *)asZero, GSC_FRMINC << 1);
#endif
                }
            }
        }
    }

    return iLen;
}

/************************************************************
  Function   : GSC_FeedParallel()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/11/09, Youhai.Jiang create
    2019/04/24, Chao.Xu modify

************************************************************/
GSC_VISIBILITY_ATTR S32 GSC_FeedParallel(GSC_Api_S *pstGscApi, S8 *pcChan1, S8 *pcChan2, S32 iLen)
{
#ifdef LUDA_SUPPORT_FREQOUT_DIRECT
    S32 aiZero[GSC_FFTBINS * 2] = {0};
#else
    S16 asZero[GSC_FRMINC] = {0};
#endif
    S32 iRet  = -1;
    S32 iRet1 = -1;
    S32 iIdx  = 0;

    if ((GSC_FRMINC << 1) != iLen)
    {
#ifdef AISP_TSL_INFO
        printf("Input Length is not supported.\n");
#endif
        return iRet;
    }

    iRet = fftProcessParallel(pstGscApi, pcChan1, pcChan2);

    if (!iRet)
    {
        /* VAD update */
#ifdef GSC_ENABLE_HALF_VAD
        iRet1 = gscUpdateHalfVAD(pstGscApi, (GSC_COMPLEX_T *)pstGscApi->pstGscInstance->pcpxXFFT);
#else
        iRet1 = gscUpdateVAD(pstGscApi, (GSC_COMPLEX_T *)pstGscApi->pstGscInstance->pcpxXFFT);
#endif

        if (iRet1 == 0) /* Normal gsc process. */
        {
            /* gsc process */
            gscProcess(pstGscApi, (GSC_COMPLEX_T *)pstGscApi->pstGscInstance->pcpxXFFT);
            /* data output */
            dataPop(pstGscApi, pstGscApi->pstGscInstance->pcpxBlockOut);
        }
        else
        {
            /* fd.ent_block(:,kk,:)=0 */
            if (pstGscApi->pfuncGscOutputHook)
            {
                for (iIdx = 0; iIdx < pstGscApi->pstGscCfg->iGsc; iIdx++)
                {
#ifdef LUDA_SUPPORT_FREQOUT_DIRECT
                    pstGscApi->pfuncGscOutputHook(pstGscApi->pvUsrData, iIdx, (S8 *)aiZero, GSC_FFTBINS * sizeof(GSC_COMPLEX_T));
#else
                    pstGscApi->pfuncGscOutputHook(pstGscApi->pvUsrData, iIdx, (S8 *)asZero, GSC_FRMINC << 1);
#endif
                }
            }
        }
    }

    return iLen;
}

/************************************************************
  Function   : GSC_Reset()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/28, Chao.Xu create

 ************************************************************/
GSC_VISIBILITY_ATTR VOID GSC_Reset(GSC_Api_S *pstGscApi)
{
    S32 iTaps = 0;
    S32 iGsc  = 0;
    GSC_Core_S *pstGscInst = NULL;

    if ((pstGscApi == NULL) || (pstGscApi->pstGscInstance == NULL) || (pstGscApi->pstGscCfg == NULL))
    {
#ifdef AISP_TSL_INFO
        printf("GSC_Reset: NULL pointer.\n");
#endif
    }
    else
    {
        iGsc       = pstGscApi->pstGscCfg->iGsc;
        iTaps      = pstGscApi->pstGscCfg->iTaps;
        pstGscInst = pstGscApi->pstGscInstance;
        /* Shared memory */
        AISP_TSL_memset(pstGscInst->pcpxXFFT, 0, GSC_MICS * GSC_FFTBINS * sizeof(GSC_COMPLEX_T));
        AISP_TSL_memset(pstGscInst->pcpxMic1, 0,            GSC_FFTBINS * sizeof(GSC_COMPLEX_T));
        AISP_TSL_memset(pstGscInst->pcpxMic2, 0,            GSC_FFTBINS * sizeof(GSC_COMPLEX_T));
        AISP_TSL_memset(pstGscInst->pcpxRef1, 0,            GSC_FFTBINS * sizeof(GSC_COMPLEX_T));
        AISP_TSL_memset(pstGscInst->pcpxRef2, 0,            GSC_FFTBINS * sizeof(GSC_COMPLEX_T));
        /* Unshared memory */
        AISP_TSL_memset(pstGscInst->pcpxGscX,      0, iGsc * GSC_FFTBINS * iTaps * sizeof(GSC_COMPLEX_T));
        AISP_TSL_memset(pstGscInst->pcpxGscWeight, 0, iGsc * GSC_FFTBINS * iTaps * sizeof(GSC_COMPLEX_T));
        AISP_TSL_memset(pstGscInst->pcpxBlockOut,  0, iGsc * GSC_FFTBINS * sizeof(GSC_COMPLEX_T));
        AISP_TSL_memset(pstGscInst->pcpxGscH,      0, iGsc * GSC_FFTBINS * sizeof(GSC_COMPLEX_T));
        AISP_TSL_memset(pstGscInst->pGscXPowerSum, 0, iGsc * GSC_FFTBINS * sizeof(GSC_S64_T));
        AISP_TSL_memset(pstGscInst->pGscYPowerSum, 0, iGsc * GSC_FFTBINS * sizeof(GSC_S64_T));
        AISP_TSL_memset(pstGscInst->pGscNoisePSD,  0,        GSC_FFTBINS * sizeof(GSC_S64_T));
        AISP_TSL_memset(pstGscInst->pGscPH1,       0,        GSC_FFTBINS * sizeof(GSC_S32_T));
        AISP_TSL_memset(pstGscInst->pGscPH1mean,   0,        GSC_FFTBINS * sizeof(GSC_S32_T));
        AISP_TSL_memset(pstGscInst->piRaw,         0, GSC_MICS * GSC_FRMINC * sizeof(GSC_S32_T));
        AISP_TSL_memset(pstGscInst->psDataLeft,    0, iGsc * GSC_FRMINC  * sizeof(GSC_S16_T));
    }

    return ;
}

/************************************************************
 *   Function   : GSC_CfgDelete()
 *   Description:
 *   Calls      :
 *   Called By  :
 *   Input      :
 *   Output     :
 *   Return     :
 *   Others     : Used in UDA
 *   History    :
 *       2019/04/30, chao.xu create
 *
 *************************************************************/
VOID GSC_CfgDelete(GSC_CoreCfg_S *pstGscCfg)
{
    if (pstGscCfg)
    {
        AISP_PTR_FREE(pstGscCfg);
    }
}

/************************************************************
  Function   : GSC_Delete()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/28, Chao.Xu create

************************************************************/
GSC_VISIBILITY_ATTR VOID GSC_Delete(GSC_Api_S *pstGscApi)
{
    if (pstGscApi != NULL)
    {
#ifndef STATIC_MEM_FIXED_FFT
        fftDelete(&pstGscApi->stFFTEng);
#endif
    }

    return;
}

/************************************************************
  Function   : GSC_Version()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/28, Chao.Xu create

************************************************************/
GSC_VISIBILITY_ATTR S8 *GSC_Version(VOID)
{
    return LUDA_VERSION;
}

