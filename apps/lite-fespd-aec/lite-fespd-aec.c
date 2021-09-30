/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : lite-fespd-aec.c
  Project    :
  Module     :
  Version    :
  Date       : 2019/08/28
  Author     : Youhai.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Youhai.Jiang  2019/08/28         1.00              Create

************************************************************/
#ifdef AISP_TSL_INFO
//#include <stdio.h>
#endif
//#include <stdlib.h>
//#include <string.h>
#include "lite-fespd-aec.h"
#include "gsc_api.h"
#include "api_nwakeup.h"
#include "AISP_TSL_common.h"
#include "AISP_TSL_str.h"
#include "aisp_memmng.h"
#include "aec_api.h"

#define LDEEW_WAKEUP_ENABLE_DELAY

#define LDEEW_API_ATTR      __attribute__((visibility("default")))
#define SHORT_FRAME     (5)
#define LONG_FRAME      (30)
#define US_SCALAR           (1000000)
#define MS_SCALAR           (1000)
#define FRM_INC_TIME        (16)        /* 16 ms */
#define WAV_HEADER_LEN      (44)
#ifdef ENABLE_LUDA_FOR_MOBILE
#define BEAMS               (3)         /* mics */
#elif defined ENABLE_LUDA_FOR_HOME
#define BEAMS               (3)         /* mics */
#else
#define BEAMS               (1)         /* mics */
#endif
#define MAX_BEAMS           BEAMS
#define LDEEW_WKP_MEM_SIZE  (MAX_BEAMS*52*1024)   /* 52 KB */
#define LDEEW_AEC_MEM_SIZE  (165 * 1024)            /* 150 KB */

#ifdef LFESPD_DEBUG_RELEASE
#define LDEEW_ACTIVE_TIME   (450000)              /* active time is about 2 hours */
#endif

#define AEC_GSC_FRAME_RATIO (2)
#define LDEEW_BUFFER_LEN    (6144)
#define AEC2GSC_BUFFER_LEN  (2048)
#define AEC_POP_BUFFER_LEN  (2560)
#define AEC_POP_LEN         (1280)
#define LDEEW_VER           "lite-fespd-aec v1.0.1-2019.11.12 09:21"

#ifdef USE_PLATFORM_DEBUG
extern void dump_cs0(int16_t *data, int16_t size);
extern void dump_cs1(int16_t *data, int16_t size);
extern void printf_log(const char *fmt, ...);
#define printf printf_log
#endif

extern void aisp_write2sd_ex(char *data, int len,int index);

typedef struct LiteAEC_S
{
    int              iSharedMemSize;
    int              iUnsharedMemSize;
    int              iMemPoolSize;
    VOID            *pstAecEng;
    S8              *pcSharedMem;
    S8              *pcUnsharedMem;
    S8              *pcAecMemPool;
    void            *pvAecData;
    LDEEW_AEC_hook_t pfuncAecHandler;
    void            *arg;
} LiteAEC_t;

typedef struct LiteUDA_S
{
    int              iSharedMemSize;
    int              iUnsharedMemSize;
    GSC_CoreCfg_S    stGscCfg;
    GSC_Api_S       *pstUDAEng;
    S8              *pcSharedMem;
    S8              *pcUnsharedMem;
    LDEEW_BF_hook_t  pfuncUDAHandler;
    void            *pvBfData;
    LDEEW_ASR_hook_t pfAsrHandler;
    void            *pvAsrData;
} LiteUDA_t;

typedef struct LiteWakeup_S
{
    wakeup_Nchans_t  *pstNWkpEng;
    int               iMemPoolSize;
    S8               *pcWKPMemPool; //size is WKP_MEM_SIZE
    LDEEW_WKP_hook_t pfuncNWkpHandler;
    LDEEW_VAD_hook_t pfuncVadHandler;
    void             *pvWkpData; // Private parameter for wkp handler
    void             *pvVadData; // Private parameter for vad handler
} LiteWakeup_t;

