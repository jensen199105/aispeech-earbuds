/**
 * @file wtk_fsmn_cfg.c
 * @brief feedforward sequential memory networks
 * @author xtg
 * @version 1.0
 * @date 2018-11-19
 */
#include "wtk_fsmn_cfg.h"

extern const short trans_bias[];
extern const short trans_window[];

extern const char fsmn_win;
extern const char fsmn_skip_frame;

extern const short fsmn_in_col;
extern const short fsmn_out_col;
extern const short fsmn_max_col;
extern const short fsmn_max_history_frame;
extern const short fsmn_max_history_col;

extern const char fsmn_layer_num;
extern const short fsmn_layerX_in_dims[];
extern const short fsmn_layerX_out_dims[];
extern const short fsmn_layerX_history_frames[];
extern const short fsmn_layerX_dense1_shapes[];
extern const short fsmn_layerX_dense2_shapes[];
extern const short fsmn_layerX_filter_shapes[];
extern const short *fsmn_layerX_biases[];
extern const char *fsmn_layerX_dense1s[];
extern const char *fsmn_layerX_dense2s[];
extern const char *fsmn_layerX_filters[];

extern const short fsmn_bias2[];
extern const short fsmn_dense2_shape[];
extern const short fsmn_dense2[];

extern const short fsmn_linear_shape[];
extern const short fsmn_linear[];

int wtk_fsmn_cfg_init(wtk_fsmn_cfg_t *cfg)
{
    AISP_TSL_memset(cfg, 0, sizeof(*cfg));
    return 0;
}

int wtk_fsmn_cfg_clean(wtk_fsmn_cfg_t *cfg)
{
    return 0;
}

int wtk_fsmn_cfg_update2(wtk_fsmn_cfg_t *cfg)
{
    wtk_fsmn_layer_t *layer;
    int i;
    cfg->win = fsmn_win;
    cfg->skip_frame = fsmn_skip_frame;
    cfg->in_col = fsmn_in_col;
    cfg->out_col = fsmn_out_col;
    cfg->max_col = fsmn_max_col;
    cfg->max_history_frame = fsmn_max_history_frame;
    cfg->max_history_col = fsmn_max_history_col;
    cfg->trans_bias = trans_bias;
    cfg->trans_window = trans_window;
    cfg->bias2 = fsmn_bias2;
    cfg->dense2.row = fsmn_dense2_shape[0];
    cfg->dense2.col = fsmn_dense2_shape[1];
    cfg->dense2.m = fsmn_dense2;
    cfg->linear.row = fsmn_linear_shape[0];
    cfg->linear.col = fsmn_linear_shape[1];

    if (cfg->linear.row > 0 && cfg->linear.col > 0)
    {
        cfg->linear.m = fsmn_linear;
    }
    else
    {
        cfg->linear.m = NULL;
    }

    for (i = 0; i < fsmn_layer_num; i++)
    {
        layer = (wtk_fsmn_layer_t *)wtk_calloc(1, sizeof(wtk_fsmn_layer_t));
        layer->in_dim = fsmn_layerX_in_dims[i];
        layer->out_dim = fsmn_layerX_out_dims[i];
        layer->history_frame = fsmn_layerX_history_frames[i];
        layer->bias = fsmn_layerX_biases[i];
        layer->dense1.row = fsmn_layerX_dense1_shapes[2 * i];
        layer->dense1.col = fsmn_layerX_dense1_shapes[2 * i + 1];
        layer->dense1.m = (int8_t *)fsmn_layerX_dense1s[i];
        layer->dense2.row = fsmn_layerX_dense2_shapes[2 * i];
        layer->dense2.col = fsmn_layerX_dense2_shapes[2 * i + 1];
        layer->dense2.m = (int8_t *)fsmn_layerX_dense2s[i];
        layer->filter.row = fsmn_layerX_filter_shapes[2 * i];
        layer->filter.col = fsmn_layerX_filter_shapes[2 * i + 1];
        layer->filter.m = (int8_t *)fsmn_layerX_filters[i];
        wtk_queue_push(&cfg->layers, &layer->qn);
    }

    return 0;
}
