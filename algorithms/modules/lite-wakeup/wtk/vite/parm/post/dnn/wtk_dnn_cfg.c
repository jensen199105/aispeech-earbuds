#include "wtk_dnn_cfg.h"
#include "wtk/vite/math/wtk_math.h"

extern const char dnn_win;
extern const char dnn_skip_frame;

int wtk_dnn_cfg_init(wtk_dnn_cfg_t *cfg)
{
    cfg->use_custom_win = 0;
    cfg->left_win = cfg->right_win = 5;
    cfg->win = dnn_win;
    cfg->use_cblas = 1;
    cfg->min_flush_frame = 0;
    cfg->padding_frame = 0;
    cfg->use_expand_vector = 0;
    cfg->sil_init_count = 0;
    cfg->speech_init_count = 0;
    //cfg->cache_size=8;
    wtk_cblas_cfg_init(&(cfg->cblas));
    cfg->attach_htk_log = 0;
    cfg->skip_frame = dnn_skip_frame;
    cfg->use_lazy_out = 0;
    cfg->use_ivector = 0;
    cfg->expand_array = NULL;
    return 0;
}

int wtk_dnn_cfg_clean(wtk_dnn_cfg_t *cfg)
{
    if (cfg->use_cblas)
    {
        wtk_cblas_cfg_clean(&(cfg->cblas));
    }

    wtk_free(cfg->expand_array);
    return 0;
}

int wtk_dnn_cfg_update2(wtk_dnn_cfg_t *cfg)
{
    int ret;
    int k;
    cfg->expand_array = (float *)wtk_calloc(cfg->padding_frame, sizeof(float));

    for (k = 0; k < cfg->padding_frame; k++)
    {
        cfg->expand_array[k] = 0.0f;
    }

    if (cfg->use_cblas)
    {
        ret = wtk_cblas_cfg_update(&(cfg->cblas));

        if (ret != 0)
        {
            goto end;
        }

        cfg->out_cols = wtk_cblas_cfg_out_cols(&(cfg->cblas));
    }

    if (cfg->skip_frame > 0)
    {
        ++cfg->skip_frame;
        cfg->min_flush_frame *= cfg->skip_frame;
    }

    ret = 0;
end:
    return ret;
}

int wtk_dnn_cfg_update(wtk_dnn_cfg_t *cfg)
{
    return wtk_dnn_cfg_update2(cfg);
}
