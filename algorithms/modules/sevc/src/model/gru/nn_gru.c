#include<stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
//#include "rbin.h"
#include "AISP_TSL_str.h"
#include "sevc_types.h"
#include "sevc_util.h"
#include "nn_gru.h"
#include "nn_common.h"
#include "AISP_TSL_base.h"
#include "AISP_TSL_complex.h"
#include "AISP_TSL_sigmoid.h"

#define SEVC_GRU_WQ  8
#ifdef AISPEECH_FIXED
#define SEVC_FIX_Q  24
#define SEVC_NN_ONE  16777216
#else
#define SEVC_NN_ONE  1
#endif
//#define DUMP_RNNMODEL

#ifdef AISPEECH_FIXED
#if 0
extern int iDenseLayerInputBiasSize[];
extern int iDenseLayerInputBias[];
extern int iDenseLayerInputWeightSize[];
extern signed char    cDenseLayerInputWeight[];
extern unsigned char ucDenseLayerInputAct;

extern int iGRULayer1BiasSize[];
extern int iGRULayer1Bias[];
extern int iGRULayer1InputWeightSize[];
extern signed char    cGRULayer1InputWeight[];
extern int            iGRULayer1RecurrentWeightSize[];
extern signed char    cGRULayer1RecurrentWeight[];
extern unsigned char ucGRULayer1Act;

extern int iGRULayer2BiasSize[];
extern int iGRULayer2Bias[];
extern int iGRULayer2InputWeightSize[];
extern signed char    cGRULayer2InputWeight[];
extern int            iGRULayer2RecurrentWeightSize[];
extern signed char    cGRULayer2RecurrentWeight[];
extern unsigned char ucGRULayer2Act;

extern int iGRULayer3BiasSize[];
extern int iGRULayer3Bias[];
extern int iGRULayer3InputWeightSize[];
extern signed char    cGRULayer3InputWeight[];
extern int            iGRULayer3RecurrentWeightSize[];
extern signed char    cGRULayer3RecurrentWeight[];
extern unsigned char ucGRULayer3Act;


extern int iDenseLayerOutputBiasSize[];
extern int iDenseLayerOutputBias[];
extern int iDenseLayerOutputWeightSize[];
extern signed char    cDenseLayerOutputWeight[];
extern unsigned char ucDenseLayerOutputAct;

extern int iMiu[];
extern unsigned short usFull2BarkMapping[];
extern unsigned char ucNBBands;
#endif
//#include "dnn_resource/rnn_resource_b1_n_a1_nb64_3gru_128_64_128_loss0.0297_val0.0299.data"
//#include "rnn_resource_b1_n_a1_nb64_3gru_128_64_128_loss0.0297_val0.0299.data"
#include "rnn_resource_b1_n_a1_nb64_3gru_64_64_64_89k_loss0.0329_val_0.0330.data"
#endif

#ifndef AISPEECH_FIXED
extern rbin2_item_t *rbin2_get(rbin2_st_t *rb, char *name, int len);
extern void rbin2_rbin_reverse_data(unsigned char *p, int len);
#endif

#ifdef DUMP_RNNMODEL
static VOID sevcRnnModelDenseLayerShow(DENSE_LAYER_S *pstDenseLayer)
{
    U32 uiLoop;
    U32 ulOffset;
    MSG_DBG("iInputs=%d\r\n", pstDenseLayer->iInputs);
    MSG_DBG("iOutputs=%d\r\n", pstDenseLayer->iOutputs);
    MSG_DBG("iActivation=%d\r\n", pstDenseLayer->iActivation);

    for (uiLoop = 0; uiLoop < pstDenseLayer->iOutputs; uiLoop++)
    {
        MSG_DBG("pfBias[%d]=%f\r\n", uiLoop, pstDenseLayer->pfBias[uiLoop]);
    }

    //ulOffset = pstDenseLayer->iInputs*63;
    ulOffset = 0;

    for (uiLoop = 0; uiLoop < pstDenseLayer->iInputs; uiLoop++)
    {
        MSG_DBG("pfInputWeights[%d]=%f\r\n", uiLoop, pstDenseLayer->pfInputWeights[uiLoop + ulOffset]);
    }
}

static VOID sevcRnnModelGruLayerShow(GRU_LAYER_S *pstGruLayer)
{
    U32 uiLoop, uiLoop2;
    MSG_DBG("iInputs=%d\r\n", pstGruLayer->iInputs);
    MSG_DBG("iOutputs=%d\r\n", pstGruLayer->iOutputs);
    MSG_DBG("iActivation=%d\r\n", pstGruLayer->iActivation);

    for (uiLoop = 0; uiLoop < pstGruLayer->iOutputs * 3; uiLoop++)
    {
        MSG_DBG("pfBias[%d]=%f\r\n", uiLoop, pstGruLayer->pfBias[uiLoop]);
    }

    for (uiLoop = 0; uiLoop < pstGruLayer->iOutputs * 3; uiLoop++)
    {
        for (uiLoop2 = 0; uiLoop2 < pstGruLayer->iInputs; uiLoop2++)
        {
            MSG_DBG("I_W[%03d-%03d]=%f\r\n", uiLoop, uiLoop2, pstGruLayer->pfInputWeights[uiLoop2]);
        }
    }

    for (uiLoop = 0; uiLoop < pstGruLayer->iOutputs * 3; uiLoop++)
    {
        for (uiLoop2 = 0; uiLoop2 < pstGruLayer->iInputs; uiLoop2++)
        {
            MSG_DBG("R_W[%03d-%03d]=%f\r\n", uiLoop, uiLoop2,
                    pstGruLayer->pfRecurrentWeights[uiLoop2]);
        }
    }
}

static VOID sevcRnnModelShow(RNN_MODEL_S *pstModel)
{
    S32 ii;
    MSG_DBG("iDenseInSize=%d\r\n", pstModel->iDenseInSize);

    for (ii = 0; ii < 3; ii++)
    {
        MSG_DBG("iGruLayerSize[%d]=%d\r\n", ii, pstModel->iGruLayerSize[ii]);
    }

    MSG_DBG("iDenseOutSize=%d\r\n", pstModel->iDenseOutSize);

    //sevcRnnModelDenseLayerShow(pstModel->pDenseInLayer);

    for (ii = 0; ii < 3; ii++)
    {
        sevcRnnModelGruLayerShow(pstModel->ppGruLayer[ii]);
        exit(0) ;
    }

    sevcRnnModelDenseLayerShow(pstModel->pDenseOutLayer);
}

