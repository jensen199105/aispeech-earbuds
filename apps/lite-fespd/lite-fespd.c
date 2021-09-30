/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : test.c
  Project    :
  Module     :
  Version    :
  Date       : 2018/11/15
  Author     : Youhai.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Youhai.Jiang  2018/11/15         1.00              Create

************************************************************/
#ifdef AISP_TSL_INFO
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "lite-fespd.h"
#include "gsc_api.h"
#include "api_nwakeup.h"
#include "AISP_TSL_common.h"
#include "AISP_TSL_str.h"

#define LFESPD_API_ATTR      __attribute__((visibility("default")))
#define WAKEUP_INTERVAL     (25)
#define US_SCALAR           (1000000)
#define MS_SCALAR           (1000)
#define FRM_INC_TIME        (16)        /* 16 ms */
#define WAV_HEADER_LEN      (44)
#ifdef ENABLE_LUDA_FOR_MOBILE
#define BEAMS               (3)         /* mics */
#elif defined ENABLE_LUDA_FOR_HOME
#define BEAMS               (3)         /* mics */
#elif defined ENABLE_LUDA_FOR_HEADSET
#define BEAMS               (1)         /* mics */
#endif
#define MAX_BEAMS           BEAMS
#define LFESPD_WKP_MEM_SIZE (MAX_BEAMS*52*1024)   /* 52 KB */
#define LFESPD_ACTIVE_TIME  (450000)              /* active time is about 2 hours */

#ifdef LFESPD_BUFFER_EN
#define LESPD_BUFFER_LEN    (2048)
#endif

#ifdef LFESPD_INTERNAL_MEM_MNG
#define LFESPD_MALLOC(size)  calloc(1, size)
#endif
#define LFESPD_VER           "lite-fespd v1.0.2.2019.8.1"

typedef struct LiteUDA_S
{
    int              iSharedMemSize;
    int              iUnsharedMemSize;
    GSC_CoreCfg_S    stGscCfg;
    GSC_Api_S       *pstUDAEng;
    char            *pcSharedMem;
    char            *pcUnsharedMem;
    LFESPD_BF_hook_t pfuncUDAHandler;
    void            *pvBfData;
} LiteUDA_t;

typedef struct LiteWakeup_S
{
    wakeup_Nchans_t  *pstNWkpEng;
    int               iMemPoolSize;
    char             *pcWKPMemPool; //size is WKP_MEM_SIZE
    LFESPD_WKP_hook_t pfuncNWkpHandler;
    LFESPD_VAD_hook_t pfuncVadHandler;
    void             *pvWkpData; // Private parameter for wkp handler
    void             *pvVadData; // Private parameter for vad handler
} LiteWakeup_t;

struct LFESPD_ENGINE
{
#ifdef LFESPD_WAKEUP_ENABLE_DELAY
    int               iLatestFrm;
#endif
    int               iAuthSum;
    LiteUDA_t         stLiteUDA;
    LiteWakeup_t      stLiteWkp;
#ifdef LFESPD_BUFFER_EN
    unsigned int      uiCurLen;
    char              acTmpDataBuffer[LESPD_BUFFER_LEN];
#endif
};

