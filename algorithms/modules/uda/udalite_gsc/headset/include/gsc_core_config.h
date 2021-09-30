
/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : gsc_core_config.h
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
#ifndef __GSC_CORE_CONFIG_H__
#define __GSC_CORE_CONFIG_H__

#include "gsc_core_types.h"

S32 gscDefaultCfgInit(GSC_CoreCfg_S *pstGscCfg);
S32 gscCfgUpdate(GSC_CoreCfg_S *pstGscCfg, const char *pcBinFn);

#endif
