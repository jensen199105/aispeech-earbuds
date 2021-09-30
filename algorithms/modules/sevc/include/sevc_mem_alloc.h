/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : aispeech_mem_alloc.h
  Project    :
  Module     :
  Version    :
  Date       : 2017/08/03
  Author     : ziyuan.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  ziyuan.Jiang  2016/08/03         1.00              Create

************************************************************/
#ifndef __AISP_TSL_MEM_ALLOC_H__
#define __AISP_TSL_MEM_ALLOC_H__

#include "AISP_TSL_types.h"


#ifdef JIELI_BR28
extern void *calloc2(unsigned long count, unsigned long size);
extern void free2(void *rmem);
#define   dlmalloc(_size_)      calloc2(1, _size_)
#define   dlcalloc(_size_)      calloc2(1, _size_)
#define   dlfree(_point_)       free2(_point_)
#else
#define   dlmalloc(_size_)      calloc(1, _size_)
#define   dlcalloc(_size_)      calloc(1, _size_)
#define   dlfree(_point_)       free(_point_)
#endif

#define AISP_ALIGN_OFF     (0)
#define AISP_ALIGN_ON      (1)

#define AISP_ALIGN_HEX_MASK      (0x07)

#define AISP_TSL_MEM_ROUND_8Byte(size) ((((size)&7) == 0) ? (size) : ((size) + 8 - ((size)&7)))

typedef struct tagAisp_Tsl_memPointerNode
{
    void **pPointer;
    size_t offset;
    struct tagAisp_Tsl_memPointerNode *next;
} AISP_TSL_memPointerNode_T;

typedef struct tagAisp_Tsl_memAlloc
{
    char error;
    size_t totalSize;
    AISP_TSL_memPointerNode_T *front;
} AISP_TSL_memAllocator_T;

#define AISP_TSL_err(...)           do    \
                                {    \
                                    printf("\n****************** AISP_TSL Error in %s() %d *********************\n", __FUNCTION__, __LINE__);    \
                                    printf(__VA_ARGS__); \
                                    printf("******************    AISP_TSL Error End     *********************\n"); \
                                } while (0)

#define AISP_TSL_info(...)          do    \
                                {    \
                                    printf("\n****************** AISP_TSL Info in %s() %d ***********************\n", __FUNCTION__, __LINE__);    \
                                    printf(__VA_ARGS__); \
                                    printf("******************    AISP_TSL Info End     ***********************\n"); \
                                } while (0)

#define AISP_TSL_PTR_FREE(ptr)   do{\
                                        if (ptr) \
                                        {   \
                                            dlfree(ptr);\
                                            ptr = NULL;\
                                        }   \
                                    } while (0)

#define AISP_TSL_PTR_CHECK(ptr, flag) do{\
                                        if (NULL == ptr) \
                                        {   \
                                          AISP_TSL_err("%s(): %d point is NULL.\n", __FUNCTION__, __LINE__);\
                                            goto flag;\
                                        }   \
                                       } while (0)

/* initialze memory allocator */
AISP_TSL_memAllocator_T *AISP_TSL_mem_allocatorInit();

/*
 * calloc one big block of memory and assigned the registered
 * pointer with correct memory position in the memory block.
 * return the pointer of the memory block on success, NULL on fail.
 */
void *AISP_TSL_mem_allocatorAlloc(AISP_TSL_memAllocator_T *pMemAllocator, unsigned int *pulSize);

/*
 * register the pointer and the size that need to be allocated to it.
 * it also record whether the memory need to be 8 byte alligned.
 */
void AISP_TSL_mem_allocatorRegister(AISP_TSL_memAllocator_T *pMemAllocator, void **pointer, size_t size,
                                    char eightByteAlignment);

/* delete the memory allocator */
void AISP_TSL_mem_allocatorDelete(AISP_TSL_memAllocator_T *pMemAllocator);

U32 AISP_TSL_mask_align(U32 size, U32 mask);

#endif