struct LDEEW_ENGINE
{
#ifdef LDEEW_WAKEUP_ENABLE_DELAY
    S8              cWtkStatus;
    U8              ucAsrBeam;
    U8              ucWkpBeams;
    U32             uiWtkFrm;
    U32             uiLatestFrm;
#endif
#ifdef LFESPD_DEBUG_RELEASE
    int             iAuthSum;
#endif
    LiteAEC_t       stLiteAEC;
    LiteUDA_t       stLiteUDA;
    LiteWakeup_t    stLiteWkp;
    unsigned int    uiCurLen;
    unsigned int    acTmpAecPopBuffer_remain;
    S8              acTmpDataBuffer[LDEEW_BUFFER_LEN];
    S8              acTmpAecPopBuffer[AEC_POP_BUFFER_LEN];
    S8              acTmpAec2GscBuffer[AEC2GSC_BUFFER_LEN];
};

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

    LDEEW_ENGINE_S *pstLdeewEng = (LDEEW_ENGINE_S *)pvUsrData;
    LDEEW_WKP_hook_t pfuncNWkpHandler = (LDEEW_WKP_hook_t)pstLdeewEng->stLiteWkp.pfuncNWkpHandler;

#ifdef LDEEW_WAKEUP_ENABLE_DELAY
    if(!pstLdeewEng->cWtkStatus)
    {
        if (pfuncNWkpHandler)
        {
            pfuncNWkpHandler(pstLdeewEng->stLiteWkp.pvWkpData, iIdx, pcJson);
        }
        pstLdeewEng->cWtkStatus = 1;
    }
    if(pstLdeewEng->uiLatestFrm < SHORT_FRAME)
    {
        pstLdeewEng->ucWkpBeams |= 1 << (iIdx - 1);
    }
    
#else

    if (pfuncNWkpHandler)
    {
        pfuncNWkpHandler(pstLdeewEng->stLiteWkp.pvWkpData, iIdx, pcJson);
    }

#endif
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
    LDEEW_ENGINE_S *pstLdeewEng = (LDEEW_ENGINE_S *)pvUsrData;
    LDEEW_VAD_hook_t pfuncVadHandler = (LDEEW_VAD_hook_t)pstLdeewEng->stLiteWkp.pfuncVadHandler;

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
    LDEEW_ENGINE_S *pstLdeewEng = (LDEEW_ENGINE_S *)pvUsrData;
    LDEEW_BF_hook_t pfuncUDAHandler = (LDEEW_BF_hook_t)pstLdeewEng->stLiteUDA.pfuncUDAHandler;
    LDEEW_ASR_hook_t pfAsrHandler = (LDEEW_ASR_hook_t)pstLdeewEng->stLiteUDA.pfAsrHandler;

#ifdef LUDA_SUPPORT_FREQOUT_DIRECT
    /* Taked from gscProcessMobile directly. */
    int *pcpxEntBlock = (int *)pcData;

    if (NULL == pcpxEntBlock)
    {
#if defined AISP_TSL_INFO || defined USE_PLATFORM_DEBUG
        printf("%s:%d: pcpxEntBlock NULL pointer!\n", __FUNCTION__, __LINE__);
#endif
    }

#endif

    wakeup_Nchans_feed(pstLdeewEng->stLiteWkp.pstNWkpEng, iIdx, pcData, iLen, 0);
    //aisp_write2sd_ex(pcData,iLen,iIdx);
    if(pstLdeewEng->cWtkStatus && (iIdx == (BEAMS - 1)))
    {
        pstLdeewEng->uiLatestFrm++;
        if(pstLdeewEng->uiLatestFrm > LONG_FRAME)
        {
            pstLdeewEng->uiLatestFrm = 0;
            pstLdeewEng->cWtkStatus = 0;
            pstLdeewEng->ucWkpBeams = 0;
        }
        else if(pstLdeewEng->uiLatestFrm == SHORT_FRAME)
        {
            /*chose beam*/
            pstLdeewEng->ucAsrBeam = GSC_BeamUpdata(pstLdeewEng->stLiteUDA.pstUDAEng , pstLdeewEng->ucWkpBeams);
            wakeup_Nchans_reset(pstLdeewEng->stLiteWkp.pstNWkpEng);
            printf("== pstLdeewEng->ucWkpBeams: %d  ucAsrBeam : %d \n",pstLdeewEng->ucWkpBeams,pstLdeewEng->ucAsrBeam);
        }
    }
    

    if(pfAsrHandler)
    {
        if(pstLdeewEng->ucAsrBeam == iIdx)
        {
            pfAsrHandler(pstLdeewEng->stLiteUDA.pvAsrData,pcData,iLen,(pstLdeewEng->uiLatestFrm == SHORT_FRAME));
        }
    }
    if (pfuncUDAHandler)
    {
        pfuncUDAHandler(pstLdeewEng->stLiteUDA.pvBfData, iIdx, pcData, iLen);
    }
}

