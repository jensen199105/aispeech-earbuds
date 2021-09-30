#ifndef _SEVC_AEC_H_
#define _SEVC_AEC_H_

#include "sevc_types.h"

//default Q15

U32 SEVC_AEC_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
U32 SEVC_AEC_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
SEVC_AEC_S *SEVC_AEC_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator);
VOID SEVC_AEC_CallBackFuncRegister(SEVC_AEC_S *pstAecEng, VOID *pCBFunc, VOID *pUsrData);
VOID SEVC_AEC_Feed(SEVC_AEC_S *pstAecEng, aisp_cpx_s32_t *pstMicFrame, aisp_cpx_s32_t *pstRefFrame);
VOID SEVC_AEC_Reset(SEVC_AEC_S *pstAecEng);
VOID SEVC_AEC_Delete(SEVC_AEC_S *pstAecEng);

#endif

