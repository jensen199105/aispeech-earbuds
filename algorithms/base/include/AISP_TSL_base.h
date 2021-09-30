/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : AISP_TSL_types.h
  Project    :
  Module     :
  Version    :
  Date       : 2019/04/28
  Author     : Chao.Xu
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Chao.Xu       2019/04/28        1.00              Create

************************************************************/
#ifndef __AISP_TSL_BASE_H__
#define __AISP_TSL_BASE_H__

#define AISP_TSL_Q5_ONE         (32)
#define AISP_TSL_Q5_ONE_MINI    (31)
#define AISP_TSL_Q5_SHIFT       (5)
#define AISP_TSL_Q6_ONE         (64)
#define AISP_TSL_Q6_SHIFT       (6)
#define AISP_TSL_Q10_SHIFT      (10)
#define AISP_TSL_Q15_ONE        (32768)
#define AISP_TSL_Q15_ONE_MINI   (32767)
#define AISP_TSL_Q15_SHIFT      (15)
#define AISP_TSL_Q16_ONE        (65536)
#define AISP_TSL_Q20_ONE        (1048576)
#define AISP_TSL_Q20_MINI       (1048575)
#define AISP_TSL_Q20_SHIFT      (20)
#define AISP_TSL_Q21_ONE        (2097152)
#define AISP_TSL_Q22_ONE        (4194304)
#define AISP_TSL_Q22_SHIFT      (22)
#define AISP_TSL_Q23_ONE        (8388608)
#define AISP_TSL_Q23_SHIFT      (23)
#define AISP_TSL_Q24_ONE        (16777216)
#define AISP_TSL_Q24_SHIFT      (24)
#define AISP_TSL_Q31_ONE        (2147483648)
#define AISP_TSL_Q31_ONE_MINI   (2147483647)
#define AISP_TSL_Q31_ONE_LONG_TYPE (2147483648LL)
#define AISP_TSL_Q31_SHIFT      (31)
#define AISP_TSL_Q39_ONE        (549755813888LL)
#define AISP_TSL_Q39_SHIFT      (39)
#define AISP_TSL_Q63_ONE        (9223372036854775000LL)

#ifdef AISPEECH_FIXED
#define MULT32_32(a, b)         (((aisp_s64_t)(aisp_s32_t)(a))*((aisp_s64_t)(aisp_s32_t)(b)))
#define MULT32_32_SQ(a, b, q) PSHR((aisp_s64_t)MULT32_32(a, b), (q))
#define MULT16_16(a,b)      (((aisp_s32_t)(aisp_s16_t)(a))*((aisp_s32_t)(aisp_s16_t)(b)))
#define MULT16_16_P15(a,b)  (PSHR(MULT16_16((a),(b)),15))

#define MULT16_32(a,b)      ((aisp_s32_t)((aisp_s16_t)(a))*((aisp_s32_t)(b)))
#define MULT16_32_P15(a,b)  ((aisp_s32_t)(PSHR(MULT16_32((a),(b)),15)))
#define MULT32_32(a,b)      (((aisp_s64_t)(aisp_s32_t)(a))*((aisp_s64_t)(aisp_s32_t)(b)))
#define MULT32_32_P15(a,b)  ((aisp_s32_t)(PSHR(MULT32_32((a),(b)) , 15)))
#define MIN(a,b)            ((a)>(b)?(b):(a))
#define MAX(a,b)            ((a)>(b)?(a):(b))

#else
#define MULT32_32(a, b)         ((a) * (b))
#endif
#endif

