#ifndef _SEVC_STATE_H_
#define _SEVC_STATE_H_

#include "sevc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

U32 SEVC_SD_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
U32 SEVC_SD_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
SEVC_STATE_S *SEVC_SD_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator);
VOID SEVC_SD_Feed(SEVC_STATE_S *pstSevcStateEng, aisp_cpx_s32_t *pMic);
VOID SEVC_SD_CallBackFuncRegister(SEVC_STATE_S *pstSevcStateEng, VOID *pCBFunc, VOID *pUsrData);
VOID SEVC_SD_Reset(SEVC_STATE_S *pstSevcStateEng);
VOID SEVC_SD_Delete(SEVC_STATE_S *pstSevcStateEng);

#ifdef __cplusplus
}
#endif

#endif