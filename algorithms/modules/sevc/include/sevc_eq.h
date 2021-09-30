#ifndef _SEVC_EQ_H_
#define _SEVC_EQ_H_
#include "sevc_types.h"

U32 SEVC_EQ_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
U32 SEVC_EQ_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
SEVC_EQ_S *SEVC_EQ_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator);
VOID SEVC_EQ_Feed(SEVC_EQ_S *pstEqEng, aisp_cpx_s32_t *pFreqFrameInOut);
VOID SEVC_EQ_Reset(SEVC_EQ_S *pstEqEng);
VOID SEVC_EQ_Delete(SEVC_EQ_S *pstEqEng);

#endif

