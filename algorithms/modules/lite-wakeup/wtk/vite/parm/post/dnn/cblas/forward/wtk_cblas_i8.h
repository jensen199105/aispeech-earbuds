/**
 * @file wtk_cblas_i8.h
 * @brief
 * @author xtg
 * @version 0.1
 * @date 2017-05-26
 */

#ifndef __WTK_VITE_PARM_POST_DNN_CBLAS_FORWARD_WTK_CBLAS_I8_H__
#define __WTK_VITE_PARM_POST_DNN_CBLAS_FORWARD_WTK_CBLAS_I8_H__

#include "wtk/vite/math/cblas.h"
#include "../wtk_cblas.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

void wtk_cblas_process_matrix_i8(wtk_cblas_t *d, wtk_cblas_matrix_t *m);

#ifdef __cplusplus
};
#endif //__cplusplus

#endif //__WTK_VITE_PARM_POST_DNN_FORWARD_WTK_CBLAS_I8_H__
