/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : AISP_TSL_types.h
  Project    :
  Module     :
  Version    :
  Date       : 2019/04/28
  Author     : Chao.Xu
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Chao.Xu       2019/04/28        1.00              Create

************************************************************/
#ifndef __AISP_TSL_TYPES_H__
#define __AISP_TSL_TYPES_H__

#ifdef UNIT_TEST
#define STATIC
#else
#define STATIC static
#endif

typedef char                S8;
typedef short               S16;
typedef int                 S32;
typedef long long           S64;
typedef unsigned char       U8;
typedef unsigned short      U16;
typedef unsigned int        U32;
typedef unsigned long long  U64;
typedef float               F32;
#ifdef _WIN32
/*
 * Windows use #ifdef to test whether VOID defined,
 * If it is defined, it also means CHAR, SHORT and LONG
 * are all defined by typedef.
 */
#ifndef VOID
#define VOID                void
typedef char                CHAR;
typedef short               SHORT;
typedef long                LONG;
#endif
#else
#ifndef VOID
typedef void                VOID;
#endif
#endif

#ifdef AISPEECH_FIXED
typedef S8        aisp_s8_t;
typedef S16       aisp_s16_t;
typedef S32       aisp_s32_t;
typedef S64       aisp_s64_t;
typedef U16       aisp_u16_t;
typedef U32       aisp_u32_t;
typedef U64       aisp_u64_t;
#else
typedef float     aisp_s16_t;
typedef float     aisp_s32_t;
typedef double    aisp_s64_t;
typedef float     aisp_u16_t;
typedef float     aisp_u32_t;
typedef double    aisp_u64_t;
#endif

typedef struct S64_WITH_SYMBLE
{
#ifdef __PLATFORM_LE__
    unsigned long long value: 63;
    unsigned long long sign: 1;
#else
    unsigned long long sign: 1;
    unsigned long long value: 63;
#endif
} S64_WITH_SYMBLE_T;

typedef struct S32_WITH_SYMBLE
{
#ifdef __PLATFORM_LE__
    unsigned int value: 31;
    unsigned int sign: 1;
#else
    unsigned int sign: 1;
    unsigned int value: 31;
#endif
} S32_WITH_SYMBLE_T;

//#ifndef BOOL
//typedef char BOOL;
//#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif
