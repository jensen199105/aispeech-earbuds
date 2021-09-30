/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : aispeech_mem_alloc.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AISP_TSL_str.h"
#include "sevc_mem_alloc.h"

/************************************************************
  Function   : aispeech_mem_allocatorInit()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2017/08/03, ziyuan.Jiang create

************************************************************/
AISP_TSL_memAllocator_T *AISP_TSL_mem_allocatorInit(void)
{
    AISP_TSL_memAllocator_T *pstMemAlloc = NULL;
    pstMemAlloc = (AISP_TSL_memAllocator_T *)dlmalloc(sizeof(AISP_TSL_memAllocator_T));

    if (pstMemAlloc)
    {
        pstMemAlloc->error = 0;
        pstMemAlloc->totalSize = 0;
        pstMemAlloc->front = NULL;
    }

    return pstMemAlloc;
}

/************************************************************
  Function   : aispeech_mem_allocatorDelete()

  Description: Pop all the elements in the stack and
               delete the memory allocator
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create

************************************************************/
void AISP_TSL_mem_allocatorDelete(AISP_TSL_memAllocator_T *pstMemAllocator)
{
    AISP_TSL_memPointerNode_T *pstNode = NULL;

    while (NULL != pstMemAllocator->front)
    {
        pstNode = pstMemAllocator->front;
        dlfree(pstNode);
    }

    dlfree(pstMemAllocator);
}

/************************************************************
  Function   : aispeech_mem_allocatorRegister()

  Description: Register the pointer and the size that need to
               be allocated to it. It also record whether the
               memory need to be 8 byte alligned.

               This function push the pointer and the compute
               the offset and save them to the stack.
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create

************************************************************/
void AISP_TSL_mem_allocatorRegister(AISP_TSL_memAllocator_T *pstMemAllocator,
                                    void **pPointer, size_t size, char eightByteAlignment)
{
    AISP_TSL_memPointerNode_T *pstNode = NULL;
    pstNode = (AISP_TSL_memPointerNode_T *)dlmalloc(sizeof(AISP_TSL_memPointerNode_T));

    if (NULL == pstNode)
    {
        pstMemAllocator->error = 1;
        return;
    }

    pstNode->pPointer = pPointer;

    if (eightByteAlignment)
    {
        size_t offsetTmp = pstMemAllocator->totalSize;
        pstNode->offset = AISP_TSL_MEM_ROUND_8Byte(offsetTmp);
        pstMemAllocator->totalSize = pstNode->offset + size;
    }
    else
    {
        pstNode->offset = pstMemAllocator->totalSize;
        pstMemAllocator->totalSize += size;
    }

    pstNode->next = pstMemAllocator->front;
    pstMemAllocator->front = pstNode;
}

/************************************************************
  Function   : aispeech_mem_allocatorPop()

  Description: pop one pointer index pair
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2017/03/03, Youhai.Jiang create

************************************************************/
static inline AISP_TSL_memPointerNode_T *AISP_TSL_mem_allocatorPop(AISP_TSL_memAllocator_T *pstMemAllocator)
{
    AISP_TSL_memPointerNode_T *pstNode = NULL;

    if (pstMemAllocator->front)
    {
        pstNode = pstMemAllocator->front;
        pstMemAllocator->front = pstNode->next;
        pstNode->next = NULL;
        return pstNode;
    }
    else
    {
        return NULL;
    }
}

/************************************************************
  Function   : aispeech_mem_allocatorAlloc()

  Description: calloc a big block of memory and pop each of
               the pointer and the index and assigned the
               pointer with correct address.
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create

************************************************************/
void *AISP_TSL_mem_allocatorAlloc(AISP_TSL_memAllocator_T *pstMemAllocator,
                                  unsigned int *pulSize)
{
    char *pcTmp = NULL;
    void *memoryPool = NULL;
    AISP_TSL_memPointerNode_T *pstNode = NULL;

    if (pstMemAllocator->error == 0)
    {
        memoryPool = (void *)dlmalloc(pstMemAllocator->totalSize);

        if (memoryPool)
        {
            AISP_TSL_memset(memoryPool, 0, pstMemAllocator->totalSize);
        }
    }

    if (pstMemAllocator->error || memoryPool == NULL)
    {
        while ((pstNode = AISP_TSL_mem_allocatorPop(pstMemAllocator)))
        {
            *(pstNode->pPointer) = NULL;
            dlfree(pstNode);
        }

        return NULL;
    }

    *pulSize = pstMemAllocator->totalSize;
    pcTmp = (char *)memoryPool;

    while ((pstNode = AISP_TSL_mem_allocatorPop(pstMemAllocator)))
    {
        *(pstNode->pPointer) = (void *)(pcTmp + pstNode->offset);
        dlfree(pstNode);
    }

    return memoryPool;
}

/************************************************************
  Function   : AISP_TSL_mask_align(S32 size, S32 mask)

  Description:  memory size align
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/03, ziyuan.Jiang create

************************************************************/

U32 AISP_TSL_mask_align(U32 uiSize, U32 uiMask)
{
    return (uiSize + uiMask) & (~uiMask);
}


#ifdef MEM_ALLOC_TEST
struct test
{
    aisp_s16_t *p1;
    aisp_s16_t *p2;
    void *memoryPool;
};

int main(void)
{
    float tmp;
    struct test *p = (struct test *)dlmalloc(sizeof(struct test));
    AISP_TSL_memAllocator_T *allocator;
    allocator = AISP_TSL_mem_allocatorInit();
    AISP_TSL_mem_allocatorRegister(allocator, (void **) & (p->p1), 3 * sizeof(float), 0);
    AISP_TSL_mem_allocatorRegister(allocator, (void **) & (p->p2), 3 * sizeof(float), 1);
    p->memoryPool = AISP_TSL_mem_allocatorAlloc(allocator);
    AISP_TSL_mem_allocatorDelete(allocator);
    tmp = p->p1[0];
    tmp = p->p1[1];
    tmp = p->p1[2];
    tmp = p->p2[0];
    tmp = p->p2[1];
    tmp = p->p2[2];
    printf("%x\t%x\t%x\n", p->p1, p->p2, p->memoryPool);
    dlfree(p->memoryPool);
    dlfree(p);
}
#endif
