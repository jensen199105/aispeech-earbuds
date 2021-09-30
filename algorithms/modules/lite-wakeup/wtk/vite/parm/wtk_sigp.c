#ifdef USE_HIFI4_OPTIMIZE
#include "NatureDSP_Signal.h"
#endif

#ifdef USE_CM4_OPTIMIZE
#include "arm_math.h"
#endif
#include "fft_wrapper.h"
#include "AISP_TSL_transQ.h"
#include "AISP_TSL_str.h"
#include "wtk_sigp.h"
#define FFT_POINTS_N    (512)
#define FFT_N_SHIFT     (9)

extern char aisp_fftHeap[];
extern const int fft_table[];
extern const int fft_supertable[];
extern void kf_factor_fixed(int n, int *facbuf);

#ifndef JIELI_BR28
extern void jl_fft_512(int *in , int *out);
//extern void fft_out_write(void *data, u32 len);
/************************************************************
  Function   : wtk_sigp_fftInit()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/11/15, Youhai.Jiang create

************************************************************/
S32 wtk_sigp_fftInit(NR_kiss_fftr_cfg *fftCfg, S32 fftSize)
{
#ifdef STATIC_MEM_FIXED_FFT
    /* alloc a whole memory to avoid using malloc and use fft table instead of float calculation */
    char *pcRdmaFftHeap = (char *)&aisp_fftHeap[0];
    NR_kiss_fftr_cfg pstFftCfg = NULL;
    int iMemSizeAlloc = 0;
    /* fftCfg size */
    *fftCfg = pstFftCfg = (NR_kiss_fftr_cfg)pcRdmaFftHeap;
    iMemSizeAlloc += sizeof(struct NR_kiss_fftr_state);
    pstFftCfg->substate = (NR_kiss_fft_cfg)(pcRdmaFftHeap + iMemSizeAlloc);
    iMemSizeAlloc += sizeof(struct NR_kiss_fft_state);
    pstFftCfg->tmpbuf = (NR_kiss_fft_cpx *)(pcRdmaFftHeap + iMemSizeAlloc);
    iMemSizeAlloc += sizeof(NR_kiss_fft_cpx) * (fftSize >> 1);     /* tmpbuf */
    /* Init the FFTENG struction for fftCfg */
    pstFftCfg->substate->nfft = fftSize >> 1;
    pstFftCfg->substate->inverse = 0;
    kf_factor_fixed(fftSize >> 1, &(pstFftCfg->substate->factors[0]));
    pstFftCfg->substate->twiddles = (NR_kiss_fft_cpx *)fft_table;
    pstFftCfg->super_twiddles     = (NR_kiss_fft_cpx *)fft_supertable;
#else
    *fftCfg = NR_kiss_fftr_alloc(fftSize, 0, 0, 0);

    if (NULL == *fftCfg)
    {
#ifdef WKP_DEBUG
        printf("KISS fft config alloc fail.\n");
#endif
        return -1;
    }

#endif
    return 0;
}

/************************************************************
  Function   : wtk_sigp_fftDelete()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/11/15, Youhai.Jiang create

************************************************************/
VOID wtk_sigp_fftDelete(NR_kiss_fftr_cfg fftCfg)
{
    AISP_PTR_FREE(fftCfg);
}
#endif
void wtk_parm_init_fbankinfo(FBankInfo *fb, wtk_parm_cfg_t *cfg)
{
    int k = cfg->frame_size;
    int iFftN = 2;

    while (k > iFftN)
    {
        iFftN <<= 1;
    }

    if (cfg->double_fft)
    {
        iFftN <<= 1;
    }

    fb->klo = 2;
    fb->khi = iFftN >> 1;
#ifdef USE_HIFI4_OPTIMIZE
    fb->x = (int *)wtk_calloc(iFftN + 2, sizeof(int));
#else
    fb->x = (int *)wtk_calloc(2 + AISP_TSL_MAX(iFftN, FFT_POINTS_N) + 2, sizeof(int));
#endif
    fb->fftN = fb->x[0] = iFftN;
}

void wtk_param_clean_fbankinfo(FBankInfo *fb)
{
    if (fb && fb->x)
    {
        wtk_vector_delete(fb->x);
    }
}

int wtk_sigp_init(wtk_sigp_t *s, wtk_parm_cfg_t *cfg)
{
    AISP_TSL_memset(s, 0, sizeof(*s));
    s->cfg = cfg;
#ifndef JIELI_BR28
    wtk_sigp_fftInit(&s->fftCfg, FFT_POINTS_N);
#endif
    wtk_parm_init_fbankinfo(&(s->fbInfo), cfg);
    /* total FFT_POINTS_N/2 + 1 freq points */
#ifdef USE_HIFI4_OPTIMIZE
    s->fft_out = (aisp_s32_t *)wtk_calloc(FFT_POINTS_N + 2, sizeof(aisp_s32_t));
#else
    s->fft_out = (aisp_s32_t *)(s->fbInfo.x + 2);
#endif
    return 0;
}

int wtk_sigp_clean(wtk_sigp_t *s)
{
    if (s)
    {
        wtk_param_clean_fbankinfo(&(s->fbInfo));

        if (s->fft_out)
        {
            wtk_vector_delete(s->fft_out);
        }

#ifndef JIELI_BR28
        wtk_sigp_fftDelete(s->fftCfg);
#endif
    }

    return 0;
}

