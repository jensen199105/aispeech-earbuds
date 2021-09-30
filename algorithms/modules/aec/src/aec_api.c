/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : aec_api.c
  Project    :
  Module     :
  Version    :
  Date       : 2016/08/06
  Author     : ziyuan.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >         <desc>
  ziyuan.Jiang  2016/08/06         1.00              Create

************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "aec_api.h"
#include "aec.h"
#include "AISP_TSL_base.h"
#include "AISP_TSL_common.h"
#include "aisp_memmng.h"

static char aec_api_run(void *pstAecApi);
/************************************************************
  Function   : AECAPI_16bitTofloat()

  Description: transform 16bit data to float
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/06, ziyuan.Jiang create

************************************************************/
static inline aisp_s16_t AECAPI_16bitTofloat(const char *p, short Index)
{
    short tmp = 0;
    p = p + (Index << 1);
    tmp = *((short *)p);
    return (aisp_s16_t)tmp;
}

/************************************************************
  Function   : AECAPI_24bitTofloat()

  Description: transform 24bit data to float
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/06, ziyuan.Jiang create

************************************************************/
static inline aisp_s16_t AECAPI_24bitTofloat(const char *p, short Index)
{
    int I0 = 0;
    int I1 = 0;
    int I2 = 0;
    p = p + ((Index << 1) + Index);
    I0 = (unsigned char)p[0];
    I1 = (unsigned char)p[1];
    I1 <<= 8;
    I2 = p[2];
    I2 <<= 16;
    I0 = I0 | I1 | I2;
    return (aisp_s16_t)(I0);
}

aec_coreConf_T *sspe_aec_config_Init(void)
{
    aec_coreConf_T *pstCfg = NULL;
    pstCfg = (aec_coreConf_T *)AISP_MEM_MALLOC(sizeof(aec_coreConf_T));
    pstCfg->sil2spe        = 1;
    pstCfg->spe2sil        = 5;
    pstCfg->miu            = 32767;      /* AISP_FIXED16(1.0) */
    pstCfg->lembda         = 32112;      /* AISP_FIXED16(0.98) */
    pstCfg->deltamax       = 4915;       /* AISP_FIXED16(0.15) */
    pstCfg->Mu             = 13107;      /* AISP_FIXED16(0.4) */
    pstCfg->Noise_floor    = 1;          /* AISP_FIXED16(1.0/32767) */
    pstCfg->amth           = 15;
    pstCfg->upStep         = 5;          /* AISP_FIXED16(0.00016) */
    pstCfg->downStep       = 3;          /* AISP_FIXED16(9.92e-5) */
    pstCfg->lembda_mu      = 32439;      /* AISP_FIXED16(0.99) */
    pstCfg->preemph        = 29490;      /* AISP_FIXED16(0.90) */
    pstCfg->scaft          = 16384;      /* AISP_FIXED16(0.5) */
    pstCfg->lamda_x        = 29490;   /* AISP_FIXED16(0.9) */
    pstCfg->sgmS2thr       = 13;      /* AISP_FIXED16(0.0004) */
    pstCfg->lamda_xPower   = 9830;    /* AISP_FIXED16(0.3) */
    pstCfg->xCoherence     = 1;
    pstCfg->gainFlag       = 0;
    pstCfg->outGain        = 1;
    pstCfg->DcRemoveFlag   = 0;
    pstCfg->frameSize      = 512;
    pstCfg->wavChan        = MIC_CHANNEL_NUM + REF_CHANNEL_NUM;//---
    pstCfg->micNum         = MIC_CHANNEL_NUM;
    pstCfg->refNum         = REF_CHANNEL_NUM;
    pstCfg->fs             = 16000;
    pstCfg->hdr            = 0;
    pstCfg->vad            = 1;
    pstCfg->mute           = 0;
    pstCfg->nlp            = 1;//0402
    pstCfg->emd            = 1;
    pstCfg->mute_time_ms   = 300;
    pstCfg->constrain      = 1;
    pstCfg->freqOut        = 0;
    return pstCfg;
}