/************************************************************
  Function   : LDEEW_memSize()

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
LDEEW_API_ATTR int LDEEW_memSize(void)
{
    int iLen = AISP_ALIGN_SIZE(sizeof(LDEEW_ENGINE_S), AISP_ALIGN_8_MASK);
    /* Add shared memory for GSC */
    iLen += GSC_SharedMemSizeGet(NULL);
    /* Add unshared memory for GSC */
    iLen += GSC_UnsharedMemSizeGet(NULL);
    iLen += LDEEW_WKP_MEM_SIZE;
    iLen += LDEEW_AEC_MEM_SIZE;
    return iLen;
}

/************************************************************
  Function   : LDEEW_aecPopControl()

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
static int LDEEW_aecPopBufferPop(LDEEW_ENGINE_S *pstLdeewEng, S8 *data, int len)
{
    S8 *acTmpAecPop  = NULL;
    int acTmpAecPopBuffer_remain = 0;
    int total_len = 0;
    int cpy_size  = 0;

    if (pstLdeewEng->stLiteAEC.pfuncAecHandler)
    {
        acTmpAecPop  = pstLdeewEng->acTmpAecPopBuffer;
        acTmpAecPopBuffer_remain = pstLdeewEng->acTmpAecPopBuffer_remain;
        total_len = acTmpAecPopBuffer_remain + len;

        if (total_len > AEC_POP_LEN)
        {
            cpy_size = AEC_POP_LEN - acTmpAecPopBuffer_remain;
            /* push data to remain data buffer */
            AISP_TSL_memcpy(acTmpAecPop + acTmpAecPopBuffer_remain, data, cpy_size);
            pstLdeewEng->stLiteAEC.pfuncAecHandler(pstLdeewEng->stLiteAEC.arg, acTmpAecPop, AEC_POP_LEN);
            total_len -= AEC_POP_LEN;
            data      += cpy_size;

            while (total_len > AEC_POP_LEN)
            {
                pstLdeewEng->stLiteAEC.pfuncAecHandler(pstLdeewEng->stLiteAEC.arg, data, AEC_POP_LEN);
                total_len -= AEC_POP_LEN;
                data      += AEC_POP_LEN;
            }

            if (total_len)
            {
                acTmpAecPopBuffer_remain = total_len;
                AISP_TSL_memcpy(acTmpAecPop, data, acTmpAecPopBuffer_remain);
            }
        }
        else
        {
            /* push data to remain data buffer */
            AISP_TSL_memcpy(acTmpAecPop + acTmpAecPopBuffer_remain, data, len);
            cpy_size = len;
            acTmpAecPopBuffer_remain += cpy_size;
        }

        pstLdeewEng->acTmpAecPopBuffer_remain = acTmpAecPopBuffer_remain;
    }

    return pstLdeewEng->acTmpAecPopBuffer_remain;
}
/************************************************************
  Function   : LDEEW_new()

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
LDEEW_API_ATTR LDEEW_ENGINE_S *LDEEW_new(void *pvMemBase, int len)
{
    LDEEW_ENGINE_S *pstLdeewEng = NULL;
#if defined AISP_TSL_INFO || defined USE_PLATFORM_DEBUG
    printf("AISpeech FESPD Version: " LDEEW_VER "\n\r");
#endif

    if ((NULL == pvMemBase) || (len < LDEEW_memSize()))
    {
        return NULL;
    }

    AISP_TSL_memset(pvMemBase, 0, len);
    pstLdeewEng = (LDEEW_ENGINE_S *)pvMemBase;
    /* lite-uda */
    pstLdeewEng->stLiteUDA.iSharedMemSize   = GSC_SharedMemSizeGet(NULL);
    pstLdeewEng->stLiteUDA.iUnsharedMemSize = GSC_UnsharedMemSizeGet(NULL);
    pstLdeewEng->stLiteUDA.pcSharedMem   = AISP_MEM_ALIGN((char *)(pstLdeewEng + 1), 8);
    pstLdeewEng->stLiteUDA.pcUnsharedMem = pstLdeewEng->stLiteUDA.pcSharedMem + pstLdeewEng->stLiteUDA.iSharedMemSize;
    /* lite-wakeup memory init */
    pstLdeewEng->stLiteWkp.iMemPoolSize = LDEEW_WKP_MEM_SIZE;
    pstLdeewEng->stLiteWkp.pcWKPMemPool = pstLdeewEng->stLiteUDA.pcUnsharedMem + pstLdeewEng->stLiteUDA.iUnsharedMemSize;
    /* lite-aec memory init */
    pstLdeewEng->stLiteAEC.iMemPoolSize = LDEEW_AEC_MEM_SIZE;
    pstLdeewEng->stLiteAEC.pcAecMemPool = pstLdeewEng->stLiteWkp.pcWKPMemPool + LDEEW_WKP_MEM_SIZE;
    AISP_MemInit(pstLdeewEng->stLiteAEC.pcAecMemPool, LDEEW_AEC_MEM_SIZE);
    return pstLdeewEng;
}