VOID sevcNnRnnModelInfoShow(RNN_MODEL_INFO_S *pstRnnModelInfo)
{
    S32 ii;
    MSG_DBG("uiMagic:              %08X\r\n", pstRnnModelInfo->uiMagic);
    MSG_DBG("usDenseInInput:           %d\r\n", pstRnnModelInfo->usDenseInInput);
    MSG_DBG("usDenseInNeurons:         %d\r\n", pstRnnModelInfo->usDenseInNeurons);
    MSG_DBG("ucDenseInActoin:        %d\r\n", pstRnnModelInfo->ucDenseInActoin);

    for (ii = 0; ii < 3; ii++)
    {
        MSG_DBG("usGruL%dInput:        %d\r\n", ii, pstRnnModelInfo->usGruLayerInput[ii]);
        MSG_DBG("usGruL%dNeurons:      %d\r\n", ii, pstRnnModelInfo->usGruLayerNeurons[ii]);
        MSG_DBG("ucGruL%dActoin:       %d\r\n", ii, pstRnnModelInfo->ucGruLayerActoin[ii]);
    }

    MSG_DBG("usDenseOutInput:    %d\r\n", pstRnnModelInfo->usDenseOutInput);
    MSG_DBG("usDenseOutNeurons:  %d\r\n", pstRnnModelInfo->usDenseOutNeurons);
    MSG_DBG("ucDenseOutActoin:      %d\r\n", pstRnnModelInfo->ucDenseOutActoin);
    MSG_DBG("usChecksum:           %d\r\n", pstRnnModelInfo->usChecksum);
}

#endif

#ifdef AISPEECH_FIXED

static S32 sevcRnnModelInfoInit(RNN_MODEL_INFO_S *pstModelInfo)
{
    /* Update RNN model info. */
    pstModelInfo->uiMagic    = 0;/* Not used. */
    pstModelInfo->usChecksum = 0;/* Not used. */
    pstModelInfo->ucWQ = SEVC_GRU_WQ;
    pstModelInfo->ucDataQ = SEVC_FIX_Q;
    pstModelInfo->ucNBBands = ucNBBands;
    pstModelInfo->usDenseInInput    = iDenseLayerInputWeightSize[1];
    pstModelInfo->usDenseInNeurons  = iDenseLayerInputWeightSize[0];
    pstModelInfo->ucDenseInActoin = ucDenseLayerInputAct;
    pstModelInfo->usGruLayerInput[0]   = iGRULayer1InputWeightSize[1];
    pstModelInfo->usGruLayerNeurons[0] = iGRULayer1InputWeightSize[0] / 3;
    pstModelInfo->ucGruLayerActoin[0]  = ucGRULayer1Act;
    pstModelInfo->usGruLayerInput[1]   = iGRULayer2InputWeightSize[1];
    pstModelInfo->usGruLayerNeurons[1] = iGRULayer2InputWeightSize[0] / 3;
    pstModelInfo->ucGruLayerActoin[1] = ucGRULayer2Act;
    pstModelInfo->usGruLayerInput[2]   = iGRULayer3InputWeightSize[1];
    pstModelInfo->usGruLayerNeurons[2] = iGRULayer3InputWeightSize[0] / 3;
    pstModelInfo->ucGruLayerActoin[2]  = ucGRULayer3Act;
    pstModelInfo->usDenseOutInput   = iDenseLayerOutputWeightSize[1];
    pstModelInfo->usDenseOutNeurons = iDenseLayerOutputWeightSize[0];
    pstModelInfo->ucDenseOutActoin     = ucDenseLayerOutputAct;
    return 0;
}

static S32 sevcRnnModelDataInit(RNN_STATE_S *pstRnn, SEVC_CONFIG_S *pstSevcCfg)
{
    S32 ii;
    RNN_MODEL_S *pstModel = pstRnn->pstModel;
    pstRnn->pfMiu = iMiu;

    if (pstSevcCfg->nnBarkSupport)
    {
        pstRnn->psFull2BarkMapping = usFull2BarkMapping;
    }

    pstModel->pDenseInLayer->pfBias                   = (aisp_s32_t *)iDenseLayerInputBias;
    pstModel->pDenseInLayer->pfInputWeights          = (rnn_weight *)cDenseLayerInputWeight;
    pstModel->pDenseInLayer->ucWQ = SEVC_GRU_WQ;
    pstModel->pDenseInLayer->ucDataQ = SEVC_FIX_Q;
    pstModel->ppGruLayer[0]->pfBias                  = (aisp_s32_t *)iGRULayer1Bias;
    pstModel->ppGruLayer[0]->pfInputWeights         = (rnn_weight *)cGRULayer1InputWeight;
    pstModel->ppGruLayer[0]->pfRecurrentWeights     = (rnn_weight *)cGRULayer1RecurrentWeight;
    pstModel->ppGruLayer[1]->pfBias                  = (aisp_s32_t *)iGRULayer2Bias;
    pstModel->ppGruLayer[1]->pfInputWeights         = (rnn_weight *)cGRULayer2InputWeight;
    pstModel->ppGruLayer[1]->pfRecurrentWeights     = (rnn_weight *)cGRULayer2RecurrentWeight;
    pstModel->ppGruLayer[2]->pfBias                  = (aisp_s32_t *)iGRULayer3Bias;
    pstModel->ppGruLayer[2]->pfInputWeights         = (rnn_weight *)cGRULayer3InputWeight;
    pstModel->ppGruLayer[2]->pfRecurrentWeights     = (rnn_weight *)cGRULayer3RecurrentWeight;

    for (ii = 0; ii < 3; ii++)
    {
        pstModel->ppGruLayer[ii]->ucWQ = SEVC_GRU_WQ;
        pstModel->ppGruLayer[ii]->ucDataQ = SEVC_FIX_Q;
    }

    pstModel->pDenseOutLayer->pfBias              = (aisp_s32_t *)iDenseLayerOutputBias;
    pstModel->pDenseOutLayer->pfInputWeights     = (rnn_weight *)cDenseLayerOutputWeight;
    pstModel->pDenseOutLayer->ucWQ = SEVC_GRU_WQ;
    pstModel->pDenseOutLayer->ucDataQ = SEVC_FIX_Q;
    return 0;
}

