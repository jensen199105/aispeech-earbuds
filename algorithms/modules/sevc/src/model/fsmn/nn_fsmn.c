#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "sevc_types.h"
#include "sevc_util.h"
#include "nn_common.h"
#include "nn_fsmn.h"
#include "AISP_TSL_base.h"
#include "AISP_TSL_complex.h"
#include "AISP_TSL_str.h"
//#include "AISP_TSL_sigmoid.h"

extern FILE *fpYyPcm;

#if SEVC_SWITCH_NN

#define MODEL_INFO_SHOW 0

#ifdef AISPEECH_FIXED
#ifdef SEVC_PHONE
#if SEVC_SWITCH_NN_AES
//#include "fsmn_resource_phone_nn_aes_bq20_wq6_20200609.data"
#include "fsmn_resource_phone_nn_aes_bq20_wq6_20200928.data"
#include "fsmn_resource_phone_nn_aes_nb_bq20_wq6_20200930.data"
#else
#include "fsmn_resource_phone_bq20_wq6_20201119.data"
#include "fsmn_resource_phone_nb_bq20_wq6_20201119.data"
#endif
#elif (defined FAR_FIELD_COMMUNICATION)
#include "fsmn_resource_meeting_bq20_wq6_20200522.data"
#elif (defined SEVC_CAR)
#include "fsmn_resource_car_bq20_wq6_20200430.data"
#else
#error "please select one model"
#endif
#endif

VOID nnModelInfoShow(NN_MODEL_INFO_S *pstNnModelInfo)
{
#if MODEL_INFO_SHOW
    S32 ii;
    S32 ucInLinears = pstNnModelInfo->ucInLinears;
    S32 ucNetLayer = pstNnModelInfo->ucNetLayers;
    S32 ucOutLinears = pstNnModelInfo->ucOutLinears;
    MSG_DBG("ucWQ            = %d\r\n", pstNnModelInfo->ucWQ);
    MSG_DBG("ucNBBands       = %d\r\n", pstNnModelInfo->ucNBBands);
    MSG_DBG("ucInLinears     = %d\r\n", pstNnModelInfo->ucInLinears);
    MSG_DBG("ucNetLayer      = %d\r\n", pstNnModelInfo->ucNetLayers);
    MSG_DBG("ucOutLinears    = %d\r\n", pstNnModelInfo->ucOutLinears);

    for (ii = 0; ii < ucInLinears; ii++)
    {
        MSG_DBG("INusInputSize%d  = %d\r\n", ii, pstNnModelInfo->pInLinears[ii].usInputSize);
        MSG_DBG("INusOutputSize%d = %d\r\n", ii, pstNnModelInfo->pInLinears[ii].usOutputSize);
        MSG_DBG("INucAction%d     = %d\r\n", ii, pstNnModelInfo->pInLinears[ii].ucAction);
    }

    for (ii = 0; ii < ucNetLayer; ii++)
    {
        MSG_DBG("NusInputSize%d   = %d\r\n", ii, pstNnModelInfo->pNets[ii].usInputSize);
        MSG_DBG("NusHiddenSize%d  = %d\r\n", ii, pstNnModelInfo->pNets[ii].usHiddenSize);
        MSG_DBG("NusOutputSize%d  = %d\r\n", ii, pstNnModelInfo->pNets[ii].usOutputSize);
        MSG_DBG("NusMemBlock%d    = %d\r\n", ii, pstNnModelInfo->pNets[ii].usMemBlock);
        MSG_DBG("NucAction%d      = %d\r\n", ii, pstNnModelInfo->pNets[ii].ucAction);
    }

    for (ii = 0; ii < ucOutLinears; ii++)
    {
        MSG_DBG("OUTusInputSize%d  = %d\r\n", ii, pstNnModelInfo->pOutLinears[ii].usInputSize);
        MSG_DBG("OUTusOutputSize%d = %d\r\n", ii, pstNnModelInfo->pOutLinears[ii].usOutputSize);
        MSG_DBG("OUTucAction%d     = %d\r\n", ii, pstNnModelInfo->pOutLinears[ii].ucAction);
    }

#endif
}

#ifdef AISPEECH_FIXED
static S32 nnNbModelInfoInit(NN_MODEL_INFO_S *pstModelInfo)
{
    S32 ii;
    /* Update FSMN model info. */
    pstModelInfo->ucWQ = g_ucNbFsmnWQ;
    pstModelInfo->ucDataQ = g_ucNbFsmnDataQ;
    pstModelInfo->ucNBBands = g_ucNbNBBands;
    pstModelInfo->ucInLinears = g_ucNbInLinears;
    pstModelInfo->ucNetLayers = g_ucNbNetLayerNum;
    pstModelInfo->ucOutLinears = g_ucNbOutLinears;
    pstModelInfo->usFeatureTotalDim = 0;

    //input
    if (pstModelInfo->ucInLinears)
    {
        pstModelInfo->pInLinears =
            (LINEAR_LAYER_s *)NN_CALLOC(pstModelInfo->ucInLinears * sizeof(LINEAR_LAYER_s));

        if (NULL == pstModelInfo->pInLinears)
        {
            MSG_DBG("%s [%d] mem alloc fail\r\n", __func__, __LINE__);
            return -1;
        }

        pstModelInfo->usFeatureTotalDim = g_usNbInDenseInputSize[0];

        for (ii = 0; ii < pstModelInfo->ucInLinears; ii++)
        {
            pstModelInfo->pInLinears[ii].usInputSize = g_usNbInDenseInputSize[ii];
            pstModelInfo->pInLinears[ii].usOutputSize = g_usNbInDenseOutputSize[ii];
            pstModelInfo->pInLinears[ii].ucAction = g_usNbInDenseAction[ii];
            pstModelInfo->pInLinears[ii].ucWQ = pstModelInfo->ucWQ;
            pstModelInfo->pInLinears[ii].ucDataQ = pstModelInfo->ucDataQ;
        }
    }
    else
    {
        pstModelInfo->pInLinears = NULL;
    }

    //fsmn
    pstModelInfo->pNets = (FSMN_LAYER_s *)NN_CALLOC(pstModelInfo->ucNetLayers * sizeof(FSMN_LAYER_s));

    if (NULL == pstModelInfo->pNets)
    {
        MSG_DBG("%s [%d] mem alloc fail\r\n", __func__, __LINE__);
        return -1;
    }

    pstModelInfo->usFeatureTotalDim = (pstModelInfo->usFeatureTotalDim ?
                                       pstModelInfo->usFeatureTotalDim
                                       : g_usNbNetInputSize[0]);

    for (ii = 0; ii < pstModelInfo->ucNetLayers; ii++)
    {
        pstModelInfo->pNets[ii].usInputSize = g_usNbNetInputSize[ii];
        pstModelInfo->pNets[ii].usHiddenSize = g_usNbNetHiddenSize[ii];
        pstModelInfo->pNets[ii].usOutputSize = g_usNbNetOutputSize[ii];
        pstModelInfo->pNets[ii].usMemBlock = g_usNbNetMemBlocks[ii];
        pstModelInfo->pNets[ii].ucAction = g_usNbNetAction[ii];
        pstModelInfo->pNets[ii].ucWQ = pstModelInfo->ucWQ;
        pstModelInfo->pNets[ii].ucDataQ = pstModelInfo->ucDataQ;
    }

    //out
    if (pstModelInfo->ucOutLinears)
    {
        pstModelInfo->pOutLinears =
            (LINEAR_LAYER_s *)NN_CALLOC(pstModelInfo->ucOutLinears * sizeof(LINEAR_LAYER_s));

        if (NULL == pstModelInfo->pOutLinears)
        {
            MSG_DBG("%s [%d] mem alloc fail\r\n", __func__, __LINE__);
            return -1;
        }

        for (ii = 0; ii < pstModelInfo->ucOutLinears; ii++)
        {
            pstModelInfo->pOutLinears[ii].usInputSize = g_usNbOutDenseInputSize[ii];
            pstModelInfo->pOutLinears[ii].usOutputSize = g_usNbOutDenseOutputSize[ii];
            pstModelInfo->pOutLinears[ii].ucAction = g_usNbOutDenseAction[ii];
            pstModelInfo->pOutLinears[ii].ucWQ = pstModelInfo->ucWQ;
            pstModelInfo->pOutLinears[ii].ucDataQ = pstModelInfo->ucDataQ;
        }
    }
    else
    {
        pstModelInfo->pOutLinears = NULL;
    }

    return 0;
}

