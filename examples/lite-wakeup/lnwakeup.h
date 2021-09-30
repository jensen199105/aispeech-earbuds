/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : lwakeup.h
  Project    :
  Module     :
  Version    :
  Date       : 2019/08/02
  Author     : Youhai.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Jin.Chen      2019/08/02         1.00              Create

************************************************************/
#ifndef __LNWAKEUP_H__
#define __LNWAKEUP_H__

typedef struct nwakeup nwakeup_t;

typedef int (*LNWKP_handler_t)(void* argv, int chan_index, char* json);

nwakeup_t* LNWKP_new(int chans, char* mem_ptr, int size);
void LNWKP_delete(nwakeup_t* w);
void LNWKP_reset(nwakeup_t* w);
int LNWKP_start(nwakeup_t* w, char* env, int bytes);
int LNWKP_feed(nwakeup_t* w, int chan_index, char* data, int bytes, unsigned is_end);
int LNWKP_end(nwakeup_t* w);
void LNWKP_registerHandler(nwakeup_t* w, void* user_data, LNWKP_handler_t func);
char* LNWKP_version(void);

#endif