#endif

#ifndef AISPEECH_FIXED
static S32 sevcRnnModelInfoLoad(RNN_MODEL_INFO_S *pstRnnModelInfo,
                                S8 *pcModel, S8 *pcResource)
{
    S32 iRet;
    S32 iReadCount;
    S32 ii;
    FILE *pf = NULL;
    rbin2_st_t *rbin;
    rbin2_item_t *item;
    U32 uiSize = sizeof(RNN_MODEL_INFO_S);
    rbin = rbin2_new();

    if (NULL == rbin)
    {
        MSG_DBG("rbin create fail\r\n");
        return -1;
    }

    iRet = rbin2_read(rbin, (S8 *)pcResource);

    if (iRet != 0)
    {
        MSG_DBG("%s [%d] read failed\n", __func__, __LINE__);
        goto end1;
    }

    pf = rbin->f;
    item = rbin2_get(rbin, pcModel, strlen(pcModel));

    if (!item)
    {
        MSG_DBG("%s not found\n", pcModel);
        iRet = -2;
        goto end1;
    }

    if (item->len < uiSize)
    {
        MSG_DBG("%s data len error %d %d\n", pcModel, item->len, uiSize);
        iRet = -3;
        goto end1;
    }

    fseek(pf, item->pos, SEEK_SET);
    iReadCount = fread((S8 *)pstRnnModelInfo, 1, uiSize, pf);

    if (uiSize != iReadCount)
    {
        MSG_DBG("%s read file %s fail %d %d\r\n", __func__, pcModel, iReadCount, uiSize);
        iRet = -4;
        goto end1;
    }

    if (item->reverse)
    {
        rbin2_rbin_reverse_data((U8 *)pstRnnModelInfo, uiSize);
    }

    pstRnnModelInfo->uiMagic = ntohl(pstRnnModelInfo->uiMagic);
    pstRnnModelInfo->usDenseInInput = ntohs(pstRnnModelInfo->usDenseInInput);
    pstRnnModelInfo->usDenseInNeurons = ntohs(pstRnnModelInfo->usDenseInNeurons);

    for (ii = 0; ii < 3; ii++)
    {
        pstRnnModelInfo->usGruLayerInput[ii] = ntohs(pstRnnModelInfo->usGruLayerInput[ii]);
        pstRnnModelInfo->usGruLayerNeurons[ii] = ntohs(pstRnnModelInfo->usGruLayerNeurons[ii]);
    }

    pstRnnModelInfo->usDenseOutInput = ntohs(pstRnnModelInfo->usDenseOutInput);
    pstRnnModelInfo->usDenseOutNeurons = ntohs(pstRnnModelInfo->usDenseOutNeurons);
    pstRnnModelInfo->usChecksum = ntohs(pstRnnModelInfo->usChecksum);
    iRet = 0;
end1:
    rbin2_delete(rbin);
    return iRet;
}
#endif

U32 sevcNnRnnModelDenseLayerMemSizeGet(U32 uiInputs, U32 uiNeurous)
{
    U32 uiTotalMemSize = 0;
    /* structure */
    uiTotalMemSize += SEVC_SIZE_ALIGN(sizeof(DENSE_LAYER_S));
#ifndef AISPEECH_FIXED
    /* pfBias */
    uiTotalMemSize += SEVC_SIZE_ALIGN(uiNeurous * sizeof(aisp_s32_t));
    /* pfInputWeights */
    uiTotalMemSize += SEVC_SIZE_ALIGN(uiInputs * uiNeurous * sizeof(rnn_weight));
#endif
    //MSG_DBG("%s [%d] uiTotalMemSize=%d\r\n", __func__, __LINE__, uiTotalMemSize);
    return uiTotalMemSize;
}


U32 sevcNnRnnModelGruLayerMemSizeGet(U32 uiInputs, U32 uiNeurous)
{
    U32 uiTotalMemSize = 0;
    /* structure */
    uiTotalMemSize += SEVC_SIZE_ALIGN(sizeof(GRU_LAYER_S));
#ifndef AISPEECH_FIXED
    /* pfBias */
    uiTotalMemSize += 3 * SEVC_SIZE_ALIGN(uiNeurous * sizeof(aisp_s32_t));
    /* pfInputWeights */
    uiTotalMemSize += 3 * SEVC_SIZE_ALIGN(uiInputs * uiNeurous * sizeof(rnn_weight));
    /* pfRecurrentWeights */
    uiTotalMemSize += 3 * SEVC_SIZE_ALIGN(uiNeurous * uiNeurous * sizeof(rnn_weight));
#endif
    /* pfState */
    uiTotalMemSize += SEVC_SIZE_ALIGN(uiNeurous * sizeof(aisp_s32_t));
    /* pfZ pfR pfH */
    uiTotalMemSize += 3 * SEVC_SIZE_ALIGN(uiNeurous * sizeof(aisp_s32_t));
    /* pfDataTmp */
    uiTotalMemSize += SEVC_SIZE_ALIGN(uiNeurous * sizeof(aisp_s32_t));
    //MSG_DBG("%s [%d] uiTotalMemSize=%d\r\n", __func__, __LINE__, uiTotalMemSize);
    return uiTotalMemSize;
}

