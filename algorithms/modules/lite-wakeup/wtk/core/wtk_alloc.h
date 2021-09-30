#ifndef WLIB_CORE_WTK_ALLOC_H_
#define WLIB_CORE_WTK_ALLOC_H_

#include "wtk_type.h"
#ifdef __cplusplus
extern "C" {
#endif

#define wtk_align_ptr(ptr, align) \
    ((align)>0 ? (void*)(((unsigned long)(ptr)+(align-1)) & (~(align-1))) : ptr)

#define wtk_round(size,align) \
 ((align)>0 ? (((size)+((align)-1))&(~((align)-1))) : size)

#define wtk_round_8(size) ((((size)&7)==0)? (size) : ((size)+8-((size)&7)))
#define wtk_round_16(size) ((((size)&15)==0)? (size) : ((size)+16-((size)&15)))

#ifndef NULL
#define NULL 0
#endif

typedef struct _m_malloc_dev _m_malloc_dev_t;

#define MEM_BLOCK_SIZE          (8)         // aligned by 8 bytes

void wtk_mem_memset(void *s, unsigned char c, unsigned int count);
void wtk_mem_memcpy(void *des, void *src, unsigned int  n);
void wtk_memInit(char *mem_ptr, unsigned int size);
char *wtk_memAlloc(unsigned int size);
void *wtk_mem_malloc(unsigned int size);
void *wtk_mem_calloc(unsigned int nem, unsigned int size);

struct _m_malloc_dev
{
    char  *cur;                        // the current address, alloc from hign address to low address
    char  *end;                        // the end     address
    char  *start;                      // the start   address
    unsigned char  memrdy;
};

#ifdef PC_PLATFORM
#define wtk_free(p)              free(p)
#define wtk_malloc(n)            malloc(n)
#define wtk_calloc(nmem,size)    calloc(nmem,size)
#endif

#ifdef DSP_PLATFORM
#define wtk_free(p)             (p)=(p)
#define wtk_malloc(n)           wtk_mem_malloc(n)
#define wtk_calloc(nmem,size)   wtk_mem_calloc(nmem,size)
#endif


#ifdef __cplusplus
};
#endif

#endif

