#include <stdio.h>
#include "AISP_TSL_base.h"
#include "AISP_TSL_shift.h"
#include "AISP_TSL_common.h"
#include "AISP_TSL_math.h"
#include "wtk/vite/math/cblas.h"

#ifdef USE_HIFI4_OPTIMIZE
#include <xtensa/tie/xt_misc.h>
#ifdef USE_KUNLUN_TIE
#include <xtensa/tie/taihang1b.h>
#endif
#ifndef USE_CSTUB_HIFI4
#include <xtensa/tie/xt_hifi4.h>
#else
#include "xtensa/tie/xt_hifi4.h"
#endif
//#include "cstub_hifi4.h"
#elif defined(USE_CM4_OPTIMIZE)
#include "arm_math.h"
#endif

/*
 * Description:
 *   calculate exp(-x) while x >= 0
 * Input:
 *   x      --> input data intepreted as Q16
 * Output:
 *          --> output data interpreted as Q31
 *
 * History:
 *   2017/01/08 chao.xu create
 */
#ifdef JIELI_BR28
extern void vector_real_zs16_ys8_mul_xs8(volatile long *zptr, long *yptr, long *xptr, short len, char q);
extern void vector_real_zs32_ys8_dotpdt_xs8(volatile long *zptr, long *yptr, long *xptr, short len, char q);
extern void vector_real_zs32_ys32_add_xs16(volatile long *zptr, long *yptr, long *xptr, short len);
extern void vector_real_zs32_ys32_mul_xs16(volatile long *zptr, long *yptr, long *xptr, short len, char q);
#else
extern int jl_vector_multadd(int8_t *x1, int8_t *x2, int size);
#endif
int64_t fxexp_q16(int x)
{
    int64_t y = 0x80000000; /* y = 1 (Q31) */;

    if (x >= 0xb1721)
    {
        x -= 0xb1721, y <<= 16;
    }

    if (x >= 0x58b91)
    {
        x -= 0x58b91, y <<= 8;
    }

    if (x >= 0x2c5c8)
    {
        x -= 0x2c5c8, y <<= 4;
    }

    if (x >= 0x162e4)
    {
        x -= 0x162e4, y <<= 2;
    }

    if (x >= 0x0b172)
    {
        x -= 0x0b172, y <<= 1;
    }

    if (x >= 0x067cd)
    {
        x -= 0x067cd, y += y >> 1;
    }

    if (x >= 0x03920)
    {
        x -= 0x03920, y += y >> 2;
    }

    if (x >= 0x01e27)
    {
        x -= 0x01e27, y += y >> 3;
    }

    if (x >= 0x00f85)
    {
        x -= 0x00f85, y += y >> 4;
    }

    if (x >= 0x007e1)
    {
        x -= 0x007e1, y += y >> 5;
    }

    if (x >= 0x003f8)
    {
        x -= 0x003f8, y += y >> 6;
    }

    if (x >= 0x001fe)
    {
        x -= 0x001fe, y += y >> 7;
    }

    if (x & 0x100)
    {
        y += y >> 8;
    }

    if (x & 0x080)
    {
        y += y >> 9;
    }

    if (x & 0x040)
    {
        y += y >> 10;
    }

    if (x & 0x020)
    {
        y += y >> 11;
    }

    if (x & 0x010)
    {
        y += y >> 12;
    }

    if (x & 0x008)
    {
        y += y >> 13;
    }

    if (x & 0x004)
    {
        y += y >> 14;
    }

    if (x & 0x002)
    {
        y += y >> 15;
    }

    if (x & 0x001)
    {
        y += y >> 16;
    }

    y = (((int64_t)1 << 62) + (y >> 1)) / y;
    return y;
}

/*
 * Input:
 *          data    --> input data interpreted as Q24
 *          nsize   --> size of the input data
 * Output:
 *          data    --> output data interpreted as Q31
 * Others:
 *          In this function, use fxexp_q16 as core, not q24 exp. and error/priceison is listed as below:
 *              int32(Q16) --> 10e-6
 *              int32(Q12) --> 5*10e-6
 *
 * History:
 *   2018/01/08 chao.xu edit
 */
