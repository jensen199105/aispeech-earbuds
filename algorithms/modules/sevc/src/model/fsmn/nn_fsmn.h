#ifndef _NN_FSMN_H_
#define _NN_FSMN_H_

NN_STATE_S *nnProcessorNew(SEVC_CONFIG_S *pstNnCfg, MEM_LOCATOR_S *pstMemLocator);
U32 nnProcessorSharedMemSizeGet(SEVC_CONFIG_S *pstNnCfg);
U32 nnProcessorMemSizeGet(SEVC_CONFIG_S *pstNnCfg);
S32 nnProcessorQValueDataGet(NN_STATE_S *pstNn);
U16 nnProcessorFeatureContextRGet(U32 uiSampleRate);
S8 *nnProcessorResourceInfoGet(U32 uiSampleRate);
S32 nnProcessorOutDataGet(NN_STATE_S *pstNn, aisp_s32_t **ppOutput);
VOID nnProcessorProc(NN_STATE_S *pstNn, aisp_s32_t *pfInputs);
VOID nnProcessorDelete(NN_STATE_S *pstNn);

#endif
