#ifndef __KISS_FFT_H__
#define __KISS_FFT_H__

#include <stdlib.h>
#ifdef AISP_TSL_INFO
#include <stdio.h>
#endif
#include <math.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 ATTENTION!
 If you would like a :
 -- a utility that will handle the caching of fft objects
 -- real-only (no imaginary time component ) FFT
 -- a multi-dimensional FFT
 -- a command-line utility to perform ffts
 -- a command-line utility to perform fast-convolution filtering

 Then see kfc.h NR_kiss_fftr.h NR_kiss_fftnd.h fftutil.c NR_kiss_fastfir.c
  in the tools/ directory.
*/

#ifdef USE_SIMD
# include <xmmintrin.h>
# define NR_kiss_fft_scalar __m128
#define NR_KISS_FFT_MALLOC(nbytes) _mm_malloc(nbytes,16)
#define NR_KISS_FFT_FREE _mm_free
#else
#define NR_KISS_FFT_MALLOC malloc
#define NR_KISS_FFT_FREE free
#endif

#ifdef AISPEECH_FIXED
#ifndef FIXED_POINT
#define FIXED_POINT  (32)
#endif
#endif
#ifdef FIXED_POINT
//#include <sys/types.h>
#include <stdint.h>			/* added by xuchao */
# if (FIXED_POINT == 32)
#  define NR_kiss_fft_scalar int32_t
# else
#  define NR_kiss_fft_scalar int16_t
# endif
#else
# ifndef NR_kiss_fft_scalar
/*  default is float */
#   define NR_kiss_fft_scalar float
# endif
#endif

typedef struct {
    NR_kiss_fft_scalar r;
    NR_kiss_fft_scalar i;
}NR_kiss_fft_cpx;

#define MAXFACTORS 32
/* e.g. an fft of length 128 has 4 factors 
 as far as kissfft is concerned
 4*4*4*2
 */
typedef struct NR_kiss_fft_state* NR_kiss_fft_cfg;
struct NR_kiss_fft_state
{
    int nfft;
    int inverse;
    int factors[2*MAXFACTORS];
#ifdef STATIC_MEM_FIXED_FFT
    NR_kiss_fft_cpx *twiddles;
#else
    NR_kiss_fft_cpx twiddles[1];
#endif
};

/* 
 *  NR_kiss_fft_alloc
 *  
 *  Initialize a FFT (or IFFT) algorithm's cfg/state buffer.
 *
 *  typical usage:      NR_kiss_fft_cfg mycfg=NR_kiss_fft_alloc(1024,0,NULL,NULL);
 *
 *  The return value from fft_alloc is a cfg buffer used internally
 *  by the fft routine or NULL.
 *
 *  If lenmem is NULL, then NR_kiss_fft_alloc will allocate a cfg buffer using malloc.
 *  The returned value should be free()d when done to avoid memory leaks.
 *  
 *  The state can be placed in a user supplied buffer 'mem':
 *  If lenmem is not NULL and mem is not NULL and *lenmem is large enough,
 *      then the function places the cfg in mem and the size used in *lenmem
 *      and returns mem.
 *  
 *  If lenmem is not NULL and ( mem is NULL or *lenmem is not large enough),
 *      then the function returns NULL and places the minimum cfg 
 *      buffer size in *lenmem.
 * */

NR_kiss_fft_cfg NR_kiss_fft_alloc(int nfft,int inverse_fft,void * mem,size_t * lenmem); 

/*
 * NR_kiss_fft(cfg,in_out_buf)
 *
 * Perform an FFT on a complex input buffer.
 * for a forward FFT,
 * fin should be  f[0] , f[1] , ... ,f[nfft-1]
 * fout will be   F[0] , F[1] , ... ,F[nfft-1]
 * Note that each element is complex and can be accessed like
    f[k].r and f[k].i
 * */
void NR_kiss_fft(NR_kiss_fft_cfg cfg,const NR_kiss_fft_cpx *fin,NR_kiss_fft_cpx *fout);

/*
 A more generic version of the above function. It reads its input from every Nth sample.
 * */
void NR_kiss_fft_stride(NR_kiss_fft_cfg cfg,const NR_kiss_fft_cpx *fin,NR_kiss_fft_cpx *fout,int fin_stride);

/* If NR_kiss_fft_alloc allocated a buffer, it is one contiguous 
   buffer and can be simply free()d when no longer needed*/
#define NR_kiss_fft_free free

/*
 Cleans up some memory that gets managed internally. Not necessary to call, but it might clean up 
 your compiler output to call this before you exit.
*/
void NR_kiss_fft_cleanup(void);
    

/*
 * Returns the smallest integer k, such that k>=n and k has only "fast" factors (2,3,5)
 */
int NR_kiss_fft_next_fast_size(int n);

/* for real ffts, we need an even size */
#define NR_kiss_fftr_next_fast_size_real(n) \
        (NR_kiss_fft_next_fast_size(((n)+1)>>1)<<1)

#ifdef __cplusplus
} 
#endif

#endif