static void _ann_softmax_q24(int *data, int nsize)
{
    int  max, in;
    int64_t sum;
    int *p, *e;
#ifdef USE_HIFI4_OPTIMIZE
    int nHifiSize;
    int i;
    nHifiSize = nsize & (~3);
    max = vec_max32x32_fast(data, nHifiSize);

    for (i = nHifiSize; i < nsize; i++)
    {
        if (max < data[i])
        {
            max = data[i];
        }
    }

#else
    unsigned int index;
    AISP_TSL_max_q31(data, nsize, &max, &index);
#endif
    sum = 0;
    p = data;
    e = p + nsize;

    while (p < e)
    {
        in = max - *p;
#ifdef USE_EXP_OPTIMIZE

        if (in >= 335544320)    /*  20, Q24 data type */
        {
            *p = 0;
        }
        else
            if (in == 0)
            {
                *p = 0x7fffffff;
            }
            else
            {
                *p = AISP_TSL_exp_minus_xW32Q24_yW32(in, 31);
            }

#else
#ifdef OUTPUT_Q24
        in = (int)PSHR_POSITIVE(in, 8); // Q24--> Q16  try
#endif

        if (in > 1408209) /* lg(1/2^31)*2^16 */
        {
            *p = 0;
        }
        else
            if (in == 0)
            {
                *p = 0x7fffffff;
            }
            else
            {
                *p = (int)fxexp_q16(in);
            }

#endif
        sum += (int64_t)(*p);
        ++p;
    } /* check ok */

    sum = (((int64_t)1 << 62) + (sum >> 1)) / sum;  // Q31 long check ok
    p = data;
    e = p + nsize;

    while (p < e)
    {
        *p = (int)(((int64_t)(*p) * sum + (int64_t)1073741824) >> 31); // Q31 + Q31  >> Q31
        ++p;
    } /* check ok */
}

#ifndef USE_EXP_OPTIMIZE
/*
 * Description:
 *   calculate exp(x) while x >= 0
 * Input:
 *   x      --> input data intepreted as Q24
 * Output:
 *   y      --> output data interpreted as Q24
 *
 * History:
 *   2017/01/08 chao.xu edit
 */
static int64_t _exp_i_q24(int x)
{
    int64_t y;
    y = (int64_t)0x1000000;     /* Q24 */

    if (x >= 0xb17217f)
    {
        x -= 0xb17217f, y <<= 16;
    }

    if (x >= 0x58b90c0)
    {
        x -= 0x58b90c0, y <<= 8;
    }

    if (x >= 0x2c5c860)
    {
        x -= 0x2c5c860, y <<= 4;
    }

    if (x >= 0x162e430)
    {
        x -= 0x162e430, y <<= 2;
    }

    if (x >= 0x0b17218)
    {
        x -= 0x0b17218, y <<= 1;
    }

    if (x >= 0x067cc90)
    {
        x -= 0x067cc90, y += y >> 1;
    }

    if (x >= 0x0391ff0)
    {
        x -= 0x0391ff0, y += y >> 2;
    }

    if (x >= 0x01e2707)
    {
        x -= 0x01e2707, y += y >> 3;
    }

    if (x >= 0x00f8518)
    {
        x -= 0x00f8518, y += y >> 4;
    }

    if (x >= 0x007e0a7)
    {
        x -= 0x007e0a7, y += y >> 5;
    }

    if (x >= 0x003f815)
    {
        x -= 0x003f815, y += y >> 6;
    }

    if (x >= 0x001fe03)
    {
        x -= 0x001fe03, y += y >> 7;
    }

    if (x >= 0x000ff80)
    {
        x -= 0x000ff80, y += y >> 8;
    }

    if (x >= 0x0007fe0)
    {
        x -= 0x0007fe0, y += y >> 9;
    }

    if (x >= 0x0003ff8)
    {
        x -= 0x0003ff8, y += y >> 10;
    }

    if (x >= 0x0001ffe)
    {
        x -= 0x0001ffe, y += y >> 11;
    }

    if (x & 0x0001000)
    {
        y += y >> 12;
    }

    if (x & 0x0000800)
    {
        y += y >> 13;
    }

    if (x & 0x0000400)
    {
        y += y >> 14;
    }

    if (x & 0x0000200)
    {
        y += y >> 15;
    }

    if (x & 0x0000100)
    {
        y += y >> 16;
    }

    if (x & 0x0000080)
    {
        y += y >> 17;
    }

    if (x & 0x0000040)
    {
        y += y >> 18;
    }

    if (x & 0x0000020)
    {
        y += y >> 19;
    }

    if (x & 0x0000010)
    {
        y += y >> 20;
    }

    if (x & 0x0000008)
    {
        y += y >> 21;
    }

    if (x & 0x0000004)
    {
        y += y >> 22;
    }

    if (x & 0x0000002)
    {
        y += y >> 23;
    }

    if (x & 0x0000001)
    {
        y += y >> 24;
    }

    return y;
}

/*
 * Input:
 *   x      --> input data intepreted as Q24
 * Output:
 *   y      --> output data interpreted as Q24
 *
 * History:
 *   2017/01/08 chao.xu edit
 */
static int _sigmoid_i_q24(int x)
{
    int64_t tmp = _exp_i_q24(AISP_TSL_abs(x));
    int64_t q24 = ((int64_t)1 << 24);
    int64_t q48 = q24 * q24;
    tmp = (q48 + ((q24 + tmp) >> 1)) / (q24 + tmp);

    if (x > 0)
    {
        tmp = q24 - tmp;
    }

    return (int32_t)tmp;
}
#endif

void cblas_trans_process(aisp_s32_t *c, int row, int col, const aisp_s16_t *w, const aisp_s16_t *b)
{
    int i, j;
    aisp_s32_t *pc;
    const aisp_s16_t *pw;
    const aisp_s16_t *pb;
    pc = c;
#ifdef USE_HIFI4_OPTIMIZE
    int size = AISP_TSL_FLOOR(col, 4);

    for (i = 0; i < row; i++)
    {
        pw = w;
        pb = b;

        for (j = 0; j < size; j += 4)
        {
            *pc = (*pc + *pb) * (*pw);
            pc++;
            pb++;
            pw++;
            *pc = (*pc + *pb) * (*pw);
            pc++;
            pb++;
            pw++;
            *pc = (*pc + *pb) * (*pw);
            pc++;
            pb++;
            pw++;
            *pc = (*pc + *pb) * (*pw);
            pc++;
            pb++;
            pw++;
        }

        for (; j < col; j++)
        {
            *pc = (*pc + *pb) * (*pw);
            pc++;
            pb++;
            pw++;
        }
    }

#elif (defined JIELI_BR28)

    for (i = 0; i < row; i++)
    {
        pw = w;
        pb = b;
        vector_real_zs32_ys32_add_xs16(pc, pc, pb, col);
        vector_real_zs32_ys32_mul_xs16(pc, pc, pw, col, 0);
        pc += col;
    }

#else

    for (i = 0; i < row; i++)
    {
        pw = w;
        pb = b;

        for (j = 0; j < col; j++)
        {
            *pc = (*pc + *pb) * (*pw);
            pc++;
            pb++;
            pw++;
        }
    }

#endif
}

#if defined(WAKEUP_NN_DNN)
/**
 * @brief
 *
 * @param M
 * @param N
 * @param K
 * @param A   Q6
 * @param lda
 * @param B   Q6
 * @param ldb
 * @param C   float
 * @param ldc
 * @param bias
 * @param A_row_maxs  each row max
 * @param B_row_maxs  each row max
 *
 * @return
 */
