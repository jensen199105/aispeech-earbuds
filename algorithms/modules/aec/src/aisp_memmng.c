/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : aisp_memmng.c
  Project    :
  Module     :
  Version    :
  Date       : 2019/03/05
  Author     : Youhai.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  chfs          2019/02/20         1.00             Create
  Youhai.Jiang  2019/03/05         1.01             Modified

************************************************************/
#include <stdlib.h>
#include <string.h>
#include "AISP_TSL_str.h"
#include "aisp_memmng.h"


typedef long SIZE_T;

/* Define the linked list structure.  This is used to link free blocks in order
of their memory address. */
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK *pstNextFreeBlock;  /*<< The next free block in the list. */
    size_t xBlockSize;                      /*<< The size of the free block. */
} BlockLink_t;

/* configurations */
//#define MULTI_THREAD_ENABLE
#if (MIC_CHANNEL_NUM == 2)
#ifdef FESPL_TOOLS
#define AISP_MEM_TOTAL_HEAP_SIZE    (600*(1<<10))
#else
#if defined(GSC_WKP) || defined(ORI_WKP)
#define AISP_MEM_TOTAL_HEAP_SIZE    (400*(1<<10)) // 10MB564
#else
#define AISP_MEM_TOTAL_HEAP_SIZE    (564*(1<<10)) // 10MB564
#endif
#endif
#else
#ifdef FESPL_TOOLS
#define AISP_MEM_TOTAL_HEAP_SIZE    (1000*(1<<10))
#else
#if defined(GSC_WKP) || defined(ORI_WKP)
#define AISP_MEM_TOTAL_HEAP_SIZE    (800*(1<<10)) // 10MB564
#else
#define AISP_MEM_TOTAL_HEAP_SIZE    (1000*(1<<10)) // 10MB564
#endif
#endif

#endif


#define AISP_MEM_ALIGN              (8)
#define AISP_MEM_ALIGN_MASK         (AISP_MEM_ALIGN - 1)
//#define AISP_ASSERT(x)              assert(x)
#define AISP_ASSERT(x)

#if (AISP_LOG_LEVEL == 0)
#define AISP_MEM_LOG(fmt, args...) printf(fmt, ##args)
#endif

/* Block sizes must not get too small. */
#define MINIMUM_BLOCK_SIZE  ((size_t) (((sizeof(BlockLink_t) + (AISP_MEM_ALIGN - 1)) & ~AISP_MEM_ALIGN_MASK)<<1))

/* Assumes 8bit bytes! */
#define BITS_PER_BYTE       ((size_t) 8)
/* Allocate the memory for the heap. */
//static uint8_t ucHeap[AISP_MEM_TOTAL_HEAP_SIZE];
//uint8_t* ucHeap = (uint8_t*)calloc(1,550*1024);  //[AISP_MEM_TOTAL_HEAP_SIZE];
#ifndef USE_HEAP
#define USE_HEAP
#endif

#ifdef USE_HEAP
uint8_t *ucHeap = NULL;  //[AISP_MEM_TOTAL_HEAP_SIZE];
#else
uint8_t ucHeap[AISP_MEM_TOTAL_HEAP_SIZE];
#endif
#ifdef MULTI_THREAD_ENABLE
pthread_mutex_t mutex_l;
#endif

/*
 * The size of the structure placed at the beginning of each allocated memory
 * block must by correctly byte aligned.
 */
static const size_t xHeapStructSize = ((sizeof(BlockLink_t) + (AISP_MEM_ALIGN - 1)) & ~AISP_MEM_ALIGN_MASK);

/* Create a couple of list links to mark the start and end of the list. */
static BlockLink_t  xStart;
static BlockLink_t *pstEndBlock = NULL;

/* Keeps track of the number of free bytes remaining, but says nothing about fragmentation. */
static size_t xFreeBytesRemaining = 0U;
static size_t xMinimumEverFreeBytesRemaining = 0U;

/*
 * Gets set to the top bit of an size_t type.  When this bit in the xBlockSize
 * member of an BlockLink_t structure is set then the block belongs to the
 * application.  When the bit is free the block is still part of the free heap
 * space.
 */
static size_t xBlockAllocatedBit = 0;
static int aisp_mem_initilized   = 0;

#ifdef MTK_HEAP_SIZE_GUARD_ENABLE
/* record first block of heap for heap walk */
BlockLink_t *pxFirstBlock;
#endif

