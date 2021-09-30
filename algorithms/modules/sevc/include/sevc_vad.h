#ifndef _SEVC_VAD_H_
#define _SEVC_VAD_H_

#include "sevc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

U32 SEVC_VAD_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
U32 SEVC_VAD_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
SEVC_VAD_S *SEVC_VAD_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator);
S32 SEVC_VAD_Feed(SEVC_VAD_S *pstSevcVadEng, aisp_s16_t *pRefFrame);
VOID SEVC_VAD_Reset(SEVC_VAD_S *pstSevcVadEng);
VOID SEVC_VAD_Delete(SEVC_VAD_S *pstSevcVadEng);


#ifdef __cplusplus
}
#endif

#endif