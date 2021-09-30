/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : AISP_TSL_transQ.h
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

#ifndef __AISP_TSL_TRANS_Q_H__
#define __AISP_TSL_TRANS_Q_H__

#include "AISP_TSL_types.h"
#include "AISP_TSL_base.h"

VOID AISP_TSL_copy_q31(const S32 * piSrc, S32 * piDst, U32 uiLen);
VOID AISP_TSL_max_q31(const S32 * piSrc, U32 uiBlockSize, S32 * piResult, U32 * puiIdx);
VOID AISP_TSL_min_q31(const S32 * piSrc, U32 uiBlockSize, S32 * piResult, U32 * puiIdx);
VOID AISP_TSL_max_q15( const S16 * psSrc, U32 uiBlockSize, S16 * pResult, U32 * puiIdx);
VOID AISP_TSL_min_q15(const S16 * psSrc, U32 uiBlockSize, S16 * pResult, U32 * puiIdx);

#endif
