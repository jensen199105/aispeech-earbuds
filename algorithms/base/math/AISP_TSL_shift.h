/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : AISP_TSL_shift.h
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
#ifndef __AISP_TSL_SHIFT_H__
#define __AISP_TSL_SHIFT_H__

#include "AISP_TSL_types.h"

U64 AISP_TSL_PSHR64(S64 a, S32 shift);
VOID AISP_TSL_shr_vec_32_16(S16 *psSrc, S32 *piDst, U32 uiLen, S8 shiftBits);
#endif