static S32 nnNbModelDataInit(NN_STATE_S *pstFsmn, SEVC_CONFIG_S *pstSevcCfg)
{
    S32 ii;
    NN_MODEL_S *pstModel = pstFsmn->pstModel;
    aisp_s32_t *pCmnBuffer;
    U16 usFeatureDim = pstSevcCfg->ucNnFeatureDim;
    U16 usFeatureContext = pstFsmn->usFeatureContext;
    U32 uiOffset0, uiOffset1, uiOffset2, uiOffset3;
    U32 uiSize;
#if 0
    int size = 0;
    size = sizeof(iInDenseLayerBias) + sizeof(cInDenseLayerWeight)
           + sizeof(iFsmnLayerBias) + sizeof(cFsmnLayerW1) + sizeof(cFsmnLayerW2) + sizeof(cFsmnLayerH)
           + sizeof(iOutDenseLayerBias) + sizeof(cOutDenseLayerWeight);
    MSG_DBG("model size is %d\r\n", size);
#endif
    uiSize = usFeatureDim * usFeatureContext * sizeof(aisp_s32_t);
    AISP_TSL_memcpy(pstFsmn->piFeatureMean, g_iNbFeatureMean, uiSize);
    AISP_TSL_memcpy(pstFsmn->piFeatureVar, g_iNbFeatureVar, uiSize);
    // pstFsmn->piFeatureMean = (aisp_s32_t *)g_iNbFeatureMean;
    // pstFsmn->piFeatureVar = (aisp_s32_t *)g_iNbFeatureVar;

    if (pstSevcCfg->ucNnBarkSupport)
    {
        pstFsmn->psFull2BarkMapping = g_usNbFull2BarkMapping;
    }

    //dataShow16((aisp_s16_t*)pstFsmn->psFull2BarkMapping, 64);

    if (pstModel->iInDenseLayers)
    {
        uiOffset0 = uiOffset1 = 0;

        for (ii = 0; ii < pstModel->iInDenseLayers; ii++)
        {
            pstModel->ppInFcLayer[ii]->pfBias = (aisp_s32_t *)g_iNbInDenseLayerBias + uiOffset0;
            pstModel->ppInFcLayer[ii]->pfInputWeights =
                (aisp_s8_t *)(g_cNbInDenseLayerWeight + uiOffset1);
            uiOffset0 += pstModel->ppInFcLayer[ii]->iOutputs;
            uiOffset1 += pstModel->ppInFcLayer[ii]->iOutputs
                         * pstModel->ppInFcLayer[ii]->iInputs;
        }
    }

    if (pstModel->iFsmnLayers)
    {
        uiOffset0 = uiOffset1 = uiOffset2 = uiOffset3 = 0;

        for (ii = 0; ii < pstModel->iFsmnLayers; ii++)
        {
            uiSize = pstModel->ppFsmnLayer[ii]->iHiddens * sizeof(aisp_s32_t);
            AISP_TSL_memcpy(pstModel->ppFsmnLayer[ii]->pfInputBias, (aisp_s32_t *)g_iNbFsmnLayerBias + uiOffset0, uiSize);
            uiSize = pstModel->ppFsmnLayer[ii]->iHiddens * pstModel->ppFsmnLayer[ii]->iInputs * sizeof(aisp_s8_t);
            AISP_TSL_memcpy(pstModel->ppFsmnLayer[ii]->pfInputWeights, (aisp_s8_t *)(g_cNbFsmnLayerW1 + uiOffset1), uiSize);
            uiSize = pstModel->ppFsmnLayer[ii]->iHiddens * pstModel->ppFsmnLayer[ii]->iOutputs * sizeof(aisp_s8_t);
            AISP_TSL_memcpy(pstModel->ppFsmnLayer[ii]->pfWeights, (aisp_s8_t *)(g_cNbFsmnLayerW2 + uiOffset2), uiSize);
            uiSize = pstModel->ppFsmnLayer[ii]->iOutputs * pstModel->ppFsmnLayer[ii]->usHleftTaps * sizeof(aisp_s8_t);
            AISP_TSL_memcpy(pstModel->ppFsmnLayer[ii]->pfH, (aisp_s8_t *)(g_cNbFsmnLayerH + uiOffset3), uiSize);
            // pstModel->ppFsmnLayer[ii]->pfInputBias = (aisp_s32_t *)g_iNbFsmnLayerBias + uiOffset0;
            // pstModel->ppFsmnLayer[ii]->pfInputWeights = (aisp_s8_t *)(g_cNbFsmnLayerW1 + uiOffset1);
            // pstModel->ppFsmnLayer[ii]->pfWeights = (aisp_s8_t *)(g_cNbFsmnLayerW2 + uiOffset2);
            // pstModel->ppFsmnLayer[ii]->pfH = (aisp_s8_t *)(g_cNbFsmnLayerH + uiOffset3);
            uiOffset0 += pstModel->ppFsmnLayer[ii]->iHiddens;
            uiOffset1 += pstModel->ppFsmnLayer[ii]->iHiddens
                         * pstModel->ppFsmnLayer[ii]->iInputs;
            uiOffset2 += pstModel->ppFsmnLayer[ii]->iOutputs
                         * pstModel->ppFsmnLayer[ii]->iHiddens;
            uiOffset3 += pstModel->ppFsmnLayer[ii]->iOutputs
                         * pstModel->ppFsmnLayer[ii]->usHleftTaps;
        }
    }

    if (pstModel->iOutDenseLayers)
    {
        uiOffset0 = uiOffset1 = 0;

        for (ii = 0; ii < pstModel->iOutDenseLayers; ii++)
        {
            pstModel->ppOutFcLayer[ii]->pfBias = (aisp_s32_t *)g_iNbOutDenseLayerBias + uiOffset0;
            pstModel->ppOutFcLayer[ii]->pfInputWeights =
                (aisp_s8_t *)(g_cNbOutDenseLayerWeight + uiOffset1);
            uiOffset0 += pstModel->ppOutFcLayer[ii]->iOutputs;
            uiOffset1 += pstModel->ppOutFcLayer[ii]->iOutputs
                         * pstModel->ppOutFcLayer[ii]->iInputs;
        }
    }

    //if (pstNnCfg->use_smooth_cmn_flag)
    if (g_ucNbCmnSupport)
    {
        S32 jj;

        for (ii = 0; ii < g_ucNbCmnLen + 1; ii++)
        {
            pCmnBuffer = pstFsmn->ppCmnBuffer[ii];

            for (jj = 0; jj < usFeatureDim; jj++)
            {
                pCmnBuffer[jj] = -pstFsmn->piFeatureMean[jj];
            }
        }

        for (jj = 0; jj < usFeatureDim; jj++)
        {
            pstFsmn->pCmnFeatureSum[jj] = -1 * (g_ucNbCmnLen + 1) * pstFsmn->piFeatureMean[jj];
        }
    }

    return 0;
}

