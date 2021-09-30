/**
 * @file wtk_vad2.h
 * 
 * @brief 各个vad的封装模块 
 * 
 * @date 2014-09-15
 * 
 * Last modified: 2014-09-28 11:38
 */


#ifndef WTK_VAD_WTK_VAD2_H_
#define WTK_VAD_WTK_VAD2_H_
#include "wtk/vad/dnnvad/wtk_dnnvad.h"
#include "wtk/vite/parm/wtk_feature.h"
#include "wtk/vite/parm/wtk_parm.h"
#include "wtk_vad2_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct wtk_vad2 wtk_vad2_t;

typedef enum {
    WTK_VAD2_SIL,
    WTK_VAD2_SPEECH,
    WTK_VAD2_MIN_SPEECH,
    WTK_VAD2_SPEECH_END,
} wtk_vad2_state_t;


struct wtk_vad2 {
    wtk_vad2_cfg_t *cfg;

    wtk_dnnvad_t *dnnvad;
    wtk_queue_t cache_q;        ///< 缓冲队列
    wtk_queue_t *output_queue;  ///< 最终的输出队列

    wtk_vad2_state_t state;     ///< 当前VAD状态
    wtk_parm_t *parm;
    int speech_end_sil_frames;  ///< speech结束之后接收到的sil帧数，用来处理右边界扩展
    int min_speech_count;       ///< 最小语音帧数
};


/**
 * @brief output_queue is wtk_vframe_t queue;
 */
wtk_vad2_t* wtk_vad2_new(wtk_vad2_cfg_t *cfg,wtk_parm_t *p,wtk_queue_t *output_queue);

void wtk_vad2_delete(wtk_vad2_t *v);
int wtk_vad2_start(wtk_vad2_t *v);
int wtk_vad2_reset(wtk_vad2_t *v);
int wtk_vad2_restart(wtk_vad2_t *vad);
int wtk_vad2_feed(wtk_vad2_t *vad,wtk_parm_state_t s,wtk_frv_t *frv);
void wtk_vad2_flush(wtk_vad2_t *vad);
void wtk_vad2_end(wtk_vad2_t *vad);
void wtk_vad2_flush_cache(wtk_vad2_t *v);
/**
 * @brief when frame poped from output_queue and no longer used, call wtk_vad_push_vframe
 *  to send vframe back to vad;
 */
void wtk_vad2_push_frv(wtk_vad2_t *vad,wtk_frv_t *f);


#ifdef __cplusplus
};
#endif
#endif
