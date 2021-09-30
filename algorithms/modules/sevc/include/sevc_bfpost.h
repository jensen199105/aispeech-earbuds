#ifndef _SEVC_BFPOST_H_
#define _SEVC_BFPOST_H_

#include "sevc_types.h"

#define EPS          1 // eps * 2^21 = 0 Q24

U32 SEVC_BFPOST_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
U32 SEVC_BFPOST_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
SEVC_BFPOST_S *SEVC_BFPOST_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator);
VOID SEVC_BFPOST_CallBackFuncRegister(SEVC_BFPOST_S *pstBfpostEng,
                                      VOID *pCBFunc, VOID *pUsrData);
VOID SEVC_BFPOST_Feed(SEVC_BFPOST_S *pstBfpostEng, aisp_cpx_s32_t *pNoisyGsc,
                      aisp_s32_t *pfGain, aisp_s32_t *pfSpp, aisp_s32_t iBfFlag, aisp_s32_t noiseUpdate);
VOID SEVC_BFPOST_Reset(SEVC_BFPOST_S *pstBfpostEng);
VOID SEVC_BFPOST_Delete(SEVC_BFPOST_S *pstBfpostEng);
#endif