static S32 nnModelInfoInit(NN_MODEL_INFO_S *pstModelInfo, U32 uiSamplRate)
{
    S32 ii;

    if (8000 == uiSamplRate)
    {
        return nnNbModelInfoInit(pstModelInfo);
    }

#if 1
    /* Update FSMN model info. */
    pstModelInfo->ucWQ = g_ucFsmnWQ;
    pstModelInfo->ucDataQ = g_ucFsmnDataQ;
    pstModelInfo->ucNBBands = g_ucNBBands;
    pstModelInfo->ucInLinears = g_ucInLinears;
    pstModelInfo->ucNetLayers = g_ucNetLayerNum;
    pstModelInfo->ucOutLinears = g_ucOutLinears;
    pstModelInfo->usFeatureTotalDim = 0;

    //input
    if (pstModelInfo->ucInLinears)
    {
        pstModelInfo->pInLinears =
            (LINEAR_LAYER_s *)NN_CALLOC(pstModelInfo->ucInLinears * sizeof(LINEAR_LAYER_s));

        if (NULL == pstModelInfo->pInLinears)
        {
            MSG_DBG("%s [%d] mem alloc fail\r\n", __func__, __LINE__);
            return -1;
        }

        pstModelInfo->usFeatureTotalDim = g_usInDenseInputSize[0];

        for (ii = 0; ii < pstModelInfo->ucInLinears; ii++)
        {
            pstModelInfo->pInLinears[ii].usInputSize = g_usInDenseInputSize[ii];
            pstModelInfo->pInLinears[ii].usOutputSize = g_usInDenseOutputSize[ii];
            pstModelInfo->pInLinears[ii].ucAction = g_usInDenseAction[ii];
            pstModelInfo->pInLinears[ii].ucWQ = pstModelInfo->ucWQ;
            pstModelInfo->pInLinears[ii].ucDataQ = pstModelInfo->ucDataQ;
        }
    }
    else
    {
        pstModelInfo->pInLinears = NULL;
    }

    //fsmn
    pstModelInfo->pNets = (FSMN_LAYER_s *)NN_CALLOC(pstModelInfo->ucNetLayers * sizeof(FSMN_LAYER_s));

    if (NULL == pstModelInfo->pNets)
    {
        MSG_DBG("%s [%d] mem alloc fail\r\n", __func__, __LINE__);
        return -1;
    }

    pstModelInfo->usFeatureTotalDim = (pstModelInfo->usFeatureTotalDim ?
                                       pstModelInfo->usFeatureTotalDim
                                       : g_usNetInputSize[0]);

    for (ii = 0; ii < pstModelInfo->ucNetLayers; ii++)
    {
        pstModelInfo->pNets[ii].usInputSize = g_usNetInputSize[ii];
        pstModelInfo->pNets[ii].usHiddenSize = g_usNetHiddenSize[ii];
        pstModelInfo->pNets[ii].usOutputSize = g_usNetOutputSize[ii];
        pstModelInfo->pNets[ii].usMemBlock = g_usNetMemBlocks[ii];
        pstModelInfo->pNets[ii].ucAction = g_usNetAction[ii];
        pstModelInfo->pNets[ii].ucWQ = pstModelInfo->ucWQ;
        pstModelInfo->pNets[ii].ucDataQ = pstModelInfo->ucDataQ;
    }

    //out
    if (pstModelInfo->ucOutLinears)
    {
        pstModelInfo->pOutLinears =
            (LINEAR_LAYER_s *)NN_CALLOC(pstModelInfo->ucOutLinears * sizeof(LINEAR_LAYER_s));

        if (NULL == pstModelInfo->pOutLinears)
        {
            MSG_DBG("%s [%d] mem alloc fail\r\n", __func__, __LINE__);
            return -1;
        }

        for (ii = 0; ii < pstModelInfo->ucOutLinears; ii++)
        {
            pstModelInfo->pOutLinears[ii].usInputSize = g_usOutDenseInputSize[ii];
            pstModelInfo->pOutLinears[ii].usOutputSize = g_usOutDenseOutputSize[ii];
            pstModelInfo->pOutLinears[ii].ucAction = g_usOutDenseAction[ii];
            pstModelInfo->pOutLinears[ii].ucWQ = pstModelInfo->ucWQ;
            pstModelInfo->pOutLinears[ii].ucDataQ = pstModelInfo->ucDataQ;
        }
    }
    else
    {
        pstModelInfo->pOutLinears = NULL;
    }

#endif
    return 0;
}

static VOID nnModelInfoFree(NN_MODEL_INFO_S *pstModelInfo)
{
    if (pstModelInfo->pInLinears != NULL)
    {
        NN_FREE(pstModelInfo->pInLinears);
        pstModelInfo->pInLinears = NULL;
    }

    if (pstModelInfo->pNets != NULL)
    {
        NN_FREE(pstModelInfo->pNets);
        pstModelInfo->pNets = NULL;
    }

    if (pstModelInfo->pOutLinears != NULL)
    {
        NN_FREE(pstModelInfo->pOutLinears);
        pstModelInfo->pOutLinears = NULL;
    }
}

static S32 nnModelDataInit(NN_STATE_S *pstFsmn, SEVC_CONFIG_S *pstSevcCfg)
{
    S32 ii;
    NN_MODEL_S *pstModel = pstFsmn->pstModel;
    aisp_s32_t *pCmnBuffer;
    U16 usFeatureDim = pstSevcCfg->ucNnFeatureDim;
    U16 usFeatureContext = pstFsmn->usFeatureContext;
    U32 uiOffset0, uiOffset1, uiOffset2, uiOffset3;
    U32 uiSize;
#if 0
    int size = 0;
    size = sizeof(iInDenseLayerBias) + sizeof(cInDenseLayerWeight)
           + sizeof(iFsmnLayerBias) + sizeof(cFsmnLayerW1) + sizeof(cFsmnLayerW2) + sizeof(cFsmnLayerH)
           + sizeof(iOutDenseLayerBias) + sizeof(cOutDenseLayerWeight);
    MSG_DBG("model size is %d\r\n", size);
#endif

    if (8000 == pstSevcCfg->fs)
    {
        return nnNbModelDataInit(pstFsmn, pstSevcCfg);
    }

    uiSize = usFeatureContext * usFeatureDim * sizeof(aisp_s32_t);
    AISP_TSL_memcpy(pstFsmn->piFeatureMean, g_iFeatureMean, uiSize);
    AISP_TSL_memcpy(pstFsmn->piFeatureVar, g_iFeatureVar, uiSize);
    // pstFsmn->piFeatureMean = (aisp_s32_t *)g_iFeatureMean;
    // pstFsmn->piFeatureVar = (aisp_s32_t *)g_iFeatureVar;

    if (pstSevcCfg->ucNnBarkSupport)
    {
        pstFsmn->psFull2BarkMapping = g_usFull2BarkMapping;
    }

    //dataShow16((aisp_s16_t*)pstFsmn->psFull2BarkMapping, 64);

    if (pstModel->iInDenseLayers)
    {
        uiOffset0 = uiOffset1 = 0;

        for (ii = 0; ii < pstModel->iInDenseLayers; ii++)
        {
            pstModel->ppInFcLayer[ii]->pfBias = (aisp_s32_t *)g_iInDenseLayerBias + uiOffset0;
            pstModel->ppInFcLayer[ii]->pfInputWeights =
                (aisp_s8_t *)(g_cInDenseLayerWeight + uiOffset1);
            uiOffset0 += pstModel->ppInFcLayer[ii]->iOutputs;
            uiOffset1 += pstModel->ppInFcLayer[ii]->iOutputs
                         * pstModel->ppInFcLayer[ii]->iInputs;
        }
    }

    if (pstModel->iFsmnLayers)
    {
        uiOffset0 = uiOffset1 = uiOffset2 = uiOffset3 = 0;

        for (ii = 0; ii < pstModel->iFsmnLayers; ii++)
        {
            uiSize = pstModel->ppFsmnLayer[ii]->iHiddens * sizeof(aisp_s32_t);
            AISP_TSL_memcpy(pstModel->ppFsmnLayer[ii]->pfInputBias, (aisp_s32_t *)g_iFsmnLayerBias + uiOffset0, uiSize);
            uiSize = pstModel->ppFsmnLayer[ii]->iHiddens * pstModel->ppFsmnLayer[ii]->iInputs * sizeof(aisp_s8_t);
            AISP_TSL_memcpy(pstModel->ppFsmnLayer[ii]->pfInputWeights, (aisp_s8_t *)(g_cFsmnLayerW1 + uiOffset1), uiSize);
            uiSize = pstModel->ppFsmnLayer[ii]->iHiddens * pstModel->ppFsmnLayer[ii]->iOutputs * sizeof(aisp_s8_t);
            AISP_TSL_memcpy(pstModel->ppFsmnLayer[ii]->pfWeights, (aisp_s8_t *)(g_cFsmnLayerW2 + uiOffset2), uiSize);
            uiSize = pstModel->ppFsmnLayer[ii]->iOutputs * pstModel->ppFsmnLayer[ii]->usHleftTaps * sizeof(aisp_s8_t);
            AISP_TSL_memcpy(pstModel->ppFsmnLayer[ii]->pfH, (aisp_s8_t *)(g_cFsmnLayerH + uiOffset3), uiSize);
            // pstModel->ppFsmnLayer[ii]->pfInputBias = (aisp_s32_t *)g_iFsmnLayerBias + uiOffset0;
            // pstModel->ppFsmnLayer[ii]->pfInputWeights = (aisp_s8_t *)(g_cFsmnLayerW1 + uiOffset1);
            // pstModel->ppFsmnLayer[ii]->pfWeights = (aisp_s8_t *)(g_cFsmnLayerW2 + uiOffset2);
            // pstModel->ppFsmnLayer[ii]->pfH = (aisp_s8_t *)(g_cFsmnLayerH + uiOffset3);
            uiOffset0 += pstModel->ppFsmnLayer[ii]->iHiddens;
            uiOffset1 += pstModel->ppFsmnLayer[ii]->iHiddens
                         * pstModel->ppFsmnLayer[ii]->iInputs;
            uiOffset2 += pstModel->ppFsmnLayer[ii]->iOutputs
                         * pstModel->ppFsmnLayer[ii]->iHiddens;
            uiOffset3 += pstModel->ppFsmnLayer[ii]->iOutputs
                         * pstModel->ppFsmnLayer[ii]->usHleftTaps;
        }
    }

    if (pstModel->iOutDenseLayers)
    {
        uiOffset0 = uiOffset1 = 0;

        for (ii = 0; ii < pstModel->iOutDenseLayers; ii++)
        {
            pstModel->ppOutFcLayer[ii]->pfBias = (aisp_s32_t *)g_iOutDenseLayerBias + uiOffset0;
            pstModel->ppOutFcLayer[ii]->pfInputWeights =
                (aisp_s8_t *)(g_cOutDenseLayerWeight + uiOffset1);
            uiOffset0 += pstModel->ppOutFcLayer[ii]->iOutputs;
            uiOffset1 += pstModel->ppOutFcLayer[ii]->iOutputs
                         * pstModel->ppOutFcLayer[ii]->iInputs;
        }
    }

    //if (pstNnCfg->use_smooth_cmn_flag)
    if (g_ucCmnSupport)
    {
        S32 jj;

        for (ii = 0; ii < g_ucCmnLen + 1; ii++)
        {
            pCmnBuffer = pstFsmn->ppCmnBuffer[ii];

            for (jj = 0; jj < usFeatureDim; jj++)
            {
                pCmnBuffer[jj] = -pstFsmn->piFeatureMean[jj];
            }
        }

        for (jj = 0; jj < usFeatureDim; jj++)
        {
            pstFsmn->pCmnFeatureSum[jj] = -1 * (g_ucCmnLen + 1) * pstFsmn->piFeatureMean[jj];
        }
    }

    return 0;
}

