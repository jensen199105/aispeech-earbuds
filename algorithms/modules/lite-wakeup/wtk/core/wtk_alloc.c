/*
 * =====================================================================================
 *
 *       Filename:  wtk_alloc.c
 *
 *    Description:  mempool for wakeup
 *
 *        Version:  1.0
 *        Created:  2018年01月03日 21时03分43秒
 *       Revision:  none
 *       Compiler:  gcc/xt-cc
 *
 *         Author:  Shengwei.Bai
 *   Organization:  AISPEECH Co.,Ltd.
 *
 * =====================================================================================
 */
#include "wtk_alloc.h"
#include "AISP_TSL_str.h"

/* The block size of memory is 8 by default. */
const unsigned int memblksize = MEM_BLOCK_SIZE;

_m_malloc_dev_t malloc_dev = {NULL, NULL, NULL, 0};

/************************************************************
 *   Function   : wtk_mem_memcpy()
 *   Description:
 *   Calls      :
 *   Called By  :
 *   Input      :
 *                des: destination address
 *                src: source address
 *                n:   n bytes
 *   Output     :
 *   Return     :
 *   Others     :
 *   History    :
 *       2019/10/16, chao.xu create
 *
 *************************************************************/
void wtk_mem_memcpy(void *des, void *src, unsigned int n)
{
    unsigned char *xdes = (unsigned char *)des;
    unsigned char *xsrc = (unsigned char *)src;

    while (n--)
    {
        *xdes++ = *xsrc++;
    }
}

/************************************************************
 *   Function   : wtk_mem_set()
 *   Description:
 *   Calls      :
 *   Called By  :
 *   Input      :
 *                src:   source address
 *                c:     the value to be set, 8-bit
 *                count: bytes
 *   Output     :
 *   Return     :
 *   Others     :
 *   History    :
 *       2019/10/16, chao.xu create
 *
 *************************************************************/
void wtk_mem_memset(void *s, unsigned char c, unsigned int count)
{
    unsigned char *xs = (unsigned char *)s;

    while (count--)
    {
        *xs++ = c;
    }
}

/************************************************************
 *   Function   : wtk_memInit()
 *   Description:
 *   Calls      :
 *   Called By  :
 *   Input      :
 *                mem_ptr: source address
 *                size:    bytes
 *   Output     :
 *   Return     :
 *   Others     :
 *   History    :
 *       2019/10/16, chao.xu create
 *
 *************************************************************/
void wtk_memInit(char *mem_ptr, unsigned int size)
{
    AISP_TSL_memset(mem_ptr, 0, size);
    malloc_dev.cur    = mem_ptr + size;
    malloc_dev.end    = mem_ptr + size - 1;
    malloc_dev.start  = mem_ptr;
    malloc_dev.memrdy = 1;  /* 1: initializa successfully. */
}

/************************************************************
 *   Function   : wtk_memAlloc()
 *   Description:
 *   Calls      :
 *   Called By  :
 *   Input      :
 *                size: bytes
 *   Output     :
 *   Return     : The address of alloced memory
 *   Others     :
 *   History    :
 *       2019/10/16, chao.xu create
 *
 *************************************************************/
char *wtk_memAlloc(unsigned int size)
{
    unsigned int nmemb = 0; // block number of memory

    //if(!malloc_dev.memrdy)wtk_memInit();    // if not intialized
    if (size == 0)
    {
        return NULL;
    }

    nmemb = size / memblksize;

    if (size % memblksize) // if divided without remainder
    {
        nmemb++;
    }

    malloc_dev.cur -= nmemb * memblksize;

    if (malloc_dev.cur < malloc_dev.start)
    {
        /* There is no memory to be alloced. */
        return NULL;
    }
    else
    {
        return malloc_dev.cur;
    }
}

/************************************************************
 *   Function   : wtk_mem_malloc()
 *   Description:
 *   Calls      :
 *   Called By  :
 *   Input      :
 *                size: bytes
 *   Output     :
 *   Return     : The address of alloced memory
 *   Others     :
 *   History    :
 *       2019/10/16, chao.xu create
 *
 *************************************************************/
void *wtk_mem_malloc(unsigned int size)
{
    char *p;
    p = wtk_memAlloc(size);
    return (void *)p;
}

/************************************************************
 *   Function   : wtk_mem_calloc()
 *   Description:
 *   Calls      :
 *   Called By  :
 *   Input      :
 *                size: bytes
 *   Output     :
 *   Return     : The address of alloced memory
 *   Others     :
 *   History    :
 *       2019/10/16, chao.xu create
 *
 *************************************************************/
void *wtk_mem_calloc(unsigned int nmem, unsigned int size)
{
    char *p;
    p = wtk_memAlloc(nmem * size);

    if (p == NULL)
    {
        return NULL;
    }

    wtk_mem_memset(p, 0, nmem * size);
    return (void *)(p);
}

