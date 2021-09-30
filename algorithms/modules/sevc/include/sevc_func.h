#ifndef __SEVC_FUNC_H__
#define __SEVC_FUNC_H__

#include "sevc_types.h"

S8 *SEVC_ModelInfoGet(SEVC_S *pstSevc);
U32 SEVC_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
U32 SEVC_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
SEVC_S *SEVC_New(SEVC_CONFIG_S *pstSevcConfig, MEM_LOCATOR_S *pstMemLocator);
VOID SEVC_CbFuncRegister(SEVC_S *pstSevc, VOID *pCBFunc, VOID *pUsrData);
VOID SEVC_Feed(SEVC_S *pstSevcEng, aisp_s16_t *pfFrame);
VOID SEVC_Reset(SEVC_S *pstSevcEng);
VOID SEVC_Delete(SEVC_S *pstSevcEng);
#ifdef SEVC_MULTI_CORE
VOID SEVC_MiddleCbFuncRegister(SEVC_S *pstSevc, VOID *pCBFunc, VOID *pUsrData);
VOID SEVC_MiddleFreqBinsFeed(SEVC_S *pstSevcEng, U8 *pucBuff, U32 uiSize);

#endif

#endif