/************************************************************
  Function   : LDEEW_start()

  Description: Start Lite-fespd Engine and register callback hook
  Calls      :
  Called By  :
  Input      : pstLdeewEng: Lite-fespd Engine
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
LDEEW_API_ATTR int LDEEW_start(LDEEW_ENGINE_S *pstLdeewEng, void *pvBfHandler, void *pvBfData, void *pvWkpHandler,
                               void *pvWkpData, void *pvWkpEnv)
{
    int iRet = -1;

    /* create aec engine*/
    pstLdeewEng->stLiteAEC.pstAecEng = aec_api_new(NULL);

    if (NULL == pstLdeewEng->stLiteAEC.pstAecEng)
    {
        goto FAIL;
    }


    if (GSC_CfgInit(&pstLdeewEng->stLiteUDA.stGscCfg, NULL) < 0)
    {
        goto FAIL;
    }


    /* create rdma engine */
    pstLdeewEng->stLiteUDA.pstUDAEng = (GSC_Api_S *)GSC_New(&pstLdeewEng->stLiteUDA.stGscCfg,
                                       pstLdeewEng->stLiteUDA.pcSharedMem,
                                       pstLdeewEng->stLiteUDA.pcUnsharedMem,
                                       (GSC_FUNCHOOK_T)uda_handler, pstLdeewEng);

    if (NULL == pstLdeewEng->stLiteUDA.pstUDAEng)
    {
        goto FAIL;
    }

    /* Create N-Wakeup Engine */
    pstLdeewEng->stLiteWkp.pstNWkpEng = wakeup_Nchans_new(pstLdeewEng->stLiteWkp.pcWKPMemPool, BEAMS,
                                        pstLdeewEng->stLiteWkp.iMemPoolSize,
                                        pstLdeewEng, (wakeup_Nchans_handler)wkp_handler,
                                        (wakeup_Nchans_vad_handler)vad_handler);

    if (NULL == pstLdeewEng->stLiteWkp.pstNWkpEng)
    {
        goto FAIL;
    }

    /* wakeup env setting */
    wakeup_Nchans_start(pstLdeewEng->stLiteWkp.pstNWkpEng, (char *)pvWkpEnv, AISP_TSL_strlen((char *)pvWkpEnv));
    /* start rdma engine */
    GSC_Start(pstLdeewEng->stLiteUDA.pstUDAEng, NULL, NULL);
    pstLdeewEng->stLiteUDA.pfuncUDAHandler  = pvBfHandler;
    pstLdeewEng->stLiteUDA.pvBfData         = pvBfData;
    pstLdeewEng->stLiteWkp.pfuncNWkpHandler = pvWkpHandler;
    pstLdeewEng->stLiteWkp.pvWkpData        = pvWkpData;
    return 0;
