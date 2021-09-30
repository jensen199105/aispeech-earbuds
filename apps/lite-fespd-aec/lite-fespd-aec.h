/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : lite-fespd-aec.h
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
#ifndef __LITE_FESPD_AEC_H__
#define __LITE_FESPD_AEC_H__
#include "AISP_TSL_types.h"

typedef struct LDEEW_ENGINE LDEEW_ENGINE_S;
typedef void (*LDEEW_ASR_hook_t)(void *pvUsrData, S8 *pcData, S32 iLen,S8 iStatus);
typedef void (*LDEEW_AEC_hook_t)(void *pvUsrData, S8 *pcData, S32 iLen);

/************************************************************
  Function   : LDEEW_BF_hook_t()

  Description: Output bf data 
  Calls      :
  Called By  :
  Input      : pvUsrData : private usr parameter
  Output     : iChanIdx  : indicates which channel
               pcData    : indicates data buffer 
               pcJson    : indicates length of data
  Return     :
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
typedef void (*LDEEW_BF_hook_t)(void *pvUsrData, int iChanIdx, char *pcData, int iLen);

/************************************************************
  Function   : LDEEW_WKP_hook_t()

  Description: Output wakeup info
  Calls      :
  Called By  :
  Input      : pvUsrData : private usr parameter
  Output     : iChanIdx  : indicates which channel
               pcJson    : indicates wakeup status
  Return     :
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
typedef void (*LDEEW_WKP_hook_t)(void *pvUsrData, int iChanIdx, char *pcJson);

/************************************************************
  Function   : LDEEW_VAD_hook_t()

  Description: Output VAD status
  Calls      :
  Called By  :
  Input      : pvUsrData : private usr parameter
  Output     : iChanIdx  : indicates which channel
               iFrmStatus: indicates status
               iFrmIndex : indicates which frame
  Return     :
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
typedef void (*LDEEW_VAD_hook_t)(void *pvUsrData, int iChanIdx, int iFrmStatus, int iFrmIdx);

int LDEEW_memSize(void);
LDEEW_ENGINE_S *LDEEW_new(void *pvMemBase, int len);
int LDEEW_start(LDEEW_ENGINE_S *pstLfespdEng, void *pvBfHandler, 
                void *pvBfData, void *pvVadHandler, void *pvWkpHandler, void *pvWkpEnv);
int LDEEW_feed(LDEEW_ENGINE_S *pstLfespdEng, char *pcData, int iLen);
int LDEEW_end(LDEEW_ENGINE_S *pstLfespdEng);
char *LDEEW_version(void);
int LDEEW_lenPerSend(LDEEW_ENGINE_S *pstLfespdEng);
int LDEEW_delete(LDEEW_ENGINE_S *pstLfespdEng);
int LDEEW_aecCallbackRegister(LDEEW_ENGINE_S *pstLdeewEng, void *pvAecHandler, void *arg);
int LDEEW_AsrCallbackRegister(LDEEW_ENGINE_S *pstLdeewEng, void *pvAsrHandler, void *arg);
void LDEEW_GscFeed(LDEEW_ENGINE_S *pstLdeewEng,char * data, int iLen);

#endif
