/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : gsc_core_tables.h
  Project    :
  Module     :
  Version    :
  Date       : 2019/06/21
  Author     : Youhai.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Youhai.Jiang  2019/06/21         1.00              Create

************************************************************/
#ifndef __LUDA_TABLES_H__
#define __LUDA_TABLES_H__

#include "gsc_core_types.h"

extern const GSC_S32_T g_asGscWq[];
extern const GSC_S32_T g_asGscB[];

extern const GSC_S32_T g_aHanningWindow512[GSC_FRMLEN];
extern const GSC_S64_T g_aDeltaFloor[257];

#endif

