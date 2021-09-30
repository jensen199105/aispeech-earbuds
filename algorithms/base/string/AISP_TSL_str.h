/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : AISP_TSL_str.h
  Project    :
  Module     :
  Version    :
  Date       : 2019/08/28
  Author     : Jin.Chen
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Jin.Chen      2019/07/15        1.00              Create

************************************************************/

#ifndef __AISP_TSL_STR_H__
#define __AISP_TSL_STR_H__

#include "AISP_TSL_types.h"

S32 AISP_TSL_isspace(S32 iChr);
S32 AISP_TSL_strlen(const S8 *pcStr);
F32 AISP_TSL_strtof(const S8 *pcStr);
S8 *AISP_TSL_strncat(S8 *dest, const S8 *src, S32 count);
VOID AISP_TSL_itoa(S32 n, S8 *s);
VOID *AISP_TSL_memset(VOID *s, S32 c, U64 count);
S32 AISP_TSL_strcmp(const S8 *cs, const S8 *ct);
S32 AISP_TSL_strncmp(const S8 *cs, const S8 *ct, S32 count);
VOID *AISP_TSL_memcpy(VOID *dest, const VOID *src, U64 count);
VOID *AISP_TSL_memmove(VOID *dest, const VOID *src, U64 count);
S32 AISP_TSL_memcmp(const VOID *cs, const VOID *ct, U64 count);
S8 *AISP_TSL_strstr(const S8 *s1, const S8 *s2);
S8 *AISP_TSL_strcpy(S8 *dest, const S8 *src);
U32 AISP_TSL_atoi(const S8 *str);
S8 *AISP_TSL_strchr(const S8 *s, S32 c);
S8 *AISP_TSL_strtok_r(S8 *s, const S8 *delim, S8 **save_ptr);
#endif