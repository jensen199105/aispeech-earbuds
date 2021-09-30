#ifndef __AEC_H__
#define __AEC_H__
 
#include "naes.h"
#include "AISP_TSL_types.h"
#include "AISP_TSL_base.h"
#include "AISP_TSL_complex.h"
#include "AISP_TSL_fft.h"

#define MIC_CHANNEL_NUM (2)
#define REF_CHANNEL_NUM (1)

typedef struct aec_api
{
    short  nmembSize;
    aec_coreConf_T *config;
    char *aMicChannelIndex;
    char *aRefChannelIndex;
    short *pMicBuffer[MIC_CHANNEL_NUM];
    short *pRefBuffer[REF_CHANNEL_NUM];
    aec_core_T **pAECInstance;
    void *memoryPool;
} aec_api_T;


#ifndef AEC_VISIBLE_ATTR
#define AEC_VISIBLE_ATTR    __attribute__ ((visibility ("default")))
#endif

#define AEC_POINTER_OF_POINTER(ptr) ((void *)&ptr)

#ifdef AISP_DEBUG
#define aec_debug(...)         do    \
                                {    \
                                    printf("\n\n****************** aec dbg in %s() %d *********************\n\n", __FUNCTION__, __LINE__);    \
                                    printf(__VA_ARGS__); \
                                    printf("\n\n******************    aec dbg End     *********************\n\n"); \
                                } while (0)
#else
#define aec_debug(...)
#endif

void aec_api_paramSet(aec_coreConf_T *config, short argc);

void aec_api_paramReset(aec_coreConf_T *config,aec_coreConfReset_T *newconfig);

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
void * aec_new(void *filename);

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
void aec_run(aec_core_T * pstAec);

/************************************************************
  Function   : aec_feed()

  Description: Feed data to AEC.
               The actual computation also take place here.
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
void aec_feed(aec_core_T * pstAec, const short* micFrame, const short* refFrame0,
                                                    const short* refFrame1, const short* refFrame2);

/************************************************************
  Function   : aec_feedSharedRef_float()

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
void aec_feedSharedRef(aec_core_T * pstAec, const short * micFrame, const aec_core_T * pRefSrcAec);

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
void aec_popTimeDomainPcm(aec_core_T * pstAec, short *pErr);

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
void aec_popOverlappedSpectrum(aec_core_T * pstAec, aisp_cpx_s32_t *pERR);

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
void aec_reset(aec_core_T * pstAec);

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
void aec_delete(aec_core_T * pstAec);

#endif
