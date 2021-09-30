#ifndef _SEVC_GRU_H_
#define _SEVC_GRU_H_
//#include "nn_common.h"

U32 sevcNnRnnMemSizeGet(SEVC_CONFIG_S *pstSevcCfg);
RNN_STATE_S *sevcRnnNew(SEVC_CONFIG_S *pstSevcCfg, SEVC_MEM_LOCATOR_S *pstSevcMemLoc);
S32 sevcQValueDataGet(RNN_STATE_S *pstRnn);
U16 sevcFeatureContextRGet(VOID);
S32 sevcNNOutDataGet(RNN_STATE_S *pstRnn, aisp_s32_t **ppOutput);
void sevcNnRnnGainCalc(RNN_STATE_S *rnn, aisp_s32_t *pfFeatures);

#endif

