#ifndef KISS_FTR_H
#define KISS_FTR_H

#include "_kiss_fft_guts.h"
#ifdef __cplusplus
extern "C" {
#endif

    
/* 
 
 Real optimized version can save about 45% cpu time vs. complex fft of a real seq.

 
 
 */

typedef struct NR_kiss_fftr_state *NR_kiss_fftr_cfg;
struct NR_kiss_fftr_state{
    NR_kiss_fft_cfg substate;
    NR_kiss_fft_cpx * tmpbuf;
    NR_kiss_fft_cpx * super_twiddles;
#ifdef USE_SIMD
    void * pad;
#endif
};

NR_kiss_fftr_cfg NR_kiss_fftr_alloc(int nfft,int inverse_fft,void * mem, size_t * lenmem);
/*
 nfft must be even

 If you don't care to allocate space, use mem = lenmem = NULL 
*/


void NR_kiss_fftr(NR_kiss_fftr_cfg cfg,const NR_kiss_fft_scalar *timedata,NR_kiss_fft_cpx *freqdata);
/*
 input timedata has nfft scalar points
 output freqdata has nfft/2+1 complex points
*/

void NR_kiss_fftri(NR_kiss_fftr_cfg cfg,const NR_kiss_fft_cpx *freqdata,NR_kiss_fft_scalar *timedata);
/*
 input freqdata has  nfft/2+1 complex points
 output timedata has nfft scalar points
*/

#define NR_kiss_fftr_free free

#ifdef __cplusplus
}
#endif
#endif
