#ifndef _SEVC_BF_H_
#define _SEVC_BF_H_
#include "sevc_types.h"

U32 SEVC_BF_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
U32 SEVC_BF_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
SEVC_BF_S *SEVC_BF_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator);
VOID SEVC_BF_CallBackFuncRegister(SEVC_BF_S *pstBfEng, VOID *pCBFunc, VOID *pUsrData);
S32 SEVC_BF_Feed(SEVC_BF_S *pstBfEng, aisp_cpx_s32_t *pFreqFrame);
S8 *SEVC_BF_ResourceInfoGet(SEVC_BF_S *pstBfEng);
VOID SEVC_BF_Reset(SEVC_BF_S *pstBfEng);
VOID SEVC_BF_Delete(SEVC_BF_S *pstBfEng);

#endif
