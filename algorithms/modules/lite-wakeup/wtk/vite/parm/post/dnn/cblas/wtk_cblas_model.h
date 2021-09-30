/**
 * @file wtk_cblas_model.h
 * @brief    dnn model file
 * @author xtg
 * @version 0.1
 * @date 2017-05-25
 */

#ifndef __WTK_VITE_PARM_POST_DNN_WTK_CBLAS_MODEL_H__
#define __WTK_VITE_PARM_POST_DNN_WTK_CBLAS_MODEL_H__

#include "../wtk_dnn_type.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/vite/math/wtk_cblas_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
typedef struct wtk_cblas_cfg wtk_cblas_cfg_t;
typedef struct {
  wtk_queue_node_t q_n;
  wtk_dnn_post_type_t type;
  wtk_cblas_matrix_t *w; // wx+b
  wtk_cblas_vector_t *b;
} wtk_cblas_layer_t;

typedef struct {
  wtk_queue_node_t q_n;
  wtk_dnn_post_type_t type;
  wtk_cblas_matrix_i8_t *w; // wx+b
  wtk_cblas_vector_i32_t *b;
} wtk_cblas_layer_i8_t;

typedef struct {
  wtk_cblas_vector_t *b;
  wtk_cblas_vector_t *w; // window
} wtk_cblas_trans_t;

wtk_cblas_trans_t *wtk_cblas_trans_new(void);
void wtk_cblas_trans_delete(wtk_cblas_trans_t *t);

void *wtk_cblas_layer_new(wtk_cblas_cfg_t *cfg);
void wtk_cblas_layer_delete(wtk_cblas_cfg_t *cfg);

#ifdef __cplusplus
};
#endif //__cplusplus
#endif //__WTK_VITE_PARM_POST_DNN_WTK_CBLAS_MODEL_H__
