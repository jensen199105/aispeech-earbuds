/**
 * @file wtk_fsmn_cfg.h
 * @brief feedforward sequential memory networks
 * @author xtg
 * @version 1.0
 * @date 2018-11-19
 */

#ifndef __AISPEECH__WTK_VITE_PARM_POST_FSMN_WTK_FSMN_CFG_H__
#define __AISPEECH__WTK_VITE_PARM_POST_FSMN_WTK_FSMN_CFG_H__

#include "wtk/core/wtk_queue.h"
#include "wtk/vite/math/wtk_vector.h"
#include "wtk/vite/math/wtk_cblas_matrix.h"
#include "wtk/vite/math/cblas.h"

typedef struct wtk_fsmn_layer
{
    wtk_queue_node_t qn;

    const short *bias;
    wtk_cblas_matrix_i8_t dense1;
    wtk_cblas_matrix_i8_t dense2;
    wtk_cblas_matrix_i8_t filter;

    int in_dim;
    int out_dim;
    int history_frame;
} wtk_fsmn_layer_t;

typedef struct wtk_fsmn_cfg
{
    int win;
    int skip_frame;
    wtk_queue_t layers;
    const aisp_s16_t *trans_bias;
    const aisp_s16_t *trans_window;
    const aisp_s16_t *bias2;
    wtk_cblas_matrix_i16_t dense2;
    wtk_cblas_matrix_i16_t linear;

    int max_col;
    int max_history_col;
    int in_col;
    int out_col;
    int max_history_frame;
} wtk_fsmn_cfg_t;

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

int wtk_fsmn_cfg_init(wtk_fsmn_cfg_t *cfg);
int wtk_fsmn_cfg_clean(wtk_fsmn_cfg_t *cfg);
int wtk_fsmn_cfg_update2(wtk_fsmn_cfg_t *cfg);

#ifdef __cplusplus
}
#endif //__cplusplus
#endif //__AISPEECH__WTK_VITE_PARM_POST_FSMN_WTK_FSMN_CFG_H__