U32 sevcNnRnnModelMemSizeGet(SEVC_CONFIG_S *pstSevcCfg, RNN_MODEL_INFO_S *pRnnModelInfo)
{
    U32 uiTotalMemSize = 0;
    S32 ii;
    uiTotalMemSize += SEVC_SIZE_ALIGN(sizeof(RNN_MODEL_S));
    uiTotalMemSize += sevcNnRnnModelDenseLayerMemSizeGet(pRnnModelInfo->usDenseInInput,
                      pRnnModelInfo->usDenseInNeurons);
    uiTotalMemSize += SEVC_SIZE_ALIGN(3 * sizeof(VOID *));

    for (ii = 0; ii < 3; ii++)
    {
        uiTotalMemSize += sevcNnRnnModelGruLayerMemSizeGet(pRnnModelInfo->usGruLayerInput[ii],
                          pRnnModelInfo->usGruLayerNeurons[ii]);
    }

    uiTotalMemSize += sevcNnRnnModelDenseLayerMemSizeGet(pRnnModelInfo->usDenseOutInput,
                      pRnnModelInfo->usDenseOutNeurons);
    uiTotalMemSize += pRnnModelInfo->usDenseOutNeurons * sizeof(aisp_s32_t);
    //MSG_DBG("%s [%d] uiTotalMemSize=%d\r\n", __func__, __LINE__, uiTotalMemSize);
    return uiTotalMemSize;
}

U32 sevcNnRnnMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    RNN_MODEL_INFO_S rnnModelInfo;
    S32 iRet           = 0;
    U32 uiTotalMemSize = 0;
    U16 usNbBands      = 0;
    //U16 usFeatureContext = usFeatureContextL + usFeatureContextR + 1;
    U16 usFeatureContext = 1 + 1 + 1;
    S32 iCmnWinLen = pstSevcCfg->cmn_win_len + 1;
#ifdef AISPEECH_FIXED
    iRet = sevcRnnModelInfoInit(&rnnModelInfo);
#else
    iRet = sevcRnnModelInfoLoad(&rnnModelInfo, pstSevcCfg->nnRnnModelFile,
                                pstSevcCfg->acResourseFile);
#endif

    if (0 != iRet)
    {
        return 0;
    }

    uiTotalMemSize += SEVC_SIZE_ALIGN(sizeof(RNN_STATE_S));

    if (pstSevcCfg->nnBarkSupport)
    {
        usNbBands = rnnModelInfo.ucNBBands;
        /* psFull2BarkMapping */
        uiTotalMemSize += SEVC_SIZE_ALIGN(usNbBands * sizeof(U16));
        /* pfNbBandsSum */
        uiTotalMemSize += SEVC_SIZE_ALIGN(usNbBands * sizeof(aisp_s64_t));
        /* pfFeatures */
        uiTotalMemSize += SEVC_SIZE_ALIGN(usFeatureContext * usNbBands * sizeof(aisp_s32_t));

        if (pstSevcCfg->use_smooth_cmn_flag)
        {
            //pCmnFeatureSum
            uiTotalMemSize += SEVC_SIZE_ALIGN(usNbBands * sizeof(aisp_s64_t));
            //ppCmnBuffer
            uiTotalMemSize += SEVC_SIZE_ALIGN(iCmnWinLen * sizeof(VOID *));
            uiTotalMemSize += iCmnWinLen * SEVC_SIZE_ALIGN(usNbBands * sizeof(aisp_s32_t));
        }
    }
    else
    {
        /* pfFeatures */
        uiTotalMemSize += SEVC_SIZE_ALIGN(usFeatureContext * pstSevcCfg->usFftBin
                                          * sizeof(aisp_s32_t));
    }

#if 0
    /* pfVadGruState */
    uiTotalMemSize += SEVC_SIZE_ALIGN(rnnModelInfo.usGruVadNeurons * sizeof(aisp_s32_t));
    /* pfNoiseGruState */
    uiTotalMemSize += SEVC_SIZE_ALIGN(rnnModelInfo.usGruNoiseNeurons * sizeof(aisp_s32_t));
    /* pfDenoiseGruState */
    uiTotalMemSize += SEVC_SIZE_ALIGN(rnnModelInfo.usGruDenoiseNeurons * sizeof(aisp_s32_t));
    /* pfDenseVadOut */
    uiTotalMemSize += SEVC_SIZE_ALIGN(rnnModelInfo.usVadNeurons * sizeof(aisp_s32_t));
    /* pfNoiseInput */
    uiTotalMemSize += SEVC_SIZE_ALIGN(rnnModelInfo.usGruNoiseInput * sizeof(aisp_s32_t));
    /* pfDenoiseInput */
    uiTotalMemSize += SEVC_SIZE_ALIGN(rnnModelInfo.usGruDenoiseInput * sizeof(aisp_s32_t));
#endif
    /* pfMiu */
    uiTotalMemSize += SEVC_SIZE_ALIGN(rnnModelInfo.usDenseInInput * sizeof(aisp_s32_t));
    //dense_out
    uiTotalMemSize += SEVC_SIZE_ALIGN(rnnModelInfo.usDenseInNeurons * sizeof(aisp_s32_t));
    /* pfRnnOut */
    uiTotalMemSize += SEVC_SIZE_ALIGN(rnnModelInfo.usDenseOutNeurons * sizeof(aisp_s32_t));
    /* pstModel */
    uiTotalMemSize += sevcNnRnnModelMemSizeGet(pstSevcCfg, &rnnModelInfo);
    //MSG_DBG("%s [%d] uiTotalMemSize=%d\r\n", __func__, __LINE__, uiTotalMemSize);
    return uiTotalMemSize;
}


#ifndef AISPEECH_FIXED
S32 sevcModelDataRead(aisp_s16_t *pfData, U32 uiLen, FILE *pf, U8 ucReverse)
{
    S32 iReadCount;
    U32 uiSize = uiLen * sizeof(aisp_s16_t);
    U32 uiLoop;
    U32 *puiData = (U32 *)pfData;
    iReadCount = fread((S8 *)pfData, 1, uiSize, pf);

    if (uiSize != iReadCount)
    {
        MSG_DBG("%s read fail %d %d\r\n", __func__, iReadCount, uiSize);
        return -1;
    }

    if (ucReverse)
    {
        rbin2_rbin_reverse_data((U8 *)pfData, uiSize);
    }

    for (uiLoop = 0; uiLoop < uiLen; uiLoop++)
    {
        puiData[uiLoop] = ntohl(puiData[uiLoop]);
    }

    return 0;
}

