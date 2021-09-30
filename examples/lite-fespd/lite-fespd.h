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
#ifndef __LFESPD_H__
#define __LFESPD_H__

typedef struct LFESPD_ENGINE LFESPD_ENGINE_S;

/************************************************************
  Function   : LFESPD_BF_hook_t()

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
typedef void (*LFESPD_BF_hook_t)(void *pvUsrData, int iChanIdx, char *pcData, int iLen);

/************************************************************
  Function   : LFESPD_WKP_hook_t()

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
typedef void (*LFESPD_WKP_hook_t)(void *pvUsrData, int iChanIdx, char *pcJson);

/************************************************************
  Function   : LFESPD_VAD_hook_t()

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
typedef void (*LFESPD_VAD_hook_t)(void *pvUsrData, int iChanIdx, int iFrmStatus, int iFrmIdx);

char *LFESPD_version(void);
LFESPD_ENGINE_S *LFESPD_new(void *pvMemBase, int len);
void  LFESPD_vadRegister(LFESPD_ENGINE_S *pstLfespdEng, void *pvVadHandler, void *pvVadData);
int   LFESPD_memSize(void);
int   LFESPD_start(LFESPD_ENGINE_S *pstLfespdEng, void *pvBfHandler, 
                   void *pvBfData, void *pvVadHandler, void *pvWkpHandler, void *pvWkpEnv);
int   LFESPD_feed(LFESPD_ENGINE_S *pstLfespdEng, char *pcData, int iLen);
int   LFESPD_end(LFESPD_ENGINE_S *pstLfespdEng);
int   LFESPD_lenPerSend(LFESPD_ENGINE_S *pstLfespdEng);
int   LFESPD_delete(LFESPD_ENGINE_S *pstLfespdEng);

#endif
