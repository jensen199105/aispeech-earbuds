/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : lite-uda.c
  Project    :
  Module     :
  Version    :
  Date       : 2019/10/10
  Author     : Chao.Xu
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Chao.Xu       2019/10/10         1.00              Create

************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "lite-uda.h"

//#define TEST_RESET
//#define USE_PARALLEL_FEED
#define US_SCALAR       (1000000)
#define MS_SCALAR       (1000)
#define FRM_INC_TIME    (16)        /* 16 ms */
#define WAV_HEADER_LEN  (44)
#define LUDA_IN_CHAN    (2)

static void usage(char *tool);
static void *getFileName(char *file);
static void bf_outputHook(void *usrdata, int idx, char *data, int len);
#ifdef ENABLE_LUDA_FOR_HEADSET
static void bf_windHook(void *usrdata, int idx, char *data, int len);
#endif
extern int AISP_TSL_pcmAddWavHeader(FILE *fp, int channels, int bits, int sample_rate, int len);

/************************************************************
  Function   : main()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/11/15, Chao.Xu create

 ************************************************************/
int main(int argc, char *argv[])
{
    char   acBuf[1024] = {0};
    int    iLenPerSnd  = 0;
    int    iLen        = 0;
    int    iCnt        = 0;
    int    iIdx        = 0;
    int    iOutFlag    = 0;
    float  fElapseTime = 0.0;
    char *pcData       = NULL;
    FILE *pfIn         = NULL;
    FILE *pfGscOut[3] = {NULL, NULL, NULL};
    GSC_Api_S     *pstGscApi = NULL;
    GSC_CoreCfg_S *pstGscCfg = NULL;
    int iRet            =   -1;
    int iSize           =    0;
    char *pcSharedBuf   = NULL;
    char *pcUnsharedBuf = NULL;
    struct timeval tvStart;
    struct timeval tvEnd;
#ifdef USE_PARALLEL_FEED
    short chan1[256] = {0};
    short chan2[256] = {0};
    short *psRaw     = NULL;
    int iLoop        = 0;
#endif
    usage(argv[0]);

    if (argc != 5)
    {
        printf("The number of parameters does not match.\n");
        return -1;
    }

    /**************************** Create wav ******************************/
    if (!strcmp(argv[3], "all"))
    {
        iOutFlag = 0x7;
    }
    else
    {
        iOutFlag = 1 << (atoi(argv[3]) - 1);
    }

    /************************ open input wav file *************************/
    pfIn = fopen(argv[1], "rb");
    fseek(pfIn, WAV_HEADER_LEN, SEEK_SET);
    pcData = strtok(getFileName(argv[1]), ".");

    for (iIdx = 0; iIdx < 3; iIdx++)
    {
        if ((1 << iIdx) & iOutFlag)
        {
            memset(acBuf, 0, sizeof(acBuf));
            sprintf(acBuf, "%s/%s.wkp-%d.wav", argv[2], pcData, iIdx + 1);
            pfGscOut[iIdx] = fopen(acBuf, "wb");

            if (NULL == pfGscOut[iIdx])
            {
                printf("Create %s fail.\n", acBuf);
                goto FAIL;
            }

            fseek(pfGscOut[iIdx], WAV_HEADER_LEN, SEEK_SET);
        }
        else
        {
            pfGscOut[iIdx] = NULL;
        }
    }

    /******************* Create udalite-gsc engine *********************/
    iSize = LUDA_CfgMemSizeGet();
//  printf("LUDA CfgMemsize = %d\n", iSize);
    pstGscCfg = (GSC_CoreCfg_S *)calloc(1, iSize);

    if (NULL == pstGscCfg)
    {
        printf("GSC_CoreCfg_S struct alloc fail.\n");
        goto LUDA_EXIT;
    }

    iRet = LUDA_CfgInit(pstGscCfg, argv[4]);

    if (iRet < 0)
    {
        printf("LUDA_CfgInit fail.\n");
        goto LUDA_CFG_FAIL;
    }

    /* SharedBuf/UnsharedBuf size */
    iSize = LUDA_SharedMemSizeGet(pstGscCfg);
//  printf("LUDA sharedMemsize = %d\n", iSize);
    pcSharedBuf = (char *)calloc(1, iSize);

    if (pcSharedBuf == NULL)
    {
        goto LUDA_CFG_FAIL;
    }

    iSize = LUDA_UnsharedMemSizeGet(pstGscCfg);
//  printf("GSC unsharedMemsize = %d\n", iSize);
    pcUnsharedBuf = (char *)calloc(1, iSize);

    if (pcUnsharedBuf == NULL)
    {
        goto LUDA_UNSHAREDBUF_FAIL;
    }

    pstGscApi = (GSC_Api_S *)LUDA_New(pstGscCfg, pcSharedBuf, pcUnsharedBuf, (GSC_FUNCHOOK_T)bf_outputHook, pfGscOut);
#ifdef ENABLE_LUDA_FOR_HEADSET
    LUDA_WindCbRegister(pstGscApi, bf_windHook, NULL);
#endif
    iLenPerSnd = LUDA_LenPerSnd(pstGscApi);
    pcData     = calloc(1, iLenPerSnd);
    gettimeofday(&tvStart, NULL);
#if 1

    while (!feof(pfIn))
#else
    for (int ii = 0; ii < 101; ii++)
#endif
    {
        iCnt++;
        iLen = fread(pcData, 1, iLenPerSnd, pfIn);
#ifdef USE_PARALLEL_FEED
        /* input chan is 4, [mic0 mic1 ref1 ref2] */
        psRaw = (short *)pcData;

        for (loop = 0; loop < 256; loop++, psRaw += LUDA_IN_CHAN)
        {
            chan1[loop] = *psRaw;
            chan2[loop] = *(psRaw + 1);
        }

        LUDA_FeedParallel(pstGscApi, (char *)chan1, (char *)chan2, iLen / LUDA_IN_CHAN);
#else
        LUDA_Feed(pstGscApi, pcData, iLen);
#endif
    }

    gettimeofday(&tvEnd, NULL);
#ifdef TEST_RESET
    LUDA_Reset(pstGscApi);
    fseek(pfIn, WAV_HEADER_LEN, SEEK_SET);

    while (!feof(pfIn))
    {
        iCnt++;
        iLen = fread(pcData, 1, iLenPerSnd, pfIn);
#ifdef USE_PARALLEL_FEED
        /* input chan is 4, [mic0 mic1 ref1 ref2] */
        psRaw = (short *)pcData;

        for (loop = 0; loop < 256; loop++, psRaw += LUDA_IN_CHAN)
        {
            chan1[loop] = *psRaw;
            chan2[loop] = *(psRaw + 1);
        }

        LUDA_FeedParallel(pstGscApi, (char *)chan1, (char *)chan2, iLen / LUDA_IN_CHAN);
#else
        LUDA_Feed(pstGscApi, pcData, iLen);
#endif
    }

#endif
    /* release rdma engine */
    LUDA_Delete(pstGscApi);
    fElapseTime = (tvEnd.tv_sec - tvStart.tv_sec) + (float)(tvEnd.tv_usec - tvStart.tv_usec) / US_SCALAR;
    printf("Elapse Time %f\n", fElapseTime);
    printf("RRT: %f\n", (float)fElapseTime * MS_SCALAR / (iCnt * FRM_INC_TIME));

    /* add wav header */
    for (iIdx = 0; iIdx < 3; iIdx++)
    {
        if (pfGscOut[iIdx])
        {
            iLen = ftell(pfGscOut[iIdx]);
            AISP_TSL_pcmAddWavHeader(pfGscOut[iIdx], 1, 16, 16000, iLen);
            fclose(pfGscOut[iIdx]);
            pfGscOut[iIdx] = NULL;
        }
    }

FAIL:

    /* release resources */
    if (pcData)
    {
        free(pcData);
    }

    if (pfIn)
    {
        fclose(pfIn);
    }

    for (iIdx = 0; iIdx < 3; iIdx++)
    {
        if (pfGscOut[iIdx])
        {
            fclose(pfGscOut[iIdx]);
        }
    }

    if (pcUnsharedBuf)
    {
        free(pcUnsharedBuf);
    }

LUDA_UNSHAREDBUF_FAIL:

    if (pcSharedBuf)
    {
        free(pcSharedBuf);
    }

LUDA_CFG_FAIL:
    LUDA_CfgDelete(pstGscCfg);
LUDA_EXIT:
    printf("GSC exit.\n");
    return 0;
}