S32 sevcModelDataReadInt16(U16 *pusData, U32 uiLen, FILE *pf, U8 ucReverse)
{
    S32 iReadCount;
    U32 uiSize = uiLen * sizeof(U16);
    U32 uiLoop;
    iReadCount = fread((S8 *)pusData, 1, uiSize, pf);

    if (uiSize != iReadCount)
    {
        MSG_DBG("%s read fail %d %d\r\n", __func__, iReadCount, uiSize);
        return -1;
    }

    if (ucReverse)
    {
        rbin2_rbin_reverse_data((U8 *)pusData, uiSize);
    }

    for (uiLoop = 0; uiLoop < uiLen; uiLoop++)
    {
        pusData[uiLoop] = ntohs(pusData[uiLoop]);
    }

    return 0;
}

VOID sevcModelDenseLayerRead(DENSE_LAYER_S *pDenseLayer, FILE *pf, U8 ucReverse)
{
    S32 iLoop;
    aisp_s16_t *pWeight = (aisp_s16_t *)pDenseLayer->pfInputWeights;
    aisp_s32_t *pBias   = (aisp_s32_t *)pDenseLayer->pfBias;
    sevcModelDataRead(pBias, pDenseLayer->iOutputs, pf, ucReverse);

    for (iLoop = 0; iLoop < pDenseLayer->iOutputs; iLoop++)
    {
        sevcModelDataRead(pWeight, pDenseLayer->iInputs, pf, ucReverse);
        pWeight += pDenseLayer->iInputs;
    }
}

VOID sevcModelGruLayerRead(GRU_LAYER_S *pGruLayer, FILE *pf, U8 ucReverse)
{
    S32 iLoop, iTotal;
    aisp_s16_t *pWeight = pGruLayer->pfInputWeights;
    iTotal = 3 * pGruLayer->iOutputs;
    sevcModelDataRead(pGruLayer->pfBias, iTotal, pf, ucReverse);

    for (iLoop = 0; iLoop < iTotal; iLoop++)
    {
        sevcModelDataRead(pWeight, pGruLayer->iInputs, pf, ucReverse);
        pWeight += pGruLayer->iInputs;
    }

    pWeight = pGruLayer->pfRecurrentWeights;

    for (iLoop = 0; iLoop < iTotal; iLoop++)
    {
        sevcModelDataRead(pWeight, pGruLayer->iOutputs, pf, ucReverse);
        pWeight += pGruLayer->iOutputs;
    }
}

static S32 sevcRnnModelDataLoad(RNN_STATE_S *pstRnn, SEVC_CONFIG_S *pstSevcCfg)
{
    S32 iRet;
    S32 ii;
    FILE *pf = NULL;
    S8 *pcModel;
    S8 *pcResource;
    rbin2_st_t *rbin;
    rbin2_item_t *item;
    U32 uiSize = sizeof(RNN_MODEL_INFO_S);
    RNN_MODEL_S *pstRnnModel = pstRnn->pstModel;
    RNN_MODEL_INFO_S *pstRnnModelInfo = &pstRnn->modelInfo;
    pcModel = pstSevcCfg->nnRnnModelFile;
    pcResource = pstSevcCfg->acResourseFile;
    rbin = rbin2_new();

    if (NULL == rbin)
    {
        MSG_DBG("rbin create fail\r\n");
        return -1;
    }

    iRet = rbin2_read(rbin, (S8 *)pcResource);

    if (iRet != 0)
    {
        MSG_DBG("%s [%d] read failed\n", __func__, __LINE__);
        goto end1;
    }

    pf = rbin->f;
    item = rbin2_get(rbin, pcModel, strlen(pcModel));

    if (!item)
    {
        MSG_DBG("%s not found\n", pcModel);
        iRet = -2;
        goto end1;
    }

    if (item->len < uiSize)
    {
        MSG_DBG("%s data len error %d %d\n", pcModel, item->len, uiSize);
        iRet = -3;
        goto end1;
    }

    fseek(pf, item->pos + sizeof(RNN_MODEL_INFO_S), SEEK_SET);
    sevcModelDenseLayerRead(pstRnnModel->pDenseInLayer, pf, item->reverse);
    sevcModelDenseLayerRead(pstRnnModel->pDenseOutLayer, pf, item->reverse);

    for (ii = 0; ii < 3; ii++)
    {
        sevcModelGruLayerRead(pstRnnModel->ppGruLayer[ii], pf, item->reverse);
    }

    if (pstSevcCfg->nnBarkSupport)
    {
        sevcModelDataReadInt16(pstRnn->psFull2BarkMapping, pstRnn->usNbBands, pf, item->reverse);
    }

    sevcModelDataRead(pstRnn->pfMiu, pstRnnModelInfo->ucNBBands, pf, item->reverse);
    //sevcRnnModelShow(pstRnnModel);
    iRet = 0;
end1:
    rbin2_delete(rbin);
    return iRet;
}
#endif

DENSE_LAYER_S *sevcRnnDenseLayerNew(LAYER_INFO_S *pstLayerInfo,
                                    SEVC_MEM_LOCATOR_S *pstSevcMemLoc)
{
    DENSE_LAYER_S *pstDenoiseLayer = NULL;
    U32 uiSize = sizeof(DENSE_LAYER_S);
    pstDenoiseLayer = (DENSE_LAYER_S *)sevcMemLocatorGet(pstSevcMemLoc, uiSize, AISP_ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstDenoiseLayer, FAIL);
    pstDenoiseLayer->iInputs  = pstLayerInfo->iInputs;
    pstDenoiseLayer->iOutputs = pstLayerInfo->iOutputs;
    pstDenoiseLayer->iActivation = pstLayerInfo->iActivation;
#ifndef AISPEECH_FIXED
    /* pfBias */
    uiSize = pstDenoiseLayer->iOutputs * sizeof(aisp_s32_t);
    SEVC_BUFF_POINT_SET_ALIGN(pstDenoiseLayer->pfBias, uiSize, pstSevcMemLoc, FAIL);
    /* pfInputWeights */
    uiSize = pstDenoiseLayer->iInputs * pstDenoiseLayer->iOutputs * sizeof(rnn_weight);
    SEVC_BUFF_POINT_SET_ALIGN(pstDenoiseLayer->pfInputWeights, uiSize, pstSevcMemLoc, FAIL);
#endif
    return pstDenoiseLayer;
FAIL:
    return NULL;
}

