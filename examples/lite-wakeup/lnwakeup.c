/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : lwakeup.c
  Project    :
  Module     :
  Version    :
  Date       : 2019/08/22
  Author     : Youhai.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Jin.Chen      2019/08/22         1.00              Create

************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lnwakeup.h"

#define FEED_SIZE						(640)
#define MEM_MAX_SIZE               		(120*1024)

static int _handler(void *argv,int chan_index, char *json)
{
   printf("[%d] %s\n", chan_index, json);
   LNWKP_reset(*(nwakeup_t **)argv);
   return 0;
}

int main(int argc,char *argv[])
{
    const int memsize = MEM_MAX_SIZE;
	int chans;
    char* wavFile;
    char* pcWakeupWords;
    FILE* pstWavFile;
    char* pmembase = NULL;
    nwakeup_t* pstNwkp;
	char* buf = NULL;
	char data[FEED_SIZE];
	int ret = -1;

    if (argc == 4)
    {
        chans = atoi(argv[1]);
        wavFile = argv[2];
        pcWakeupWords = argv[3];
    }
    else
    {
        printf("***************************************************\n"
            "version: %s\n"
            "usage:\n"
            "    %s <chans> <wav> <env>\n"
            "***************************************************\n",
            LNWKP_version(), argv[0]);
        return ret;
    }

    pstWavFile = fopen(wavFile, "rb");
    if (NULL == pstWavFile)
    {
        printf("failed to open file\n");
        goto out;
    }

    fseek(pstWavFile, 44, SEEK_SET);

    pmembase = calloc(1, memsize);
    if (NULL == pmembase)
    {
        goto out;
    }

    buf = malloc(chans * sizeof(data));
    if (NULL == buf)
    {
        goto out;
    }

    pstNwkp = LNWKP_new(chans, pmembase, memsize);
    if (NULL == pstNwkp)
    {
        printf("failed to new wakeup\n");
        goto out;
    }

    LNWKP_reset(pstNwkp);
    LNWKP_registerHandler(pstNwkp, &pstNwkp, _handler);
    LNWKP_start(pstNwkp, pcWakeupWords, strlen(pcWakeupWords));

	while (1)
	{
        int n;
        int index;
        int i;

        n = fread(buf, 1, chans * sizeof(data), pstWavFile);
		if (n <= 0)
			break;

		for (index = 0; index < chans; index++)
		{
			for (i = 0; i < n/2/chans; i++)
			{
				data[2*i] = buf[2*chans*i+2*index];
				data[2*i+1] = buf[2*chans*i+2*index+1];
			}

            LNWKP_feed(pstNwkp, index, data, n/chans, 0);
		}
	}

	ret = 0;

out:
	if (NULL != pstNwkp)
	{
        LNWKP_end(pstNwkp);
		LNWKP_delete(pstNwkp);
	}
	if (NULL != pstWavFile)
		fclose(pstWavFile);
	if (NULL != pmembase)
		free(pmembase);
	if (NULL != buf)
		free(buf);

    return ret;
}
