/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : aisp_memmng.h
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
#ifndef __AISP_MEMMNG_H__
#define __AISP_MEMMNG_H__

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#define AISP_MEM_MNG_EN

#ifdef __cplusplus
extern "C" {
#endif

void   AISP_MemInit(void *pvStarAddr, unsigned long size);
void * AISP_MemMalloc(size_t xWantedSize);
void * AISP_MemCalloc(size_t nmemb, size_t size);
void * AISP_MemRealloc(void *pv, size_t size);
void   AISP_MemFree(void *pv);
size_t AISP_MemGetMinEverFreeSize(void);
size_t AISP_MemGetFreeSize(void);

#ifdef AISP_MEM_MNG_EN
#define AISP_MEM_INIT(pvStarAddr, size) AISP_MemInit(pvStarAddr, size)
#define AISP_MEM_MALLOC(size)           AISP_MemMalloc(size)
#define AISP_MEM_CALLOC(nmemb, size)    AISP_MemCalloc(nmemb, size)
#define AISP_MEM_REALLOC(pv, size)      AISP_MemRealloc(pv, size)
#define AISP_MEM_FREE(pv)               AISP_MemFree(pv)
#else
#define AISP_MEM_INIT(pvStarAddr, size)
#define AISP_MEM_MALLOC(size)           malloc(size)
#define AISP_MEM_CALLOC(nmemb, size)    calloc(nmemb, size)
#define AISP_MEM_REALLOC(pv, size)      realloc(pv, size)
#define AISP_MEM_FREE(pv)               free(pv)
#endif

#ifdef __cplusplus
}
#endif
#endif

