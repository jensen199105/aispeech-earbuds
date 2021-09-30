/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : lite-wakeup.c
  Project    :
  Module     :
  Version    :
  Date       : 2019/10/10
  Author     : Chao.Xu
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Chao.Xu       2019/10/10         1.00              Create

************************************************************/
#ifndef __LITEUDA_H__
#define __LITEUDA_H__

typedef struct tagGSC_CoreCfg GSC_CoreCfg_S;
typedef struct tagGSC_Api     GSC_Api_S;

typedef void (*GSC_FUNCHOOK_T)(void *, int, char *, int);

int LUDA_CfgInit(GSC_CoreCfg_S *pstGscCfg, const char *pcBinFn);
int LUDA_CfgMemSizeGet(void);
int LUDA_SharedMemSizeGet(GSC_CoreCfg_S *pstGscCfg);
int LUDA_UnsharedMemSizeGet(GSC_CoreCfg_S *pstGscCfg);
void *LUDA_New(GSC_CoreCfg_S *pstGscCfg, char *pcSharedBuf, char *pcUnsharedBuf, GSC_FUNCHOOK_T pGscCallback,
               void *pvUsrData);
int LUDA_LenPerSnd(GSC_Api_S *pstGscApi);
int LUDA_Feed(GSC_Api_S *pstGscApi, char *pcData, int iLen);
int LUDA_FeedParallel(GSC_Api_S *pstGscApi, char *pcChan1, char *pcChan2, int iLen);
void LUDA_Reset(GSC_Api_S *pstGscApi);
void LUDA_CfgDelete(GSC_CoreCfg_S *pstGscCfg);
void LUDA_Delete(GSC_Api_S *pstGscApi);
char *LUDA_Version(void);
#ifdef ENABLE_LUDA_FOR_HEADSET
int LUDA_WindCbRegister(GSC_Api_S *pstGscApi, void *pvHandler, void *pvData);
#endif

#endif