GRU_LAYER_S *sevcRnnGruLayerNew(LAYER_INFO_S *pstLayerInfo,
                                SEVC_MEM_LOCATOR_S *pstSevcMemLoc)
{
    GRU_LAYER_S *pstGruLayer = NULL;
    U32 uiSize = 0;
    uiSize      = sizeof(GRU_LAYER_S);
    pstGruLayer = (GRU_LAYER_S *)sevcMemLocatorGet(pstSevcMemLoc, uiSize, AISP_ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstGruLayer, FAIL);
    pstGruLayer->iInputs  = pstLayerInfo->iInputs;
    pstGruLayer->iOutputs = pstLayerInfo->iOutputs;
    pstGruLayer->iActivation = pstLayerInfo->iActivation;
#ifndef AISPEECH_FIXED
    /* pfBias */
    uiSize = 3 * pstGruLayer->iOutputs * sizeof(aisp_s32_t);
    SEVC_BUFF_POINT_SET_ALIGN(pstGruLayer->pfBias, uiSize, pstSevcMemLoc, FAIL);
    /* pfInputWeights */
    uiSize = 3 * pstGruLayer->iInputs * pstGruLayer->iOutputs * sizeof(rnn_weight);
    SEVC_BUFF_POINT_SET_ALIGN(pstGruLayer->pfInputWeights, uiSize, pstSevcMemLoc, FAIL);
    /* pfRecurrentWeights */
    uiSize = 3 * pstGruLayer->iOutputs * pstGruLayer->iOutputs * sizeof(rnn_weight);
    SEVC_BUFF_POINT_SET_ALIGN(pstGruLayer->pfRecurrentWeights, uiSize, pstSevcMemLoc, FAIL);
#endif
    /* pfState */
    uiSize = pstGruLayer->iOutputs * sizeof(aisp_s32_t);
    SEVC_BUFF_POINT_SET_ALIGN(pstGruLayer->pfState, uiSize, pstSevcMemLoc, FAIL);
    /* pfZ */
    uiSize = pstGruLayer->iOutputs * sizeof(aisp_s32_t);
    SEVC_BUFF_POINT_SET_ALIGN(pstGruLayer->pfZ, uiSize, pstSevcMemLoc, FAIL);
    /* pfR */
    uiSize = pstGruLayer->iOutputs * sizeof(aisp_s32_t);
    SEVC_BUFF_POINT_SET_ALIGN(pstGruLayer->pfR, uiSize, pstSevcMemLoc, FAIL);
    /* pfH */
    uiSize = pstGruLayer->iOutputs * sizeof(aisp_s32_t);
    SEVC_BUFF_POINT_SET_ALIGN(pstGruLayer->pfH, uiSize, pstSevcMemLoc, FAIL);
    /* pfDataTmp */
    uiSize = pstGruLayer->iOutputs * sizeof(aisp_s32_t);
    SEVC_BUFF_POINT_SET_ALIGN(pstGruLayer->pfDataTmp, uiSize, pstSevcMemLoc, FAIL);
    return pstGruLayer;
FAIL:
    return NULL;
}


RNN_MODEL_S *sevcRnnModelNew(RNN_MODEL_INFO_S *pstModelInfo,
                             SEVC_MEM_LOCATOR_S *pstSevcMemLoc)
{
    RNN_MODEL_S *pstModel;
    U32 uiSize;
    LAYER_INFO_S layer;
    S32 ii;

    if (NULL == pstModelInfo || NULL == pstSevcMemLoc)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
    }

    MEM_LOCATOR_SHOW(pstSevcMemLoc);
    uiSize   = sizeof(RNN_MODEL_S);
    pstModel = (RNN_MODEL_S *)sevcMemLocatorGet(pstSevcMemLoc, uiSize, AISP_ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstModel, FAIL);
    pstModel->iDenseInSize    = pstModelInfo->usDenseInNeurons;
    pstModel->iGruLayerSize[0]        = pstModelInfo->usGruLayerNeurons[0];
    pstModel->iGruLayerSize[1]      = pstModelInfo->usGruLayerNeurons[1];
    pstModel->iGruLayerSize[2]    = pstModelInfo->usGruLayerNeurons[2];
    pstModel->iDenseOutSize = pstModelInfo->usDenseOutNeurons;
    /* pVadInLayer */
    layer.iInputs       = pstModelInfo->usDenseInInput;
    layer.iOutputs      = pstModelInfo->usDenseInNeurons;
    layer.iActivation      = pstModelInfo->ucDenseInActoin;
    pstModel->pDenseInLayer = sevcRnnDenseLayerNew(&layer, pstSevcMemLoc);
    AISP_TSL_PTR_CHECK(pstModel->pDenseInLayer, FAIL);
    SEVC_BUFF_POINT_SET_ALIGN(pstModel->ppGruLayer, 3 * sizeof(VOID *), pstSevcMemLoc, FAIL);

    for (ii = 0; ii < 3; ii++)
    {
        /* pVadGruLayer */
        layer.iInputs        = pstModelInfo->usGruLayerInput[ii];
        layer.iOutputs       = pstModelInfo->usGruLayerNeurons[ii];
        layer.iActivation       = pstModelInfo->ucGruLayerActoin[ii];
        pstModel->ppGruLayer[ii] = sevcRnnGruLayerNew(&layer, pstSevcMemLoc);
        AISP_TSL_PTR_CHECK(pstModel->ppGruLayer[ii], FAIL);
    }

    /* pDenoiseOutLayer */
    layer.iInputs            = pstModelInfo->usDenseOutInput;
    layer.iOutputs           = pstModelInfo->usDenseOutNeurons;
    layer.iActivation           = pstModelInfo->ucDenseOutActoin;
    pstModel->pDenseOutLayer = sevcRnnDenseLayerNew(&layer, pstSevcMemLoc);
    AISP_TSL_PTR_CHECK(pstModel->pDenseOutLayer, FAIL);
    SEVC_BUFF_POINT_SET_ALIGN(pstModel->pfOutDenseOutput,
                              layer.iOutputs * sizeof(aisp_s32_t), pstSevcMemLoc, FAIL);
    pstModel->pOutput = pstModel->pfOutDenseOutput;
    pstModel->iOutLen = layer.iOutputs;
    MEM_LOCATOR_SHOW(pstSevcMemLoc);
    return pstModel;
