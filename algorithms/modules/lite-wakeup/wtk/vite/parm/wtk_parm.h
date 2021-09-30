#ifndef WTK_MATH_WTK_PARM_H_
#define WTK_MATH_WTK_PARM_H_

#include "wtk/vite/math/wtk_vector_buffer.h"
#include "wtk_sigp.h"
#include "wtk_feature.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_robin.h"
#include "wtk/vite/parm/post/cmn/wtk_zmean.h"
#if defined(WAKEUP_NN_DNN)
#include "wtk/vite/parm/post/dnn/wtk_dnn.h"
#elif defined(WAKEUP_NN_FSMN)
#include "wtk/vite/parm/post/fsmn/wtk_fsmn.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wtk_parm wtk_parm_t;

typedef enum {
    WTK_PARM_APPEND,
    WTK_PARM_END,
} wtk_parm_state_t;

#define FRV_NODE_NUM (30) // 7+8-1

/**
* @brief wtk_parm_post_f want to do some post process after get features.
* @param hook, application data
* @param feature, if null means is end,want to flush robins.
*/
typedef void (*wtk_parm_post_f)(void *inst, wtk_feature_t *feature);

/**
 * @brief wtk_parm_static_post_f want to do some process after get static features;
 */
typedef void (*wtk_parm_static_post_f)(void *inst, wtk_feature_t *feature);

typedef void (*wtk_parm_feature_notify_f)(void *ths, wtk_feature_t *f);

typedef void (*wtk_parm_window_notify_f)(void *ths, wtk_feature_t *f);

typedef int (*wtk_parm_feature_hook_f)(void* data, int* f, int count);

struct wtk_parm {
    wtk_queue_t          *feature_hoard;
    wtk_queue_t          *frv_hoard;
    wtk_parm_cfg_t*      cfg;
    wtk_sigp_t           sigp;
    wtk_short_vector_buffer_t* frame_buffer;
    wtk_short_vector_t*        frame_vector;
    wtk_robin_t*         robins[3];
    aisp_s32_t**         v_tmp_array;  // vector temp array used for window;
    wtk_feature_t**      f_tmp_array; // feature tmp array;
    int diffs;
    int feature_pos[3];
    int win[3];
    int xform_rows;
    int xform_cols;
    int n_frame_index;
    wtk_zmean_t *zmean;
    S64 llAbsFrameIndex;                // absoulte frame index, never reset
    //================ call after static feature is processed ==============
    // post process must be rewrite later.
    wtk_parm_static_post_f static_post_f;
    void *static_post_hook;
    //================ call after static and dynamic feature is processed ===
    wtk_parm_post_f post_f;
    void *post_hook;
	//================ forword feature to other module ===
	wtk_parm_feature_hook_f feature_hook_f;
	void * feature_hook;
    //======================= post proc ==========
#if defined(WAKEUP_NN_DNN)
    wtk_dnn_t *dnn;
#elif defined(WAKEUP_NN_FSMN)
	wtk_fsmn_t* fsmn;
#endif

    //-------------------- check output queue--------------
    wtk_queue_t *output_queue; // wtk_feature_t queue;
};

wtk_parm_t *wtk_parm_new(wtk_parm_cfg_t *cfg);
wtk_parm_t *wtk_parm_new2(wtk_parm_cfg_t *cfg);
int wtk_parm_delete(wtk_parm_t *p);
int wtk_parm_init(wtk_parm_t *p, wtk_parm_cfg_t *cfg);
int wtk_parm_clean(wtk_parm_t *p);
int wtk_parm_reset(wtk_parm_t *p);
int wtk_parm_feature_alloc(wtk_parm_t *p);
int wtk_parm_frv_alloc(wtk_parm_t *p,int n_wchars);

/**
 * @brief wtk_feature_t queue
 */
void wtk_parm_set_output_queue(wtk_parm_t *p, wtk_queue_t *q);

/**
 * @brief reuse feature;
 */
int wtk_parm_push_feature(wtk_parm_t *p, wtk_feature_t *f);
/**
 * @brief reuse frv;
 */
int wtk_parm_push_frv(wtk_parm_t *p, wtk_frv_t *frv);
/**
 * @brief pop feature for use;
 */
wtk_feature_t *wtk_parm_pop_feature(wtk_parm_t *p);

/**
 * @brief reuse feature;
 */
int wtk_parm_reuse_feature(wtk_parm_t *p, wtk_feature_t *f);
int wtk_parm_reuse_frv(wtk_parm_t *p, wtk_frv_t *frv);

/**
 * @brief raise feature and there is some post function to process like cvn;
 */
void wtk_parm_output_feature(wtk_parm_t *p, wtk_feature_t *f);

/**
 * @biref raise feature to output queue;
 */
void wtk_parm_output_feature_to_queue(wtk_parm_t *p, wtk_feature_t *f);
/**
 * @biref raise frv to output queue;
 */
void wtk_parm_output_frv_to_queue(wtk_parm_t *p, wtk_frv_t *f);

/**
 * @brief set post call-back.
 *
 * when parm get static and dynamic feature,will call post to do static feature post process,
 */
void wtk_parm_set_post(wtk_parm_t *p, wtk_parm_post_f post, void *hook);

/**
 *	@brief set static post call-back;
 */
void wtk_parm_set_static_post(wtk_parm_t *p, wtk_parm_static_post_f static_post, void *hook);

/**
 *	@brief feed sample data, each sample type is short .
 */
int wtk_parm_feed(wtk_parm_t *p, wtk_parm_state_t state, short *data, int samples);

/**
 *	@brief feed raw data, data can have odd byte, parm will save odd data wait for next byte.
 */
int wtk_parm_feed2(wtk_parm_t *p, wtk_parm_state_t state, char *data, int bytes);

/**
 *	@brief write feature as HTK format;
 *	@param sample_peroid like TARGETRATE in configure;
 *	@param target_kind like "MFCC_D_A";
 */
//------------------ feture process --------------------------
void wtk_parm_feed_input_feature(wtk_parm_t *p, wtk_feature_t *f);
void wtk_parm_feed_end(wtk_parm_t *p);

//---------------- process raw plp --------------------------
/*
 * 	while file:
 * 		feature=wtk_param_pop_feature(p);
 * 		wtk_feature_read(feature,file);
 *		wtk_parm_feed_plp(p,feature);
 * wtk_parm_flush_end(p)
 */

/**
 * 	@brief feature=wtk_parm_pop_feature(p);
 */
void wtk_parm_feed_plp(wtk_parm_t *p, wtk_feature_t *f);
void wtk_parm_flush_end(wtk_parm_t *p);
void wtk_parm_flush_robin_all(wtk_parm_t *p, wtk_robin_t *r);
//----------------------------------------------------------

#ifdef __cplusplus
};
#endif
#endif
