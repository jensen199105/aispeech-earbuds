/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : lwakeup.h
  Project    :
  Module     :
  Version    :
  Date       : 2019/07/22
  Author     : Youhai.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Youhai.Jiang  2019/07/22         1.00              Create

************************************************************/
#ifndef __LWAKEUP_H__
#define __LWAKEUP_H__

typedef struct wakeup wakeup_t;
typedef int (*LWKP_handler_t)(void *user_data, int status, char *json, int bytes);

wakeup_t *LWKP_new(char *mem_ptr, int size);
void LWKP_delete(wakeup_t *w);
void LWKP_reset(wakeup_t *w);
int LWKP_start(wakeup_t *w, char *env, int bytes);
int LWKP_feed(wakeup_t *w, char *data, int bytes);
int LWKP_end(wakeup_t *w);
void LWKP_registerHandler(wakeup_t *w, void *user_data, LWKP_handler_t func);
void LWKP_registerFeatureHandler(wakeup_t* w, void* user_data, void* func);
const char *LWKP_version(void);

#endif