FAIL:

    /* release Lite-AEC engine */
    if (pstLdeewEng->stLiteAEC.pstAecEng)
    {
        GSC_Delete(pstLdeewEng->stLiteAEC.pstAecEng);
        pstLdeewEng->stLiteAEC.pstAecEng = NULL;
    }

    /* release Lite-UDA engine */
    if (pstLdeewEng->stLiteUDA.pstUDAEng)
    {
        GSC_Delete(pstLdeewEng->stLiteUDA.pstUDAEng);
        pstLdeewEng->stLiteUDA.pstUDAEng = NULL;
    }

    /* release lite-wakeup engine */
    if (pstLdeewEng->stLiteWkp.pstNWkpEng)
    {
        wakeup_Nchans_delete(pstLdeewEng->stLiteWkp.pstNWkpEng);
        pstLdeewEng->stLiteWkp.pstNWkpEng = NULL;
    }

    return iRet;
}

/************************************************************
  Function   : LDEEW_vadRegister()

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
LDEEW_API_ATTR void LDEEW_vadRegister(LDEEW_ENGINE_S *pstLdeewEng, void *pvVadHandler, void *pvVadData)
{
    pstLdeewEng->stLiteWkp.pfuncVadHandler  = pvVadHandler;
    pstLdeewEng->stLiteWkp.pvVadData        = pvVadData;
}

/************************************************************
  Function   : LDEEW_a2bbufferAdapter()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/08/29, Youhai.Jiang create

************************************************************/
static void LDEEW_a2bbufferAdapter(LDEEW_ENGINE_S *pstLdeewEng, U16 *dst, U16 *src)
{
    U32   uiMicIdx      = 0;
    U32   uiFrmIdx      = 0;
    U32   uiAec_frmSize = 0;
    U32   uiAecMicNum   = 0;
    U32   uiOffTmp      = 0;
    VOID *pstAecEng     = NULL;
    pstAecEng   = pstLdeewEng->stLiteAEC.pstAecEng;
    uiAecMicNum = aec_api_micNum(pstAecEng);
    uiAec_frmSize = aec_api_frameSize(pstAecEng);

    for (uiMicIdx = 0; uiMicIdx < uiAecMicNum; uiMicIdx++)
    {
        uiOffTmp = uiMicIdx * uiAec_frmSize;

        for (uiFrmIdx = 0; uiFrmIdx < uiAec_frmSize; uiFrmIdx++)
        {
            dst[uiFrmIdx * uiAecMicNum + uiMicIdx] = src[uiOffTmp + uiFrmIdx];
        }
    }

    return ;
}

extern void aisp_write2sd(char *data, int len);