void cblas_sgemm_i8(const int M, const int N, const int K, const int8_t *A,
                    const int lda, const int8_t *B, const int ldb, int *C, const int ldc,
                    const int *bias, int *A_row_maxs, int *B_row_maxs)
{
    int i, j, k;
    int n1 = M;
    int n2 = N;
    int ldf = lda;
    int ldg = ldb;
    const int8_t *F = A;
    const int8_t *G = B;
    int tmp_n = AISP_TSL_FLOOR(K, 32);
    int64_t scale = 0;
    int *pc;
    int8_t *p1, * p2;
    const int *pbias;
    int64_t temp_q24;
    int32_t temp;
#ifdef USE_CM4_OPTIMIZE
    int32_t temp_2;
#endif

    for (i = 0; i < n1; i++)
    {
        for (j = 0; j < n2; j++)
        {
            C[ldc * i + j] = 0;
        }
    }

#ifdef USE_HIFI4_OPTIMIZE
    ae_int16x4 h_p1;
    ae_int16x4 h_p2;
    ae_int64   h_tmp;
    int64_t    h_tmpx2;
#endif

    /* form  C := A * B' + C */
    for (i = 0; i < n1; i++)
    {
        pc = (int *)(&(C[ldc * i]));
        pbias = bias;

        for (j = 0; j < n2; j++)
        {
            p1 = (int8_t *)(&(F[ldf * i]));
            p2 = (int8_t *)(&(G[ldg * j]));
            // scale = B_row_maxs[j] / (64 * A_row_maxs[i]);
            scale = AISP_TSL_PSHR64(MULT32_32(B_row_maxs[j], A_row_maxs[i]), 12);      // Q24 * Q24 ==> Q36 try
            temp = 0;
            // unroll loop 32 is best in R16 chip
#ifdef USE_CM4_OPTIMIZE
            arm_dot_prod_q7(p1, p2, 32, &temp);
#else
#ifdef USE_HIFI4_OPTIMIZE
            h_tmp = AE_CVT64A32_CSTUB(0);

            for (k = 0; k < tmp_n; k += 4)
            {
                AE_L8X4F_IP(h_p1, p1, +4);
                AE_L8X4F_IP(h_p2, p2, +4);
                AE_MULAAAAQ16(h_tmp, h_p1, h_p2);
            }

#elif (defined JIELI_BR28)
            vector_real_zs32_ys8_dotpdt_xs8(&temp, p1, p2, tmp_n, 0);
            p1 += tmp_n;
            p2 += tmp_n;
#else

            for (k = 0; k < tmp_n; k += 32)
            {
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
                temp += (*(p1++)) * (*(p2++));
            }

#endif
#ifdef USE_HIFI4_OPTIMIZE
            //                AE_S64_I(h_tmp, (ae_int64*)&h_tmpx2,0);
            //                temp = h_tmpx2;
            ae_q56s tempQ56s = ae_int64_rtor_ae_q56s(h_tmp);
            temp = AE_TRUNCA32Q48(tempQ56s);
#endif

            for (; k < K; ++k)
            {
                temp += (*(p1++)) * (*(p2++));
            }

#endif
            // *(pc++) += scale * temp + *(pbias++);
            temp_q24 = AISP_TSL_PSHR64((int64_t)(scale) * (int64_t)(temp), 24) + (int64_t)(*pbias++); // Q28
            *(pc++) += (int)temp_q24;  // Q24 out try
        }
    }
}