FAIL:
    return NULL;
}

RNN_STATE_S *sevcRnnNew(SEVC_CONFIG_S *pstSevcCfg, SEVC_MEM_LOCATOR_S *pstSevcMemLoc)
{
    S32 iRet = 0;
    S32 ii;
    U16 usSize;
    RNN_STATE_S *pstRnn;
    RNN_MODEL_INFO_S *pstRnnModelInfo;
    S32 iCmnWinLen = pstSevcCfg->cmn_win_len + 1;

    if (NULL == pstSevcCfg || NULL == pstSevcMemLoc)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
    }

    MEM_LOCATOR_SHOW(pstSevcMemLoc);
    usSize = sizeof(RNN_STATE_S);
    pstRnn = (RNN_STATE_S *)sevcMemLocatorGet(pstSevcMemLoc, usSize, AISP_ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstRnn, FAIL);
    pstRnn->usFeatureContextL = 1;
    pstRnn->usFeatureContextR = 1;
    pstRnn->usFeatureContext = pstRnn->usFeatureContextL + pstRnn->usFeatureContextR + 1;
#ifdef AISPEECH_FIXED
    iRet = sevcRnnModelInfoInit(&pstRnn->modelInfo);
#else
    iRet = sevcRnnModelInfoLoad(&pstRnn->modelInfo, pstSevcCfg->nnRnnModelFile,
                                pstSevcCfg->acResourseFile);
#endif

    if (0 != iRet)
    {
        return NULL;
    }

    pstRnnModelInfo = &pstRnn->modelInfo;
    //sevcNnRnnModelInfoShow(pstRnnModelInfo);
    pstRnn->usNbBands = pstRnnModelInfo->ucNBBands;

    if (pstSevcCfg->nnBarkSupport)
    {
        //psFull2BarkMapping
        SEVC_BUFF_POINT_SET_ALIGN(pstRnn->psFull2BarkMapping, (pstRnn->usNbBands * sizeof(U16)),
                                  pstSevcMemLoc, FAIL);
        //pfNbBandsSum
        SEVC_BUFF_POINT_SET_ALIGN(pstRnn->pfNbBandsSum, (pstRnn->usNbBands * sizeof(aisp_s64_t)),
                                  pstSevcMemLoc, FAIL);
        //pfFeatures
        SEVC_BUFF_POINT_SET_ALIGN(pstRnn->pfFeatures,
                                  (pstRnn->usFeatureContext * pstRnn->usNbBands * sizeof(aisp_s32_t)),
                                  pstSevcMemLoc, FAIL);

        if (pstSevcCfg->use_smooth_cmn_flag)
        {
            //pCmnFeatureSum
            SEVC_BUFF_POINT_SET_ALIGN(pstRnn->pCmnFeatureSum,
                                      (pstRnn->usNbBands * sizeof(aisp_s64_t)),
                                      pstSevcMemLoc, FAIL);
            //ppCmnBuffer
            SEVC_BUFF_POINT_SET_ALIGN(pstRnn->ppCmnBuffer,
                                      (iCmnWinLen * sizeof(VOID *)),
                                      pstSevcMemLoc, FAIL);

            for (ii = 0; ii < iCmnWinLen; ii++)
            {
                SEVC_BUFF_POINT_SET_ALIGN(pstRnn->ppCmnBuffer[ii],
                                          (pstRnn->usNbBands * sizeof(aisp_s32_t)),
                                          pstSevcMemLoc, FAIL);
            }
        }
    }
    else
    {
        //pfFeatures
        SEVC_BUFF_POINT_SET_ALIGN(pstRnn->pfFeatures,
                                  (pstRnn->usFeatureContext * pstSevcCfg->usFftBin * sizeof(aisp_s32_t)),
                                  pstSevcMemLoc, FAIL);
    }

#if 0
    //pfVadGruState
    usSize = pstRnnModelInfo->usGruVadNeurons * sizeof(aisp_s32_t);
    SEVC_BUFF_POINT_SET_ALIGN(pstRnn->pfVadGruState, usSize, pstSevcMemLoc, FAIL);
    //pfNoiseGruState
    usSize = pstRnnModelInfo->usGruNoiseNeurons * sizeof(aisp_s32_t);
    SEVC_BUFF_POINT_SET_ALIGN(pstRnn->pfNoiseGruState, usSize, pstSevcMemLoc, FAIL);
    //pfDenoiseGruState
    usSize = pstRnnModelInfo->usGruDenoiseNeurons * sizeof(aisp_s32_t);
    SEVC_BUFF_POINT_SET_ALIGN(pstRnn->pfDenoiseGruState, usSize, pstSevcMemLoc, FAIL);
#endif
    //pfMiu
    usSize = pstRnnModelInfo->usDenseInInput * sizeof(aisp_s32_t);
    SEVC_BUFF_POINT_SET_ALIGN(pstRnn->pfMiu, usSize, pstSevcMemLoc, FAIL);
    //dense_out
    usSize = pstRnnModelInfo->usDenseInNeurons * sizeof(aisp_s32_t);
    SEVC_BUFF_POINT_SET_ALIGN(pstRnn->pfDenseVadOut, usSize, pstSevcMemLoc, FAIL);
    //pfRnnOut
    usSize = pstRnnModelInfo->usDenseOutNeurons * sizeof(aisp_s32_t);
    SEVC_BUFF_POINT_SET_ALIGN(pstRnn->pfRnnOut, usSize, pstSevcMemLoc, FAIL);
    /* Alloc memory for RNN model struction. */
    pstRnn->pstModel = sevcRnnModelNew(pstRnnModelInfo, pstSevcMemLoc);
    AISP_TSL_PTR_CHECK(pstRnn->pstModel, FAIL);
