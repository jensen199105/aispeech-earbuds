#ifndef _SEVC_GSC_H_
#define _SEVC_GSC_H_
#include "sevc_types.h"

U32 SEVC_GSC_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
U32 SEVC_GSC_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
SEVC_GSC_S *SEVC_GSC_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator);
VOID SEVC_GSC_CallBackFuncRegister(SEVC_GSC_S *pstGscEng, VOID *pCBFunc, VOID *pUsrData);
S32 SEVC_GSC_Feed(SEVC_GSC_S *pstGscEng, aisp_cpx_s32_t *pFreqFrame,
                  aisp_cpx_s32_t *pNoisyBlock, aisp_s32_t *pfSpp);
S8 *SEVC_GSC_ResourceInfoGet(SEVC_GSC_S *pstGscEng);
VOID SEVC_GSC_Reset(SEVC_GSC_S *pstGscEng);
VOID SEVC_GSC_Delete(SEVC_GSC_S *pstGscEng);

#endif

