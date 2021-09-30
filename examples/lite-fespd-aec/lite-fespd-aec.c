/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : test.c
  Project    : 
  Module     : 
  Version    : 
  Date       : 2018/11/15
  Author     : Youhai.Jiang
  Document   : 
  Description: 

  Function List :

  History :
  <author>      <time>            <version >        <desc> 
  Youhai.Jiang  2018/11/15         1.00              Create

************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lite-fespd-aec.h"

#define US_SCALAR       (1000000)
#define MS_SCALAR       (1000)
#define FRM_INC_TIME    (16)        /* 16 ms */
#define WAV_HEADER_LEN  (44)
#define BEAMS           (3)

extern int AISP_TSL_pcmAddWavHeader(FILE *fp, int channels, int bits, int sample_rate, int len);

/************************************************************
  Function   :  aec_handler()

  Description: Dump the output data of AEC.
  Calls      :
  Called By  : LDEEW_start(...)
  Input      : 
               usrdata --> callback arg
			   data    --> aec processed data
			   len     --> bytes of data
  Output     :
  Return     :
  Others     :

  History    :
    2018/11/15, Youhai.Jiang create

************************************************************/
void aec_handler(void *usrdata, char *data, int len)
{
    FILE *fp_aec = (FILE *)usrdata;

    fwrite (data, 1, len, fp_aec);
}

/************************************************************
  Function   : bf_outputHook()

  Description: Dump the output data of BF, the data may be in time domain or in freq domain.
  Calls      :
  Called By  : LDEEW_start(...)
  Input      : 
               usrdata --> File pointer for BF output
			   idx     --> The beam idx for BF output
			   data    --> Pointer to time-domain data(256 char) or freq-domian data(257 cpx int)
			   len     --> 256 for time domain 
			               257 for freq domain 
  Output     :
  Return     :
  Others     :

  History    :
    2018/11/15, Youhai.Jiang create

************************************************************/
void bf_handler(void *usrdata, int idx, char *data, int len)
{
#ifdef LUDA_SUPPORT_FREQOUT_DIRECT
	/* Taked from gscProcessMobile directly. */
    int *pcpxEntBlock = (int *)data;

	if (NULL == pcpxEntBlock)
	{
		printf("%s:%d: pcpxEntBlock NULL pointer!\n", __FUNCTION__, __LINE__);
	}

#else
    FILE **fp = (FILE **)usrdata;

    if (fp[idx])
    {
        fwrite (data, 1, len, fp[idx]);
    }
#endif
}

/************************************************************
  Function   : wkp_handler()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
static int wkp_handler(void *pvUsrData, int iIdx, char *pcJson)
{
    printf("Channel %d wakeuped; Details: %s\n", iIdx, pcJson);
    return 0;
}

/************************************************************
  Function   : vad_handler()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/16, Youhai.Jiang create

************************************************************/
int vad_handler(void *pvUsrData, int iChanIdx, int iFrmStatus, int iFrmIdx)
{
    printf("Chan%d: Vad status: %d; Frame: %d\n", iChanIdx, iFrmStatus, iFrmIdx);
    return 0;
}

/************************************************************
  Function   : usage()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/11/16, Youhai.Jiang create

************************************************************/
void usage(char *tool)
{
    printf("*****************************************\n"
           "   Copyright (C) AIspeech Co., Ltd.      \n"
           "   Version : %s\n"
           "   Usage   :\n"
           "   %s <input.wav> <output dir> <index> <env> \n"
           "*****************************************\n", LDEEW_version(), tool);
}

/************************************************************
  Function   : getFilename()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2018/11/30, Youhai.Jiang create

************************************************************/
static void *getFileName(char *file)
{
    char ch = '/';
    char *name = strrchr(file, ch) + 1;

    return name;
}

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
    2018/11/15, Youhai.Jiang create

