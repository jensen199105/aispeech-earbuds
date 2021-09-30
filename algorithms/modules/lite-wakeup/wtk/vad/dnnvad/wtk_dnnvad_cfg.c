#include "wtk_dnnvad_cfg.h"

int wtk_dnnvad_cfg_init(wtk_dnnvad_cfg_t *cfg)
{
    cfg->win = 5;
    cfg->siltrap = 10;
    cfg->speechtrap = 13;
    cfg->startfrm = 0;
    cfg->sil_thresh = 1590895016;
    return 0;
}

int wtk_dnnvad_cfg_clean(wtk_dnnvad_cfg_t *cfg)
{
    return 0;
}

