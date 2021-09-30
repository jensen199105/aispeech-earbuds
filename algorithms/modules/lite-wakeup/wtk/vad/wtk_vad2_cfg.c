#include "wtk_vad2_cfg.h"

int wtk_vad2_cfg_init(wtk_vad2_cfg_t *cfg)
{
    int ret;

    cfg->left_margin = 15;
    cfg->right_margin = 1;
    cfg->min_speech = 0;
    ret = wtk_dnnvad_cfg_init(&(cfg->dnnvad));
    return ret;
}

int wtk_vad2_cfg_clean(wtk_vad2_cfg_t *cfg)
{
    wtk_dnnvad_cfg_clean(&(cfg->dnnvad));
    return 0;
}

