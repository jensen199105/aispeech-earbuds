#ifndef WTK_VAD_DNNVAD_WTK_DNNVAD_CFG_H_
#define WTK_VAD_DNNVAD_WTK_DNNVAD_CFG_H_
#include "wtk/core/wtk_type.h"
#include "wtk/vite/parm/wtk_parm.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_dnnvad_cfg wtk_dnnvad_cfg_t;
struct wtk_dnnvad_cfg {
    int win;
    int siltrap;
    int speechtrap;
    int startfrm;
    int sil_thresh;

    unsigned use_custom_vad : 1;
};

int wtk_dnnvad_cfg_init(wtk_dnnvad_cfg_t *cfg);
int wtk_dnnvad_cfg_clean(wtk_dnnvad_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
