/**
 * @file wtk_fsmn.h
 * @brief feedforward sequential memory networks
 * @author xtg
 * @version 1.0
 * @date 2018-11-19
 */

#ifndef __AIPSEECH_WTK_VITE_PARM_POST_FSMN_WTK_FSMN_H__
#define __AIPSEECH_WTK_VITE_PARM_POST_FSMN_WTK_FSMN_H__

#include "wtk/core/wtk_robin.h"
#include "wtk/vite/parm/wtk_feature.h"
#include "wtk_fsmn_cfg.h"

typedef struct wtk_fsmn
{
    wtk_fsmn_cfg_t *cfg;
    struct wtk_parm *parm;
    wtk_robin_t *feature_win;
    int16_t feat_col;
    int16_t input_col;
    int16_t output_col;
    int32_t *feat;
    int8_t *input;
    int8_t *output;
    int32_t *output32;
    int32_t *tmp_history;
    int32_t *tmp;
    wtk_feature_t **features;
    wtk_cblas_matrix_i8_t **history;
    void *hook;
} wtk_fsmn_t;

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

wtk_fsmn_t *wtk_fsmn_new(wtk_fsmn_cfg_t *cfg, struct wtk_parm *parm);
void wtk_fsmn_delete(wtk_fsmn_t *fsmn);
void wtk_fsmn_reset(wtk_fsmn_t *fsmn);
void wtk_fsmn_feed(wtk_fsmn_t *fsmn, wtk_feature_t *f);
void wtk_fsmn_flush(wtk_fsmn_t *fsmn);

#ifdef __cplusplus
}
#endif //__cplusplus
#endif //__AIPSEECH_WTK_VITE_PARM_POST_FSMN_WTK_FSMN_H__
