#ifndef WTK_VITE_PARM_POST_DNN_WTK_DNN_TYPE_H_
#define WTK_VITE_PARM_POST_DNN_WTK_DNN_TYPE_H_
#include "wtk/core/wtk_type.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    wtk_dnn_sigmoid,
    wtk_dnn_softmax,
    wtk_dnn_relu,
    wtk_dnn_linear,
} wtk_dnn_post_type_t;

#ifdef __cplusplus
};
#endif
#endif
