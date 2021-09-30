#ifndef WTK_VITE_REC_DNN_WTK_DNN_H_
#define WTK_VITE_REC_DNN_WTK_DNN_H_

#include "wtk/core/wtk_robin.h"
#include "wtk/vite/math/wtk_math.h"
#include "wtk/vite/parm/post/dnn/cblas/wtk_cblas.h"
#include "wtk/vite/parm/wtk_feature.h"
#include "wtk_dnn_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wtk_dnn wtk_dnn_t;
struct wtk_parm;

struct wtk_dnn {
    wtk_dnn_cfg_t *cfg;
    struct wtk_parm *parm;
    wtk_cblas_t *cblas;
    wtk_robin_t *robin;
    wtk_feature_t **features;
    float *padding;
};

wtk_dnn_t *wtk_dnn_new(wtk_dnn_cfg_t *cfg, struct wtk_parm *parm);
void wtk_dnn_delete(wtk_dnn_t *d);
void wtk_dnn_reset(wtk_dnn_t *d);
void wtk_dnn_feed(wtk_dnn_t *d, wtk_feature_t *f);
void wtk_dnn_flush(wtk_dnn_t *d);
void wtk_dnn_flush_robin_all(wtk_dnn_t *d);
void wtk_dnn_raise_feature(wtk_dnn_t *d, wtk_feature_t *f);
void wtk_dnn_raise_frv(wtk_dnn_t *d, wtk_frv_t *frv);
void wtk_dnn_skip_feature(wtk_dnn_t *d, wtk_feature_t *f);

#ifdef __cplusplus
};
#endif
#endif
