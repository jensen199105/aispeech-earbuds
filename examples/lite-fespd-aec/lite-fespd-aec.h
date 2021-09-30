/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : lfespd.h
  Project    : 
  Module     : 
  Version    : 
  Date       : 2019/04/16
  Author     : Youhai.Jiang
  Document   : 
  Description: 

  Function List :

  History :
  <author>      <time>            <version >        <desc> 
  Youhai.Jiang  2019/04/16         1.00              Create

************************************************************/
#ifndef __LITE_FESPD_AEC_H__
#define __LITE_FESPD_AEC_H__

typedef struct LDEEW_ENGINE LDEEW_ENGINE_S;



/************************************************************
  Function       : LDEEW_ASR_hook_t
 
  Description    : output data for asr
  Input          : pvUsrData : private usr parameter
  Output         : pcData    : indicates data buffer
                 : iLen      : indicates length of data
                 : iStatus   : wakeup  status 
                            0: no wakeup
                            1: wakeup 
  Return         :
  Others         :
 
  History        : 
  2019/12/20 10:59:37,  Chengfei.Song
************************************************************/
typedef void (*LDEEW_ASR_hook_t)(void *pvUsrData, char *pcData, int iLen,char iStatus);

/************************************************************
  Function   : LDEEW_AEC_hook_t()

  Description: Output bf data
  Calls      :
  Called By  :
  Input      : pvUsrData : private usr parameter
  Output     : pcData    : indicates data buffer
               iLen      : indicates length of data
  Return     :
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
typedef void (*LDEEW_AEC_hook_t)(void *pvUsrData, char *pcData, int iLen);

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

char *LDEEW_version(void);
LDEEW_ENGINE_S *LDEEW_new(void *pvMemBase, int len);
int   LDEEW_memSize(void);
int   LDEEW_start(LDEEW_ENGINE_S *pstLfespdEng, void *pvBfHandler, 
                   void *pvBfData, void *pvVadHandler, void *pvWkpHandler, void *pvWkpEnv);
int   LDEEW_feed(LDEEW_ENGINE_S *pstLfespdEng, char *pcData, int iLen);
int   LDEEW_end(LDEEW_ENGINE_S *pstLfespdEng);
int   LDEEW_lenPerSend(LDEEW_ENGINE_S *pstLfespdEng);
int   LDEEW_delete(LDEEW_ENGINE_S *pstLfespdEng);
int   LDEEW_aecCallbackRegister(LDEEW_ENGINE_S *pstLdeewEng, void *pvAecHandler, void *arg);
int   LDEEW_AsrCallbackRegister(LDEEW_ENGINE_S *pstLdeewEng, void *pvAsrHandler, void *arg);
void  LDEEW_GscFeed(LDEEW_ENGINE_S *pstLdeewEng,char * data, int iLen);

#endif
