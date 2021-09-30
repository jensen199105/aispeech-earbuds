#ifndef _SEVC_AES_H_
#define _SEVC_AES_H_

#include "sevc_types.h"

//default Q15
extern aisp_s32_t innerGainNormalRef[257];
extern aisp_s32_t micGain1[257];
extern aisp_s32_t micGain2[257];
extern aisp_s32_t innerMicGainLoose[257];
extern aisp_s32_t innerMicGainNormal[257];
extern aisp_s32_t innerMicGainNormalTest[257];
extern aisp_s32_t innerMicGainTight[257];
extern aisp_s32_t eqGain[257];
extern aisp_s32_t eqGain2[257];


U32 SEVC_AES_LocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
U32 SEVC_AES_ShMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
SEVC_AES_S *SEVC_AES_New(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator);
VOID SEVC_AES_CallBackFuncRegister(SEVC_AES_S *pstAesEng, VOID *pCBFunc, VOID *pUsrData);
VOID SEVC_AES_Feed(SEVC_AES_S *pstAesEng, aisp_cpx_s32_t *pMic, aisp_cpx_s32_t *pErr, aisp_cpx_s32_t *pEcho,
                   aisp_s32_t refVad);
VOID SEVC_AES_Reset(SEVC_AES_S *pstAesEng);
VOID SEVC_AES_Delete(SEVC_AES_S *pstAesEng);

#endif