int wtk_sigp_procss(wtk_sigp_t *s, wtk_short_vector_t *v, aisp_s32_t *feature)
{
    wtk_parm_cfg_t *cfg        = s->cfg;
    aisp_s32_t  i              = 0;
    int iLoopLen               = 0;
#ifndef LUDA_SUPPORT_FREQOUT_DIRECT
    aisp_s16_t *psSrc          = NULL;
    int *piTimedata            = NULL;
    aisp_s16_t  v_max          = 0;
    aisp_s16_t  shift          = 0;
    aisp_s16_t  v_min          = 0;
    //TODO: changren check
#if 0
    aisp_s16_t  preemph_fixed  = 0; // fixed value, Q5,  31/32 ~= 0.97;
#else
    aisp_s16_t  preemph_fixed  = 31; // fixed value, Q5,  31/32 ~= 0.97;
#endif
#endif
    aisp_s16_t  bin            = 0;
    aisp_s32_t  k              = 0;
#ifndef LUDA_SUPPORT_FREQOUT_DIRECT
    aisp_s32_t  vsize          = 0;
#endif
    aisp_s32_t  tmp_r          = 0;
    aisp_s32_t  tmp_i          = 0;
    aisp_s64_t  fb_tmp         = 0;
    aisp_s64_t  fb_ek          = 0;
    aisp_s64_t  fb_local[WAKEUP_FBANK_CHANS_MAX]   = {0};
    aisp_s64_t *pllFb          = NULL;
    aisp_s16_t  shift_comp_table[8] = {0, 1420, 2839, 4259, 5678, 7098, 8517, 9937};
    aisp_s16_t  shift_comp     = 0;  // fbank shift compensation
#ifndef LUDA_SUPPORT_FREQOUT_DIRECT
    unsigned int index = 0;
#endif
    aisp_s32_t *piFFTVal = NULL;
#ifndef LUDA_SUPPORT_FREQOUT_DIRECT
    /*Normalization*/
    wtk_vector_zero_mean_frame(v);
    vsize = wtk_short_vector_size(v);
    AISP_TSL_max_q15(&v[1], vsize, &v_max, &index);
    AISP_TSL_min_q15(&v[1], vsize, &v_min, &index);

    if (v_max < -v_min)
    {
        v_max = -v_min;
    }

    while (v_max < 8000 && v_max > 0)
    {
        v_max <<= 1;
        shift++;
    }

    if (shift > 7)
    {
        shift = 7;
    }

    /* preemph */
    v[vsize] <<= shift;

    for (i = vsize - 1; i > 0; i--)
    {
        v[i] <<= shift;
        v[i + 1] = v[i + 1] - PSHR((int)v[i] * preemph_fixed , 5);
    }

    v[1] = PSHR(v[1] , 5);

    /*add win*/
    if (NULL != cfg->hwin)
    {
        for (i = 0; i < vsize; i++)
        {
            v[i + 1] = PSHR((int)v[i + 1] * cfg->hwin[i], 15);
        }
    }

    /* left shift original int16 data and store it in int32 */
    piTimedata = s->fbInfo.x + 2;
    psSrc      = &v[1];

    //AISP_TSL_shr_vec_32_16(psSrc, piTimedata, vsize, -FFT_N_SHIFT);
    for (i = 0; i < vsize; ++i)
    {
        piTimedata[i] = psSrc[i];
    }

    for (i = vsize; i < FFT_POINTS_N; i++)
    {
        *((int *)(s->fbInfo.x + 2) + i) = 0;
    }

#ifdef USE_HIFI4_OPTIMIZE
    int ret = 0;
    ret = fft_real32x16(s->fft_out, (s->fbInfo.x + 2), FFT_N_SHIFT, h_fft_real_x16_512);

    for (i = 0; i < FFT_POINTS_N + 2; i++)
    {
        s->fft_out[i] <<= ret;
    }

#else
    piTimedata = s->fbInfo.x + 2;
    //NR_kiss_fftr(s->fftCfg, piTimedata, (NR_kiss_fft_cpx *)s->fft_out);
#ifdef JIELI_BR28
    AISP_TSL_FFT(NULL, piTimedata, s->fft_out);
#else
    jl_fft_512(piTimedata, s->fft_out);
#endif
    //fft_out_write(s->fft_out, (FFT_POINTS_N + 2)*sizeof(int));
#endif
#endif
#ifdef LUDA_SUPPORT_FREQOUT_DIRECT
    AISP_TSL_memcpy(s->fft_out, v + 1, (FFT_POINTS_N + 2)*sizeof(int));
    iLoopLen = 256;
#else
    iLoopLen = s->fbInfo.khi;
#endif
    piFFTVal = &s->fft_out[(s->fbInfo.klo - 1) << 1];

    for (i = s->fbInfo.klo - 1; i < iLoopLen; i++)
    {
        bin = cfg->loChan[i];
        tmp_r  = *piFFTVal++;
        tmp_i  = *piFFTVal++;
        fb_ek  = (S64)tmp_r * tmp_r + (S64)tmp_i * tmp_i;
        fb_tmp = (S64)PSHR_POSITIVE(fb_ek * cfg->loWt[i], 15);

        if (bin > 0)
        {
            fb_local[bin - 1] += fb_tmp;
        }

        if (bin < cfg->fbank_num_chans)
        {
            fb_local[bin] += fb_ek - fb_tmp;
        }
    }

#ifdef LUDA_SUPPORT_FREQOUT_DIRECT
    shift_comp = shift_comp_table[0];
#else
    shift_comp = shift_comp_table[shift];
#endif
    pllFb = fb_local;

    for (k = 0; k < cfg->fbank_num_chans; k++)
    {
        if (*pllFb >= AISP_TSL_Q31_ONE_LONG_TYPE /*1 << 31*/)
        {
            /* 12776 = round(log(2^18)*2^10) */
            *feature++ = wtk_fxlog((int)PSHR_POSITIVE(*pllFb, 18)) + 12776 - shift_comp;
        }
        else
        {
            *feature++ = wtk_fxlog((int) * pllFb) - shift_comp;
        }

        pllFb++;
    }

    return 0;
}

