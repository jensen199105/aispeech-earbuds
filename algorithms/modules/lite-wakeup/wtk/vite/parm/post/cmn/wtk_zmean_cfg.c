#include "wtk_zmean_cfg.h"

extern const short cmn_start_min_frame;
extern const short cmn_post_update_frame;
extern const short cmn_left_seek_frame;
extern const short cmn_min_flush_frame;
extern const short cmn_win_size;

int wtk_zmean_cfg_init(wtk_zmean_cfg_t *cfg)
{
    cfg->start_min_frame = cmn_start_min_frame;
    cfg->post_update_frame = cmn_post_update_frame;
    cfg->smooth = 1;
    cfg->left_seek_frame = cmn_left_seek_frame;
    cfg->min_flush_frame = cmn_min_flush_frame;
    cfg->win_size = cmn_win_size;//need +1 ，和研究算法有出入
    return 0;
}

int wtk_zmean_cfg_clean(wtk_zmean_cfg_t *cfg)
{
    return 0;
}
