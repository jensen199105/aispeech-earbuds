#ifndef WTK_VAD_DNNVAD_WTK_DNNVAD_H_
#define WTK_VAD_DNNVAD_WTK_DNNVAD_H_
#include "wtk/core/wtk_type.h"
#include "wtk/vite/parm/wtk_parm.h"
#include "wtk/vite/parm/wtk_feature.h"
#include "wtk_dnnvad_cfg.h"
#include "wtk_dnnvad_post.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_dnnvad wtk_dnnvad_t;
typedef void (*wtk_vframe_raise_f)(void *hook,wtk_frv_t *f);

struct wtk_dnnvad {
    wtk_dnnvad_cfg_t *cfg;
    //---------- audio section -----------
    wtk_robin_t *feat_robin; // save feature,used as window;
    //---------------------------------
    void *raise_ths;
    wtk_vframe_raise_f raise;
    wtk_dnnvad_post_t *post;
};

wtk_dnnvad_t *wtk_dnnvad_new(wtk_dnnvad_cfg_t *cfg, void *raise_ths, wtk_vframe_raise_f raise);
void wtk_dnnvad_delete(wtk_dnnvad_t *v);
void wtk_dnnvad_reset(wtk_dnnvad_t *v);
int wtk_dnnvad_feed(wtk_dnnvad_t *v, wtk_parm_state_t state,wtk_frv_t *frv);
void wtk_dnnvad_push_frv(wtk_dnnvad_t *v, wtk_frv_t *f);
void wtk_dnnvad_raise_frv(wtk_dnnvad_t *v, wtk_frv_t *frv, int is_sil);
void wtk_dnnvad_push_feat(wtk_dnnvad_t *v, wtk_dnnvad_feat_t *f);
void wtk_dnnvad_flush_frame_queue(wtk_dnnvad_t *v, wtk_queue_t *q);
void wtk_dnnvad_flush(wtk_dnnvad_t *v);
void wtk_dnnvad_feed_end(wtk_dnnvad_t *v);
#ifdef __cplusplus
};
#endif
#endif
