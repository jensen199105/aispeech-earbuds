#ifndef _NN_COMMON_H_
#define _NN_COMMON_H_

#ifdef JIELI_BR28
extern void *calloc2(unsigned long count, unsigned long size);
extern void free2(void *rmem);
#define NN_CALLOC(_size_)     calloc2(1, _size_)
#define NN_FREE(_point_)      free2(_point_)
#else
#define NN_CALLOC(_size_)     calloc(1, _size_)
#define NN_FREE(_point_)      free(_point_)
#endif


#define NN_ABS(x)               ((x)<0? -(x) : (x))
#define NN_MAX(a, b)            ((a)<(b)? (b) : (a))
aisp_s32_t nnSigmoidApprox(aisp_s32_t iX, S32 iInQ, S32 iOutQ);
aisp_s32_t nnTansigApprox(aisp_s32_t iX, S32 iInQ, S32 iOutQ);
aisp_s32_t nnRelu(aisp_s32_t iX, S32 iInQ, S32 iOutQs);
VOID nnActivation(aisp_s32_t *pOutput, aisp_s32_t *pInput, U32 uiLen, U8 ucAction,
                  U32 usReluMax, S32 iInQ, S32 iOutQ);

VOID nnVec32Add32_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                       aisp_s32_t *pSrcX, U32 uiLen, S32 rq);
VOID nnVecMul32X8_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                       aisp_s8_t *pSrcX, U32 uiLen, S32 rq);
VOID nnVecMul32X16_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                        aisp_s16_t *pSrcX, U32 uiLen, S32 rq);
VOID nnVecMul32X32_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                        aisp_s32_t *pSrcX, U32 uiLen, S32 rq);
VOID nnVecMulSum32X8_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                          aisp_s8_t *pSrcX, U32 uiLen, S32 rq);
VOID nnVecMulSum32X16_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                           aisp_s16_t *pSrcX, U32 uiLen, S32 rq);
VOID nnVecMulSum32X32_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                           aisp_s32_t *pSrcX, U32 uiLen, S32 rq);
VOID nnVecMulAdd32X8_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                          aisp_s8_t *pSrcX, aisp_s32_t *pSrcB, U32 uiLen, S32 rq);
VOID nnVecMulAdd32X16_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                           aisp_s16_t *pSrcX, aisp_s32_t *pSrcB, U32 uiLen, S32 rq);
VOID nnVecMulAdd32X32_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                           aisp_s32_t *pSrcX, aisp_s32_t *pSrcB, U32 uiLen, S32 rq);
VOID nnLinearCalc(S32 iInSize, S32 iOutSize, aisp_s32_t *pOutput, aisp_s32_t *pInput,
                  aisp_s8_t *pW, aisp_s32_t *pBias, S32 iWQ);

VOID nnFcCalc(FC_LAYER_S *pLayer, aisp_s32_t *pfOutputs, aisp_s32_t *pfInputs);


#endif