#elif defined(WAKEUP_NN_FSMN)
void cblas_sgemm_i8(const int M, const int N, const int K, const int8_t *A,
                    const int lda, const int8_t *B, const int ldb, int8_t *C, const int ldc,
                    const int16_t *bias, const int qfmta, const int qfmtb, const int qfmtc)
{
    int i, j;
    int n1 = M;
    int n2 = N;
    int ldf = lda;
    int ldg = ldb;
    const int8_t *F = A;
    const int8_t *G = B;
    const int8_t *p1 = NULL;
    const int8_t *p2 = NULL;
    int8_t *pc = C;
    const int8_t *pTmpG = NULL;
    const int16_t *pb = NULL;
    int sub_qfmt = qfmta + qfmtb - qfmtc;
#if defined(USE_KUNLUN_TIE)
    int tmp_n = flooring(K, 32);
#else
#ifndef USE_CM4_OPTIMIZE
    int tmp_n = AISP_TSL_FLOOR(K, 4);
#endif
#endif
    int temp  = 0;
#ifndef USE_CM4_OPTIMIZE
    int temp1 = 0;
    int temp2 = 0;
    int temp3 = 0;
    int temp4 = 0;
    int k;
#endif
#ifdef USE_HIFI4_OPTIMIZE
#ifdef USE_KUNLUN_TIE
    ae_int64 h64_p1;
    ae_int64 h64_p2;
    ae_int32 h_temp0;
    ae_int32 h_temp1;
    ae_int32 h_temp2;
    ae_int32 h_temp3;
#else
    ae_int16x4 h_p1;
    ae_int16x4 h_p2;
    ae_int64   h_tmp;
    int64_t    h_tmpx2;
#endif
#endif

    for (i = 0; i < n1; i++)
    {
        for (j = 0; j < n2; j++)
        {
            pc[j] = 0;
        }

        pc += ldc;
    }

    pc = (int8_t *)C;

    /* form  C := A * B' + C */
    for (i = 0; i < n1; i++)
    {
        pb = bias;
        pTmpG = G;

        for (j = 0; j < n2; j++)
        {
            p1 = F;
            p2 = pTmpG;
#ifdef USE_CM4_OPTIMIZE
            arm_dot_prod_q7((q7_t *)p1, (q7_t *)p2, (uint32_t)K, (q31_t *)&temp);
#else
#ifdef USE_HIFI4_OPTIMIZE
#ifdef USE_KUNLUN_TIE
#ifdef USE_CSTUB_HIFI4
            h_temp0 = int32_rtor_ae_int32(0);
            h_temp1 = int32_rtor_ae_int32(0);
            h_temp2 = int32_rtor_ae_int32(0);
            h_temp3 = int32_rtor_ae_int32(0);
            ae_int64 *ae_p1 = (ae_int64 *)p1;
            ae_int64 *ae_p2 = (ae_int64 *)p2;
#else
            h_temp0 = 0;
            h_temp1 = 0;
            h_temp2 = 0;
            h_temp3 = 0;
#endif

            for (k = 0; k < tmp_n; k += 32)
            {
#ifdef USE_CSTUB_HIFI4
                AE_L64_IP(h64_p1, ae_p1, +8);
                AE_L64_IP(h64_p2, ae_p2, +8);
                KL_R8xR8(h_temp0, h64_p1, h64_p2);
                AE_L64_IP(h64_p1, ae_p1, +8);
                AE_L64_IP(h64_p2, ae_p2, +8);
                KL_R8xR8(h_temp1, h64_p1, h64_p2);
                AE_L64_IP(h64_p1, ae_p1, +8);
                AE_L64_IP(h64_p2, ae_p2, +8);
                KL_R8xR8(h_temp2, h64_p1, h64_p2);
                AE_L64_IP(h64_p1, ae_p1, +8);
                AE_L64_IP(h64_p2, ae_p2, +8);
                KL_R8xR8(h_temp3, h64_p1, h64_p2);
                p1 += 32;
                p2 += 32;
#else
                AE_L64_IP(h64_p1, (ae_int64 *)p1, +8);
                AE_L64_IP(h64_p2, (ae_int64 *)p2, +8);
                KL_R8xR8(h_temp0, h64_p1, h64_p2);
                AE_L64_IP(h64_p1, (ae_int64 *)p1, +8);
                AE_L64_IP(h64_p2, (ae_int64 *)p2, +8);
                KL_R8xR8(h_temp1, h64_p1, h64_p2);
                AE_L64_IP(h64_p1, (ae_int64 *)p1, +8);
                AE_L64_IP(h64_p2, (ae_int64 *)p2, +8);
                KL_R8xR8(h_temp2, h64_p1, h64_p2);
                AE_L64_IP(h64_p1, (ae_int64 *)p1, +8);
                AE_L64_IP(h64_p2, (ae_int64 *)p2, +8);
                KL_R8xR8(h_temp3, h64_p1, h64_p2);
#endif
            }

            temp = ae_int32_rtor_int32(h_temp0) + ae_int32_rtor_int32(h_temp1)
                   + ae_int32_rtor_int32(h_temp2) + ae_int32_rtor_int32(h_temp3);
#else
            h_tmp = AE_ZERO64();

            for (k = 0; k < tmp_n; k += 4)
            {
                AE_L8X4F_IP(h_p1, p1, +4);
                AE_L8X4F_IP(h_p2, p2, +4);
                AE_MULAAAAQ16(h_tmp, h_p1, h_p2);
            }

            ae_q56s h_tmp1 = ae_int64_rtor_ae_q56s(h_tmp);
            temp = AE_TRUNCA32Q48(h_tmp1);
#endif
#else
            /*for (k = 0; k < tmp_n; k += 4)
            {
                temp1 += *(p1++) * *(p2++);
                temp2 += *(p1++) * *(p2++);
                temp3 += *(p1++) * *(p2++);
                temp4 += *(p1++) * *(p2++);
            }*/
#ifdef JIELI_BR28
            vector_real_zs32_ys8_dotpdt_xs8(&temp, (int *)p1, (int *)p2, tmp_n, 0);
#else
            temp = jl_vector_multadd(p1, p2, tmp_n / 4);
#endif
            p1 = p1 + tmp_n;
            p2 = p2 + tmp_n;
#endif

            for (k = tmp_n; k < K; ++k)
            {
                temp += *(p1++) * *(p2++);
            }

#ifndef USE_HIFI4_OPTIMIZE
            //temp += temp1 + temp2 + temp3 + temp4;
#endif
#endif

            if (pb != NULL)
            {
                temp += *pb++;
            }

#ifdef USE_HIFI4_OPTIMIZE
            temp = PSHR32(temp, sub_qfmt);
#else
            temp = PSHR(temp, sub_qfmt);
#endif

            if (temp > 0x7F)
            {
                temp = 0x7F;
            }
            else
                if (temp < -0x80)
                {
                    temp = -0x80;
                }

            *pc++ = (int8_t)temp;
#ifdef USE_CM4_OPTIMIZE
            temp = 0;
#else
            //temp = temp1 = temp2 = temp3 = temp4 = 0;
#endif
            pTmpG += ldg;
        }

        pc += ldc;
        F  += ldf;
    }
}