/************************************************************
  Function   : LDEEW_feed()

  Description: Feed data to Lite-fespd Engine
  Calls      :
  Called By  :
  Input      : pstLdeewEng: Lite-fespd Engine
               pcData      : data buffer
               iLen        : length of data in Bytes
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
LDEEW_API_ATTR int LDEEW_feed(LDEEW_ENGINE_S *pstLdeewEng, char *pcData, int iLen)
{
    U32   uiMicIdx      = 0;
    U32   uiAecMicNum   = 0;
    U32   uiAecWavChan  = 0;
    U32   uiAec_frmSize = 0;
    U32   uiGscPerSnd   = 0;
    U32   uiAecOutLen   = 0;
    U32   uiOffTmp      = 0;
    U32   iLenPerSnd    = 0;
    VOID  *pstAecEng    = NULL;
    S8 *acTmpDataBuffer = pstLdeewEng->acTmpDataBuffer;
    S8 *acTmpAec2GscBuffer = pstLdeewEng->acTmpAec2GscBuffer;
    pstAecEng    = pstLdeewEng->stLiteAEC.pstAecEng;
    uiAecMicNum  = aec_api_micNum(pstAecEng);
    uiAecWavChan = aec_api_wavChan(pstAecEng);
    uiAec_frmSize = aec_api_frameSize(pstAecEng);
    uiGscPerSnd   = GSC_LenPerSnd(pstLdeewEng->stLiteUDA.pstUDAEng);
    uiAecOutLen   = uiAec_frmSize * uiAecMicNum * sizeof(U16);
    iLenPerSnd    = uiAec_frmSize * uiAecWavChan * sizeof(U16);
#ifdef LFESPD_DEBUG_RELEASE

    if (pstLdeewEng->iAuthSum > LDEEW_ACTIVE_TIME)
    {
        return -1;
    }

    pstLdeewEng->iAuthSum++;
#endif
    AISP_TSL_memcpy(acTmpDataBuffer + pstLdeewEng->uiCurLen, pcData, iLen);
    pstLdeewEng->uiCurLen += iLen;
#ifdef USE_PLATFORM_DEBUG
    dump_cs0((int16_t *)pcData, iLen);
#endif
#if defined AISP_TSL_INFO || defined  USE_PLATFORM_DEBUG
    printf("Data memory: %p; len: %d; aec: %d\n\r", acTmpDataBuffer, pstLdeewEng->uiCurLen, iLenPerSnd);
#endif

    while (pstLdeewEng->uiCurLen >= iLenPerSnd)
    {
        aec_api_feed(pstAecEng, (unsigned char *)acTmpDataBuffer, uiAec_frmSize);
#ifdef USE_PLATFORM_DEBUG
        dump_cs1((int16_t *)acTmpDataBuffer, iLenPerSnd);
#endif

        for (uiMicIdx = 0; uiMicIdx < uiAecMicNum; uiMicIdx++)
        {
            uiOffTmp = uiMicIdx * uiAec_frmSize;
            aec_api_pop(pstAecEng, (short *)(acTmpDataBuffer + uiOffTmp * sizeof(U16)), uiMicIdx);
        }
        
        //aisp_write2sd(acTmpDataBuffer,uiAec_frmSize * 2);

        LDEEW_a2bbufferAdapter(pstLdeewEng, (U16 *)acTmpAec2GscBuffer, (U16 *)acTmpDataBuffer);
        //LDEEW_aecPopBufferPop(pstLdeewEng, acTmpAec2GscBuffer, uiAecOutLen);
#if 1
        /* feed data to gsc */
        while (uiAecOutLen >= uiGscPerSnd)
        {
            //GSC_Feed(pstLdeewEng->stLiteUDA.pstUDAEng, acTmpAec2GscBuffer, uiGscPerSnd);
            //aisp_write2sd(acTmpAec2GscBuffer,uiGscPerSnd);
            if (pstLdeewEng->stLiteAEC.pfuncAecHandler)
            {
                pstLdeewEng->stLiteAEC.pfuncAecHandler(pstLdeewEng->stLiteAEC.arg, acTmpAec2GscBuffer,uiGscPerSnd);
            }
            acTmpAec2GscBuffer += uiGscPerSnd;
            uiAecOutLen -= uiGscPerSnd;
        }
#endif

        /* restore ptr */
        acTmpAec2GscBuffer = pstLdeewEng->acTmpAec2GscBuffer;
        /* update data*/
        pstLdeewEng->uiCurLen -= iLenPerSnd;

        if (pstLdeewEng->uiCurLen)
        {
            AISP_TSL_memmove(pstLdeewEng->acTmpDataBuffer, acTmpDataBuffer + iLenPerSnd, pstLdeewEng->uiCurLen);
        }