//extern void (*AISP_DEBUG_PRINT)(char *fmt, ...);

/************************************************************
  Function   : memInsertBlockIntoFreeList()

  Description: Inserts a block of memory that is being freed
               into the correct position in the list of free
               memory blocks.  The block being freed will be
               merged with the block in front it and/or the
               block behind it if the memory blocks are
               adjacent to each other.
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/03/06, Youhai.Jiang create

************************************************************/
static void memInsertBlockIntoFreeList(BlockLink_t *pstBlockToInsert)
{
    uint8_t *pucAddr = NULL;
    BlockLink_t *pstIterator = &xStart;

    /*
     * Iterate through the list until a block is found that has a higher address
     * than the block being inserted.
     */
    for (; pstIterator->pstNextFreeBlock < pstBlockToInsert; pstIterator = pstIterator->pstNextFreeBlock)
    {
        /* Nothing to do here, just iterate to the right position. */
    }

    /*
     * Do the block being inserted, and the block it is being inserted after
     * make a contiguous block of memory?
     */
    pucAddr = (uint8_t *) pstIterator;

    if ((pucAddr + pstIterator->xBlockSize) == (uint8_t *) pstBlockToInsert)
    {
        pstIterator->xBlockSize += pstBlockToInsert->xBlockSize;
        pstBlockToInsert = pstIterator;
    }

    /*
     * Do the block being inserted, and the block it is being inserted before
     * make a contiguous block of memory?
     */
    pucAddr = (uint8_t *) pstBlockToInsert;

    if ((pucAddr + pstBlockToInsert->xBlockSize) == (uint8_t *) pstIterator->pstNextFreeBlock)
    {
        if (pstIterator->pstNextFreeBlock != pstEndBlock)
        {
            /* Form one big block from the two blocks. */
            pstBlockToInsert->xBlockSize += pstIterator->pstNextFreeBlock->xBlockSize;
            pstBlockToInsert->pstNextFreeBlock = pstIterator->pstNextFreeBlock->pstNextFreeBlock;
        }
        else
        {
            pstBlockToInsert->pstNextFreeBlock = pstEndBlock;
        }
    }
    else
    {
        pstBlockToInsert->pstNextFreeBlock = pstIterator->pstNextFreeBlock;
    }

    /* If the block being inserted plugged a gab, so was merged with the block
    before and the block after, then it's pstNextFreeBlock pointer will have
    already been set, and should not be set here as that would make it point
    to itself. */
    if (pstIterator != pstBlockToInsert)
    {
        pstIterator->pstNextFreeBlock = pstBlockToInsert;
    }
}

/************************************************************
  Function   : AISP_MemInit()

  Description: Called automatically to setup the required
               heap structures the first time AISP_MemMalloc()
               is called
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/03/06, Youhai.Jiang create

************************************************************/
void AISP_MemInit(void *pvStartAddr, unsigned long heapSize)
{
    char *pcAlignedHeap = NULL;
    BlockLink_t *pxFirstFreeBlock = NULL;
    SIZE_T address = 0;
    size_t xTotalHeapSize = heapSize;
#ifdef USE_HEAP

    if (pvStartAddr)
    {
        AISP_MEM_FREE(pvStartAddr);
    }

#endif

    /* Ensure the heap starts on a correctly aligned boundary. */
    if (NULL == pvStartAddr)
    {
#ifdef USE_HEAP
        ucHeap = (uint8_t *)calloc(1, AISP_MEM_TOTAL_HEAP_SIZE); //[AISP_MEM_TOTAL_HEAP_SIZE];

        if (ucHeap == NULL)
        {
#ifdef AISP_TSL_INFO
            printf("uHeap malloc FAIL\n");
#endif
            return ;
        }

        AISP_TSL_memset(ucHeap, 0, AISP_MEM_TOTAL_HEAP_SIZE);
#endif
        pvStartAddr = ucHeap;
        AISP_MEM_LOG("ucHeap:%p\n", ucHeap);
    }

    address = (SIZE_T) pvStartAddr;

    if (address & AISP_MEM_ALIGN_MASK)
    {
        address += (AISP_MEM_ALIGN - 1);
        address &= ~AISP_MEM_ALIGN_MASK;
        xTotalHeapSize -= address - (SIZE_T) pvStartAddr;
    }

    pcAlignedHeap = (char *) address;
    /* xStart is used to hold a pointer to the first item in the list of free
    blocks.  The void cast is used to prevent compiler warnings. */
    xStart.pstNextFreeBlock = (void *)pcAlignedHeap;
    xStart.xBlockSize = (size_t) 0;
    /* pstEndBlock is used to mark the end of the list of free blocks and is inserted
    at the end of the heap space. */
    address = ((SIZE_T)pcAlignedHeap) + xTotalHeapSize;
    address -= xHeapStructSize;
    address &= ~AISP_MEM_ALIGN_MASK;
    pstEndBlock = (void *) address;
    pstEndBlock->xBlockSize = 0;
    pstEndBlock->pstNextFreeBlock = NULL;
    /*
     * To start with there is a single free block that is sized to
     * take up the entire heap space, minus the space taken by
     * pstEndBlock
     */
    pxFirstFreeBlock = (void *) pcAlignedHeap;
    pxFirstFreeBlock->xBlockSize = address - (SIZE_T) pxFirstFreeBlock;
    pxFirstFreeBlock->pstNextFreeBlock = pstEndBlock;
#ifdef MTK_HEAP_SIZE_GUARD_ENABLE
    pxFirstBlock = pxFirstFreeBlock;
#endif
    /* Only one block exists - and it covers the entire usable heap space. */
    xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
    xFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
    /* Work out the position of the top bit in a size_t variable. */
    xBlockAllocatedBit = ((size_t) 1) << ((sizeof(size_t) * BITS_PER_BYTE) - 1);
}

