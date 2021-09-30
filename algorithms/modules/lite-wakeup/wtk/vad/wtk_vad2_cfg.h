#ifndef WTK_VAD_WTK_VAD2_CFG_H_
#define WTK_VAD_WTK_VAD2_CFG_H_
#include "wtk/vad/dnnvad/wtk_dnnvad_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_vad2_cfg wtk_vad2_cfg_t;

struct wtk_vad2_cfg
{
    wtk_dnnvad_cfg_t dnnvad;        ///< DNN VAD配置资源

    int left_margin;		//left margin frame count;
    int right_margin;		//right margin frame count;
    int min_speech;
};

int wtk_vad2_cfg_init(wtk_vad2_cfg_t *cfg);
int wtk_vad2_cfg_clean(wtk_vad2_cfg_t *cfg);
int wtk_vad2_cfg_update_local(wtk_vad2_cfg_t *cfg,void *lc);
int wtk_vad2_cfg_update(wtk_vad2_cfg_t *cfg);

int wtk_vad2_cfg_delete(wtk_vad2_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
