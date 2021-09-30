/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : AISP_TSL_add.h
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

#ifndef __AISP_TSL_ADD_H__
#define __AISP_TSL_ADD_H__

#include "AISP_TSL_types.h"

VOID AISP_TSL_add_vec_32(S32 *piSrcA, S32 *piSrcB, S32 *piDst, S32 len);
VOID AISP_TSL_sub_vec_32(S32 *piSrcA, S32 *piSrcB, S32 *piDst, S32 len);

#endif