void cblas_sgemm_i32i16i32(const int M, const int N, const int K, const int32_t *A,
                           const int lda, const int16_t *B, const int ldb, int32_t *C, const int ldc, const int16_t *bias,
                           const int qfmta, const int qfmtb, const int qfmtc)
{
    int i, j, k;
    int n1 = M;
    int n2 = N;
    int ldf = lda;
    int ldg = ldb;
    const int32_t *F = A;
    const int16_t *G = B;
    const int16_t *pb;
    int32_t *pc;
    int sub_qfmt = qfmta + qfmtb - qfmtc;
    int sub_qfmt2 = -sub_qfmt;
    int tmp_n = AISP_TSL_FLOOR(K, 4);
#ifdef USE_HIFI4_OPTIMIZE
    ae_int16x4 h_p1;
    ae_int16x4 h_p2;
    ae_int64   h_tmp;
    int64_t    h_tmpx2;
#endif

    /* form  C := A * B' + C */
    for (i = 0; i < n1; i++)
    {
        pc = &C[ldc * i];
        pb = bias;

        for (j = 0; j < n2; j++)
        {
            int64_t temp = 0;
            const int32_t *p1 = &F[ldf * i];
            const int16_t *p2 = &G[ldg * j];
#ifdef USE_HIFI4_OPTIMIZE
            h_tmp = AE_ZERO64();
            ae_int16x4 *ae_p2 = (ae_int16x4 *)p2;

            for (k = 0; k < tmp_n; k += 4)
            {
                AE_L8X4F_IP(h_p1, p1, +4);
                AE_L16X4_IP(h_p2, ae_p2, 0);
                ae_p2++;
                p2 += 4;
                AE_MULAAAAQ16(h_tmp, h_p1, h_p2);
            }

#else

            for (k = 0; k < tmp_n; k += 4)
            {
                temp += *(p1++) * *(p2++);
                temp += *(p1++) * *(p2++);
                temp += *(p1++) * *(p2++);
                temp += *(p1++) * *(p2++);
            }

#endif
#ifdef USE_HIFI4_OPTIMIZE
            h_tmp =  AE_SLAI64(h_tmp, 24);
            temp = AE_TRUNCA32Q64(h_tmp);
#endif

            for (; k < K; k++)
            {
                temp += *(p1++) * *(p2++);
            }

            if (pb != NULL)
            {
                temp += *pb++ << qfmta;
            }

            if (sub_qfmt != 0)
            {
                temp = sub_qfmt < 0 ? (temp << sub_qfmt2) : PSHR(temp, sub_qfmt);
            }

            *pc++ = temp;
        }
    }
}
#endif

/*
 * Input:
 *   data   --> input data interpreted as Q24
 *   row    --> num of row
 *   col    --> num of colume
 * Output:
 *   data   --> output data interpreted as Q31, used for dnn score function
 *
 * History:
 *   2018/01/08 chao.xu edit
 */
void cblas_softmax_q24(int *data, int row, int col)
{
    int i;
    int *pdata = data;

    for (i = 0; i < row; i++)
    {
        _ann_softmax_q24(pdata, col);
        pdata += col;
    }
}

/*
 * Input:
 *   data  --> input data intepreted as Q24
 *   size  --> size of input vector
 * Output:
 *   data  --> output data interpreted as Q24
 *
 * History:
 *   2017/01/08 chao.xu edit
 */
void cblas_sigmoid_fix_q24(int *data, int size)
{
    int i;
    int *p = data;
    int max = (1 << 24) - 1;

    for (i = 0; i < size; ++i)
    {
        if (*p > 335544320)
        {
            *p = max;
        }
        else
            if (*p < -335544320)
            {
                *p = 1;
            }
            else
            {
#ifdef JIELI_BR28
                extern int sigmoid_q24(int iData);
                *p = sigmoid_q24(*p);
#elif defined USE_EXP_OPTIMIZE
                *p = AISP_TSL_sigmoid_xW32Q24_yW32Q24(*p);
#else
                *p = _sigmoid_i_q24(*p);
#endif
            }

        p++;
    }
}

