/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : test_api_serial.c
  Project    :
  Module     :
  Version    :
  Date       : 2016/12/05
  Author     : Youhai.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  changren.tan  2016/12/05         1.00              Create

************************************************************/
#include "sevc_api.h"
#include "wavfile.h"
#include <errno.h>
//#include <pthread.h>
//#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <time.h>
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif
// #define TEST_RESET
// #define DTD_TEST
//#define SEVC_MALLOC_OUT
#define SEVC_COMPARE_DBG         0
#define SEVC_INFO_SAVE           0

#define DBG_HERE()          printf("%s [%d] +++++++++++\r\n", __func__, __LINE__)


typedef struct tagTIMEVAL_S
{
    long tv_sec;
    long tv_usec;
} TIMEVAL_S;

typedef struct  tagTIMEZONE_S
{
    int tz_minuteswest;
    int tz_dsttime;
} TIMEZONE_S;

#if SEVC_COMPARE_DBG
FILE *g_pfDisp = NULL;
#endif

FILE *fpYyPcm;

short *psTmpBuf = NULL;
char g_cFlag = 0;

static int SAL_GetTimeofDay(TIMEVAL_S *tv, TIMEZONE_S *tz)
{
#ifdef _WIN32
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;

    if (tv)
    {
        GetLocalTime(&wtm);
        tm.tm_year = wtm.wYear - 1900;
        tm.tm_mon = wtm.wMonth - 1;
        tm.tm_mday = wtm.wDay;
        tm.tm_hour = wtm.wHour;
        tm.tm_min = wtm.wMinute;
        tm.tm_sec = wtm.wSecond;
        tm.tm_isdst = -1;
        clock = mktime(&tm);
        tv->tv_sec = clock;
        tv->tv_usec = wtm.wMilliseconds * 1000;
    }

#else
    struct timeval tvTmp;
    struct timezone tzTmp;
    gettimeofday(&tvTmp, &tzTmp);
    tv->tv_sec = tvTmp.tv_sec;
    tv->tv_usec = tvTmp.tv_usec;
    tz->tz_minuteswest = tzTmp.tz_minuteswest;
    tz->tz_dsttime = tzTmp.tz_dsttime;
#endif
    return 0;
}

unsigned char g_ucBuff[8192];

void sevcMiddleFeed(void  *pstSevcEngine, unsigned char *pucBuff, unsigned int uiSize)
{
    int len = uiSize;
    SEVC_API_IoCtrl(pstSevcEngine, IOCTL_SET_MIDDLE_FREQ_BINS, pucBuff, &len);
}

void sevcMiddleFrameProc(unsigned char *pucBuff, unsigned int uiSize, void *pUsrData)
{
    void  *pstSevcEngine = pUsrData;
    memcpy(g_ucBuff, pucBuff, uiSize);
    sevcMiddleFeed(pstSevcEngine, g_ucBuff, uiSize);
}

void sevcOutFrameProc(unsigned char *pucBuff, unsigned int uiSize, void *pUsrData)
{
    unsigned int uiDataLen = uiSize >> 1;
    short *psData = (short *)pucBuff;
    FILE **pfAddr = (FILE **)pUsrData;
    FILE *pf = (FILE *)*pfAddr;

    if (NULL != pf)
    {
        wavfile_write(pf, psData, uiDataLen);
    }
}

#if SEVC_INFO_SAVE
void sevcDataSave(unsigned char *pucData, unsigned int uiLen)
{
    FILE *pfData = NULL;
    pfData = fopen("sevcInfo.bin", "wb+");

    if (NULL != pfData)
    {
        fwrite(pucData, uiLen, 1, pfData);
        fclose(pfData);
        pfData = NULL;
    }
}
#endif

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
    2016/12/05, changren.tan create
    2018/09/05, kun.niu  modified