/************************************************************
  Function   : AISP_ALG_aec_unsharedMemSizeGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/06/05, niukun create

************************************************************/
S32 AISP_ALG_aec_unsharedMemSizeGet(aec_coreConf_T *pstAecCfg)
{
    S32 loop = 0;
    S32 subloop = 0;
    S32 totalMemSize = 0;
    S32 fftBins = pstAecCfg->frameSize + 1;
    S32 sizeFloatFftbins   = sizeof(aisp_s32_t) * fftBins;
    S32 sizeComplexFftbins = fftBins * sizeof(aisp_cpx_s32_t);
    S32 sizeFloatFrame  = pstAecCfg->frameSize * sizeof(aisp_s16_t);
    S32 sizeFloatWide      = (pstAecCfg->frameSize << 1) * sizeof(aisp_s16_t);

    for (loop = 0; loop < pstAecCfg->micNum; loop++)
    {
        //pPs_old
        totalMemSize += sizeFloatFftbins;
        totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
        //PPyy
        totalMemSize += sizeFloatFftbins;
        totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
        //PPee
        totalMemSize += sizeFloatFftbins;
        totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
        //PPey
        totalMemSize += sizeComplexFftbins;
        totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
        //aERRPower
        totalMemSize += sizeFloatFftbins;
        totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);

        //aW
        for (subloop = 0; subloop < AEC_TAPS; subloop++)
        {
            totalMemSize += sizeComplexFftbins * pstAecCfg->refNum;
            totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
        }

        //aMicFrameOld
        totalMemSize += sizeFloatFrame;
        totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
        //aERR
        totalMemSize += sizeComplexFftbins;
        totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
        //aMicFrameWide
        totalMemSize += sizeFloatWide;
        totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
        //aErrPreOld
        totalMemSize += sizeFloatFrame;
        totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
        //aEstOld
        totalMemSize += sizeFloatFrame;
        totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
        //aErrOld
        totalMemSize += sizeFloatFrame;
        totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
    }

    return (totalMemSize + 8);
}

/************************************************************
  Function   : AISP_ALG_aec_hybridsharedMemSizeGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/06/05, niukun create
************************************************************/
S32 AISP_ALG_aec_hybridSharedMemSizeGet(aec_coreConf_T *pstAecCfg)
{
    S32 totalMemSize = 0;
    S32 fftBins = pstAecCfg->frameSize + 1;
    S32 sizeFloatFftbins   = sizeof(aisp_s32_t) * fftBins;
    S32 sizeComplexFftbins = fftBins * sizeof(aisp_cpx_s32_t);
    S32 sizeFloatFrame  = pstAecCfg->frameSize * sizeof(aisp_s16_t);
    int loop = 0;
    //aR_Qbase
    totalMemSize += fftBins * sizeof(unsigned char);
    totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
    // //aDeltafloor
    // totalMemSize += sizeFloatFftbins;
    // totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
    //aR12
    totalMemSize += sizeComplexFftbins;
    totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
    //aR22
    totalMemSize += sizeFloatFftbins;
    totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
    //aR11
    totalMemSize += sizeFloatFftbins;
    totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);

    //aX
    for (loop = 0; loop < AEC_TAPS; loop++)
    {
        totalMemSize += sizeComplexFftbins * pstAecCfg->refNum;
        totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
    }

    //aXPower
    totalMemSize += sizeFloatFftbins * pstAecCfg->refNum;
    totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
    //aXPowerSum
    totalMemSize += sizeFloatFftbins;
    totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
    //aXPowerSumSmooth
    totalMemSize += sizeFloatFftbins;
    totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);

    //aRefFrameOld
    for (loop = 0; loop < pstAecCfg->refNum; loop++)
    {
        totalMemSize += sizeFloatFrame;
        totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
    }

    return (totalMemSize + 8);
}

