#ifndef _SEVC_NN_H_
#define _SEVC_NN_H_
#include "sevc_types.h"

U32 SEVC_NN_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
U32 SEVC_NN_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
aisp_s32_t SEVC_NN_VadGet(SEVC_NN_S *pstNnEng);
S8 *SEVC_NN_ModelInfoGet(SEVC_NN_S *pstNnEng);
SEVC_NN_S *SEVC_NN_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator);
VOID SEVC_NN_CallBackFuncRegister(SEVC_NN_S *pstNnEng, VOID *pCBFunc, VOID *pUsrData);
S32 SEVC_NN_Feed(SEVC_NN_S *pstNnEng, aisp_cpx_s32_t *pFreqFrame, aisp_cpx_s32_t *pEstFrame,
                 U32 uiWindNoiseState, U32 uiQuietState);
VOID SEVC_NN_ErrPop(VOID *pstNnEng, U8 *pucErr);
VOID SEVC_NN_Reset(SEVC_NN_S *pstNnEng);
VOID SEVC_NN_Delete(SEVC_NN_S *pstNnEng);


#endif

