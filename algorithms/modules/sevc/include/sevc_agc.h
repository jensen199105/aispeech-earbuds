#ifndef _SEVC_AGC_H_
#define _SEVC_AGC_H_
#include "sevc_types.h"

#ifdef AISPEECH_FIXED
#define AGC_ONE       16777216
#else
#define AGC_ONE       1
#endif
U32 SEVC_AGC_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
U32 SEVC_AGC_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
SEVC_AGC_S *SEVC_AGC_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator);
VOID SEVC_AGC_CallBackFuncRegister(SEVC_AGC_S *pstAgcEng, VOID *pCBFunc, VOID *pUsrData);
S32 SEVC_AGC_Feed(SEVC_AGC_S *pstAgcEng, aisp_s16_t *pFrame, aisp_s32_t fVad);
VOID SEVC_AGC_ErrPop(VOID *pstAgcEng, U8 *pucErr);
VOID SEVC_AGC_Reset(SEVC_AGC_S *pstAgcEng);
VOID SEVC_AGC_Delete(SEVC_AGC_S *pstAgcEng);


#endif

