/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : lite-wakeup.c
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

#ifdef WAKEUP_NCHANNEL

#include "api_nwakeup.h"
#include "lite-nwakeup.h"

#ifdef LWKP_API_EXPORT_EN
#define WKP_API_PUBLIC      __attribute__ ((visibility ("default")))
#else
#define WKP_API_PUBLIC
#endif

WKP_API_PUBLIC nwakeup_t *LNWKP_new(int chans, char *mem_ptr, int size)
{
    return (nwakeup_t *)wakeup_Nchans_new((char *)mem_ptr, chans, size, 0, 0, 0);
}

WKP_API_PUBLIC void LNWKP_delete(nwakeup_t *w)
{
    wakeup_Nchans_delete((wakeup_Nchans_t *)w);
}

WKP_API_PUBLIC void LNWKP_reset(nwakeup_t *w)
{
    wakeup_Nchans_reset((wakeup_Nchans_t *)w);
}

WKP_API_PUBLIC int LNWKP_start(nwakeup_t *w, char *env, int bytes)
{
    return wakeup_Nchans_start((wakeup_Nchans_t *)w, env, bytes);
}

WKP_API_PUBLIC int LNWKP_feed(nwakeup_t *w, int chan_index, char *data, int bytes, unsigned is_end)
{
    return wakeup_Nchans_feed((wakeup_Nchans_t *)w, chan_index, data, bytes, is_end);
}

WKP_API_PUBLIC int LNWKP_end(nwakeup_t *w)
{
    return wakeup_Nchans_end((wakeup_Nchans_t *)w);
}

WKP_API_PUBLIC void LNWKP_registerHandler(nwakeup_t *w, void *user_data, LNWKP_handler_t func)
{
    wakeup_Nchans_register_handler((wakeup_Nchans_t *)w, user_data, func);
}

#ifndef WKP_VAD_DISABLE
WKP_API_PUBLIC void LNWKP_registerVadHandler(nwakeup_t *w, void *user_data, LNWKP_vad_handler_t func)
{
    wakeup_Nchans_register_vad_handler((wakeup_Nchans_t *)w, user_data, func);
}
#endif

WKP_API_PUBLIC const char *LNWKP_version(void)
{
    return wakeup_Nchans_version();
}

#endif