void cblas_relu2_fixed8(int8_t *data, size_t vsize, int qfmt)
{
#ifdef USE_HIFI4_OPTIMIZE
    int i = 0;
    int max = (1 << (qfmt + 1));
    int min = 0;
    int tmp1, tmp2, tmp3, tmp4;
    ae_int32x2 ae_tmpValue;
    ae_int32x2 ae_tmpValue1;
    ae_int32x2 ae_maxValue;
    ae_int32x2 ae_minValue;
    ae_maxValue = AE_MOVDA32X2(max, max);
    ae_minValue = AE_MOVDA32X2(min, min);
    int nsize = vsize & (~3);

    for (i = 0; i < nsize; i += 4)
    {
        ae_tmpValue = AE_MOVDA32X2((int)data[i], (int)data[i + 1]);
        ae_tmpValue1 = AE_MOVDA32X2((int)data[i + 2], (int)data[i + 3]);
        ae_tmpValue =  AE_MIN32(ae_maxValue, ae_tmpValue);
        ae_tmpValue1 =  AE_MIN32(ae_maxValue, ae_tmpValue1);
        ae_tmpValue =  AE_MAX32(ae_minValue, ae_tmpValue);
        ae_tmpValue1 =  AE_MAX32(ae_minValue, ae_tmpValue1);
        tmp1 = AE_MOVAD32_H(ae_tmpValue);
        data[i] = tmp1;
        tmp2 = AE_MOVAD32_L(ae_tmpValue);
        data[i + 1] = tmp2;
        tmp3 = AE_MOVAD32_H(ae_tmpValue1);
        data[i + 2] = tmp3;
        tmp4 = AE_MOVAD32_L(ae_tmpValue1);
        data[i + 3] = tmp4;
    }

    for (; i < vsize; i++)
    {
        if (data[i] < 0)
        {
            data[i] = 0;
        }
        else
            if (data[i] > max)
            {
                data[i] = max;
            }
    }

#else
    int i = 0;
    int iUpFloor = (1 << (qfmt + 1));

    for (i = 0; i < vsize; i++)
    {
        if (data[i] < 0)
        {
            data[i] = 0;
        }
        else
            if (data[i] > iUpFloor)
            {
                data[i] = iUpFloor;
            }
    }

#endif
}

#if defined(WAKEUP_NN_DNN)
void cblas_fix32tofix8(int8_t *out, aisp_s32_t *in, int row, int col, aisp_s32_t *max_rows)
{
    int i = 0;
    int j = 0;
    int8_t *pout = out;
    aisp_s32_t max = 1;
    aisp_s32_t scale = 1;
    aisp_s32_t *pin = in;
    int tmp;

    for (i = 0; i < row; i++)
    {
        for (j = 0; j < 264; j++)                  // 264 = 24(fbank) * 11 (frames)
        {
            tmp = AISP_TSL_ABS(pin[j] * 100);           // 100 mean sigma;  ((1 + 2^2)*2)^2

            if (max < tmp)
            {
                max = tmp;
            }
        }

        for (; j < 528; j++)
        {
            tmp = AISP_TSL_ABS(pin[j] * 10);

            if (max < tmp)
            {
                max = tmp;
            }
        }

        for (; j < col; j++)
        {
            tmp = AISP_TSL_ABS(pin[j]);

            if (max < tmp)
            {
                max = tmp;
            }
        }

        pin += col;
        //max = _abs_max(in + i * col, col);
        //assert(max != 0);
        //max_rows[i] = Q6VALUE / max;
        max_rows[i] = max;
        max = 0;
    }

    pin = in;
    pout = out;

    for (i = 0; i < row; i++)
    {
        scale = max_rows[i];
        max_rows[i] = ((max_rows[i] + 50) / 100) << 4;

        for (j = 0; j < 264; j++)
        {
            tmp = *pin >= 0 ? (*pin * 100 + (scale >> 7)) / (scale >> 6) : ((*pin) * 100 - (scale >> 7)) / (scale >> 6);
            tmp = tmp > Q6UPPER_BOUND ? Q6UPPER_BOUND : tmp;
            tmp = tmp < Q6DOWN_BOUND ? Q6DOWN_BOUND : tmp;
            *pout = tmp;
            pout++;
            pin++;
        }

        for (; j < 528; j++)
        {
            tmp = *pin >= 0 ? (*pin * 10 + (scale >> 7)) / (scale >> 6) : ((*pin) * 10 - (scale >> 7)) / (scale >> 6);
            tmp = tmp > Q6UPPER_BOUND ? Q6UPPER_BOUND : tmp;
            tmp = tmp < Q6DOWN_BOUND ? Q6DOWN_BOUND : tmp;
            *pout = tmp;
            pout++;
            pin++;
        }

        for (; j < col; j++)
        {
            tmp = *pin >= 0 ? (*pin + (scale >> 7)) / (scale >> 6) : ((*pin) - (scale >> 7)) / (scale >> 6);
            tmp = tmp > Q6UPPER_BOUND ? Q6UPPER_BOUND : tmp;
            tmp = tmp < Q6DOWN_BOUND ? Q6DOWN_BOUND : tmp;
            *pout = tmp;
            pout++;
            pin++;
        }
    }
}
#elif defined(WAKEUP_NN_FSMN)
void cblas_fix32tofix8(int8_t *out, aisp_s32_t *in, int size, int qin, int qout)
{
    int i = 0;
    int8_t *pout = out;
    aisp_s32_t *pin = in;
    int qsub = qin - qout;
    int tmp;

    for (i = 0; i < size; i++)
    {
#ifdef USE_HIFI4_OPTIMIZE
        tmp = PSHR32(*pin, qsub);
#else
        tmp = PSHR(*pin, qsub);
#endif

        if (tmp > 127)
        {
            tmp = 127;
        }
        else
            if (tmp < -128)
            {
                tmp = -128;
            }

        *pout = tmp;
        pout++;
        pin++;
    }
}
#endif