/************************************************************
  Function   : AISP_ALG_aec_sharedMemSizeGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/06/05, niukun create

************************************************************/
S32 AISP_ALG_aec_sharedMemSizeGet(aec_coreConf_T *pstAecCfg)
{
    S32 totalMemSize = 0;
    S32 fftBins = pstAecCfg->frameSize + 1;
    S32 sizeFloatFftbins   = sizeof(aisp_s32_t) * fftBins;
    S32 sizeComplexWide = sizeof(aisp_cpx_s32_t) * (pstAecCfg->frameSize << 1);
    // S32 sizeFloatFrame  = pstAecCfg->frameSize * sizeof(aisp_s16_t);
    S32 sizeFloatWide      = (pstAecCfg->frameSize << 1) * sizeof(aisp_s16_t);
    S32 sizeComplexFftbins = fftBins * sizeof(aisp_cpx_s32_t);
    //aMicFrameWide
    totalMemSize += sizeFloatWide;
    totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
    //aEstFrameWide
    totalMemSize += sizeFloatWide;
    totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
    //aErrFrameWide
    totalMemSize += sizeFloatWide;
    totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
    //aExtdW0
    totalMemSize += sizeComplexWide * pstAecCfg->refNum;
    totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
    //aExtdw
    totalMemSize += sizeFloatWide;
    totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
    //aIR12
    totalMemSize += sizeComplexFftbins;
    totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
    //aIR22
    totalMemSize += sizeFloatFftbins;
    totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
    //aIR11
    totalMemSize += sizeFloatFftbins;
    totalMemSize = AISP_ALIGN_SIZE(totalMemSize, 8);
    return (totalMemSize + 8);
}

/************************************************************
  Function   : aec_api_new()

  Description: new aec_api engine
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/06, ziyuan.Jiang create

************************************************************/
AEC_VISIBLE_ATTR void *aec_api_new(const char *fileName)
{
    int i;
    //unsigned int memoryPoolSize;
    //AISP_TSL_memAllocator_T * pstAllocator = NULL;
    aec_api_T *pstAecApi = NULL;
    aec_coreConf_T *pstAecCfg;
    /* alloc */
    pstAecApi = (aec_api_T *)AISP_MEM_CALLOC(1, sizeof(aec_api_T));

    //memset(pstAecApi,0, sizeof(aec_api_T));
    if (NULL == pstAecApi)
    {
#ifdef AISP_TSL_INFO
        printf("(): Alloc aec api fail.\n");
#endif
        goto FAIL;
    }

    pstAecApi->config = sspe_aec_config_Init();
    pstAecCfg = pstAecApi->config;

    if (pstAecApi->config->hdr)
    {
        pstAecApi->nmembSize = pstAecApi->config->wavChan * 3;
    }
    else
    {
        pstAecApi->nmembSize = pstAecApi->config->wavChan * 2;
    }

    pstAecApi->pAECInstance = (aec_core_T **)AISP_MEM_CALLOC(pstAecCfg->micNum * sizeof(aec_core_T *), 1);

    for (i = 0; i < pstAecCfg->micNum; ++i)
    {
        pstAecApi->pMicBuffer[i] = (short *)AISP_MEM_CALLOC(sizeof(short) * pstAecCfg->frameSize, 1);
    }

    for (i = 0; i < pstAecCfg->refNum; ++i)
    {
        pstAecApi->pRefBuffer[i] = (short *)AISP_MEM_CALLOC(sizeof(short) * pstAecCfg->frameSize, 1);
    }

    pstAecApi->aMicChannelIndex = (char *)AISP_MEM_CALLOC(pstAecCfg->micNum, 1);
    pstAecApi->aRefChannelIndex = (char *)AISP_MEM_CALLOC(pstAecCfg->refNum, 1);

    for (i = 0; i < pstAecCfg->micNum; ++i)
    {
        pstAecCfg->index = i;
        /* initialze aec instance */
        pstAecApi->pAECInstance[i] = aec_new(pstAecCfg);
        /* initialze MicChannelIndex */
        pstAecApi->aMicChannelIndex[i] = i;
    }

    for (i = 0; i < pstAecCfg->refNum; ++i)
    {
        /* initialze MicChannelIndex */
        pstAecApi->aRefChannelIndex[i] = pstAecCfg->wavChan - 1 - i;
    }

    return pstAecApi;
FAIL:

    if (pstAecApi)
    {
        AISP_MEM_FREE(pstAecApi);
    }

    return NULL;
}

