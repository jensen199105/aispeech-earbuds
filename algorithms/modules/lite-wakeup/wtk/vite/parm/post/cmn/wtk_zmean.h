#ifndef WTK_CYBER_CODE_WTK_ZMEAN_H_
#define WTK_CYBER_CODE_WTK_ZMEAN_H_
#include "wtk/vite/math/wtk_vector.h"
#include "wtk/vite/parm/wtk_feature.h"
#include "wtk/core/wtk_robin.h"
#include "wtk_zmean_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * cepstral mean normalization
 */
typedef struct wtk_zmean wtk_zmean_t;

struct wtk_parm;

struct wtk_zmean
{
    wtk_zmean_cfg_t *cfg;
    struct wtk_parm *parm;
    wtk_queue_t post_feature_q;     //used for post update;
    wtk_int_vector_t *cur;          //!< current mean vector.
    wtk_int_vector_t *buf;          //!< accumulator buffer for cmn update.
    wtk_robin_t *past_feat_robin;   // used for sliding cmn
    wtk_robin_t *cmn_robin;
    long long *buf_tmp;
    int vec_size;
    int frames;
};

wtk_zmean_t* wtk_zmean_new(wtk_zmean_cfg_t *cfg,struct wtk_parm *parm);
int wtk_zmean_delete(wtk_zmean_t *z);
int wtk_zmean_reset(wtk_zmean_t *z);

/**
 * @brief parm static post callback;
 */
void wtk_zmean_static_post_feed(wtk_zmean_t *p,wtk_feature_t *f);

/**
 * @brief parm post callback;
 */
void wtk_zmean_post_feed(wtk_zmean_t *p,wtk_feature_t *f);

/**
 * @brief flush post queue;
 */
void wtk_zmean_flush_parm_post_queue(wtk_zmean_t *z);



//=======================================   private =============================
/**
 * @brief used for cache cmn buf;
 */
void wtk_zmean_update_buf(wtk_zmean_t *z,int *v);

/**
 * @brief used for vparm cmn vector update;
 */
void wtk_zmean_update_cmn(wtk_zmean_t *z);

/**
 * @brief used for vparm feature process;
 */
void wtk_zmean_process_cmn(wtk_zmean_t *z,int *feat);

/**
 * @brief flush mean;
 */
void wtk_zmean_flush(wtk_zmean_t *z,int force);

int wtk_zmean_can_flush_all(wtk_zmean_t *z);

void wtk_zmean_flush_robin(wtk_zmean_t *z);
#ifdef __cplusplus
};
#endif
#endif
