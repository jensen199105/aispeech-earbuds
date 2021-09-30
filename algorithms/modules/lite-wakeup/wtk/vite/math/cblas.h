#ifndef __CBLASH_H__
#define __CBLASH_H__

#include "AISP_TSL_types.h"
#include "wtk/core/wtk_type.h"
#include "wtk/vite/math/wtk_cblas_matrix.h"
#include "wtk/vite/parm/wtk_feature.h"

#define     Q6UPPER_BOUND       63
#define     Q6DOWN_BOUND        -64
#define     Q16VALUE            65536
#define     Q24VALUE            16777216

//TODO: let it configure as cmake var
#define     OUTPUT_Q24

void cblas_trans_process(aisp_s32_t *c, int row, int col, const aisp_s16_t *w, const aisp_s16_t *b);
void cblas_softmax_q24(int *data, int row, int col);
void cblas_sigmoid_fix_q24(int *data, int size);
void cblas_relu2_fixed8(int8_t *data, size_t vsize, int qfmt);

#if defined(WAKEUP_NN_DNN)
void cblas_sgemm_i8(const int M, const int N, const int K, const int8_t *A,
                    const int lda, const int8_t *B, const int ldb, int *C, const int ldc, const int *bias,
                    int *A_row_maxs, int *B_row_maxs);
void cblas_fix32tofix8(int8_t *out, aisp_s32_t *in, int row, int col, aisp_s32_t *max_rows);
void wtk_cblas_feature_to_matirx(wtk_cblas_matrix_t *m, int index, wtk_feature_t **pv, int step, int paddings,
                                 float *padding);
void cblas_q24_to_q6_version2(int8_t *out, int *row_maxs, int *in, int row, int col);
#elif defined(WAKEUP_NN_FSMN)
void cblas_sgemm_i8(const int M, const int N, const int K, const int8_t *A,
                    const int lda, const int8_t *B, const int ldb, int8_t *C, const int ldc, const int16_t *bias,
                    const int qfmta, const int qfmtb, const int qfmtc);
void cblas_sgemm_i32i16i32(const int M, const int N, const int K, const int32_t *A,
                           const int lda, const int16_t *B, const int ldb, int32_t *C, const int ldc, const int16_t *bias,
                           const int qfmta, const int qfmtb, const int qfmtc);
void cblas_fix32tofix8(int8_t *out, aisp_s32_t *in, int size, int qin, int qout);
void wtk_cblas_feature_to_matirx(int32_t *m, wtk_feature_t **pv, int step);
#endif


#endif
