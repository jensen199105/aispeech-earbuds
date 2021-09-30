#ifndef _SEVC_UTIL_H_
#define _SEVC_UTIL_H_

#include "sevc_mem_alloc.h"
#include "sevc_types.h"

#define FIX_HERE()          MSG_DBG("%s [%d] please fix the code there\r\n", __func__, __LINE__)
#define DBG_HERE()          MSG_DBG("%s [%d] +++++++++++\r\n", __func__, __LINE__)

#if MEM_BUFF_DBG
#define MEM_LOCATOR_SHOW(MemLocator) memLocatorInfoShow(__func__, __LINE__, MemLocator)
#else
#define MEM_LOCATOR_SHOW(MemLocator)
#endif

//#define USE_SHARE_MEM               0

#define MAX_S16                 32768 //(1<<15)
#define MAX_S24                 2097152 //(1<<6)*32768)
#define SEVC_MAX_VALUE(hdr)      (hdr ? MAX_S24:MAX_S16)

#ifdef JIELI_BR28
extern void *calloc2(unsigned long count, unsigned long size);
extern void free2(void *rmem);
#define SEVC_CALLOC(_size_)         calloc2(1, _size_)
#define SEVC_FREE(_point_)          free2(_point_)
#elif (defined BES2300)
extern void *med_calloc(size_t nmemb, size_t size);
void med_free(void *p);
#define SEVC_CALLOC(_size_)         med_calloc(1, _size_)
#define SEVC_FREE(_point_)          med_free(_point_)
#else
#define SEVC_CALLOC(_size_)         calloc(1, _size_)
#define SEVC_FREE(_point_)          free(_point_)
#endif

#ifdef JIELI_BR28
#define MSG_DBG(str, ...)           printf(str, ##__VA_ARGS__)
#define MSG_INFO(str, ...)          printf(str, ##__VA_ARGS__)
#define MSG_WARN(str, ...)          printf(str, ##__VA_ARGS__)
#define MSG_ERROR(str, ...)         printf(str, ##__VA_ARGS__)
#elif (defined BES2300)
#define TICKS_TO_US(tick)           ((U32)(tick) * 10 / (6500000 / 1000 / 100))
extern U32 hal_fast_sys_timer_get(void);
extern int hal_trace_printf(U32 lvl, const char *fmt, ...);
#define SYS_TIMER_GET()             hal_fast_sys_timer_get()
#define MSG_DBG(str, ...)           hal_trace_printf(1, str, ##__VA_ARGS__)
#define MSG_INFO(str, ...)          hal_trace_printf(1, str, ##__VA_ARGS__)
#define MSG_WARN(str, ...)          hal_trace_printf(1, str, ##__VA_ARGS__)
#define MSG_ERROR(str, ...)         hal_trace_printf(1, str, ##__VA_ARGS__)
#else
#define MSG_DBG(str, ...)           printf(str, ##__VA_ARGS__)
#define MSG_INFO(str, ...)          printf(str, ##__VA_ARGS__)
#define MSG_WARN(str, ...)          printf(str, ##__VA_ARGS__)
#define MSG_ERROR(str, ...)         printf(str, ##__VA_ARGS__)
#endif

#ifndef LOG
#define LOG(x)          (logf(x))
#endif
#ifndef LOG10
#define LOG10(x)        (log10f(x))
#endif

#define SEVC_MAX(a,b)        ((a)>(b)?(a):(b))
#define SEVC_MIN(a,b)        ((a)<(b)?(a):(b))
#define COMPLEX_ZERO(c)     do{(c).real = 0;(c).imag=0;}while(0)
#define COMPLEX_ASSIGN(a, b)     do{(a).real=(b).real; (a).imag=(b).imag;}while(0)

#define ALIGN_OFF           (0)
#define ALIGN_ON            (1)
#define ALIGN_SIZE          (0x08)
#define ALIGN_HEX_MASK      (0x07)

#define SIZE_ALIGN(_size_) AISP_TSL_mask_align(_size_, ALIGN_HEX_MASK)

#define BUFF_POINT_SET_ALIGN(point, size, mLocator, FLAG) \
    do{\
        point=(VOID*)memLocatorGet(mLocator, (U32)size, ALIGN_ON);\
        AISP_TSL_PTR_CHECK(point, FLAG);\
    }while(0)

