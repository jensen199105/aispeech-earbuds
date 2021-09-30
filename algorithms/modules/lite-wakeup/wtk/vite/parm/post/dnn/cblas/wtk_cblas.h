/**
 * @file wtk_cblas.h
 * @brief
 * @author xtg
 * @version 0.1
 * @date 2017-05-25
 */

#ifndef WTK_VITE_PARM_POST_DNN_CBLAS_WTK_CBLAS_H_
#define WTK_VITE_PARM_POST_DNN_CBLAS_WTK_CBLAS_H_

#include "wtk/core/wtk_robin.h"
#include "wtk/core/wtk_type.h"
#include "wtk/vite/parm/wtk_feature.h"
#include "wtk_cblas_cfg.h"
#include "wtk/vite/math/wtk_cblas_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wtk_cblas wtk_cblas_t;
struct wtk_dnn;

struct wtk_cblas {
	int index;
	wtk_cblas_cfg_t *cfg;
	wtk_cblas_matrix_t *feat;
	wtk_cblas_matrix_t *tmp_out;
	wtk_cblas_matrix_i32_t *tmp0; // 8bit layer0 and layer1 input

	union {
		wtk_cblas_matrix_t *tmp_input;
		wtk_cblas_matrix_i8_t *tmp_input_i8;
		wtk_cblas_matrix_i16_t *tmp_input_i16;
	};

	wtk_robin_t *input_feature_robin; // cache robin for do work;

	wtk_queue_t *feature_hoard;
	wtk_queue_t *frv_hoard;
	struct wtk_dnn *dnn;
	unsigned int ifrmIdx;
	void *hook;
};

wtk_cblas_t *wtk_cblas_new(wtk_cblas_cfg_t *cfg, struct wtk_dnn *dnn);
void wtk_cblas_delete(wtk_cblas_t *b);
void wtk_cblas_reset(wtk_cblas_t *b);
void wtk_cblas_process_layer(wtk_cblas_t *d, wtk_feature_t **pv, int npv, wtk_feature_t *f);
void wtk_cblas_flush_layer(wtk_cblas_t *d);
void wtk_cblas_flush_layer_robin(wtk_cblas_t *d);
void wtk_cblas_flush_end(wtk_cblas_t *d);

#ifdef __cplusplus
};
#endif
#endif
