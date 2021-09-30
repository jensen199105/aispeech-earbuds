
/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   :
  Project    :
  Module     :
  Version    :
  Date       : 2018/03/24
  Author     : feihu.chen
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  feihu.chen    2018/03/29          1.1              created
************************************************************/
#ifndef __AISP_TSL_COMPLEX_H__
#define __AISP_TSL_COMPLEX_H__

#include "AISP_TSL_types.h"
//#include "AISP_TSL_base.h"


/*add by cfh*/
typedef struct complex_s16{
    aisp_s16_t real;
    aisp_s16_t imag;
}aisp_cpx_s16_t;

typedef struct complex_s32{
    aisp_s32_t real;
    aisp_s32_t imag;
}aisp_cpx_s32_t;

typedef struct complex_u32{
    aisp_u32_t real;
    aisp_u32_t imag;
}aisp_cpx_u32_t;


typedef struct complex_s64{
    aisp_s64_t real;
    aisp_s64_t imag;
}aisp_cpx_s64_t;


typedef struct _float_complex
{
    aisp_s32_t real;
    aisp_s32_t imag;
} S32_COMPLEX_T;

/*add by cfh*/


#define COMPLEX_u32_SZ      sizeof(aisp_cpx_u32_t)
#define COMPLEX_S32_SZ      sizeof(aisp_cpx_s32_t)
#define COMPLEX_S16_SZ      sizeof(aisp_cpx_s16_t)


#define COMPLEX_ADD(Z,A,B) \
        do { \
            (Z).real = (A).real + (B).real; \
            (Z).imag = (A).imag + (B).imag; \
        } while (0)

#define COMPLEX_SUB(Z,A,B) \
            do { \
                (Z).real = (A).real - (B).real; \
                (Z).imag = (A).imag - (B).imag; \
            } while (0)


#define COMPLEX_CONJ(Z,A) \
        do { \
            Z.real = A.real; \
            Z.imag = -A.imag; \
        } while (0)
        

#ifdef AISPEECH_FIXED
#define COMPLEX_MUL_FLOAT(Z,A,B) \
    do { \
        (Z).real = MULT32_32_P15((A).real , (B)); \
        (Z).imag = MULT32_32_P15((A).imag , (B)); \
    } while (0)
#else
#define COMPLEX_MUL_FLOAT(Z,A,B) \
    do { \
        (Z).real = (A).real * (B); \
        (Z).imag = (A).imag * (B); \
    } while (0)
#endif


#ifdef AISPEECH_FIXED

#define COMPLEX_CONJ_MUL(Z,A,B) \
    do { \
        aisp_s32_t ARBR = MULT32_32_P15((A).real , (B).real); \
        aisp_s32_t AIBI = MULT32_32_P15((A).imag , (B).imag); \
        aisp_s32_t ARBI = MULT32_32_P15((A).real , (B).imag); \
        aisp_s32_t AIBR = MULT32_32_P15((A).imag , (B).real); \
        aisp_s32_t tmpR = ARBR + AIBI; \
        aisp_s32_t tmpI = AIBR - ARBI; \
        Z.real = tmpR; \
        Z.imag = tmpI; \
    } while (0)
	
#define COMPLEX_CONJ_MUL_PTR(Z, A, B) \
    do { \
        aisp_s32_t ARBR = MULT32_32_P15((A)->real , (B)->real); \
        aisp_s32_t AIBI = MULT32_32_P15((A)->imag , (B)->imag); \
        aisp_s32_t ARBI = MULT32_32_P15((A)->real , (B)->imag); \
        aisp_s32_t AIBR = MULT32_32_P15((A)->imag , (B)->real); \
        aisp_s32_t tmpR = ARBR + AIBI; \
        aisp_s32_t tmpI = AIBR - ARBI; \
        (Z).real = tmpR; \
        (Z).imag = tmpI; \
    } while (0)
		
#else
#define COMPLEX_CONJ_MUL_PTR(Z, A, B) \
    do { \
        aisp_s32_t ARBR = MULT32_32_P15((A)->real , (B)->real); \
        aisp_s32_t AIBI = MULT32_32_P15((A)->imag , (B)->imag); \
        aisp_s32_t ARBI = MULT32_32_P15((A)->real , (B)->imag); \
        aisp_s32_t AIBR = MULT32_32_P15((A)->imag , (B)->real); \
        aisp_s32_t tmpR = ARBR + AIBI; \
        aisp_s32_t tmpI = AIBR - ARBI; \
        (Z).real = tmpR; \
        (Z).imag = tmpI; \
    } while (0)
#define COMPLEX_CONJ_MUL(Z,A,B) \
    do { \
        aisp_s16_t ARBR = (A).real * (B).real; \
        aisp_s16_t AIBI = (A).imag * (B).imag; \
        aisp_s16_t ARBI = (A).real * (B).imag; \
        aisp_s16_t AIBR = (A).imag * (B).real; \
        Z.real = ARBR + AIBI; \
        Z.imag = AIBR - ARBI; \
    } while (0)
#endif





#endif