#ifdef USE_SHARE_MEM
#define SH_BUFF_POINT_SET_ALIGN(point, size, mLocator, FLAG) \
        do{\
            point=(VOID*)memLocatorShMemGet(mLocator, (U32)size, ALIGN_ON);\
            AISP_TSL_PTR_CHECK(point, FLAG);\
        }while(0)
#else
#define SH_BUFF_POINT_SET_ALIGN(point, size, mLocator, FLAG) \
        BUFF_POINT_SET_ALIGN(point, size, mLocator, FLAG)
#endif

#define SEVC_COMPLEX_POW_SQ(A, Q) \
    PSHR((MULT32_32((A).real, (A).real) + MULT32_32((A).imag , (A).imag)), Q)
static inline aisp_s32_t sevcAbsf(aisp_s32_t x)
{
    return ((x < 0) ? -x : x);
}

aisp_s16_t sevcCharToFloat(const U8 *p, U32 uiIndex);
aisp_s16_t sevc24bitToFloat(const U8 *p, U32 uiIndex);
VOID sevcFloatTo24bit(U8 *p, aisp_s16_t fData);
VOID sevcComplexDataShow(S8 *pcFunc, S32 iLine, aisp_cpx_s32_t *pFrame, U16 usLen);
VOID sevcFloatDataShow(S8 *pcFunc, S32 iLine, aisp_s16_t *pfFrame, U16 usLen);
VOID sevcShortDataShow(S8 *pcFunc, S32 iLine, S16 *psFrame, U16 usLen);
VOID sevcComplexDataFromFile(aisp_cpx_s32_t *pFrame, U16 usLen, S8 *pcFileName);
VOID sevcFloatDataFromFile(aisp_s16_t *pfFrame, U16 usLen, S8 *pcFileName);
aisp_s32_t SEVC_ComplexAbs(aisp_cpx_s32_t cpx);


VOID memLocatorInit(MEM_LOCATOR_S *pstAecMemLoc, S8 *pcBase,
                    S32 uiSize, S32 uiShMemSize);
VOID *memLocatorGet(MEM_LOCATOR_S *pstAecMemLoc, U32 uiSize, U8 ucIsAlign);
VOID *memLocatorShMemGet(MEM_LOCATOR_S *pstAecMemLoc, U32 uiSize, U8 ucIsAlign);
#ifdef USE_SHARE_MEM
VOID memLocatorShMemReset(MEM_LOCATOR_S *pstAecMemLoc);
#else
#define memLocatorShMemReset(x)
#endif
VOID memLocatorInfoShow(const S8 *pStr, U32 uiLine, MEM_LOCATOR_S *pstAecMemLoc);

U32 sevcFftSizeConvert(U32 uiFrameSize);
int AISP_TSL_log_xW32_yW32Q24(int x, int q_in);

VOID sevcVec16Add16_Real(aisp_s16_t *pDstZ, S16 *pSrcY, S16 *pSrcX, U32 uiLen, S32 rq);
VOID sevcVec32Add32_Real(aisp_s32_t *pDstZ, S32 *pSrcY, S32 *pSrcX, U32 uiLen, S32 rq);
VOID sevcVec32Sub32_Real(aisp_s32_t *pDstZ, S32 *pSrcY, S32 *pSrcX, U32 uiLen, S32 rq);

VOID sevcVecMulScalorShiftLeft(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                               aisp_s32_t srcX, U32 uiLen, S32 rq);
VOID sevcVecMulScalorShiftRight(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                                aisp_s32_t srcX, U32 uiLen, S32 rq);
VOID sevcVec32MUL8_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY, aisp_s8_t *pSrcX,
                        U32 uiLen, S32 rq);
VOID sevcVec32MUL32_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY, aisp_s32_t *pSrcX,
                         U32 uiLen, S32 rq);
VOID sevcVecMUL16X16_Real(aisp_s32_t *pDstZ, aisp_s16_t *pSrcY, aisp_s16_t *pSrcX,
                          U32 uiLen, S32 rq);
VOID sevcVecMUL32X16_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY, aisp_s16_t *pSrcX,
                          U32 uiLen, S32 rq);