/************************************************************
  Function   : AISP_MemMalloc()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/03/06, Youhai.Jiang create

************************************************************/
void *AISP_MemMalloc(size_t xWantedSize)
{
    void *pvReturn = NULL;
    BlockLink_t *pstBlock = NULL;
    BlockLink_t *pstPrevBlock = NULL;
    BlockLink_t *pstNewBlock  = NULL;

    if (!aisp_mem_initilized)
    {
#ifdef MULTI_THREAD_ENABLE
        pthread_mutex_init(&(mutex_l), NULL);
#endif
        aisp_mem_initilized = 1;
    }

#ifdef MULTI_THREAD_ENABLE
    pthread_mutex_lock(&(mutex_l));
#endif

    /*
     * If this is the first call to malloc then the heap will require
     * initialization to setup the list of free blocks.
     */
    if (NULL == pstEndBlock)
    {
        AISP_MEM_LOG("Use internal memory, size is %d\n", AISP_MEM_TOTAL_HEAP_SIZE);
        AISP_MemInit(ucHeap, AISP_MEM_TOTAL_HEAP_SIZE);
        AISP_MEM_LOG("memInit after...\n");
    }

    /*
     * Check the requested block size is not so large that the top bit is set.
     * The top bit of the block size member of the BlockLink_t structure
     * is used to determine who owns the block - the application or the
     * kernel, so it must be free.
     */
    if ((xWantedSize & xBlockAllocatedBit) == 0)
    {
        /*
         * The wanted size is increased so it can contain a BlockLink_t
         * structure in addition to the requested amount of bytes.
         */
        if (xWantedSize > 0)
        {
            xWantedSize += xHeapStructSize;

            /*
             * Ensure that blocks are always aligned to the required number
             * of bytes.
             */
            if (xWantedSize & AISP_MEM_ALIGN_MASK)
            {
                /* Byte alignment required. */
                xWantedSize += (AISP_MEM_ALIGN - (xWantedSize & AISP_MEM_ALIGN_MASK));
                AISP_ASSERT((xWantedSize & AISP_MEM_ALIGN_MASK) == 0);
            }

            //AISP_DEBUG_PRINT("You total %ld Bytes.\n", AISP_MEM_TOTAL_HEAP_SIZE);
            if (xWantedSize <= xFreeBytesRemaining)
            {
                /*
                 * Traverse the list from the start (lowest address) block until
                 * one of adequate size is found.
                 */
                pstPrevBlock = &xStart;
                pstBlock     = xStart.pstNextFreeBlock;

                while ((pstBlock->xBlockSize < xWantedSize) && (NULL != pstBlock->pstNextFreeBlock))
                {
                    pstPrevBlock = pstBlock;
                    pstBlock = pstBlock->pstNextFreeBlock;
                }

                /*
                 * If the end marker was reached then a block of adequate size
                 * was not found.
                 */
                if (pstBlock != pstEndBlock)
                {
                    /*
                     * Return the memory space pointed to - jumping over the
                     * BlockLink_t structure at its start.
                     */
                    pvReturn = (void *)((uint8_t *) pstBlock + xHeapStructSize);
                    /*
                     * This block is being returned for use so must be taken out
                     * of the list of free blocks.
                     */
                    pstPrevBlock->pstNextFreeBlock = pstBlock->pstNextFreeBlock;

                    /*
                     * If the block is larger than required it can be split into
                     * two.
                     */
                    if ((pstBlock->xBlockSize - xWantedSize) > MINIMUM_BLOCK_SIZE)
                    {
                        /*
                         * This block is to be split into two.  Create a new
                         * block following the number of bytes requested. The void
                         * cast is used to prevent byte alignment warnings from the
                         * compiler.
                         */
                        pstNewBlock = (void *)(((uint8_t *) pstBlock) + xWantedSize);
                        AISP_ASSERT((((SIZE_T) pstNewBlock) & AISP_MEM_ALIGN_MASK) == 0);
                        /*
                         * Calculate the sizes of two blocks split from the
                         * single block.
                         */
                        pstNewBlock->xBlockSize = pstBlock->xBlockSize - xWantedSize;
                        pstBlock->xBlockSize = xWantedSize;
                        /* Insert the new block into the list of free blocks. */
                        memInsertBlockIntoFreeList(pstNewBlock);
                    }

                    xFreeBytesRemaining -= pstBlock->xBlockSize;

                    /* xMinimumEverFreeBytesRemaining trace the minimum free memory */
                    if (xFreeBytesRemaining < xMinimumEverFreeBytesRemaining)
                    {
                        xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
                    }

                    /*
                     * The block is being returned - it is allocated and owned
                     * by the application and has no "next" block.
                     */
                    pstBlock->xBlockSize |= xBlockAllocatedBit;
                    pstBlock->pstNextFreeBlock = NULL;
                }
            }
        }
    }

#ifdef MULTI_THREAD_ENABLE
    pthread_mutex_unlock(&(mutex_l));
#endif
    return pvReturn;
}

