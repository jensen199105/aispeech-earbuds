/**
 * @file wtk_cblas_matrix.h
 * @brief
 * @author xtg
 * @version 0.1
 * @date 2017-05-24
 */
#ifndef __WTK_VITE_PARM_POST_DNN_WTK_CBLAS_MATRIX_H__
#define __WTK_VITE_PARM_POST_DNN_WTK_CBLAS_MATRIX_H__

#include "AISP_TSL_types.h"
#include "wtk/core/wtk_type.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

typedef struct wtk_cblas_vector
{
    unsigned int len;
    unsigned int bytes;
    aisp_s16_t *v;
} wtk_cblas_vector_t;

typedef struct wtk_cblas_vector_i8
{
    unsigned int len;
    unsigned int bytes;
    int8_t *v;
} wtk_cblas_vector_i8_t;

typedef struct wtk_cblas_vector_i16
{
    unsigned int len;
    unsigned int bytes;
    int16_t *v;
} wtk_cblas_vector_i16_t;

typedef struct wtk_cblas_vector_i32
{
    unsigned int len;
    unsigned int bytes;
    int *v;
} wtk_cblas_vector_i32_t;

typedef struct wtk_cblas_matrix
{
    unsigned int row;
    unsigned int col;
    aisp_s32_t *m;
} wtk_cblas_matrix_t;

typedef struct wtk_cblas_matrix_i8
{
    unsigned int row;
    unsigned int col;
    int8_t *m;
    int32_t *row_maxs; // each row max
} wtk_cblas_matrix_i8_t;

typedef struct wtk_cblas_matrix_i16
{
    unsigned int row;
    unsigned int col;
    const int16_t *m;
} wtk_cblas_matrix_i16_t;

typedef struct wtk_cblas_matrix_i32
{
    unsigned int row;
    unsigned int col;
    int32_t *m;
} wtk_cblas_matrix_i32_t;

wtk_cblas_vector_t *wtk_cblas_vector_new(int n);
void wtk_cblas_vector_delete(wtk_cblas_vector_t *v);

wtk_cblas_vector_i8_t *wtk_cblas_vector_i8_new(int n);
void wtk_cblas_vector_i8_delete(wtk_cblas_vector_i8_t *v);

wtk_cblas_vector_i16_t *wtk_cblas_vector_i16_new(int n);
void wtk_cblas_vector_i16_delete(wtk_cblas_vector_i16_t *v);

wtk_cblas_vector_i32_t *wtk_cblas_vector_i32_new(int n);
void wtk_cblas_vector_i32_delete(wtk_cblas_vector_i32_t *v);

wtk_cblas_matrix_t *wtk_cblas_matrix_new(int row, int col);
void wtk_cblas_matrix_delete(wtk_cblas_matrix_t *m);

wtk_cblas_matrix_i8_t *wtk_cblas_matrix_i8_new(int row, int col);
void wtk_cblas_matrix_i8_delete(wtk_cblas_matrix_i8_t *m);

wtk_cblas_matrix_i16_t *wtk_cblas_matrix_i16_new(int row, int col);
void wtk_cblas_matrix_i16_delete(wtk_cblas_matrix_i16_t *m);

wtk_cblas_matrix_i32_t *wtk_cblas_matrix_i32_new(int row, int col);
void wtk_cblas_matrix_i32_delete(wtk_cblas_matrix_i32_t *m);

#ifdef __cplusplus
};
#endif //__cplusplus
#endif //__WTK_VITE_PARM_POST_DNN_WTK_CBLAS_MATRIX_H__
