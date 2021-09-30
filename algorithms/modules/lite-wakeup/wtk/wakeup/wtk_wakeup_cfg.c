/**
 * Project  : AIWakeup.v1.0.7
 * FileName : third/wtk/wakeup/wtk_wakeup_cfg.c
 *
 * COPYRIGHT (C) 2014, AISpeech Ltd.. All rights reserved.
 */
#include "wtk_wakeup_cfg.h"

int wtk_wakeup_cfg_init(wtk_wakeup_cfg_t *cfg)
{
    cfg->dnn = NULL;
    return 0;
}

int wtk_wakeup_cfg_clean(wtk_wakeup_cfg_t *cfg)
{
    wtk_wakeup_dnn_cfg_clean(cfg->dnn);
    wtk_free(cfg->dnn);
    return 0;
}

int wtk_wakeup_cfg_update_local(wtk_wakeup_cfg_t *cfg)
{
    cfg->dnn = (wtk_wakeup_dnn_cfg_t *)wtk_calloc(1, sizeof(wtk_wakeup_dnn_cfg_t));

    if (0 != wtk_wakeup_dnn_cfg_init(cfg->dnn))
    {
        wtk_free(cfg->dnn);
        return -1;
    }

    wtk_wakeup_dnn_cfg_update_local(cfg->dnn);
    return 0;
}

int wtk_wakeup_cfg_update(wtk_wakeup_cfg_t *cfg)
{
    return wtk_wakeup_cfg_update2(cfg);
}

int wtk_wakeup_cfg_update2(wtk_wakeup_cfg_t *cfg)
{
    int ret;
    ret = wtk_wakeup_dnn_cfg_update(cfg->dnn);
    cfg->parm = &cfg->dnn->parm;
    return ret;
}

void wtk_wakeup_cfg_delete_bin(wtk_wakeup_cfg_t *cfg)
{
    if (cfg)
    {
        wtk_wakeup_cfg_clean(cfg);
        wtk_free(cfg);
    }
}
