#ifndef _SEVC_NR_H_
#define _SEVC_NR_H_

#include "sevc_types.h"

U32 SEVC_NR_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
U32 SEVC_NR_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
VOID SEVC_NR_CallBackFuncRegister(SEVC_NR_S *pstNrEng,
                                  VOID *pCBFunc, VOID *pUsrData);
SEVC_NR_S *SEVC_NR_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator);
VOID SEVC_NR_Feed(SEVC_NR_S *pstNrEng, aisp_cpx_s32_t *pMicFreq, aisp_s32_t *pfGain);
VOID SEVC_NR_Reset(SEVC_NR_S *pstNrEng);
VOID SEVC_NR_Delete(SEVC_NR_S *pstNrEng);

#endif