/************************************************************
  Function   : AISP_MemCalloc()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/03/06, Youhai.Jiang create

************************************************************/
void *AISP_MemCalloc(size_t nmemb, size_t size)
{
    void *pvReturn = NULL;
    int lLen = nmemb * size;
    pvReturn = AISP_MemMalloc(lLen);

    if (pvReturn)
    {
        AISP_TSL_memset(pvReturn, 0, lLen);
    }
    else
    {
#ifdef AISP_TSL_INFO
        printf("AISP_MemCalloc is fail\n");
#endif
    }

    return pvReturn;
}

/************************************************************
  Function   : AISP_MemRealloc()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/03/06, Youhai.Jiang create

************************************************************/
void *AISP_MemRealloc(void *pv, size_t size)
{
    void        *pvReturn   = NULL;
    size_t       xBlockSize = 0;
    BlockLink_t *pxLink     = NULL;
    pvReturn = AISP_MemCalloc(1, size);

    if (pv)
    {
        /*
         * The memory being freeed still has an BlockLink_t structure immediately before it.
         */
        pxLink = (BlockLink_t *)((uint8_t *)pv - xHeapStructSize);
        /* Check the block is actually allocated */
        AISP_ASSERT(pxLink->xBlockSize & xBlockAllocatedBit);
        AISP_ASSERT(NULL == pxLink->pstNextFreeBlock);
        /* Get Original Block Size */
        xBlockSize = (pxLink->xBlockSize & ~xBlockAllocatedBit);
        /* Get Original data length */
        xBlockSize = (xBlockSize - xHeapStructSize);

        if (xBlockSize < size)
        {
            memcpy(pvReturn, pv, xBlockSize);
        }
        else
        {
            memcpy(pvReturn, pv, size);
        }

        /* Free Original Ptr */
        AISP_MemFree(pv);
    }

    return pvReturn;
}

