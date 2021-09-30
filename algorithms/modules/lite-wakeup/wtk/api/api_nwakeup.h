/*
 * =====================================================================================
 *
 *       Filename:  api_nwakeup.h
 *
 *    Description:  header file for multi-channels wakeup
 *
 *        Version:  1.0
 *        Created:  2017年12月21日 08时32分53秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Shengwei.bai
 *   Organization:  AISpeech Co.,Ltd.
 *
 * =====================================================================================
 */
#ifndef __AISPEECH__API_N_WAKEUP_H__
#define __AISPEECH__API_N_WAKEUP_H__
#ifdef WAKEUP_NCHANNEL

typedef enum Nwakeup_status
{
    NWAKEUP_STATUS_ERROR = -1,     // 唤醒出现异常
    NWAKEUP_STATUS_WAIT,           // 等待唤醒
    NWAKEUP_STATUS_WOKEN,          // 已唤醒
    NWAKEUP_STATUS_WOKEN_BOUNDARY, // 当输入use_output_boundary=1时,输出唤醒词边界信息
    NWAKEUP_STATUS_RESTART,
} Nwakeup_status_t;

typedef struct wakeup_Nchans wakeup_Nchans_t;

typedef int (*wakeup_Nchans_handler)(void *argv, int chan_index, char *json);
typedef int (*wakeup_Nchans_vad_handler)(void *argv, int chan_index, int frame_state, int frame_index);

wakeup_Nchans_t *wakeup_Nchans_new(char *mem_ptr, int n, int size, void *hook, wakeup_Nchans_handler func,
                                   wakeup_Nchans_vad_handler vad_func);

void wakeup_Nchans_delete(wakeup_Nchans_t *w);

void wakeup_Nchans_reset(wakeup_Nchans_t *w);

void wakeup_Nchans_restart(wakeup_Nchans_t *w);

int wakeup_Nchans_start(wakeup_Nchans_t *w, char *env, int bytes);

int wakeup_Nchans_feed(wakeup_Nchans_t *w, int chan_index, char *data, int bytes, unsigned is_end);

int wakeup_Nchans_end(wakeup_Nchans_t *w);

int wakeup_Nchans_register_handler(wakeup_Nchans_t *w, void *hook, wakeup_Nchans_handler func);

#ifndef WKP_VAD_DISABLE
int wakeup_Nchans_register_vad_handler(wakeup_Nchans_t *w, void *hook, wakeup_Nchans_vad_handler func);
#endif

const char *wakeup_Nchans_version(void);

#endif
#endif