#if defined AISP_TSL_INFO || defined  USE_PLATFORM_DEBUG
        printf("AISpeech DBG: Left len(%u)\n\r", pstLdeewEng->uiCurLen);
#endif
    }

    return iLen;
}
void LDEEW_GscFeed(LDEEW_ENGINE_S *pstLdeewEng,char * data, int iLen)
{
    GSC_Feed(pstLdeewEng->stLiteUDA.pstUDAEng, data, iLen);
}

/************************************************************
  Function   : LDEEW_lenPerSend()

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
LDEEW_API_ATTR int LDEEW_lenPerSend(LDEEW_ENGINE_S *pstLdeewEng)
{
    return GSC_LenPerSnd(NULL);
}

/************************************************************
  Function   : LDEEW_end()

  Description: Stop lite-fespd Engine
  Calls      :
  Called By  :
  Input      : pstLdeewEng: lite-fespd Engine
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
LDEEW_API_ATTR int LDEEW_end(LDEEW_ENGINE_S *pstLdeewEng)
{
    wakeup_Nchans_end(pstLdeewEng->stLiteWkp.pstNWkpEng);
    return 0;
}

/************************************************************
  Function   : LDEEW_delete()

  Description: Delete lite-fespd Engine
  Calls      :
  Called By  :
  Input      : pstLdeewEng: lite-fespd Engine
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
LDEEW_API_ATTR int LDEEW_delete(LDEEW_ENGINE_S *pstLdeewEng)
{
    if (pstLdeewEng)
    {
        /* release Lite-AEC engine */
        if (pstLdeewEng->stLiteAEC.pstAecEng)
        {
            aec_api_delete(pstLdeewEng->stLiteAEC.pstAecEng);
            pstLdeewEng->stLiteAEC.pstAecEng = NULL;
        }

        /* release Lite-UDA engine */
        if (pstLdeewEng->stLiteUDA.pstUDAEng)
        {
            GSC_Delete(pstLdeewEng->stLiteUDA.pstUDAEng);
            pstLdeewEng->stLiteUDA.pstUDAEng = NULL;
        }

        /* release Lite-Wakeup engine */
        if (pstLdeewEng->stLiteWkp.pstNWkpEng)
        {
            wakeup_Nchans_delete(pstLdeewEng->stLiteWkp.pstNWkpEng);
            pstLdeewEng->stLiteWkp.pstNWkpEng = NULL;
        }

#ifdef LDEEW_INTERNAL_MEM_MNG
        free(pstLdeewEng);
#endif
    }

    return 0;
}

/************************************************************
  Function   : LDEEW_aecCallbackRegister()

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
LDEEW_API_ATTR int LDEEW_aecCallbackRegister(LDEEW_ENGINE_S *pstLdeewEng, void *pvAecHandler, void *arg)
{
    int ret = -1;
    pstLdeewEng->stLiteAEC.arg = arg;

    if (NULL != pvAecHandler)
    {
        pstLdeewEng->stLiteAEC.pfuncAecHandler = (LDEEW_AEC_hook_t)pvAecHandler;
        ret = 0;
    }

    return ret;
}


/************************************************************
  Function       : LDEEW_AsrCallbackRegister

  Description    :
  Iutput         :
  Output         :
  Return         :
  Others         :

  History        :
  2019/12/20 11:08:42,  Chengfei.Song
************************************************************/
LDEEW_API_ATTR int LDEEW_AsrCallbackRegister(LDEEW_ENGINE_S *pstLdeewEng, void *pvAsrHandler, void *arg)
{
    int ret = -1;

    if (NULL != pvAsrHandler)
    {
        pstLdeewEng->stLiteUDA.pfAsrHandler = (LDEEW_ASR_hook_t)pvAsrHandler;
        pstLdeewEng->stLiteUDA.pvAsrData = arg;
        ret = 0;
    }

    return ret;
}


/************************************************************
  Function   : LDEEW_version()

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
LDEEW_API_ATTR char *LDEEW_version(void)
{
    return LDEEW_VER;
}
