/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : AISP_TSL_mul.h
  Project    : 
  Module     : 
  Version    : 
  Date       : 2019/07/15
  Author     : Youhai.Jiang
  Document   : 
  Description: 

  Function List :

  History :
  <author>      <time>            <version >        <desc> 
  Youhai.Jiang  2019/07/15         1.00              Create

************************************************************/

#ifndef __AISP_TSL_MUL_H__
#define __AISP_TSL_MUL_H__

#include "AISP_TSL_types.h"

VOID AISP_TSL_mul_vec_32_32(S32 *piSrcA, S32 *piSrcB, S32 *piDst, S32 len, S32 shift);
VOID AISP_TSL_cmul_vec_32_32(S32 *piSrcA, S32 *piSrcB, S32 *piDst, S32 len, S32 shift);
VOID AISP_TSL_cmul_sum_vec_32_32(S32 *piSrcA, S32 *piSrcB, S32 *piDst, S32 len, S32 shift);
VOID AISP_TSL_cmulj_sum_vec_with_mu_32_32(S64 *piSrcA, S32 *piSrcB, S32 *piDst, S32 iMu, S32 len, S32 shift, S32 iMuQVal);
VOID AISP_TSL_add_vec_32(S32 *piSrcA, S32 *piSrcB, S32 *piDst, S32 len);
VOID AISP_TSL_sub_vec_32(S32 *piSrcA, S32 *piSrcB, S32 *piDst, S32 len);

#endif