/************************************************************
  Function   : aec_api_frameSize()

  Description: return frame size
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/06, ziyuan.Jiang create

************************************************************/
AEC_VISIBLE_ATTR int aec_api_frameSize(void *pstAecApi)
{
    aec_api_T *pstAecEng = (aec_api_T *)pstAecApi;
    return pstAecEng->config->frameSize;
}

/************************************************************
  Function   : aec_api_frameSize()

  Description: return frame size
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/06, ziyuan.Jiang create

************************************************************/
AEC_VISIBLE_ATTR short aec_api_wavChan(void *pstAecApi)
{
    aec_api_T *pstAecEng = (aec_api_T *)pstAecApi;
    return pstAecEng->config->wavChan;
}
/************************************************************
  Function   : aec_api_reset()

  Description: reset aec engine by channels
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/06, ziyuan.Jiang create

************************************************************/
AEC_VISIBLE_ATTR void aec_api_reset(void *pstAecApi)
{
    aec_api_T *pstAecEng = (aec_api_T *)pstAecApi;
    int i = 0;

    for (i = 0; i < pstAecEng->config->micNum; ++i)
    {
        aec_reset(pstAecEng->pAECInstance[i]);
    }
}

/************************************************************
  Function   : aec_api_delete()

  Description: delete aec engine and ring buffer by channels
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/06, ziyuan.Jiang create

************************************************************/
AEC_VISIBLE_ATTR void aec_api_delete(void *pstAecApi)
{
    int i = 0;
    aec_api_T *pstAecEng = (aec_api_T *)pstAecApi;
    aec_coreConf_T *pstAecCfg = pstAecEng->config;
    AISP_MEM_FREE(pstAecEng->aMicChannelIndex);
    AISP_MEM_FREE(pstAecEng->aRefChannelIndex);

    for (i = 0; i < pstAecEng->config->micNum; ++i)
    {
        aec_delete(pstAecEng->pAECInstance[i]);
    }

    for (i = 0; i < pstAecCfg->micNum; ++i)
    {
        AISP_MEM_FREE(pstAecEng->pMicBuffer[i]);
    }

    for (i = 0; i < pstAecCfg->refNum; ++i)
    {
        AISP_MEM_FREE(pstAecEng->pRefBuffer[i]);
    }

    AISP_MEM_FREE(pstAecEng->pAECInstance);
    AISP_MEM_FREE(pstAecEng->config);
    AISP_MEM_FREE(pstAecEng);
}

#define AEC_VERSION  ("v1.0.2")
/************************************************************
  Function   : aec_api_version()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/12/19, Youhai.Jiang create

************************************************************/
AEC_VISIBLE_ATTR char *aec_api_version(void)
{
    return AEC_VERSION;
}

/************************************************************
  Function   : aec_api_memSize()

  Description: return mem size for one point * all channels
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/06, ziyuan.Jiang create

************************************************************/
AEC_VISIBLE_ATTR short aec_api_memSize(void *pstAecApi)
{
    aec_api_T *pstAecEng = (aec_api_T *)pstAecApi;
    return pstAecEng->nmembSize;
}

/************************************************************
  Function   : aec_api_micNum()

  Description: return number of microphone
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/06, ziyuan.Jiang create

************************************************************/
AEC_VISIBLE_ATTR short aec_api_micNum(void *pstAecApi)
{
    aec_api_T *pstAecEng = (aec_api_T *)pstAecApi;
    return pstAecEng->config->micNum;
}

