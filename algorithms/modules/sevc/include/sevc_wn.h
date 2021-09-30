#ifndef _SEVC_WN_H_
#define _SEVC_WN_H_

#include "sevc_types.h"

U32 SEVC_WN_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
U32 SEVC_WN_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
SEVC_WN_S *SEVC_WN_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator);
VOID SEVC_WN_Feed(SEVC_WN_S *pstWnEng, aisp_s16_t *pfMicFrame);
VOID SEVC_WN_Reset(SEVC_WN_S *pstWnEng);
VOID SEVC_WN_Delete(SEVC_WN_S *pstWnEng);

#endif

