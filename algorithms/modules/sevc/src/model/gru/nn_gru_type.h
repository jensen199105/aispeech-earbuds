#ifndef _SEVC_GRU_TYPE_H_
#define _SEVC_GRU_TYPE_H_

#ifdef AISPEECH_FIXED
typedef aisp_s8_t rnn_weight;
#else
typedef aisp_s16_t rnn_weight;
#endif

typedef struct DenseLayer
{
    aisp_s32_t *pfBias;
    rnn_weight *pfInputWeights;
    S32 iInputs;
    S32 iOutputs;
    S32 iActivation;
    U8 ucWQ;
    U8 ucDataQ;
} DENSE_LAYER_S;

typedef struct
{
    S32 iInputs;
    S32 iOutputs;
    S32 iActivation;
    U8 ucWQ;
    U8 ucDataQ;

    aisp_s32_t *pfBias;
    rnn_weight *pfInputWeights;
    rnn_weight *pfRecurrentWeights;
    aisp_s32_t *pfState;
    aisp_s32_t *pfZ;
    aisp_s32_t *pfR;
    aisp_s32_t *pfH;
    aisp_s32_t *pfDataTmp;
} GRU_LAYER_S;

#endif
