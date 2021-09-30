/**
 * Project  : AIWakeup.v1.0.7
 * FileName : third/wtk/wakeup/wtk_wakeup_cfg.h
 *
 * COPYRIGHT (C) 2014, AISpeech Ltd.. All rights reserved.
 */
#ifndef __AISPEECH__WTK_WAKEUP__WTK_WAKEUP_CFG_H__
#define __AISPEECH__WTK_WAKEUP__WTK_WAKEUP_CFG_H__

#ifndef WKP_VAD_DISABLE
#include "wtk/vad/wtk_vad2.h"
#endif
#include "wtk/vite/parm/wtk_parm.h"
#include "dnn/wtk_wakeup_dnn_cfg.h"

#define WAKEUP_MAX_WORDS    (128)
#define WTK_WAKEUP_Q31_ONE  (0X7FFFFFFF)

typedef struct wtk_wakeup_cfg wtk_wakeup_cfg_t;

struct wtk_wakeup_cfg
{
    wtk_wakeup_dnn_cfg_t *dnn;
    wtk_parm_cfg_t *parm;
};

int wtk_wakeup_cfg_init(wtk_wakeup_cfg_t *cfg);
int wtk_wakeup_cfg_clean(wtk_wakeup_cfg_t *cfg);
int wtk_wakeup_cfg_update_local(wtk_wakeup_cfg_t *cfg);
int wtk_wakeup_cfg_update(wtk_wakeup_cfg_t *cfg);
int wtk_wakeup_cfg_update2(wtk_wakeup_cfg_t *cfg);
void wtk_wakeup_cfg_delete_bin(wtk_wakeup_cfg_t *cfg);

#endif //__AISPEECH__WTK_WAKEUP__WTK_WAKEUP_CFG_H__
