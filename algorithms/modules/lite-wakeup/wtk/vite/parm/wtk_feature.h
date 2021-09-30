#ifndef WTK_MATH_WTK_FEATURE_H_
#define WTK_MATH_WTK_FEATURE_H_
#include "wtk/vite/math/wtk_vector.h"
#include "wtk/core/wtk_queue.h"
#include "AISP_TSL_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wtk_feature wtk_feature_t;
typedef struct wtk_frv wtk_frv_t;
struct wtk_parm_cfg;
typedef int (*wtk_feature_sender_t)(void *, wtk_feature_t *);
typedef int (*wtk_frv_sender_t)(void *, wtk_frv_t *);

#define wtk_feature_inc(f) (++((f)->used))
#define wtk_feature_dec(f) (--((f)->used))
#define wtk_feature_size(f) wtk_vector_size((f)->rv)

typedef enum {
    wtk_vframe_sil = 0,
    wtk_vframe_speech,
    wtk_vframe_speech_end,
}wtk_vframe_state_t;

struct wtk_feature {
    wtk_queue_node_t hoard_n;    // used for hoard cache;
    wtk_queue_node_t queue_n;
    aisp_s32_t   *v;            // sigp feature, PLP,MFCC,etc feature from  sig.
    aisp_s32_t   *cmn;
    void *send_hook;            // used for send callback;
    wtk_feature_sender_t send;
    int used;                    // feature used refrence;
    int index;                    // index of feature;
};

struct wtk_frv {
    wtk_queue_node_t hoard_n;   // used for hoard cache;
    wtk_queue_node_t vadpost_n;
    wtk_queue_node_t vadout_n;
    aisp_s32_t  *dnn_v;         // dnn vector
    aisp_s32_t  *rv;            // pointer to current valid feature used for rec.
    void *send_hook;            // used for send callback;
    wtk_frv_sender_t send;
    int index;                  // index of frv;
    unsigned ref;
    unsigned is_sil;
    wtk_vframe_state_t state;
};

wtk_frv_t *wtk_frv_new(int dnn_out_cols);
wtk_feature_t *wtk_feature_new2(struct wtk_parm_cfg *cfg, int xf_size);
wtk_feature_t *wtk_feature_new(int size, int xf_size);
int wtk_feature_delete(wtk_feature_t *f);
int wtk_frv_delete(wtk_frv_t *f);

/**
 * @brief send back feature, for reuse feature;
 */
int wtk_feature_send(wtk_feature_t *f);

void wtk_feature_use_dec(wtk_feature_t *f);

/**
 * @brief decrease use and send back;
 */
void wtk_feature_push_back(wtk_feature_t *f);
void wtk_frv_push_back(wtk_frv_t *frv);

#ifdef __cplusplus
};
#endif
#endif