/************************************************************
  Function   : aec_api_freqOut()

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
AEC_VISIBLE_ATTR short aec_api_freqOut(void *pstAecApi)
{
    aec_api_T *pstAecEng = (aec_api_T *)pstAecApi;
    return pstAecEng->config->freqOut;
}

/************************************************************
  Function   : aec_api_feed()

  Description: feed data to aec engine by pcm style
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/06, ziyuan.Jiang create

************************************************************/
AEC_VISIBLE_ATTR char aec_api_feed(void *pstAecApi, unsigned char *pPcm, short nmemb)
{
    aec_api_T *pstAecEng = (aec_api_T *)pstAecApi;
    int i                = 0;
    int j                = 0;
    short nmembSize      = pstAecEng->nmembSize;
    char micNum          = pstAecEng->config->micNum;
    char refNum          = pstAecEng->config->refNum;
    const char *pLine   = (const char *)pPcm;
    short writeCount     = 0;

    if (!pstAecEng->config->hdr)
    {
        for (i = 0; i < nmemb; ++i)
        {
            for (j = 0; j < micNum; ++j)
            {
                pstAecEng->pMicBuffer[j][writeCount] =  AECAPI_16bitTofloat(pLine, pstAecEng->aMicChannelIndex[j]);
            }

            for (j = 0; j < refNum; ++j)
            {
                pstAecEng->pRefBuffer[j][writeCount] =  AECAPI_16bitTofloat(pLine, pstAecEng->aRefChannelIndex[j]);
            }

            writeCount++;
            pLine += nmembSize;
        }
    }
    else
    {
        for (i = 0; i < nmemb; ++i)
        {
            for (j = 0; j < micNum; ++j)
            {
                pstAecEng->pMicBuffer[j][writeCount] = AECAPI_24bitTofloat(pLine, pstAecEng->aMicChannelIndex[j]);
            }

            for (j = 0; j < refNum; ++j)
            {
                pstAecEng->pRefBuffer[j][writeCount] = AECAPI_24bitTofloat(pLine, pstAecEng->aRefChannelIndex[j]);
            }

            writeCount++;
            pLine += nmembSize;
        }
    }

    aec_api_run(pstAecEng);
    return writeCount;
}

/************************************************************
  Function   : aec_api_run()

  Description: run aec
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/06, ziyuan.Jiang create

************************************************************/
static char aec_api_run(void *pstAecApi)
{
    aec_api_T *pstAecEng = (aec_api_T *)pstAecApi;
    /*
     * reset maxabsRef
     */
    pstAecEng->pAECInstance[0]->maxabsRef = 0;
    pstAecEng->pAECInstance[0]->memX = pstAecEng->pAECInstance[0]->memXold[0];
    pstAecEng->pAECInstance[0]->memD[0] =  pstAecEng->pAECInstance[0]->memDold[0];
    pstAecEng->pAECInstance[0]->memD[1] =  pstAecEng->pAECInstance[0]->memDold[1];
    /* feed data for the first aec instance
     * aec_feed_float will only use refNum number of pRefFrame
     * so it is okay to feed unused frame pointer with initialzed value
     */
#if REF_CHANNEL_NUM==1
    aec_feed(pstAecEng->pAECInstance[0], pstAecEng->pMicBuffer[0], pstAecEng->pRefBuffer[0], NULL, NULL);
#elif REF_CHANNEL_NUM==2
    aec_feed(pstAecEng->pAECInstance[0], pstAecEng->pMicBuffer[0], pstAecEng->pRefBuffer[0], pstAecEng->pRefBuffer[1],
             NULL);
#else
    aec_feed(pstAecEng->pAECInstance[0], pstAecEng->pMicBuffer[0], pstAecEng->pRefBuffer[0], pstAecEng->pRefBuffer[1],
             pstAecEng->pRefBuffer[2]);
#endif
    pstAecEng->pAECInstance[0]->memXold[0] = pstAecEng->pAECInstance[0]->memX;
    pstAecEng->pAECInstance[0]->memDold[0] = pstAecEng->pAECInstance[0]->memD[0];
    pstAecEng->pAECInstance[0]->memDold[1] = pstAecEng->pAECInstance[0]->memD[1];
    aec_run(pstAecEng->pAECInstance[0]);
#if MIC_CHANNEL_NUM > 1
    int  i      = 0;
    char micNum = pstAecEng->config->micNum;

    for (i = 1; i < micNum; ++i)
    {
        /*
         * inherit memX and memD of last frame for each mic
         */
        pstAecEng->pAECInstance[i]->memX = pstAecEng->pAECInstance[i]->memXold[i];
        /*
         * maxabsRef used ref 1
         */
        pstAecEng->pAECInstance[i]->maxabsRef = pstAecEng->pAECInstance[0]->maxabsRef;
        aec_feedSharedRef(pstAecEng->pAECInstance[i], pstAecEng->pMicBuffer[i], pstAecEng->pAECInstance[0]);
        /*
         * memX will be stored to used in the next frame
         */
        pstAecEng->pAECInstance[i]->memXold[i] = pstAecEng->pAECInstance[i]->memX;
        aec_run(pstAecEng->pAECInstance[i]);
    }

#endif
    return 0;
}

