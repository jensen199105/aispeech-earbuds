#ifndef WTK_MATH_WTK_VECTOR_H_
#define WTK_MATH_WTK_VECTOR_H_

#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_alloc.h"

#ifdef AISP_TSL_INFO
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 *    v=[ 0,  1 ,  2,   3 ,  ... ,n ]
 *      size v[0] v[1] v[2]  ... v[n]
 */
#define wtk_vector_type_bytes(size,type) (wtk_round((size+1)*sizeof(type),8))

#define wtk_vector_size(v)       (*(int*)(v))
#define wtk_short_vector_size(v) (*(short*)v)

#define wtk_vector_init(v,size)  (*((int*)v)=size)
#define wtk_short_vector_init(v,size)  (*((short*)v)=size)

#define wtk_short_vector_do_p(v,pre,after) \
{ \
    wtk_short_vector_t *s,*e;\
    s=v;e=v+wtk_short_vector_size(v);\
    while((e-s)>=4)\
    {\
        pre *(++s) after;\
        pre *(++s) after;\
        pre *(++s) after;\
        pre *(++s) after;\
    }\
    while(s<e)\
    {\
        pre *(++s) after; \
    }\
}

#define wtk_vector_delete(v) wtk_free(v)

typedef int   wtk_int_vector_t;
typedef short wtk_short_vector_t;

wtk_short_vector_t *wtk_short_vector_new(int size);

#ifdef __cplusplus
};
#endif
#endif

