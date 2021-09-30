#ifndef WTK_VAD_DNNVAD_WTK_DNNVAD_POST_H_
#define WTK_VAD_DNNVAD_WTK_DNNVAD_POST_H_
#include "wtk/core/wtk_robin.h"
#include "wtk/core/wtk_type.h"
#include "wtk/vite/parm/wtk_feature.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_dnnvad_post wtk_dnnvad_post_t;

typedef enum {
    WTK_DNNVAD_SIL,
    WTK_DNNVAD_SPEECH,
} wtk_dnnvad_state_t;

typedef struct {
    wtk_frv_t *frv;
    unsigned short ref;
    unsigned char is_sil;
} wtk_dnnvad_feat_t;

struct wtk_dnnvad;
struct wtk_dnnvad_post {
    wtk_dnnvad_state_t state;
    wtk_robin_t *sil_robin;    // save sil wtk_feature;
    wtk_robin_t *speech_robin; // save speech wtk_feature;
    struct wtk_dnnvad *vad;
};

wtk_dnnvad_post_t *wtk_dnnvad_post_new(struct wtk_dnnvad *vad);
void wtk_dnnvad_post_delete(wtk_dnnvad_post_t *p);
void wtk_dnnvad_post_reset(wtk_dnnvad_post_t *p);
void wtk_dnnvad_post_feed(wtk_dnnvad_post_t *p, wtk_frv_t *feat);
void wtk_dnnvad_flush_end(wtk_dnnvad_post_t *p);
#ifdef __cplusplus
};
#endif
#endif