//#ifdef LUDA_SUPPORT_FREQOUT_DIRECT
////extern FILE* fp_disp;
//#endif
/************************************************************
  Function   : wkp_handler()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
static int wkp_handler(void *pvUsrData, int iIdx, char *pcJson)
{
    char acBuf[256] = {0};
    char *pcFrame = NULL;
    char *pcTmp   = NULL;
#ifdef LFESPD_WAKEUP_ENABLE_DELAY
    int  iFrmVal  = 0;
#endif
    LFESPD_ENGINE_S *pstLfespdEng = (LFESPD_ENGINE_S *)pvUsrData;
    LFESPD_WKP_hook_t pfuncNWkpHandler = (LFESPD_WKP_hook_t)pstLfespdEng->stLiteWkp.pfuncNWkpHandler;

    if (AISP_TSL_strlen(pcJson) < sizeof(acBuf))
    {
        AISP_TSL_memcpy(acBuf, pcJson, AISP_TSL_strlen(pcJson));
    }

    pcFrame = AISP_TSL_strstr(acBuf, "\"frame\":");
    pcFrame = AISP_TSL_strtok_r(pcFrame, ":", &pcTmp);
    pcFrame = AISP_TSL_strtok_r(NULL, "}", &pcTmp);
#ifdef LFESPD_WAKEUP_ENABLE_DELAY
    iFrmVal = atoi(pcFrame);

    if (iFrmVal - pstLfespdEng->iLatestFrm > WAKEUP_INTERVAL)
    {
        pstLfespdEng->iLatestFrm = iFrmVal;

        if (pfuncNWkpHandler)
        {
            pfuncNWkpHandler(pstLfespdEng->stLiteWkp.pvWkpData, iIdx, pcJson);
        }
    }

#else

    if (pfuncNWkpHandler)
    {
        pfuncNWkpHandler(pstLfespdEng->stLiteWkp.pvWkpData, iIdx, pcJson);
    }

#endif
    wakeup_Nchans_reset(pstLfespdEng->stLiteWkp.pstNWkpEng);
    return 0;
}

/************************************************************
  Function   : vad_handler()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
static int vad_handler(void *pvUsrData, int iChanIdx, int iFrmStatus, int iFrmIdx)
{
    LFESPD_ENGINE_S *pstLfespdEng = (LFESPD_ENGINE_S *)pvUsrData;
    LFESPD_VAD_hook_t pfuncVadHandler = (LFESPD_VAD_hook_t)pstLfespdEng->stLiteWkp.pfuncVadHandler;

    if (pfuncVadHandler)
    {
        pfuncVadHandler(pvUsrData, iChanIdx, iFrmStatus, iFrmIdx);
    }

    return 0;
}

/************************************************************
  Function   : rdma_handler()

  Description: store gsc output data
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/11/15, Youhai.Jiang create

************************************************************/
static void uda_handler(void *pvUsrData, int iIdx, char *pcData, int iLen)
{
    LFESPD_ENGINE_S *pstLfespdEng = (LFESPD_ENGINE_S *)pvUsrData;
    LFESPD_BF_hook_t pfuncUDAHandler = (LFESPD_BF_hook_t)pstLfespdEng->stLiteUDA.pfuncUDAHandler;
#ifdef LUDA_SUPPORT_FREQOUT_DIRECT
    /* Taked from gscProcessMobile directly. */
    int *pcpxEntBlock = (int *)pcData;

    if (NULL == pcpxEntBlock)
    {
#ifdef AISP_TSL_INFO
        printf("%s:%d: pcpxEntBlock NULL pointer!\n", __FUNCTION__, __LINE__);
#endif
    }

    wakeup_Nchans_feed(pstLfespdEng->stLiteWkp.pstNWkpEng, iIdx, pcData, iLen, 0);
#else
    wakeup_Nchans_feed(pstLfespdEng->stLiteWkp.pstNWkpEng, iIdx, pcData, iLen, 0);
#endif

    if (pfuncUDAHandler)
    {
        pfuncUDAHandler(pstLfespdEng->stLiteUDA.pvBfData, iIdx, pcData, iLen);
    }
}

/************************************************************
  Function   : LFESPD_memSize()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     : Lite-fespd size
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
LFESPD_API_ATTR int LFESPD_memSize(void)
{
    int iLen = AISP_ALIGN_SIZE(sizeof(LFESPD_ENGINE_S), AISP_ALIGN_8_MASK);
    /* Add shared memory for GSC */
    iLen += GSC_SharedMemSizeGet(NULL);
    /* Add unshared memory for GSC */
    iLen += GSC_UnsharedMemSizeGet(NULL);
    iLen += LFESPD_WKP_MEM_SIZE;
    return iLen;
}

/************************************************************
  Function   : LFESPD_new()

  Description: Create Lite-fespd Engine
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     : Lite-fespd Engine pointer
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
LFESPD_API_ATTR LFESPD_ENGINE_S *LFESPD_new(void *pvMemBase, int len)
{
    LFESPD_ENGINE_S *pstLfespdEng = NULL;
#ifdef LFESPD_INTERNAL_MEM_MNG
    /* Create Low Power Dual Mics Array for Mobile phone */
    pstLfespdEng = (LFESPD_ENGINE_S *)LFESPD_MALLOC(sizeof(LFESPD_ENGINE_S));

    if (NULL == pstLfespdEng)
    {
        return NULL;
    }

#else

    if ((NULL == pvMemBase) || (len < LFESPD_memSize()))
    {
        return NULL;
    }

    AISP_TSL_memset(pvMemBase, 0, len);
    pstLfespdEng = (LFESPD_ENGINE_S *)pvMemBase;