************************************************************/
int main(int argc, char *argv[])
{
    short sBlockSZ = 0;
    short sMicNum = 0;
    unsigned int lReadSZ = 0;
    unsigned int lFrameSZ = 0;
    unsigned char *pcPcmFrame = NULL;
    char *pcOutWav = NULL;
    char *pcInWav = NULL;
    char *pcOutPcm = NULL;
    char *pcCfgFile = NULL;
    char *pcCfgData = NULL;
    FILE *fpInWav = NULL;
    FILE *fpOutWav = NULL;
    void  *pstSevcEngine = NULL;
    TIMEVAL_S tv1, tv2;
    TIMEZONE_S tz;
    int iframeTotalLen = 0;
    float duration = 0.0f;
    float RRT = 0.0f;
    struct wavfile_header head;
    unsigned int uiReadCnt = 0;
    unsigned int numOfFrames;
#if SEVC_INFO_SAVE
    char acBuff[1024] = {0};
    int iDataLen = 0;
    int iRes;
#endif
    printf("*******************************************************"
           "******************\n"
           " Copyright (c) 2015 - 2025 AISPEECH.\n"
           " version : %s\n"
           " usage   :\n"
           " %s <input wav> <output wav>\n"
           "*******************************************************"
           "******************\n", SEVC_API_Version(), argv[0]);

    // if (argc != 3)
    // {
    //     return -1;
    // }

    if (argc == 4)
    {
        pcCfgFile = argv[3];
        pcOutPcm = argv[3];
    }

    pcInWav = argv[1];
    pcOutWav = argv[2];
    /* input wav */
    fpInWav = wavfile_read_open(pcInWav, &head);

    if (!fpInWav)
    {
        printf("couldn't open %s for reading: %s\n", pcInWav, strerror(errno));
        return -1;
    }

    lFrameSZ = (head.sample_rate == 8000 ? 128 : 256);
    printf("sample_rate=%d  frameShift=%d dataOffset=%d\r\n", head.sample_rate, lFrameSZ, (int)ftell(fpInWav));
    numOfFrames = head.data_length / (head.bits_per_sample / 8) / lFrameSZ / head.num_channels;
    printf("sequence has %d frames\n", numOfFrames);
    pcOutWav = argv[2];

    if (NULL != pcCfgFile)
    {
        //pcCfgData = (char *)cfgDataRead(pcCfgFile);
    }

#ifdef SEVC_MALLOC_OUT
    unsigned int memSize = SEVC_API_MemSizeGet(pcCfgData);
    // printf("%d\n", memSize);
    char *memory = (char *)calloc(1, memSize);
    char *memoryState = memory;
    pstSevcEngine = SEVC_API_New(pcCfgData, memory, memSize, head.sample_rate);
#else
    pstSevcEngine = SEVC_API_New(pcCfgData, NULL, 0, head.sample_rate);
#endif

    if (NULL == pstSevcEngine)
    {
        printf("%s() %d: sevc engine create fail.\n", __FUNCTION__, __LINE__);
        goto SEVC_ENGINE_CREATE_FAIL;
    }

#if SEVC_INFO_SAVE
    iRes = SEVC_API_IoCtrl(pstSevcEngine, IOCTL_GET_SEVC_INFO, acBuff, &iDataLen);

    if (0 == iRes && iDataLen)
    {
        sevcDataSave((unsigned char *)acBuff, iDataLen);
    }

#endif
    sBlockSZ = SEVC_API_MemSize(pstSevcEngine);
    sMicNum = SEVC_API_MicNum(pstSevcEngine);
    lFrameSZ = SEVC_API_FrameSize(pstSevcEngine);
    fpOutWav = wavfile_write_open(pcOutWav, 1, head.sample_rate);

    if (argc == 4)
    {
        fpYyPcm = fopen(pcOutPcm, "wb");

        if (!fpYyPcm)
        {
            printf("couldn't open %s for writing: %s", pcOutPcm, strerror(errno));
            goto EXIT;
        }
    }

    if (!fpOutWav)
    {
        printf("couldn't open %s for writing: %s", pcOutWav, strerror(errno));
        goto EXIT;
    }

    SEVC_API_CallbackRegister(pstSevcEngine, sevcOutFrameProc, &fpOutWav);
#ifdef SEVC_MULTI_CORE
    SEVC_API_CallbackRegisterMiddle(pstSevcEngine, sevcMiddleFrameProc, pstSevcEngine);
#endif
    /* chans * sizeof(16bit or 24bit) * 512point */
    pcPcmFrame = (unsigned char *)malloc(sBlockSZ * lFrameSZ);

    if (NULL == pcPcmFrame)
    {
        printf("%s() %d: Malloc fail.\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }

    psTmpBuf = (short *)malloc(lFrameSZ * sizeof(short) * sMicNum);

    if (NULL == psTmpBuf)
    {
        printf("%s() %d: Malloc fail.\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }

#if SEVC_COMPARE_DBG
    g_pfDisp = fopen("lite_sevc_dbg.txt", "wb");

    if (NULL == g_pfDisp)
    {
        printf("couldn't open %s for writing: %s", "lite_sevc_dbg.txt", strerror(errno));
        goto EXIT;
    }

#endif
    //int frameInit = 0;
    SEVC_API_Reset(pstSevcEngine);
    /* time calc start */
    SAL_GetTimeofDay(&tv1, &tz);
    //fseek(fpInWav, 46, SEEK_SET);
    /* muti-channel data do not use wavfile_read */
    lReadSZ = fread(pcPcmFrame, sBlockSZ, lFrameSZ, fpInWav);

    while (lReadSZ > 0 && numOfFrames > 0)
    {
        iframeTotalLen += lReadSZ;

        if (lReadSZ == lFrameSZ)
        {
            SEVC_API_Feed(pstSevcEngine, pcPcmFrame, lReadSZ * sBlockSZ);
        }
        else
        {
            /* the last block, fill with 0 */
            memset(psTmpBuf, 0, lReadSZ * sizeof(short) * sMicNum);
            wavfile_write(fpOutWav, psTmpBuf, lReadSZ * sMicNum);
            break;
        }

        numOfFrames--;
        uiReadCnt++;
        /* muti-channel data do not use wavfile_read */
        lReadSZ = fread(pcPcmFrame, sBlockSZ, lFrameSZ, fpInWav);
    }

    SAL_GetTimeofDay(&tv2, &tz);
    /* elapse time */
    duration = (float)(tv2.tv_sec - tv1.tv_sec) + (float)(tv2.tv_usec - tv1.tv_usec) / 1000000;
    RRT = ((float)iframeTotalLen) / head.sample_rate * 1000;
    RRT = duration * 1000 / RRT;
    printf("uiReadCnt=%d\r\n", uiReadCnt);
    printf("duration: %f s\n", duration);
    printf("RRT: %f\n", RRT);

    if (pcPcmFrame)
    {
        free(pcPcmFrame);
    }

    if (psTmpBuf)
    {
        free(psTmpBuf);
    }

EXIT:
#ifdef SEVC_MALLOC_OUT

    if (memoryState)
    {
        free(memoryState);
    }

#endif
SEVC_ENGINE_CREATE_FAIL:
    g_cFlag = 1;

    if (fpInWav)
    {
        fclose(fpInWav);
    }

    if (NULL != fpOutWav)
    {
        /* update wav header */
        wavfile_write_close(fpOutWav);
        fpOutWav = NULL;
    }

    if (fpYyPcm != NULL)
    {
        fclose(fpYyPcm);
        fpYyPcm = NULL;
    }

#if SEVC_COMPARE_DBG

    if (NULL != g_pfDisp)
    {
        fclose(g_pfDisp);
    }

#endif

    if (NULL != pcCfgData)
    {
        free(pcCfgData);
    }

    SEVC_API_Delete(pstSevcEngine);
    return 0;
}
