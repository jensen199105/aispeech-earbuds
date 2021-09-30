/**
 * @file wtk_cblas_cfg.h
 * @brief
 * @author xtg
 * @version 0.1
 * @date 2017-05-25
 */

#ifndef WTK_VITE_PARM_POST_DNN_WTK_CBLAS_CFG_H_
#define WTK_VITE_PARM_POST_DNN_WTK_CBLAS_CFG_H_
#include "wtk/vite/parm/post/dnn/wtk_dnn_type.h"
#include "wtk_cblas_model.h"
#ifdef __cplusplus
extern "C" {
#endif

struct wtk_cblas_cfg {
  //-------------------data section -------
  int cache_size;
  //------------------- in cols -------------
  int in_cols;
  int out_cols;
//  int max_row;
  int max_col;
  //-------------------- in bytes -----------
  int in_col_bytes;
  //------------------- resource section ----
  wtk_cblas_trans_t *trans;
  wtk_queue_t layer_q; // wtk_cblas_layer_t queue
};

int wtk_cblas_cfg_init(wtk_cblas_cfg_t *cfg);
int wtk_cblas_cfg_clean(wtk_cblas_cfg_t *cfg);
int wtk_cblas_cfg_update(wtk_cblas_cfg_t *cfg);

int wtk_cblas_cfg_out_cols(wtk_cblas_cfg_t *cfg);
int wtk_cblas_cfg_in_rows(wtk_cblas_cfg_t *cfg);
int wtk_cblas_cfg_in_cols(wtk_cblas_cfg_t *cfg);

#ifdef __cplusplus
};
#endif
#endif
