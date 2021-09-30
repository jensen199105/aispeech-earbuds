/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : gsc_api.h
  Project    :
  Module     :
  Version    :
  Date       : 2019/04/28
  Author     : Chao.Xu
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Chao.Xu       2019/04/28        1.00              Create

************************************************************/
#ifndef __GSC_API_H__
#define __GSC_API_H__

#include "gsc_core_types.h"

/* Export function APIs */
S32   GSC_CfgInit(GSC_CoreCfg_S *pstGscCfg, const char *pcBinFn);
S32   GSC_CfgMemSizeGet(VOID);
S32   GSC_SharedMemSizeGet(GSC_CoreCfg_S *pstGscCfg);
S32   GSC_UnsharedMemSizeGet(GSC_CoreCfg_S *pstGscCfg);
VOID *GSC_New(GSC_CoreCfg_S *pstGscCfg, S8 *pcSharedBuf, S8 *pcUnsharedBuf, GSC_FUNCHOOK_T pGscCallback,
              void *pvUsrData);
S32   GSC_Start(GSC_Api_S *pstGscApi, void *pvHandler, void *pvData);
S32   GSC_LenPerSnd(GSC_Api_S *pstGscApi);
S32   GSC_Feed(GSC_Api_S *pstGscApi, S8 *pcData, S32 iLen);
S32   GSC_FeedParallel(GSC_Api_S *pstGscApi, S8 *pcChan1, S8 *pcChan2, S32 iLen);
VOID  GSC_Reset(GSC_Api_S *pstGscApi);
VOID  GSC_CfgDelete(GSC_CoreCfg_S *pstGscCfg);
VOID  GSC_Delete(GSC_Api_S *pstGscApi);
S8   *GSC_Version(VOID);

#endif

