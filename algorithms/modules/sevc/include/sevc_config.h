/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : aec_api_config.h
  Project    :
  Module     :
  Version    :
  Date       : 2019/12/02
  Author     : shizhang.tang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  shizhang.tang  2019/12/02         1.00              Create
************************************************************/
#ifndef __AEC_API_CONFIG_H__
#define __AEC_API_CONFIG_H__

#if (defined FAR_FIELD_COMMUNICATION)
#include "sevc_config_far_field.h"
#elif (defined SEVC_PHONE)
#include "sevc_config_phone.h"
#elif (defined SEVC_CAR)
#include "sevc_config_car.h"
#else
#include "sevc_config_near_field.h"
#endif
// AEC turned off if there is no ref
#if SEVC_REF_CHAN == 0
#undef SEVC_SWITCH_AEC
#define SEVC_SWITCH_AEC                     0
#endif
// BF and GSC turned off if mic_chan is 1
#if SEVC_MIC_CHAN == 1
#undef SEVC_SWITCH_WN
#define SEVC_SWITCH_WN                      0
#undef SEVC_SWITCH_BF
#define SEVC_SWITCH_BF                      0
#undef SEVC_SWITCH_GSC
#define SEVC_SWITCH_GSC                     0
#endif
// EMD filter and Preemph turned off if AEC off
#if SEVC_SWITCH_AEC == 0
#undef SEVC_SWITCH_AEC_EMD
#define SEVC_SWITCH_AEC_EMD                 0
#undef SEVC_SWITCH_AEC_PREEMPH
#define SEVC_SWITCH_AEC_PREEMPH             0
#endif

#if SEVC_SWITCH_AEC == 0
#undef SEVC_SWITCH_AES
#define SEVC_SWITCH_AES                     0
#undef SEVC_SWITCH_NN_AES
#define SEVC_SWITCH_NN_AES                  0
#endif

#if SEVC_SWITCH_NN == 0
#undef SEVC_SWITCH_CNG
#define SEVC_SWITCH_CNG                     0
#endif
#if SEVC_MIC_CHAN == 1
#undef SEVC_SWITCH_BF
#define SEVC_SWITCH_BF                      0
#endif

#if SEVC_SWITCH_BF == 0
#undef SEVC_SWITCH_GSC
#define SEVC_SWITCH_GSC                     0
#endif

#if SEVC_MIC_CHAN != 3
#undef SEVC_SWITCH_AES
#define SEVC_SWITCH_AES                     0
#undef SEVC_SWITCH_VAD
#define SEVC_SWITCH_VAD                     0
#undef SEVC_SD_WEARSWITCH
#define SEVC_SD_WEARSWITCH                  0
#endif
#endif