************************************************************/
int main(int argc, char *argv[])
{
    int lenPerSnd = 0;
    int len       = 0;
    int cnt       = 0;
    int idx       = 0;
    int memPoolLen = 0;
    char *pcMemPool = NULL;
    char *pcData = NULL;
    FILE *pfIn = NULL;
    FILE *pfAec = NULL;
    FILE *pfpGscOut[3] = {NULL, NULL, NULL};
    LDEEW_ENGINE_S *pstLfespdEng = NULL;
    char buf[1024] = {0};
    char cOutFlag  = 0;
    
    usage(argv[0]);
    if (argc != 5)
    {
        return -1;
    }

    if (!strcmp(argv[BEAMS], "all"))
    {
        cOutFlag = 0x7;
    }
    else
    {
        cOutFlag = 1 << (atoi(argv[3])-1);
    }

    /* open raw data file */
    pfIn = fopen(argv[1], "rb");
    if (NULL == pfIn)
    {
        printf("Open file %s fail.\n", argv[1]);
        return -1;
    }
 
    pfAec = fopen("aec_out.pcm", "wb");
    if (NULL == pfAec)
    {
        printf("Open file aec_out.pcm fail.\n");
        return -1;
    }

    /* skip wav header */
    fseek(pfIn, WAV_HEADER_LEN, SEEK_SET);
    pcData = strtok(getFileName(argv[1]), ".");
    for (idx=0; idx < BEAMS; idx++)
    {
        if ((1<<idx) & cOutFlag)
        {
            memset(buf, 0, sizeof(buf));
            
            snprintf(buf, sizeof(buf), "%s/%s.wkp-%d.wav", argv[2], pcData, idx+1);
            pfpGscOut[idx] = fopen(buf, "wb");
            if (NULL == pfpGscOut[idx])
            {
                printf("Create %s fail.\n", buf);
                goto FAIL;
            }
            fseek(pfpGscOut[idx], WAV_HEADER_LEN, SEEK_SET);
        }
        else
        {
            pfpGscOut[idx] = NULL;
        }
    }    

    memPoolLen = LDEEW_memSize();
    
    /* mem pool for lite-fespd */
    pcMemPool = calloc(1, memPoolLen);
    if (NULL == pcMemPool)
    {
        goto FAIL;
    }
    
    /* create rdma engine */
    pstLfespdEng = LDEEW_new(pcMemPool, memPoolLen);
    
    /* start rdma engine */
    LDEEW_start(pstLfespdEng, bf_handler, pfpGscOut, wkp_handler, NULL, argv[4]);
    LDEEW_aecCallbackRegister(pstLfespdEng, aec_handler, pfAec);

    lenPerSnd = LDEEW_lenPerSend(pstLfespdEng);
    
    pcData = calloc(1, lenPerSnd);
    if (NULL == pcData)
    {
        goto FAIL;
    }

    while(!feof(pfIn))
    {
        cnt++;
        //if(cnt> 300)
        //{
        //    break;
        //}
        len = fread(pcData, 1, lenPerSnd, pfIn);
        LDEEW_feed(pstLfespdEng, pcData, len);
    }

    LDEEW_end(pstLfespdEng);
    

    /* add wav header */
    for (idx=0; idx < BEAMS; idx++)
    {
        if (pfpGscOut[idx])
        {
            len = ftell(pfpGscOut[idx]);
            AISP_TSL_pcmAddWavHeader(pfpGscOut[idx], 1, 16, 16000, len);
            fclose (pfpGscOut[idx]);
            pfpGscOut[idx] = NULL;
        }
    }

FAIL:
    /* release resources */
    if(pcData)
    {
        free(pcData);
    }

    if (pfIn)
    {
        fclose(pfIn);
    }

    if (pfAec)
    {
        fclose(pfAec);
    }

    for (idx=0; idx < BEAMS; idx++)
    {
        if (pfpGscOut[idx])
        {
            fclose (pfpGscOut[idx]);
        }
    }

    /* release rdma engine */
    if (pstLfespdEng)
    {
        LDEEW_delete(pstLfespdEng);
        pstLfespdEng = NULL;
    }

    
    if (pcMemPool)
    {
        free(pcMemPool);
        pcMemPool = NULL;
    }

    
    return 0;
}