/************************************************************
  Function   : aec_api_pop()

  Description: output time-zone data by channel
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/06, ziyuan.Jiang create

************************************************************/
AEC_VISIBLE_ATTR void aec_api_pop(void *pstAecApi, short *pErr, short index)
{
    aec_api_T *pstAecEng = (aec_api_T *)pstAecApi;
    pstAecEng->pAECInstance[index]->memE = pstAecEng->pAECInstance[index]->memEold[index];
    aec_popTimeDomainPcm(pstAecEng->pAECInstance[index], pErr);
    pstAecEng->pAECInstance[index]->memEold[index] = pstAecEng->pAECInstance[index]->memE;
}

/************************************************************
  Function   : aec_api_popSpectrum()

  Description: output frequency-zone data by channel
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/06, ziyuan.Jiang create

************************************************************/
AEC_VISIBLE_ATTR void aec_api_popSpectrum(aec_api_T *pstAecApi, aisp_cpx_s32_t *pERR, short index)
{
    aec_popOverlappedSpectrum(pstAecApi->pAECInstance[index], pERR);
}


/************************************************************
  Function   : aec_api_pesudo_pop()

  Description: main aec process
  Calls      :
  Called By  :
  Input      : pstAecApi : aec_api engine
  Output     :
  Return     :
  Others     :

  History    :
    2019/03/26, changren.tan create
************************************************************/

AEC_VISIBLE_ATTR char aec_api_pesudo_pop(void *pstAecApi, void *data, short *pErr, short index)
{
    //aec_api_T *pstAecEng = (aec_api_T *)pstAecApi;
    int ii     = 0;
    int frmSize = 512;
    int wavChan = MIC_CHANNEL_NUM + REF_CHANNEL_NUM;
    short *pMicData = NULL;
    pMicData = (short *)data + index;

    for (ii = 0; ii < frmSize; ii++)
    {
        pErr[ii] = pMicData[wavChan * ii];
    }

    return 0;
}


/************************************************************
  Function   : aec_api_hdr()

  Description: output max volume state
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/04/02, kun.niu create
************************************************************/
short aec_api_hdr(aec_coreConf_T *pstAecCfg)
{
    return pstAecCfg->hdr;
}

/************************************************************
  Function   : aec_api_getMaxVolumeState()

  Description: output max volume state
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/04/02, kun.niu create
************************************************************/
AEC_VISIBLE_ATTR short aec_getMaxVolumeState(aec_core_T *pstAec)
{
    return pstAec->maxVolumeState;
}


/************************************************************
  Function   : aec_api_getMaxVolumeState()

  Description: output max volume state
  Calls      :
  Called By  :
  Input      : pstAecApi : aec_api_engine
               index        : channel index
  Output     :
  Return     :
  Others     :

  History    :
    2018/04/02, kun.niu create
************************************************************/
AEC_VISIBLE_ATTR short aec_api_getMaxVolumeState(aec_api_T *pstAecApi, short index)
{
    return aec_getMaxVolumeState(pstAecApi->pAECInstance[index]);
}

/************************************************************
  Function   : aec_api_usMaxVolumeDetect()

  Description: if use max volume detect
  Calls      :
  Called By  :
  Input      : pstAecApi : aec_api_engine
  Output     :
  Return     :
  Others     :

  History    :
    2018/04/02, kun.niu create
************************************************************/
AEC_VISIBLE_ATTR short aec_api_usMaxVolumeDetect(aec_coreConf_T *pstAecCfg)
{
    return pstAecCfg->max_volume_detect;
}