/************************************************************
  Function   : AISP_MemFree()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/03/06, Youhai.Jiang create

************************************************************/
void AISP_MemFree(void *pvFreeAddr)
{
    BlockLink_t *pstToFreeBlock = NULL;
    uint8_t *pucAddr = (uint8_t *) pvFreeAddr;

    if (pucAddr)
    {
        /* The memory being freed will have an BlockLink_t structure immediately
        before it. */
        pucAddr -= xHeapStructSize;
        /* This casting is to keep the compiler from issuing warnings. */
        pstToFreeBlock = (void *) pucAddr;
        /* Check the block is actually allocated. */
        AISP_ASSERT((pstToFreeBlock->xBlockSize & xBlockAllocatedBit) != 0);
        AISP_ASSERT(pstToFreeBlock->pstNextFreeBlock == NULL);

        if (pstToFreeBlock->xBlockSize & xBlockAllocatedBit)
        {
            if (NULL == pstToFreeBlock->pstNextFreeBlock)
            {
                /*
                 * The block is being returned to the heap - it is no longer
                 * allocated.
                 */
                pstToFreeBlock->xBlockSize &= ~xBlockAllocatedBit;
#ifdef MULTI_THREAD_ENABLE
                pthread_mutex_lock(&(mutex_l));
#endif
                /* Add this block to the list of free blocks. */
                xFreeBytesRemaining += pstToFreeBlock->xBlockSize;
                memInsertBlockIntoFreeList(((BlockLink_t *) pstToFreeBlock));
#ifdef MULTI_THREAD_ENABLE
                pthread_mutex_unlock(&(mutex_l));
#endif
            }
        }
    }

    //AISP_DEBUG_PRINT("Remaining %ld Bytes.\n", xFreeBytesRemaining);
}

/************************************************************
  Function   : AISP_MemGetFreeSize()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/03/06, Youhai.Jiang create

************************************************************/
size_t AISP_MemGetFreeSize(void)
{
    return xFreeBytesRemaining;
}

/************************************************************
  Function   : AISP_MemGetMinEverFreeSize()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/03/06, Youhai.Jiang create

************************************************************/
size_t AISP_MemGetMinEverFreeSize(void)
{
    return xMinimumEverFreeBytesRemaining;
}

#ifdef MTK_HEAP_SIZE_GUARD_ENABLE
/************************************************************
  Function   : AISP_MemCheckAccessRegion()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/03/06, Youhai.Jiang create

************************************************************/
void AISP_MemCheckAccessRegion(void *addr, size_t size)
{
    BlockLink_t *blk_iter = pxFirstBlock;
    SIZE_T blk_size = 0;
    SIZE_T xAddr = (SIZE_T)addr;
    taskENTER_CRITICAL();

    while (blk_iter != pstEndBlock)
    {
        blk_size = (blk_iter->xBlockSize & ~xBlockAllocatedBit);

        if (xAddr >= (SIZE_T)blk_iter + sizeof(BlockLink_t)
                && xAddr < ((SIZE_T)blk_iter + blk_size))
        {
            if (xAddr + size > ((SIZE_T)blk_iter + blk_size))
            {
                AISP_ASSERT(0);
            }
        }

        blk_iter = (BlockLink_t *)((SIZE_T)blk_iter + blk_size);
    }

    taskEXIT_CRITICAL();
}

/************************************************************
  Function   : AISP_MemDumpInfo()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/03/06, Youhai.Jiang create

************************************************************/
void AISP_MemDumpInfo(void)
{
    BlockLink_t *blk_iter = pxFirstBlock;
    SIZE_T blk_size = 0;

    while (blk_iter != pstEndBlock)
    {
        blk_size = blk_iter->xBlockSize & ~xBlockAllocatedBit;
        AISP_MEM_LOG("block start = 0x%x,\t size = 0x%x \r\n", (unsigned int)blk_iter, (unsigned int)blk_iter->xBlockSize);
        blk_iter = (BlockLink_t *)((SIZE_T)blk_iter + blk_size);

        if ((SIZE_T)blk_iter > (SIZE_T)pstEndBlock)
        {
            AISP_MEM_LOG("heap crash!!!!! \r\n");
            AISP_ASSERT(0);
        }
    }

    AISP_MEM_LOG("block start = 0x%x,\t size = 0x%x \r\n", (unsigned int)blk_iter, (unsigned int)blk_iter->xBlockSize);
    AISP_MEM_LOG("reach blk_end \r\n");
}
#endif