#if defined(WAKEUP_NN_DNN)
/*
 * Description:
 *   Vector normalization from Q24 to Q6, in order to feed DNN
 * Input:
 *   in         --> input data interpreted as Q24
 *   row        --> num of the input row
 *   col        --> num of the input colum
 * Output:
 *   out        --> output data interpreted as Q6
 *   row_maxs   --> max abs value of each input row
 *
 * History:
 *   2017/01/08 chao.xu edit
 */
void cblas_q24_to_q6_version2(int8_t *out, int *row_maxs, int *in, int row, int col)
{
    int i, j;
    int tmp;
    int max_reciprocal_q24 = 1;
    int *pRow_maxs = row_maxs, * pIn = in;
    int32_t min_tmp;
    unsigned int index = 0;

    for (i = 0; i < row; i++)
    {
        *pRow_maxs = 0;
        AISP_TSL_max_q31(pIn, col, pRow_maxs, &index);
        AISP_TSL_min_q31(pIn, col, &min_tmp, &index);

        if (*pRow_maxs < (-min_tmp))
        {
            *pRow_maxs = -min_tmp;
        }

        pRow_maxs++;
    }

    pRow_maxs = row_maxs;
    pIn = in;

    /* normlize to Q6 */
    for (i = 0; i < row; i++)
    {
        max_reciprocal_q24 = ((int64_t)Q24VALUE * (int64_t)Q24VALUE + ((int64_t) * pRow_maxs >> 1)) / (int64_t) * pRow_maxs;

        for (j = 0; j < col; j++)
        {
            tmp = (int32_t)AISP_TSL_PSHR64((int64_t)(*pIn) * max_reciprocal_q24, 42);
            tmp = tmp > 63 ? 63 : tmp;
            tmp = tmp < -64 ? -64 : tmp;
            out[i * col + j] = tmp;
            pIn++;
        }

        pRow_maxs++;
    }
}
#endif

#if defined(WAKEUP_NN_DNN)
// tnet
void wtk_cblas_feature_to_matirx(wtk_cblas_matrix_t *m, int index, wtk_feature_t **pv, int step, int paddings,
                                 float *padding)
{
    aisp_s32_t *v;
    int i, j, n;
    aisp_s32_t *p;
    int k = 0;
    p = &m->m[m->col * index];

    for (i = 0; i < step; ++i)
    {
        v = pv[i]->v;
        n = wtk_vector_size(v);

        for (k = i, j = 1; j <= n; ++j, k += step)
        {
            p[k] = v[j];
        }
    }

    k -= step;

    for (k++, i = 0; i < paddings; k++, i++)
    {
        p[k] = padding[i];
    }
}
#elif defined(WAKEUP_NN_FSMN)
// kaldi
void wtk_cblas_feature_to_matirx(int32_t *m, wtk_feature_t **pv, int step)
{
    int32_t *v;
    int i;
    int j;
    int n;
    int32_t *p = m;

    for (i = 0; i < step; i++)
    {
        v = &pv[i]->v[1];
        n = wtk_vector_size(pv[i]->v);

        for (j = 0; j < n; j++)
        {
            *p++ = *v++;
        }
    }
}
#endif
