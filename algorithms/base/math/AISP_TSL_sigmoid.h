/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : AISP_TSL_sigmoid.h
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
#ifndef __AISP_TSL_SIGMOID_H__
#define __AISP_TSL_SIGMOID_H__

#include "AISP_TSL_types.h"

S32 AISP_TSL_sigmoid_xW32Q24_yW32Q24(S32 x);
int AISP_TSL_exp_minus_xW32Q24_yW32(int x, int q_out);
int AISP_TSL_exp_xW32Q24_yW32(int x, int q_out);
int AISP_TSL_log_xW32_yW32Q24(int x, int q_in);

#endif
