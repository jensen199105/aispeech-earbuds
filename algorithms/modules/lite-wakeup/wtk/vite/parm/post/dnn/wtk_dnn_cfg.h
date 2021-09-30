#ifndef WTK_VITE_REC_DNN_WTK_DNN_CFG_H_
#define WTK_VITE_REC_DNN_WTK_DNN_CFG_H_

#include "wtk/vite/math/wtk_math.h"
#include "wtk/vite/parm/post/dnn/cblas/wtk_cblas_cfg.h"
#include "wtk_dnn_type.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_dnn_cfg wtk_dnn_cfg_t;

struct wtk_dnn_cfg
{
    int left_win;
    int right_win;
    int win;
    // int in_cols;
    int out_cols;
    int min_flush_frame;
    int padding_frame;

    //-------------------- bias configure -------------
    wtk_cblas_cfg_t cblas;
    int skip_frame;
    float *expand_array;
    int sil_init_count;
    int speech_init_count;
    unsigned use_cblas : 1;
    unsigned use_lazy_out : 1;
    unsigned attach_htk_log : 1;
    unsigned use_ivector : 1;
    unsigned use_expand_vector : 1;
    unsigned use_custom_win : 1;
};

int wtk_dnn_cfg_init(wtk_dnn_cfg_t *cfg);
int wtk_dnn_cfg_clean(wtk_dnn_cfg_t *cfg);
int wtk_dnn_cfg_update(wtk_dnn_cfg_t *cfg);
int wtk_dnn_cfg_update2(wtk_dnn_cfg_t *cfg);

#ifdef __cplusplus
};
#endif
#endif