#ifdef AISPEECH_FIXED
    sevcRnnModelDataInit(pstRnn, pstSevcCfg);
#else
    sevcRnnModelDataLoad(pstRnn, pstSevcCfg);
#endif
    MEM_LOCATOR_SHOW(pstSevcMemLoc);
    return pstRnn;
FAIL:
    return  NULL;
}

S32 sevcQValueDataGet(RNN_STATE_S *pstRnn)
{
    return pstRnn->modelInfo.ucDataQ;
}

U16 sevcFeatureContextRGet(VOID)
{
    //return usFeatureContextR;
    return 1;
}

S32 sevcNNOutDataGet(RNN_STATE_S *pstRnn, aisp_s32_t **ppOutput)
{
    *ppOutput = pstRnn->pstModel->pOutput;
    return pstRnn->pstModel->iOutLen;
}

/*
pfBias:W32Q24
pfInputWeights:W8Q8
pfRecurrentWeights:W8Q8
input:W32Q24
pfState:W32Q24
*/
void sevcNnGruCalc(GRU_LAYER_S *gru, aisp_s32_t *input)
{
    S32 iIdx;
    S32 iNeurons, iInput;
    U8 ucWQ = gru->ucWQ;
    U8 ucDataQ = gru->ucDataQ;
    aisp_s32_t fSum;
    aisp_s32_t fSum1;
    aisp_s32_t fSum2;
    aisp_s32_t *pfDataTmp = gru->pfDataTmp;
    aisp_s32_t *pfBias;
    rnn_weight  *pfWeights;
    rnn_weight  *pfRecurWeights;
    aisp_s32_t *pfState = gru->pfState;
    aisp_s32_t *pfZ = gru->pfZ;
    aisp_s32_t *pfR = gru->pfR;
    aisp_s32_t *pfH = gru->pfH;
    aisp_s32_t (*pActFuncArr[3])(aisp_s32_t x, S32 iInQ, S32 iOutQ) =
    {
        nnSigmoidApprox,
        nnTansigApprox,
        nnRelu
    };
    iInput   = gru->iInputs;
    iNeurons = gru->iOutputs;
    pfBias         = (aisp_s32_t *)gru->pfBias;
    pfWeights      = (rnn_weight *)gru->pfInputWeights;
    pfRecurWeights = (rnn_weight *)gru->pfRecurrentWeights;

    for (iIdx = 0; iIdx < iNeurons; iIdx++)
    {
        nnVecMulSum32X8_Real(&fSum1, input, pfWeights, iInput, ucWQ);
        nnVecMulSum32X8_Real(&fSum2, pfState, pfRecurWeights, iNeurons, ucWQ);
        fSum = fSum1 + fSum2 + pfBias[iIdx];

        if ((aisp_s64_t)(fSum1 + fSum2 + pfBias[iIdx]) > 2.147483648000000e+09
                || (aisp_s64_t)(fSum1 + fSum2 + pfBias[iIdx]) < -4.294967296000000e+09)
        {
            MSG_DBG("%s error fSum=%d\r\n", __func__, fSum);
        }

        pfZ[iIdx] = nnSigmoidApprox((int)fSum, ucDataQ, ucDataQ);
        pfWeights      += iInput;
        pfRecurWeights += iNeurons;
    }

    pfBias += iNeurons;

    for (iIdx = 0; iIdx < iNeurons; iIdx++)
    {
        nnVecMulSum32X8_Real(&fSum1, input, pfWeights, iInput, ucWQ);
        nnVecMulSum32X8_Real(&fSum2, pfState, pfRecurWeights, iNeurons, ucWQ);
        fSum = fSum1 + fSum2 + pfBias[iIdx];
        pfR[iIdx] = nnSigmoidApprox((int)fSum, ucDataQ, ucDataQ);
        pfWeights      += iInput;
        pfRecurWeights += iNeurons;
    }

    pfBias += iNeurons;

    for (iIdx = 0; iIdx < iNeurons; iIdx++)
    {
        nnVecMulSum32X8_Real(&fSum1, input, pfWeights, iInput, ucWQ);
        nnVecMul32X32_Real(pfDataTmp, pfR, pfState, iNeurons, ucDataQ);
        nnVecMulSum32X8_Real(&fSum2, pfDataTmp, pfRecurWeights, iNeurons, ucWQ);
        fSum = fSum1 + fSum2 + pfBias[iIdx];
        fSum = pActFuncArr[gru->iActivation](fSum, ucDataQ, ucDataQ);
        pfH[iIdx] = PSHR((aisp_s64_t)pfZ[iIdx] * pfState[iIdx]
                         + (aisp_s64_t)(SEVC_NN_ONE - pfZ[iIdx]) * fSum, ucDataQ);
        pfWeights      += iInput;
        pfRecurWeights += iNeurons;
    }

    for (iIdx = 0; iIdx < iNeurons; iIdx++)
    {
        pfState[iIdx] = pfH[iIdx];
    }
}

/*
pfFeatures:W32Q24
*/
void sevcNnRnnGainCalc(RNN_STATE_S *rnn, aisp_s32_t *pfFeatures)
{
    RNN_MODEL_S *pstModel       = rnn->pstModel;
    GRU_LAYER_S **ppGruLayer = pstModel->ppGruLayer;
    aisp_s32_t  *pfOutDenseOutput   = pstModel->pfOutDenseOutput;
    aisp_s32_t  *pfDenseVadOut  = rnn->pfDenseVadOut;
    S32 ii;
    nnDenseCalc(pstModel->pDenseInLayer,  pfDenseVadOut, pfFeatures);
    sevcNnGruCalc(ppGruLayer[0], pfDenseVadOut);

    for (ii = 1; ii < 3; ii++)
    {
        sevcNnGruCalc(ppGruLayer[ii], ppGruLayer[ii - 1]->pfState);
    }

    nnDenseCalc(pstModel->pDenseOutLayer, pfOutDenseOutput,   ppGruLayer[ii - 1]->pfState);
}