/************************************************************
 *   Function   : usage()
 *   Description:
 *   Calls      :
 *   Called By  :
 *   Input      :
 *   Output     :
 *   Return     :
 *   Others     :
 *   History    :
 *     2019/10/10, Chao.Xu create
 ***********************************************************/
static void usage(char *tool)
{
    printf("*****************************************\n"
           "   Copyright (C) AIspeech Co., Ltd.      \n"
           "   Version : %s\n"
           "   Usage   :\n"
           "   %s <input> <output dir> <index> <bin>      \n"
           "*****************************************\n", LUDA_Version(), tool);
}

/************************************************************
 *   Function   : getFilename()
 *   Description:
 *   Calls      :
 *   Called By  :
 *   Input      :
 *   Output     :
 *   Return     :
 *   Others     :
 *   History    :
 *     2018/11/30, Youhai.Jiang create
 ***********************************************************/
static void *getFileName(char *file)
{
    char ch    = '/';
    char *name = strrchr(file, ch) + 1;
    return name;
}

/************************************************************
 *   Function   : bf_outputHook()
 *   Description: store gsc output data
 *   Calls      :
 *   Called By  :
 *   Input      :
 *   Output     :
 *   Return     :
 *   Others     :
 *   History    :
 *     2018/11/15, Youhai.Jiang create
 ***********************************************************/
static void bf_outputHook(void *usrdata, int idx, char *data, int len)
{
    FILE **fp = (FILE **)usrdata;

    if (fp[idx])
    {
        fwrite(data, 1, len, fp[idx]);
    }
}

#ifdef ENABLE_LUDA_FOR_HEADSET
/************************************************************
 *   Function   : bf_windHook()
 *   Description: Post the wind noise status.
 *   Calls      :
 *   Called By  :
 *   Input      :
 *   Output     :
 *   Return     :
 *   Others     :
 *   History    :
 *     2018/11/15, Youhai.Jiang create
 ***********************************************************/
static void bf_windHook(void *usrdata, int idx, char *data, int len)
{
    printf("Main:bf_windHook: The current status is %d\n", idx);
}
#endif