#endif
    /* lite-uda */
    pstLfespdEng->stLiteUDA.iSharedMemSize   = GSC_SharedMemSizeGet(NULL);
    pstLfespdEng->stLiteUDA.iUnsharedMemSize = GSC_UnsharedMemSizeGet(NULL);
    pstLfespdEng->stLiteUDA.pcSharedMem   = AISP_MEM_ALIGN((char *)(pstLfespdEng + 1), 8);
    pstLfespdEng->stLiteUDA.pcUnsharedMem = pstLfespdEng->stLiteUDA.pcSharedMem + pstLfespdEng->stLiteUDA.iSharedMemSize;
    /* lite-wakeup memory init */
    pstLfespdEng->stLiteWkp.iMemPoolSize = LFESPD_WKP_MEM_SIZE;
    pstLfespdEng->stLiteWkp.pcWKPMemPool = pstLfespdEng->stLiteUDA.pcUnsharedMem + pstLfespdEng->stLiteUDA.iUnsharedMemSize;
    return pstLfespdEng;
}

/************************************************************
  Function   : LFESPD_start()

  Description: Start Lite-fespd Engine and register callback hook
  Calls      :
  Called By  :
  Input      : pstLfespdEng: Lite-fespd Engine
               pvBfHandler : Function output beamforming data;
               pvBfData    : Private parameter for pvBfHandler func;
               pvVadHandler: Function output vad status;
               pvWkpHandler: Function output wakeup info;
               pvWkpEnv    : Private parameter for pvWkpHandler func;
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
LFESPD_API_ATTR int LFESPD_start(LFESPD_ENGINE_S *pstLfespdEng, void *pvBfHandler, void *pvBfData, void *pvWkpHandler,
                                 void *pvWkpData, void *pvWkpEnv)
{
    int iRet = -1;

    if (GSC_CfgInit(&pstLfespdEng->stLiteUDA.stGscCfg, NULL) < 0)
    {
        goto FAIL;
    }

    /* create rdma engine */
    pstLfespdEng->stLiteUDA.pstUDAEng = (GSC_Api_S *)GSC_New(&pstLfespdEng->stLiteUDA.stGscCfg,
                                        pstLfespdEng->stLiteUDA.pcSharedMem,
                                        pstLfespdEng->stLiteUDA.pcUnsharedMem,
                                        (GSC_FUNCHOOK_T)uda_handler, pstLfespdEng);

    if (NULL == pstLfespdEng->stLiteUDA.pstUDAEng)
    {
        goto FAIL;
    }

    /* Create N-Wakeup Engine */
    pstLfespdEng->stLiteWkp.pstNWkpEng = wakeup_Nchans_new(pstLfespdEng->stLiteWkp.pcWKPMemPool, BEAMS,
                                         pstLfespdEng->stLiteWkp.iMemPoolSize,
                                         pstLfespdEng, (wakeup_Nchans_handler)wkp_handler,
                                         (wakeup_Nchans_vad_handler)vad_handler);

    if (NULL == pstLfespdEng->stLiteWkp.pstNWkpEng)
    {
        goto FAIL;
    }

    /* wakeup env setting */
    wakeup_Nchans_start(pstLfespdEng->stLiteWkp.pstNWkpEng, (char *)pvWkpEnv, AISP_TSL_strlen((char *)pvWkpEnv));
    /* start rdma engine */
    GSC_Start(pstLfespdEng->stLiteUDA.pstUDAEng, NULL, NULL);
    pstLfespdEng->stLiteUDA.pfuncUDAHandler  = (LFESPD_BF_hook_t)pvBfHandler;
    pstLfespdEng->stLiteUDA.pvBfData         = pvBfData;
    pstLfespdEng->stLiteWkp.pfuncNWkpHandler = (LFESPD_WKP_hook_t)pvWkpHandler;
    pstLfespdEng->stLiteWkp.pvWkpData        = pvWkpData;
    return 0;
FAIL:

    /* release Lite-UDA engine */
    if (pstLfespdEng->stLiteUDA.pstUDAEng)
    {
        GSC_Delete(pstLfespdEng->stLiteUDA.pstUDAEng);
        pstLfespdEng->stLiteUDA.pstUDAEng = NULL;
    }

    /* release lite-wakeup engine */
    if (pstLfespdEng->stLiteWkp.pstNWkpEng)
    {
        wakeup_Nchans_delete(pstLfespdEng->stLiteWkp.pstNWkpEng);
        pstLfespdEng->stLiteWkp.pstNWkpEng = NULL;
    }

    return iRet;
}

