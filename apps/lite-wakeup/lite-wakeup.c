/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : lite-wakeup.c
  Project    :
  Module     :
  Version    :
  Date       : 2018/11/15
  Author     : Youhai.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Youhai.Jiang  2018/11/15         1.00              Create

************************************************************/
#ifdef AISP_TSL_INFO
#include <stdio.h>
#endif
#include "api_wakeup.h"
#include "lite-wakeup.h"

#ifdef LWKP_API_EXPORT_EN
#define WKP_API_PUBLIC      __attribute__ ((visibility ("default")))
#else
#define WKP_API_PUBLIC
#endif

/************************************************************
  Function   : LWKP_new()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/22, Youhai.Jiang create

************************************************************/
WKP_API_PUBLIC wakeup_t *LWKP_new(char *mem_ptr, int size)
{
    return wakeup_new(mem_ptr, size);
}

/************************************************************
  Function   : LWKP_delete()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/22, Youhai.Jiang create

************************************************************/
WKP_API_PUBLIC void LWKP_delete(wakeup_t *w)
{
    wakeup_delete(w);
}

/************************************************************
  Function   : LWKP_reset()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/22, Youhai.Jiang create

************************************************************/
WKP_API_PUBLIC void LWKP_reset(wakeup_t *w)
{
    wakeup_reset(w);
}

/************************************************************
  Function   : LWKP_start()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/22, Youhai.Jiang create

************************************************************/
WKP_API_PUBLIC int LWKP_start(wakeup_t *w, char *env, int bytes)
{
    return wakeup_start(w, env, bytes);
}

/************************************************************
  Function   : LWKP_feed()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/22, Youhai.Jiang create

************************************************************/
WKP_API_PUBLIC int LWKP_feed(wakeup_t *w, char *data, int bytes)
{
    wakeup_feed(w, data, bytes);
    return bytes;
}

/************************************************************
  Function   : LWKP_end()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/22, Youhai.Jiang create

************************************************************/
WKP_API_PUBLIC int LWKP_end(wakeup_t *w)
{
    return wakeup_end(w);
}

/************************************************************
  Function   : LWKP_registerHandler()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/22, Youhai.Jiang create

************************************************************/
WKP_API_PUBLIC void LWKP_registerHandler(wakeup_t *w, void *user_data, LWKP_handler_t func)
{
    wakeup_register_handler(w, user_data, (wakeup_handler_t)func);
}

/************************************************************
  Function   : LWKP_registerFeatureHandler()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/08/06, Yun.Ren create

************************************************************/
WKP_API_PUBLIC void LWKP_registerFeatureHandler(wakeup_t *w, void *user_data, void *func)
{
    wakeup_register_feature_handler(w, user_data, (wakeup_feature_handler_t)func);
}

/************************************************************
  Function   : LWKP_version()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/22, Youhai.Jiang create

************************************************************/
WKP_API_PUBLIC const char *LWKP_version(void)
{
    return wakeup_version();
}

#ifndef WKP_VAD_DISABLE
/************************************************************
  Function   : LWKP_vadRegisterHandler()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/07/22, Youhai.Jiang create

************************************************************/
WKP_API_PUBLIC void LWKP_registerVadHandler(wakeup_t *w, void *user_data, LWKP_vad_handler_t func)
{
    wakeup_register_vad_handler(w, user_data, (vad_handler_t)func);
}
#endif
