#include "wtk_cblas_matrix.h"
#ifdef AISP_TSL_INFO
#include <stdio.h>
#endif

#include "wtk/core/wtk_alloc.h"
wtk_cblas_vector_t *wtk_cblas_vector_new(int n)
{
    wtk_cblas_vector_t *v;
    v = (wtk_cblas_vector_t *)wtk_malloc(sizeof(*v));
    v->len = n;
    v->bytes = v->len * sizeof(float);
    v->v = NULL;
    //v->v = (float *)wtk_calloc(n, sizeof(float));
    return v;
}

void wtk_cblas_vector_delete(wtk_cblas_vector_t *v)
{
    wtk_free(v);
}

wtk_cblas_vector_i32_t *wtk_cblas_vector_i32_new(int n)
{
    wtk_cblas_vector_i32_t *v;
    v = (wtk_cblas_vector_i32_t *)wtk_malloc(sizeof(*v));
    v->len = n;
    v->bytes = v->len * sizeof(int);
    v->v = 0;
    //v->v = (int *)wtk_calloc(n, sizeof(int));
    return v;
}

void wtk_cblas_vector_i32_delete(wtk_cblas_vector_i32_t *v)
{
    //wtk_free(v->v);
    wtk_free(v);
}

wtk_cblas_vector_i8_t *wtk_cblas_vector_i8_new(int n)
{
    wtk_cblas_vector_i8_t *v;
    v = (wtk_cblas_vector_i8_t *)wtk_malloc(sizeof(*v));
    v->len = n;
    v->bytes = v->len * sizeof(int8_t);
    v->v = 0;
    v->v = (int8_t *)wtk_calloc(n, sizeof(int8_t));
    return v;
}

void wtk_cblas_vector_i8_delete(wtk_cblas_vector_i8_t *v)
{
    //wtk_free(v->v);
    wtk_free(v);
}

wtk_cblas_vector_i16_t *wtk_cblas_vector_i16_new(int n)
{
    wtk_cblas_vector_i16_t *v;
    v = (wtk_cblas_vector_i16_t *)wtk_malloc(sizeof(*v));
    v->len = n;
    v->bytes = v->len * sizeof(int8_t);
    v->v = 0;
    v->v = (int16_t *)wtk_calloc(n, sizeof(int16_t));
    return v;
}

void wtk_cblas_vector_i16_delete(wtk_cblas_vector_i16_t *v)
{
    wtk_free(v->v);
    wtk_free(v);
}

int wtk_cblas_vector_i16_bytes(wtk_cblas_vector_i16_t *v)
{
    int bytes = sizeof(wtk_cblas_vector_i16_t);
    bytes += v->len * sizeof(int16_t);
    return bytes;
}

wtk_cblas_matrix_t *wtk_cblas_matrix_new(int row, int col)
{
    wtk_cblas_matrix_t *m;
    m = (wtk_cblas_matrix_t *)wtk_malloc(sizeof(*m));
    m->row = row;
    m->col = col;
    m->m = NULL;
    //m->m = (float *)wtk_calloc(row * col, sizeof(float));
    return m;
}

void wtk_cblas_matrix_delete(wtk_cblas_matrix_t *m)
{
    wtk_free(m->m);
    wtk_free(m);
}

wtk_cblas_matrix_i8_t *wtk_cblas_matrix_i8_new(int row, int col)
{
    wtk_cblas_matrix_i8_t *m;
    m = (wtk_cblas_matrix_i8_t *)wtk_malloc(sizeof(*m));
    m->row = row;
    m->col = col;
    m->m = (int8_t *)wtk_calloc(row * col, sizeof(int8_t));
    m->row_maxs = (int32_t *)wtk_calloc(row, sizeof(int));
    return m;
}

void wtk_cblas_matrix_i8_delete(wtk_cblas_matrix_i8_t *m)
{
    wtk_free(m->m);
    wtk_free(m->row_maxs);
    wtk_free(m);
}

wtk_cblas_matrix_i16_t *wtk_cblas_matrix_i16_new(int row, int col)
{
    wtk_cblas_matrix_i16_t *m;
    m = (wtk_cblas_matrix_i16_t *)wtk_malloc(sizeof(*m));
    m->row = row;
    m->col = col;
    m->m = 0;
    m->m = (int16_t *)wtk_calloc(row * col, sizeof(int16_t));
    return m;
}

int wtk_cblas_matrix_i16_bytes(wtk_cblas_matrix_i16_t *m)
{
    int bytes = sizeof(wtk_cblas_matrix_i16_t);
    bytes += m->row * m->col * sizeof(int16_t);
    return bytes;
}

void wtk_cblas_matrix_i16_delete(wtk_cblas_matrix_i16_t *m)
{
    wtk_free(m->m);
    wtk_free(m);
}

wtk_cblas_matrix_i32_t *wtk_cblas_matrix_i32_new(int row, int col)
{
    wtk_cblas_matrix_i32_t *m;
    m = (wtk_cblas_matrix_i32_t *)wtk_malloc(sizeof(*m));
    m->row = row;
    m->col = col;
    m->m = 0;
    m->m = (int32_t *)wtk_calloc(row * col, sizeof(int32_t));
    return m;
}

void wtk_cblas_matrix_i32_delete(wtk_cblas_matrix_i32_t *m)
{
    wtk_free(m->m);
    wtk_free(m);
}
