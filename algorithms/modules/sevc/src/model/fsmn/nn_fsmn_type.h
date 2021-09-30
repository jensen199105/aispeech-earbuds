#ifndef _NN_FSMN_TYPE_H_
#define _NN_FSMN_TYPE_H_

#ifdef NN_FSMN
#ifdef AISPEECH_FIXED
typedef S8 nn_weight;
#else
typedef aisp_s16_t nn_weight;
#endif

typedef struct
{
    S32 iInputs;
    S32 iHiddens;
    S32 iOutputs;
    S32 usHleftTaps;
    U8 ucWQ;
    U8 ucDataQ;
} FSMN_LAYER_INFO_S;

typedef struct tag_fc_layer
{
    aisp_s32_t *pfBias;
    nn_weight *pfInputWeights;
    S32 iInputs;
    S32 iOutputs;
    S32 iActivation;
    U8 ucWQ;
    U8 ucDataQ;
} FC_LAYER_S;

typedef struct tag_fsmn_layer
{
    S32 iInputs;
    S32 iHiddens;
    S32 iOutputs;
    U16 usHleftTaps;
    U8 ucWQ;
    U8 ucDataQ;
    S8 *pcSharedMem;
    aisp_s32_t *pfInputBias;
    nn_weight *pfInputWeights;
    nn_weight *pfWeights;
    nn_weight *pfH;

    aisp_s32_t *pfHleftTmp;
    aisp_s32_t **ppfHleft;

    aisp_s32_t *pftmp1;
    aisp_s32_t *pfFsmnOutput;
} FSMN_LAYER_S;

#endif
#endif