VOID sevcVecMUL32X32_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcY,
                          aisp_s32_t *pSrcX, U32 uiLen, S32 rq);
VOID sevcVecComplexMulReal(aisp_cpx_s32_t *pDstZ, aisp_cpx_s32_t *pSrcY,
                           aisp_s32_t *pSrcX, U32 uiLen, S32 rq);
VOID sevcVecMUL_Complex(aisp_cpx_s32_t *pDstZ, aisp_cpx_s32_t *pSrcY,
                        aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq);
VOID sevcVecAdd_Complex(aisp_cpx_s32_t *pDstZ, aisp_cpx_s32_t *pSrcY, aisp_cpx_s32_t *pSrcX,
                        U32 uiLen, S32 rq);
VOID sevcVecConjMUL_Complex(aisp_cpx_s32_t *pDstZ, aisp_cpx_s32_t *pSrcY,
                            aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq);
VOID sevcVec64ConjMUL_Complex(aisp_cpx_s64_t *pDstZ, aisp_cpx_s32_t *pSrcY,
                              aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq);
VOID sevcVec32ConjMUL_Complex(aisp_cpx_s32_t *pDstZ, aisp_cpx_s32_t *pSrcY,
                              aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq);


VOID sevcVecMLA_Complex(aisp_cpx_s32_t *pDst, aisp_cpx_s32_t *pSrc1,
                        aisp_cpx_s32_t *pSrc2, U32 uiLen, S32 rq);
VOID sevcVecConjMLA_Complex(aisp_cpx_s32_t *pDst, aisp_cpx_s32_t *pSrc1,
                            aisp_cpx_s32_t *pSrc2, U32 uiLen, S32 rq);

VOID sevcVecMLS_Complex(aisp_cpx_s32_t *pDstZ, aisp_cpx_s32_t *pSrcY,
                        aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq);
VOID sevcVecSub_Complex(aisp_cpx_s32_t *pDstZ, aisp_cpx_s32_t *pSrcY,
                        aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq);

VOID sevcVecSum32_Real(aisp_s32_t *pDstZ, aisp_s32_t *pSrcX, U32 uiLen, S32 rq);
VOID sevcVec32MUL8Sum_Real(aisp_s32_t *pDstZ, aisp_s32_t *pDstY, aisp_s8_t *pSrcX,
                           U32 uiLen, S32 rq);
VOID sevcVecPowY32X32_Complex(aisp_s32_t *pDstY, aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq);

VOID sevcVecPowY64X32_Complex(aisp_s64_t *pDstY, aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 rq);
VOID sevcVecPowY64X64_Complex(aisp_s64_t *pDstY, aisp_cpx_s64_t *pSrcX, U32 uiLen, S32 rq);
VOID sevcVecAbs32_Complex(aisp_s32_t *pDstY, aisp_cpx_s32_t *pSrcX, U32 uiLen, S32 iInQ);

VOID sevcVecAbs64_Complex(aisp_s64_t *pDstY, aisp_cpx_s64_t *pSrcX, U32 uiLen, S32 iInQ);
VOID sevcVec32Smooth(aisp_s32_t *pDstZ, S32 *pSrcX, S32 iLamda, U32 uiLen, S32 rq);
VOID sevcVec64Smooth(aisp_s64_t *pDstZ, S64 *pSrcX, S32 iLamda, U32 uiLen, S32 rq);
VOID sevcVec32MinRestrain(S32 *pSrcX, S32 iMin, U32 uiLen);
VOID sevcVec64MinRestrain(S64 *pSrcX, S64 illMin, U32 uiLen);
VOID sevcVec32MaxRestrain(S32 *pSrcX, S32 iMax, U32 uiLen);
VOID sevcVec64MaxRestrain(S64 *pSrcX, S64 illMax, U32 uiLen);

aisp_s32_t sevcLog(aisp_s32_t iData, S32 iInQ, S32 iOutQ);
aisp_s32_t sevcExp(aisp_s32_t iData, S32 iInQ, S32 iOutQ);
aisp_s32_t sevcSin(aisp_s32_t iData);
VOID SevcSortDescend(aisp_s32_t *arr, aisp_s32_t len);
VOID SevcSortDescendLongLong(aisp_s64_t *arr, aisp_s32_t len);
#endif


