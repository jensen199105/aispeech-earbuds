#ifndef _SEVC_AUX_H_
#define _SEVC_AUX_H_

#include "sevc_types.h"

U32 SEVC_AUX_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
U32 SEVC_AUX_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
SEVC_AUX_S *SEVC_AUX_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator);
VOID SEVC_AUX_Feed(SEVC_AUX_S *pstAuxEng, aisp_cpx_s32_t *pMicFreq, aisp_s16_t *pfAuxFrame);
VOID SEVC_AUX_Reset(SEVC_AUX_S *pstAuxEng);
VOID SEVC_AUX_Delete(SEVC_AUX_S *pstAuxEng);

#endif