#endif
S32 nnModelDataLoad(NN_STATE_S *pstFsmn, NN_MODEL_INFO_S *pstModelInfo)
{
    return 0;
}

U32 nnFsmnLayerMemSizeGet(FSMN_LAYER_s *fsmnLayerInfo)
{
    U32 uiTotalMemSize = 0;
    U16 usFsmnFilterLen = fsmnLayerInfo->usMemBlock;
    U16 usFsmnW2Out = fsmnLayerInfo->usOutputSize;
    // allocate for fsmn layer struct
    uiTotalMemSize += SIZE_ALIGN(sizeof(FSMN_LAYER_S));
    // allocate for w1 weights
    uiTotalMemSize += SIZE_ALIGN(fsmnLayerInfo->usInputSize * fsmnLayerInfo->usHiddenSize * sizeof(aisp_s8_t));
    // allocate for bias
    uiTotalMemSize += SIZE_ALIGN(fsmnLayerInfo->usHiddenSize * sizeof(aisp_s32_t));
    // allocate for hidden weights
    uiTotalMemSize += SIZE_ALIGN(fsmnLayerInfo->usHiddenSize * usFsmnW2Out * sizeof(aisp_s8_t));
    // allocate for cached data
    uiTotalMemSize += SIZE_ALIGN(usFsmnFilterLen    * sizeof(VOID *));
    uiTotalMemSize += usFsmnFilterLen
                      * SIZE_ALIGN(usFsmnW2Out * sizeof(aisp_s32_t));
    // allocate for filter
    uiTotalMemSize += SIZE_ALIGN(usFsmnFilterLen * usFsmnW2Out * sizeof(aisp_s8_t));
    // allocate for output for the fsmn layer
    uiTotalMemSize += SIZE_ALIGN(usFsmnW2Out * sizeof(aisp_s32_t));
#if MEM_BUFF_DBG
    MSG_DBG("%s uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}

U32 nnFcLayerMemSizeGet(U32 uiInputs, U32 uiNeurous)
{
    U32 uiTotalMemSize = 0;
    /* structure */
    uiTotalMemSize += SIZE_ALIGN(sizeof(FC_LAYER_S));
#if MEM_BUFF_DBG
    MSG_DBG("%s uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}

U32 nnModelSharedMemSizeGet(NN_MODEL_INFO_S *pDfsmnModelInfo)
{
    U32 uiTotalMemSize = 0;
    S32 ii;
    S32 iSize;

    if (pDfsmnModelInfo->ucNetLayers)
    {
        for (ii = 0; ii < pDfsmnModelInfo->ucNetLayers; ii++)
        {
            iSize  = (pDfsmnModelInfo->pNets[ii].usOutputSize > pDfsmnModelInfo->pNets[ii].usHiddenSize ?
                      pDfsmnModelInfo->pNets[ii].usOutputSize : pDfsmnModelInfo->pNets[ii].usHiddenSize);
            iSize *= sizeof(aisp_s32_t);
            uiTotalMemSize = (uiTotalMemSize > iSize ? uiTotalMemSize : iSize);
        }
    }

    uiTotalMemSize = SIZE_ALIGN(uiTotalMemSize);
#if MEM_BUFF_DBG
    MSG_DBG("%s uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}

U32 nnModelMemSizeGet(NN_MODEL_INFO_S *pDfsmnModelInfo)
{
    U32 uiTotalMemSize = 0;
    S32 ii;
    S32 iSize;//, iSize2 = 0;
    uiTotalMemSize += SIZE_ALIGN(sizeof(NN_MODEL_S));

    if (pDfsmnModelInfo->ucInLinears)
    {
        iSize = pDfsmnModelInfo->ucInLinears * sizeof(VOID *);
        uiTotalMemSize += SIZE_ALIGN(iSize);
        //ppfInFcOutput
        uiTotalMemSize += SIZE_ALIGN(iSize);

        for (ii = 0; ii < pDfsmnModelInfo->ucNetLayers; ii++)
        {
            uiTotalMemSize += nnFcLayerMemSizeGet(
                                  pDfsmnModelInfo->pInLinears[ii].usInputSize,
                                  pDfsmnModelInfo->pInLinears[ii].usOutputSize);
        }

        for (ii = 0; ii < pDfsmnModelInfo->ucInLinears; ii++)
        {
            uiTotalMemSize += SIZE_ALIGN(pDfsmnModelInfo->pInLinears[ii].usOutputSize
                                         * sizeof(aisp_s32_t));
        }
    }

    if (pDfsmnModelInfo->ucNetLayers)
    {
        uiTotalMemSize += SIZE_ALIGN(pDfsmnModelInfo->ucNetLayers * sizeof(VOID *));

        for (ii = 0; ii < pDfsmnModelInfo->ucNetLayers; ii++)
        {
            uiTotalMemSize += nnFsmnLayerMemSizeGet(&(pDfsmnModelInfo->pNets[ii]));
        }
    }

    if (pDfsmnModelInfo->ucOutLinears)
    {
        iSize = pDfsmnModelInfo->ucOutLinears * sizeof(VOID *);
        uiTotalMemSize += SIZE_ALIGN(iSize);
        //ppfOutFcOutput
        uiTotalMemSize += SIZE_ALIGN(iSize);

        for (ii = 0; ii < pDfsmnModelInfo->ucOutLinears; ii++)
        {
            uiTotalMemSize += nnFcLayerMemSizeGet(
                                  pDfsmnModelInfo->pOutLinears[ii].usInputSize,
                                  pDfsmnModelInfo->pOutLinears[ii].usOutputSize);
        }

        for (ii = 0; ii < pDfsmnModelInfo->ucOutLinears; ii++)
        {
            uiTotalMemSize += SIZE_ALIGN(pDfsmnModelInfo->pOutLinears[ii].usOutputSize
                                         * sizeof(aisp_s32_t));
        }
    }

#ifndef USE_SHARE_MEM
    uiTotalMemSize += nnModelSharedMemSizeGet(pDfsmnModelInfo);
#endif
#if MEM_BUFF_DBG
    MSG_DBG("%s uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}

U32 nnProcessorSharedMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    //S32 iRet = 0;
    U32 uiSize = 0;
    U32 uiTotalMemSize = 0;
    U16 usFeatureDim = 0;
    U16 usFeatureContext;
    NN_MODEL_INFO_S dfsmnModelInfo;
    nnModelInfoInit(&dfsmnModelInfo, pstSevcCfg->fs);
    usFeatureDim = dfsmnModelInfo.ucNBBands;

    if (8000 == pstSevcCfg->fs)
    {
        usFeatureContext = g_usNbFeatureContextL + g_usNbFeatureContextR + 1;
        pstSevcCfg->usFeatureContextR = g_usNbFeatureContextR;
    }
    else
    {
        usFeatureContext = g_usFeatureContextL + g_usFeatureContextR + 1;
        pstSevcCfg->usFeatureContextR = g_usFeatureContextR;
    }

#if SEVC_SWITCH_NN_AES
    //usFeatureDim *= 2;
    usFeatureDim += pstSevcCfg->usAesRefBinCount;

    if (usFeatureDim * usFeatureContext != dfsmnModelInfo.usFeatureTotalDim)
    {
        MSG_DBG("%s cfg and resource error %d %d %d %d\r\n", __func__,
                dfsmnModelInfo.ucNBBands, pstSevcCfg->usAesRefBinCount,
                usFeatureContext, dfsmnModelInfo.usFeatureTotalDim);
        nnModelInfoFree(&dfsmnModelInfo);
        return 0;
    }

#endif

    if (pstSevcCfg->ucNnBarkSupport)
    {
#ifdef BAND_ENERGY_SMOOTH
        /* pfNbBandsSumTmp */
        uiSize = usFeatureDim * sizeof(aisp_s64_t);
        uiTotalMemSize += SIZE_ALIGN(uiSize);
#else
        /* pfNbBandsSum */
        uiSize = usFeatureDim * sizeof(aisp_s64_t);
        uiTotalMemSize += SIZE_ALIGN(uiSize);
#endif
        /* pfFeatures */
        uiSize = usFeatureContext * usFeatureDim * sizeof(aisp_s32_t);
        uiTotalMemSize += SIZE_ALIGN(uiSize);
    }

#ifdef USE_SHARE_MEM
    //uiSize = nnModelSharedMemSizeGet(&dfsmnModelInfo);
    //uiTotalMemSize = NN_MAX(uiTotalMemSize, uiSize);
    uiTotalMemSize += nnModelSharedMemSizeGet(&dfsmnModelInfo);
#endif
    nnModelInfoFree(&dfsmnModelInfo);
    //MSG_DBG("%s [%d] uiTotalMemSize=%d\r\n", __func__, __LINE__, uiTotalMemSize);
    return uiTotalMemSize;
}

U32 nnProcessorMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    NN_MODEL_INFO_S dfsmnModelInfo;
    S32 iRet = 0;
    U32 uiTotalMemSize = 0;
    U16 usFeatureDim = 0;
    U16 usFeatureContext;
    U8 ucCmnSupport = 0, ucCmnLen;
    iRet = nnModelInfoInit(&dfsmnModelInfo, pstSevcCfg->fs);

    if (0 != iRet)
    {
        return 0;
    }

    if (8000 == pstSevcCfg->fs)
    {
        usFeatureContext = g_usNbFeatureContextL + g_usNbFeatureContextR + 1;
        ucCmnSupport = g_ucNbCmnSupport;
        ucCmnLen = g_ucNbCmnLen;
        pstSevcCfg->usFeatureContextR = g_usNbFeatureContextR;
    }
    else
    {
        usFeatureContext = g_usFeatureContextL + g_usFeatureContextR + 1;
        ucCmnSupport = g_ucCmnSupport;
        ucCmnLen = g_ucCmnLen;
        pstSevcCfg->usFeatureContextR = g_usFeatureContextR;
    }

    usFeatureDim = dfsmnModelInfo.ucNBBands;
#if SEVC_SWITCH_NN_AES
    //usFeatureDim *= 2;
    usFeatureDim += pstSevcCfg->usAesRefBinCount;

    if (usFeatureDim * usFeatureContext != dfsmnModelInfo.usFeatureTotalDim)
    {
        MSG_DBG("%s cfg and resource error %d %d %d %d\r\n", __func__,
                dfsmnModelInfo.ucNBBands, pstSevcCfg->usAesRefBinCount,
                usFeatureContext, dfsmnModelInfo.usFeatureTotalDim);
        nnModelInfoFree(&dfsmnModelInfo);
        return 0;
    }

#endif
    uiTotalMemSize += SIZE_ALIGN(sizeof(NN_STATE_S));

    if (pstSevcCfg->ucNnBarkSupport)
    {
#ifndef AISPEECH_FIXED
        /* psFull2BarkMapping */
        uiTotalMemSize += SIZE_ALIGN(dfsmnModelInfo.ucNBBands * sizeof(U16));
#endif
#ifdef BAND_ENERGY_SMOOTH
        /* pfNbBandsSum */
        uiTotalMemSize += SIZE_ALIGN(usFeatureDim * sizeof(aisp_s64_t));
#endif
        /* pfFeatures */
        //uiTotalMemSize += SIZE_ALIGN(usFeatureContext * usFeatureDim * sizeof(aisp_s32_t));
        //pfFeaturesHistory
        uiTotalMemSize += SIZE_ALIGN(usFeatureContext * usFeatureDim * sizeof(aisp_s32_t));
#ifdef NN_FSMN
        // allocate for feature mean
        uiTotalMemSize += SIZE_ALIGN(usFeatureContext * usFeatureDim * sizeof(aisp_s32_t));
        // allocate for feature variance
        uiTotalMemSize += SIZE_ALIGN(usFeatureContext * usFeatureDim * sizeof(aisp_s32_t));
#endif

        if (ucCmnSupport)
        {
            //pCmnFeatureSum
            uiTotalMemSize += SIZE_ALIGN(usFeatureDim * sizeof(aisp_s64_t));
            //pCmnBuffer
            uiTotalMemSize += SIZE_ALIGN((ucCmnLen + 1) * sizeof(VOID *));
            uiTotalMemSize += (ucCmnLen + 1) * SIZE_ALIGN(usFeatureDim * sizeof(aisp_s32_t));
        }
    }
    else
    {
        /* pfFeatures */
        uiTotalMemSize += SIZE_ALIGN(usFeatureContext * pstSevcCfg->usFftBin
                                     * sizeof(aisp_s32_t));
    }

#ifndef AISPEECH_FIXED
    /* piFeatureMean */
    uiTotalMemSize += SIZE_ALIGN(dfsmnModelInfo.usFsmnW1In[0] * sizeof(aisp_s32_t));
    /* piFeatureVar */
    uiTotalMemSize += SIZE_ALIGN(dfsmnModelInfo.usFsmnW1In[0] * sizeof(aisp_s32_t));
#endif
    /* pstModel */
    uiTotalMemSize += nnModelMemSizeGet(&dfsmnModelInfo);
#ifndef USE_SHARE_MEM
    uiTotalMemSize += nnProcessorSharedMemSizeGet(pstSevcCfg);
#endif
#if MEM_BUFF_DBG
    MSG_DBG("%s uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
    nnModelInfoFree(&dfsmnModelInfo);
    return uiTotalMemSize;
}

FC_LAYER_S *nnFcLayerNew(LAYER_INFO_S *pstLayerInfo, MEM_LOCATOR_S *pstMemLocator)
{
    FC_LAYER_S *pstDenoiseLayer = NULL;
    U32 uiSize = 0;
    MEM_LOCATOR_SHOW(pstMemLocator);
    uiSize = sizeof(FC_LAYER_S);
    pstDenoiseLayer = (FC_LAYER_S *)memLocatorGet(pstMemLocator, uiSize, ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstDenoiseLayer, FAIL);
    pstDenoiseLayer->iInputs = pstLayerInfo->iInputs;
    pstDenoiseLayer->iOutputs = pstLayerInfo->iOutputs;
    pstDenoiseLayer->iActivation = pstLayerInfo->iActivation;
    pstDenoiseLayer->ucWQ = pstLayerInfo->ucWQ;
    pstDenoiseLayer->ucDataQ = pstLayerInfo->ucDataQ;
#ifndef AISPEECH_FIXED
    /* bias */
    uiSize = pstDenoiseLayer->iOutputs * sizeof(aisp_s32_t);
    BUFF_POINT_SET_ALIGN(pstDenoiseLayer->pfBias, uiSize, pstMemLocator, FAIL);
    /* input_weights */
    uiSize = pstDenoiseLayer->iInputs * pstDenoiseLayer->iOutputs * sizeof(aisp_s16_t);
    BUFF_POINT_SET_ALIGN(pstDenoiseLayer->pfInputWeights, uiSize, pstMemLocator, FAIL);
#endif
    MEM_LOCATOR_SHOW(pstMemLocator);
    return pstDenoiseLayer;
FAIL:
    return NULL;
}


FSMN_LAYER_S *nnFsmnLayerNew(FSMN_LAYER_INFO_S *pstFsmnLayerInfo, MEM_LOCATOR_S *pstMemLocator)
{
    FSMN_LAYER_S *pstFsmnLayer = NULL;
    S32 usTapIdx;
    U32 uiSize = 0;
    U32 uiSizeTotal = 0;
    uiSize = sizeof(FSMN_LAYER_S);
    uiSizeTotal += uiSize;
    pstFsmnLayer = (FSMN_LAYER_S *)memLocatorGet(pstMemLocator, uiSize, ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstFsmnLayer, FAIL);
    pstFsmnLayer->iInputs = pstFsmnLayerInfo->iInputs;
    pstFsmnLayer->iHiddens = pstFsmnLayerInfo->iHiddens;
    pstFsmnLayer->usHleftTaps = pstFsmnLayerInfo->usHleftTaps;
    pstFsmnLayer->iOutputs = pstFsmnLayerInfo->iOutputs;
    pstFsmnLayer->ucWQ = pstFsmnLayerInfo->ucWQ;
    pstFsmnLayer->ucDataQ = pstFsmnLayerInfo->ucDataQ;
    //=====================================
    uiSizeTotal += pstFsmnLayerInfo->iOutputs * sizeof(aisp_s32_t);
    /* ppfHleft */
    uiSizeTotal += pstFsmnLayerInfo->usHleftTaps * sizeof(VOID *);
    //uiSize = pstFsmnLayerInfo->iOutputs * pstFsmnLayerInfo->usHleftTaps  * sizeof(aisp_s32_t);
    BUFF_POINT_SET_ALIGN(pstFsmnLayer->ppfHleft,
                         pstFsmnLayerInfo->usHleftTaps * sizeof(VOID *),
                         pstMemLocator, FAIL);

    for (usTapIdx = 0; usTapIdx < pstFsmnLayerInfo->usHleftTaps; usTapIdx++)
    {
        BUFF_POINT_SET_ALIGN(pstFsmnLayer->ppfHleft[usTapIdx],
                             (pstFsmnLayerInfo->iOutputs * sizeof(aisp_s32_t)),
                             pstMemLocator, FAIL);
        uiSizeTotal += pstFsmnLayerInfo->iOutputs * sizeof(aisp_s32_t);
    }

    // allocate for input weights
    uiSize = pstFsmnLayer->iInputs * pstFsmnLayer->iHiddens * sizeof(aisp_s8_t);
    BUFF_POINT_SET_ALIGN(pstFsmnLayer->pfInputWeights, uiSize, pstMemLocator, FAIL);
    // allocate for input bias
    uiSize = pstFsmnLayer->iHiddens * sizeof(aisp_s32_t);
    BUFF_POINT_SET_ALIGN(pstFsmnLayer->pfInputBias, uiSize, pstMemLocator, FAIL);
    // allocate for hidden weights
    uiSize = pstFsmnLayer->iHiddens * pstFsmnLayer->iOutputs * sizeof(aisp_s8_t);
    BUFF_POINT_SET_ALIGN(pstFsmnLayer->pfWeights, uiSize, pstMemLocator, FAIL);
    // allocate for filter weights
    uiSize = pstFsmnLayer->iOutputs * pstFsmnLayer->usHleftTaps * sizeof(aisp_s8_t);
    BUFF_POINT_SET_ALIGN(pstFsmnLayer->pfH, uiSize, pstMemLocator, FAIL);
    uiSizeTotal += uiSize;
    /* pfFsmnOutput */
    uiSize = pstFsmnLayerInfo->iOutputs * sizeof(aisp_s32_t);
    uiSizeTotal += uiSize;
    BUFF_POINT_SET_ALIGN(pstFsmnLayer->pfFsmnOutput, uiSize, pstMemLocator, FAIL);
    return pstFsmnLayer;
FAIL:
    return NULL;
}


NN_MODEL_S *nnModelNew(NN_MODEL_INFO_S *pstModelInfo, MEM_LOCATOR_S *pstMemLocator)
{
    NN_MODEL_S *pstModel;
    U32 uiSize, uiSize2 = 0;
    LAYER_INFO_S layer;
    FSMN_LAYER_INFO_S fsmnlayer;
    S32 ii;

    if (NULL == pstModelInfo || NULL == pstMemLocator)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
    }

    MEM_LOCATOR_SHOW(pstMemLocator);
    uiSize = sizeof(NN_MODEL_S);
    pstModel = (NN_MODEL_S *)memLocatorGet(pstMemLocator, uiSize, ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstModel, FAIL);
    pstModel->iInDenseLayers = pstModelInfo->ucInLinears;
    pstModel->iFsmnLayers = pstModelInfo->ucNetLayers;
    pstModel->iOutDenseLayers = pstModelInfo->ucOutLinears;

    if (pstModelInfo->ucInLinears)
    {
        uiSize = pstModelInfo->ucInLinears * sizeof(VOID *);
        BUFF_POINT_SET_ALIGN(pstModel->ppInFcLayer, uiSize, pstMemLocator, FAIL);
        BUFF_POINT_SET_ALIGN(pstModel->ppfInFcOutput, uiSize, pstMemLocator, FAIL);

        for (ii = 0; ii < pstModelInfo->ucInLinears; ii++)
        {
            /* pDenoiseOutLayer */
            layer.iInputs = pstModelInfo->pInLinears[ii].usInputSize;
            layer.iOutputs = pstModelInfo->pInLinears[ii].usOutputSize;
            layer.iActivation = pstModelInfo->pInLinears[ii].ucAction;
            pstModel->ppInFcLayer[ii] = nnFcLayerNew(&layer, pstMemLocator);
            AISP_TSL_PTR_CHECK(pstModel->ppInFcLayer[ii], FAIL);
            uiSize = pstModelInfo->pInLinears[ii].usOutputSize * sizeof(aisp_s32_t);
            BUFF_POINT_SET_ALIGN(pstModel->ppfInFcOutput[ii],
                                 uiSize, pstMemLocator, FAIL);
        }
    }

    BUFF_POINT_SET_ALIGN(pstModel->ppFsmnLayer,
                         pstModelInfo->ucNetLayers * sizeof(VOID *),
                         pstMemLocator, FAIL);

    for (ii = 0; ii < pstModelInfo->ucNetLayers; ii++)
    {
        fsmnlayer.iInputs = pstModelInfo->pNets[ii].usInputSize;
        fsmnlayer.iHiddens = pstModelInfo->pNets[ii].usHiddenSize;
        fsmnlayer.iOutputs = pstModelInfo->pNets[ii].usOutputSize;
        fsmnlayer.usHleftTaps = pstModelInfo->pNets[ii].usMemBlock;
        fsmnlayer.ucWQ = pstModelInfo->pNets[ii].ucWQ;
        fsmnlayer.ucDataQ = pstModelInfo->pNets[ii].ucDataQ;
        pstModel->ppFsmnLayer[ii] = nnFsmnLayerNew(&fsmnlayer, pstMemLocator);
        AISP_TSL_PTR_CHECK(pstModel->ppFsmnLayer[ii], FAIL);
        uiSize  = (fsmnlayer.iOutputs > fsmnlayer.iHiddens ? fsmnlayer.iOutputs : fsmnlayer.iHiddens);
        uiSize *= sizeof(aisp_s32_t);
        uiSize2 = (uiSize2 > uiSize ? uiSize2 : uiSize);
    }

    if (uiSize2)
    {
        SH_BUFF_POINT_SET_ALIGN(pstModel->pcSharedMem, uiSize2, pstMemLocator, FAIL);
    }

    for (ii = 0; ii < pstModelInfo->ucNetLayers; ii++)
    {
        pstModel->ppFsmnLayer[ii]->pcSharedMem = pstModel->pcSharedMem;
    }

    if (pstModelInfo->ucOutLinears)
    {
        uiSize = pstModelInfo->ucOutLinears * sizeof(VOID *);
        BUFF_POINT_SET_ALIGN(pstModel->ppOutFcLayer, uiSize, pstMemLocator, FAIL);
        BUFF_POINT_SET_ALIGN(pstModel->ppfOutFcOutput, uiSize, pstMemLocator, FAIL);

        for (ii = 0; ii < pstModelInfo->ucOutLinears; ii++)
        {
            /* pDenoiseOutLayer */
            layer.iInputs = pstModelInfo->pOutLinears[ii].usInputSize;
            layer.iOutputs = pstModelInfo->pOutLinears[ii].usOutputSize;
            layer.iActivation = pstModelInfo->pOutLinears[ii].ucAction;
            layer.ucWQ       = pstModelInfo->pOutLinears[ii].ucWQ;
            layer.ucDataQ    = pstModelInfo->pOutLinears[ii].ucDataQ;
            pstModel->ppOutFcLayer[ii] = nnFcLayerNew(&layer, pstMemLocator);
            AISP_TSL_PTR_CHECK(pstModel->ppOutFcLayer[ii], FAIL);
            uiSize = pstModelInfo->pOutLinears[ii].usOutputSize * sizeof(aisp_s32_t);
            BUFF_POINT_SET_ALIGN(pstModel->ppfOutFcOutput[ii],
                                 uiSize, pstMemLocator, FAIL);
        }
    }

    MEM_LOCATOR_SHOW(pstMemLocator);

    if (pstModelInfo->ucOutLinears)
    {
        pstModel->pOutput = pstModel->ppfOutFcOutput[pstModelInfo->ucOutLinears - 1];
        pstModel->iOutLen = pstModel->ppOutFcLayer[pstModelInfo->ucOutLinears - 1]->iOutputs;
    }
    else
    {
        if (pstModelInfo->ucNetLayers)
        {
            pstModel->pOutput = pstModel->ppFsmnLayer[pstModelInfo->ucNetLayers - 1]->pfFsmnOutput;
            pstModel->iOutLen = pstModel->ppFsmnLayer[pstModelInfo->ucNetLayers - 1]->iOutputs;
        }
        else
        {
            if (pstModelInfo->ucInLinears)
            {
                pstModel->pOutput = pstModel->ppfInFcOutput[pstModelInfo->ucInLinears - 1];
                pstModel->iOutLen = pstModel->ppInFcLayer[pstModelInfo->ucInLinears - 1]->iOutputs;
            }
            else
            {
                MSG_DBG("%s [%d] ERROR\r\n", __func__, __LINE__);
                return NULL;
            }
        }
    }

    return pstModel;
FAIL:
    return NULL;
}


NN_STATE_S *nnProcessorNew(SEVC_CONFIG_S *pstSevcCfg, MEM_LOCATOR_S *pstMemLocator)
{
    S32 iRet = 0;
    S32 ii;
    U16 usSize;
    NN_STATE_S *pstDfsmn;
    NN_MODEL_INFO_S *pstNnModelInfo;
    S32 iCmnWinLen;
    U16 usFeatureDim;

    if (NULL == pstSevcCfg || NULL == pstMemLocator)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
    }

#ifdef BAND_ENERGY_SMOOTH
    pstSevcCfg->ucNnBandEnergySmooth = 1;
#endif
    MEM_LOCATOR_SHOW(pstMemLocator);
    usSize = sizeof(NN_STATE_S);
    pstDfsmn = (NN_STATE_S *)memLocatorGet(pstMemLocator, usSize, ALIGN_ON);
    AISP_TSL_PTR_CHECK(pstDfsmn, FAIL);
    iRet = nnModelInfoInit(&pstDfsmn->modelInfo, pstSevcCfg->fs);

    if (0 != iRet)
    {
        return NULL;
    }

    if (8000 == pstSevcCfg->fs)
    {
        //pstNnCfg->gain_floor = fGainFloor;
        pstDfsmn->ucCmnSupport = g_ucNbCmnSupport;
        pstDfsmn->ucCmnLen = g_ucNbCmnLen;
        pstDfsmn->usFeatureContextL = g_usNbFeatureContextL;
        pstDfsmn->usFeatureContextR = g_usNbFeatureContextR;
        pstDfsmn->usFeatureContext = g_usNbFeatureContextL + g_usNbFeatureContextR + 1;
    }
    else
    {
        //pstNnCfg->gain_floor = fGainFloor;
        pstDfsmn->ucCmnSupport = g_ucCmnSupport;
        pstDfsmn->ucCmnLen = g_ucCmnLen;
        pstDfsmn->usFeatureContextL = g_usFeatureContextL;
        pstDfsmn->usFeatureContextR = g_usFeatureContextR;
        pstDfsmn->usFeatureContext = g_usFeatureContextL + g_usFeatureContextR + 1;
    }

    pstSevcCfg->usFeatureContextR = pstDfsmn->usFeatureContextR;
    nnModelInfoShow(&pstDfsmn->modelInfo);
    pstNnModelInfo = &pstDfsmn->modelInfo;
    pstDfsmn->usNbBands = pstNnModelInfo->ucNBBands;
    usFeatureDim = pstDfsmn->usNbBands;
#ifdef NN_FSMN
    // size of feature mean and variance
    usSize = usFeatureDim * pstDfsmn->usFeatureContext * sizeof(aisp_s32_t);
    BUFF_POINT_SET_ALIGN(pstDfsmn->piFeatureMean, usSize, pstMemLocator, FAIL);
    BUFF_POINT_SET_ALIGN(pstDfsmn->piFeatureVar, usSize, pstMemLocator, FAIL);
#endif
#if SEVC_SWITCH_NN_AES
    //usFeatureDim *= 2;
    usFeatureDim += pstSevcCfg->usAesRefBinCount;

    if (usFeatureDim * pstDfsmn->usFeatureContext != pstNnModelInfo->usFeatureTotalDim)
    {
        MSG_DBG("%s cfg and resource error %d %d %d %d\r\n", __func__,
                pstNnModelInfo->ucNBBands, pstSevcCfg->usAesRefBinCount,
                pstDfsmn->usFeatureContext, pstNnModelInfo->usFeatureTotalDim);
        nnModelInfoFree(pstNnModelInfo);
        return NULL;
    }

#endif
    pstSevcCfg->ucNnFeatureDim = usFeatureDim;
    iCmnWinLen = pstDfsmn->ucCmnLen + 1;

    if (pstSevcCfg->ucNnBarkSupport)
    {
#ifndef AISPEECH_FIXED
        //psFull2BarkMapping
        BUFF_POINT_SET_ALIGN(pstDfsmn->psFull2BarkMapping,
                             (pstDfsmn->usNbBands * sizeof(U16)), pstMemLocator, FAIL);
#endif
#ifdef BAND_ENERGY_SMOOTH
        //pfNbBandsSum
        BUFF_POINT_SET_ALIGN(pstDfsmn->pfNbBandsSum,
                             (usFeatureDim * sizeof(aisp_s64_t)), pstMemLocator, FAIL);
        //pfNbBandsSumTmp
        SH_BUFF_POINT_SET_ALIGN(pstDfsmn->pfNbBandsSumTmp,
                                (usFeatureDim * sizeof(aisp_s64_t)), pstMemLocator, FAIL);
#else
        //pfNbBandsSum
        SH_BUFF_POINT_SET_ALIGN(pstDfsmn->pfNbBandsSum,
                                (usFeatureDim * sizeof(aisp_s64_t)), pstMemLocator, FAIL);
#endif
        //pfFeatures
        SH_BUFF_POINT_SET_ALIGN(pstDfsmn->pfFeatures,
                                (pstDfsmn->usFeatureContext * usFeatureDim * sizeof(aisp_s32_t)),
                                pstMemLocator, FAIL);
        //pfFeaturesHistory
        BUFF_POINT_SET_ALIGN(pstDfsmn->pfFeaturesHistory,
                             (pstDfsmn->usFeatureContext * usFeatureDim * sizeof(aisp_s32_t)),
                             pstMemLocator, FAIL);

        //if (pstNnCfg->use_smooth_cmn_flag)
        if (pstDfsmn->ucCmnSupport)
        {
            //pCmnFeatureSum
            BUFF_POINT_SET_ALIGN(pstDfsmn->pCmnFeatureSum,
                                 (usFeatureDim * sizeof(aisp_s64_t)),
                                 pstMemLocator, FAIL);
            //ppCmnBuffer
            BUFF_POINT_SET_ALIGN(pstDfsmn->ppCmnBuffer,
                                 (iCmnWinLen * sizeof(VOID *)),
                                 pstMemLocator, FAIL);

            for (ii = 0; ii < iCmnWinLen; ii++)
            {
                BUFF_POINT_SET_ALIGN(pstDfsmn->ppCmnBuffer[ii],
                                     (usFeatureDim * sizeof(aisp_s32_t)),
                                     pstMemLocator, FAIL);
            }
        }
    }
    else
    {
        //pfFeatures
        BUFF_POINT_SET_ALIGN(pstDfsmn->pfFeatures,
                             (pstDfsmn->usFeatureContext * pstSevcCfg->usFftBin * sizeof(aisp_s32_t)),
                             pstMemLocator, FAIL);
    }

#ifndef AISPEECH_FIXED
    //piFeatureMean  piFeatureVar
    usSize = pstNnModelInfo->usFsmnW1In[0] * sizeof(aisp_s32_t);
    BUFF_POINT_SET_ALIGN(pstDfsmn->piFeatureMean, usSize, pstMemLocator, FAIL);
    BUFF_POINT_SET_ALIGN(pstDfsmn->piFeatureVar, usSize, pstMemLocator, FAIL);
#endif
    MEM_LOCATOR_SHOW(pstMemLocator);
    /* Alloc memory for DFSMN model struction. */
    pstDfsmn->pstModel = nnModelNew(pstNnModelInfo, pstMemLocator);
    AISP_TSL_PTR_CHECK(pstDfsmn->pstModel, FAIL);
#ifdef AISPEECH_FIXED
    nnModelDataInit(pstDfsmn, pstSevcCfg);
#else
    nnModelDataLoad(pstDfsmn, pstNnModelInfo);
#endif
    nnModelInfoFree(pstNnModelInfo);
    MEM_LOCATOR_SHOW(pstMemLocator);
    return pstDfsmn;
FAIL:
    return  NULL;
}

/************************************************************
  Function   : nnProcessorResourceInfoGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     : resouce info
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
S8 *nnProcessorResourceInfoGet(U32 uiSampleRate)
{
    if (16000 == uiSampleRate)
    {
        return (S8 *)g_pcResource;
    }
    else
    {
        return (S8 *)g_pcNbResource;
    }
}

/************************************************************
  Function   : nnQValueDataGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     : ucDataQ
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
S32 nnProcessorQValueDataGet(NN_STATE_S *pstNn)
{
    return pstNn->modelInfo.ucDataQ;
}

/************************************************************
  Function   : nnFeatureContextRGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     : usFeatureContextR
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
U16 nnProcessorFeatureContextRGet(U32 uiSampleRate)
{
    if (16000 == uiSampleRate)
    {
        return g_usFeatureContextR;
    }
    else
    {
        return g_usNbFeatureContextR;
    }
}

/************************************************************
  Function   : nnNNOutDataGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     : ppOutput : Q20
  Return     : iOutLen
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
S32 nnProcessorOutDataGet(NN_STATE_S *pstRnn, aisp_s32_t **ppOutput)
{
    *ppOutput = pstRnn->pstModel->pOutput;
    return pstRnn->pstModel->iOutLen;
}

/************************************************************
  Function   : nnFsmnCalc()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
VOID nnFsmnCalc(FSMN_LAYER_S *pFsmnLayer, aisp_s32_t *pfOutputs, aisp_s32_t *pfInputs)
{
    S32 iLoop;
    S32 iInput = pFsmnLayer->iInputs;
    S32 iHiddens = pFsmnLayer->iHiddens;
    S32 iOutput = pFsmnLayer->iOutputs;
    U16 usTaps = pFsmnLayer->usHleftTaps;
    U8 ucWQ = pFsmnLayer->ucWQ;
    U8 ucDataQ = pFsmnLayer->ucDataQ;
    nn_weight *pW1 = pFsmnLayer->pfInputWeights;
    nn_weight *pW2 = pFsmnLayer->pfWeights;
    nn_weight *pH1 = pFsmnLayer->pfH;
#if 0
    aisp_s32_t *pftmp1 = pFsmnLayer->pftmp1;
    aisp_s32_t *pfHleftTmp = pFsmnLayer->pfHleftTmp;
#else
    aisp_s32_t *pftmp1 = (aisp_s32_t *)pFsmnLayer->pcSharedMem;
    aisp_s32_t *pfHleftTmp = (aisp_s32_t *)pFsmnLayer->pcSharedMem;
#endif
    aisp_s32_t *pfHleftBins = pFsmnLayer->ppfHleft[0];
    U32 uiReluMax;
    uiReluMax = g_iReluMax;
    nnLinearCalc(iInput, iHiddens, pftmp1, pfInputs, pW1, pFsmnLayer->pfInputBias, ucWQ);
    //tmp1 = min(6, max(0, tmp));  %ReLU6
    nnActivation(pftmp1, pftmp1, iHiddens, ACTIVATION_RELU, uiReluMax, ucDataQ, ucDataQ);
    //tmp2 = tmp1*w2.';  %project
    nnLinearCalc(iHiddens, iOutput, pfHleftBins, pftmp1, pW2, NULL, ucWQ);

    //update history memory
    for (iLoop = 0; iLoop < usTaps - 1; iLoop++)
    {
        pFsmnLayer->ppfHleft[iLoop] = pFsmnLayer->ppfHleft[iLoop + 1];
    }

    pFsmnLayer->ppfHleft[usTaps - 1] = pfHleftBins;
    //nnVecMul32X8_Real(pFsmnLayer->pfFsmnOutput, pFsmnLayer->ppfHleft[0], pH1, iOutput, ucWQ);
    nnVecMul32X8_Real(pfOutputs, pFsmnLayer->ppfHleft[0], pH1, iOutput, ucWQ);
    pH1 += iOutput;

    for (iLoop = 1; iLoop < usTaps; iLoop++)
    {
        nnVecMul32X8_Real(pfHleftTmp, pFsmnLayer->ppfHleft[iLoop], pH1, iOutput, ucWQ);
        //nnVec32Add32_Real(pFsmnLayer->pfFsmnOutput, pFsmnLayer->pfFsmnOutput, pfHleftTmp, iOutput, 0);
        nnVec32Add32_Real(pfOutputs, pfOutputs, pfHleftTmp, iOutput, 0);
        pH1 += iOutput;
    }

    //nnVec32Add32_Real(pFsmnLayer->pfFsmnOutput, pFsmnLayer->pfFsmnOutput, pfHleftBins, iOutput, 0);
    nnVec32Add32_Real(pfOutputs, pfOutputs, pfHleftBins, iOutput, 0);
}

/************************************************************
  Function   : nnRnnGainCalc()

  Description:
  Calls      :
  Called By  :
  Input      : pfFeatures : Q20
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
VOID nnProcessorProc(NN_STATE_S *pstNn, aisp_s32_t *pfInputs)
{
    NN_MODEL_S *pstModel = pstNn->pstModel;
    FSMN_LAYER_S **ppFsmnLayer = pstModel->ppFsmnLayer;
    aisp_s32_t **ppfInFcOutput = pstModel->ppfInFcOutput;
    aisp_s32_t **ppfOutFcOutput = pstModel->ppfOutFcOutput;
    aisp_s32_t *pDataIn = pfInputs;
    U8 ucDataQ = ppFsmnLayer[0]->ucDataQ;
    S32 iInDenseLayers = pstModel->iInDenseLayers;
    S32 iFsmnLayers = pstModel->iFsmnLayers;
    S32 iOutDenseLayers = pstModel->iOutDenseLayers;
    S32 ii, iLen;

    if (iInDenseLayers)
    {
        nnFcCalc(pstModel->ppInFcLayer[0], ppfInFcOutput[0], pDataIn);

        for (ii = 1; ii < iInDenseLayers; ii++)
        {
            nnFcCalc(pstModel->ppOutFcLayer[1],
                     ppfInFcOutput[ii],
                     ppfInFcOutput[ii - 1]);
        }

        pDataIn = ppfInFcOutput[iInDenseLayers - 1];
    }

    if (iFsmnLayers)
    {
        nnFsmnCalc(ppFsmnLayer[0], ppFsmnLayer[0]->pfFsmnOutput, pDataIn);

        for (ii = 1; ii < iFsmnLayers; ii++)
        {
            nnFsmnCalc(ppFsmnLayer[ii], ppFsmnLayer[ii]->pfFsmnOutput,
                       ppFsmnLayer[ii - 1]->pfFsmnOutput);
        }

        pDataIn = ppFsmnLayer[iFsmnLayers - 1]->pfFsmnOutput;
    }

    if (iOutDenseLayers)
    {
        nnFcCalc(pstModel->ppOutFcLayer[0], ppfOutFcOutput[0], pDataIn);

        for (ii = 1; ii < iOutDenseLayers; ii++)
        {
            nnFcCalc(pstModel->ppOutFcLayer[ii],
                     ppfOutFcOutput[ii],
                     ppfOutFcOutput[ii - 1]);
        }
    }
    else
    {
        iLen = ppFsmnLayer[iFsmnLayers - 1]->iOutputs;
        //nnActivation(pDataIn, pDataIn, iLen, cActivate[0], iReluMax, ucDataQ, ucDataQ);
        nnActivation(pDataIn, pDataIn, iLen, g_cActivate[0], 0, ucDataQ, ucDataQ);
        //nnActivation(pDataIn, pDataIn, iLen, g_cNbActivate[0], 0, ucDataQ, ucDataQ);DBG_HERE();
    }
}

VOID nnProcessorDelete(NN_STATE_S *pstNn)
{
}
#endif
