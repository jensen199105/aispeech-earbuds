#include "wtk_math.h"

void wtk_vector_zero_mean_frame(wtk_short_vector_t* v)
{
    int sum = 0;
    int off = 0;

    wtk_short_vector_do_p(v, sum+=, );
    off = PDIV(sum, wtk_short_vector_size(v));
    wtk_short_vector_do_p(v, , -=off);
}

const short log_table[15] = {11357,5678,2839,1420,710,415,228,121,62,32,16,8,4,2,1};
short wtk_fxlog(int x)   // fixed log , input: Q0 , output: Q10;
{
    int t = 0;
    int y = 22003;
    if(x<0x00008000) x<<=16,              y-=log_table[0];
    if(x<0x00800000) x<<= 8,              y-=log_table[1];
    if(x<0x08000000) x<<= 4,              y-=log_table[2];
    if(x<0x20000000) x<<= 2,              y-=log_table[3];
    if(x<0x40000000) x<<= 1,              y-=log_table[4];
    t=x+(x>>1); if((t&0x80000000)==0) x=t,y-=log_table[5];
    t=x+(x>>2); if((t&0x80000000)==0) x=t,y-=log_table[6];
    t=x+(x>>3); if((t&0x80000000)==0) x=t,y-=log_table[7];
    t=x+(x>>4); if((t&0x80000000)==0) x=t,y-=log_table[8];
    t=x+(x>>5); if((t&0x80000000)==0) x=t,y-=log_table[9];
    t=x+(x>>6); if((t&0x80000000)==0) x=t,y-=log_table[10];
    t=x+(x>>7); if((t&0x80000000)==0) x=t,y-=log_table[11];
    x=0x80000000-x;
    y-=x>>21;
    return y;
}

void wtk_math_do_diff(aisp_s32_t **pv,int window_size, aisp_s16_t sigma, int start_pos, int step)
{
    int i,j,k,end=start_pos+step;
    aisp_s32_t *v=pv[window_size];
    aisp_s32_t *p,*n;
    int vs=start_pos-step;
    /*
    double sigma;

    sigma=0;
    for(i=1;i<=window_size;++i)
    {
        sigma+=i*i;
    }
    sigma*=2;
    */
    for(i=1;i<=window_size;++i)
    {
        p = pv[window_size-i];
        n = pv[window_size+i];
        for(j=start_pos,k=vs;j<end;++j,++k)
        {
            if(i==1)
            {
                v[j]=(n[k]-p[k]);
            }else
            {
                v[j]+=i*(n[k]-p[k]);
            }
            if(i==window_size)
            {
                //v[j] = v[j]>=0?((v[j]+(sigma>>1))/sigma):((v[j]+(sigma>>1))/sigma);
            }
        }
    }
}

/************************************************************
  Function   : fxlog_q31()

  Description: y = ln(x)
  Calls      :
  Called By  :
  Input      : x: Q0.31
  Output     : y: Q5.26
  Return     :
  Others     :

  History    :
    2019/05/09, chao.xu create

************************************************************/
int fxlog_q31(int x)  /* x belong to  [0, 1) */
{
    int t;
    int y = 0;

    if (x < 0x00008000) x <<= 16, y -= 0x2C5C85FE; /* intermedistes stored in Q31 */
    if (x < 0x00800000) x <<= 8, y -= 0x162E42FF;
    if (x < 0x08000000) x <<= 4, y -= 0x0B17217F;
    if (x < 0x20000000) x <<= 2, y -= 0x058B90C0;
    if (x < 0x40000000) x <<= 1, y -= 0x02C5C860;

    t = x + (x >> 1); if ((t & 0x80000000) == 0) x = t, y -= 0x019F323F; /* to be sure x is < 1 */
    t = x + (x >> 2); if ((t & 0x80000000) == 0) x = t, y -= 0x00E47FBE;
    t = x + (x >> 3); if ((t & 0x80000000) == 0) x = t, y -= 0x00789C1E;
    t = x + (x >> 4); if ((t & 0x80000000) == 0) x = t, y -= 0x003E1462;
    t = x + (x >> 5); if ((t & 0x80000000) == 0) x = t, y -= 0x001F829B;
    t = x + (x >> 6); if ((t & 0x80000000) == 0) x = t, y -= 0x000FE054;
    t = x + (x >> 7); if ((t & 0x80000000) == 0) x = t, y -= 0x0007F80B;
    t = x + (x >> 8); if ((t & 0x80000000) == 0) x = t, y -= 0x0003FE01;
    t = x + (x >> 9); if ((t & 0x80000000) == 0) x = t, y -= 0x0001FF80;
    t = x + (x >> 10); if ((t & 0x80000000) == 0) x = t, y -= 0x0000FFE0;
    t = x + (x >> 11); if ((t & 0x80000000) == 0) x = t, y -= 0x00007FF8;
    t = x + (x >> 12); if ((t & 0x80000000) == 0) x = t, y -= 0x00003FFE;
    t = x + (x >> 13); if ((t & 0x80000000) == 0) x = t, y -= 0x00002000;
    t = x + (x >> 14); if ((t & 0x80000000) == 0) x = t, y -= 0x00001000;
    t = x + (x >> 15); if ((t & 0x80000000) == 0) x = t, y -= 0x00000800;
    t = x + (x >> 16); if ((t & 0x80000000) == 0) x = t, y -= 0x00000400;
    x = 0x80000000 - x;
    y -= x >> 5;

    return y;
}