/************************************************************
  Function   : LFESPD_vadRegister()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
LFESPD_API_ATTR void LFESPD_vadRegister(LFESPD_ENGINE_S *pstLfespdEng, void *pvVadHandler, void *pvVadData)
{
    pstLfespdEng->stLiteWkp.pfuncVadHandler  = (LFESPD_VAD_hook_t)pvVadHandler;
    pstLfespdEng->stLiteWkp.pvVadData        = pvVadData;
}

/************************************************************
  Function   : LFESPD_feed()

  Description: Feed data to Lite-fespd Engine
  Calls      :
  Called By  :
  Input      : pstLfespdEng: Lite-fespd Engine
               pcData      : data buffer
               iLen        : length of data in Bytes
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
LFESPD_API_ATTR int LFESPD_feed(LFESPD_ENGINE_S *pstLfespdEng, char *pcData, int iLen)
{
#ifdef LFESPD_BUFFER_EN
    int iLenPerSnd = GSC_LenPerSnd(pstLfespdEng->stLiteUDA.pstUDAEng);
#endif

    if (pstLfespdEng->iAuthSum > LFESPD_ACTIVE_TIME)
    {
        return -1;
    }

    pstLfespdEng->iAuthSum++;
#ifdef LFESPD_BUFFER_EN
    AISP_TSL_memcpy(pstLfespdEng->acTmpDataBuffer + pstLfespdEng->uiCurLen, pcData, iLen);
    pstLfespdEng->uiCurLen += iLen;

    while (pstLfespdEng->uiCurLen >= iLenPerSnd)
    {
        GSC_Feed(pstLfespdEng->stLiteUDA.pstUDAEng, pstLfespdEng->acTmpDataBuffer, iLenPerSnd);
        /* update data*/
        pstLfespdEng->uiCurLen -= iLenPerSnd;

        if (pstLfespdEng->uiCurLen)
        {
            AISP_TSL_memmove(pstLfespdEng->acTmpDataBuffer, pstLfespdEng->acTmpDataBuffer + iLenPerSnd, pstLfespdEng->uiCurLen);
        }
    }

#else
    GSC_Feed(pstLfespdEng->stLiteUDA.pstUDAEng, pcData, iLen);
#endif
    return iLen;
}

/************************************************************
  Function   : LFESPD_lenPerSend()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
LFESPD_API_ATTR int LFESPD_lenPerSend(LFESPD_ENGINE_S *pstLfespdEng)
{
    return GSC_LenPerSnd(NULL);
}

/************************************************************
  Function   : LFESPD_end()

  Description: Stop lite-fespd Engine
  Calls      :
  Called By  :
  Input      : pstLfespdEng: lite-fespd Engine
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
LFESPD_API_ATTR int LFESPD_end(LFESPD_ENGINE_S *pstLfespdEng)
{
    wakeup_Nchans_end(pstLfespdEng->stLiteWkp.pstNWkpEng);
    return 0;
}

/************************************************************
  Function   : LFESPD_delete()

  Description: Delete lite-fespd Engine
  Calls      :
  Called By  :
  Input      : pstLfespdEng: lite-fespd Engine
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
LFESPD_API_ATTR int LFESPD_delete(LFESPD_ENGINE_S *pstLfespdEng)
{
#ifdef LFESPD_INTERNAL_MEM_MNG

    if (pstLfespdEng)
    {
        /* release Lite-UDA engine */
        if (pstLfespdEng->stLiteUDA.pstUDAEng)
        {
            GSC_Delete(pstLfespdEng->stLiteUDA.pstUDAEng);
            pstLfespdEng->stLiteUDA.pstUDAEng = NULL;
        }

        /* release Lite-Wakeup engine */
        if (pstLfespdEng->pstNWkpEng)
        {
            wakeup_Nchans_delete(pstLfespdEng->stLiteWkp.pstNWkpEng);
            pstLfespdEng->stLiteWkp.pstNWkpEng = NULL;
        }

        free(pstLfespdEng);
    }

#endif
    return 0;
}

/************************************************************
  Function   : LFESPD_version()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
LFESPD_API_ATTR char *LFESPD_version(void)
{
    return LFESPD_VER;
}
