#ifndef WTK_CYBER_CODE_WTK_ZMEAN_CFG_H_
#define WTK_CYBER_CODE_WTK_ZMEAN_CFG_H_

#include "wtk/vite/math/wtk_vector.h"
#include "wtk/vite/math/wtk_math.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wtk_zmean_cfg wtk_zmean_cfg_t;
struct wtk_zmean_cfg
{
    int start_min_frame;    //!< min frame to update mean vector.
    int post_update_frame;
    int left_seek_frame;
    int min_flush_frame;
    int win_size;
    unsigned smooth:1;
};

int wtk_zmean_cfg_init(wtk_zmean_cfg_t *cfg);
int wtk_zmean_cfg_clean(wtk_zmean_cfg_t *cfg);

#ifdef __cplusplus
};
#endif
#endif
