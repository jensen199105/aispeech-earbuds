/*
A simple sound library for CSE 20211 by Douglas Thain
For course assignments, you should not change this file.
For complete documentation, see:
http://www.nd.edu/~dthain/courses/cse20211/fall2013/wavfile
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wavfile.h"

void wavFileInfoShow(struct wavfile_header *pHead)
{
    char acTags[32];
    memcpy(acTags, pHead->riff_tag, sizeof(pHead->riff_tag));
    acTags[sizeof(pHead->riff_tag)] = '\0';
    printf("RIFF_Tag %s\r\n", acTags);
    printf("RIFF_Len %d\r\n", pHead->riff_length);
    memcpy(acTags, pHead->wave_tag, sizeof(pHead->wave_tag));
    acTags[sizeof(pHead->wave_tag)] = '\0';
    printf("WaveTag %s\r\n", acTags);
    memcpy(acTags, pHead->fmt_tag, sizeof(pHead->fmt_tag));
    acTags[sizeof(pHead->fmt_tag)] = '\0';
    printf("FormatTag %s\r\n", acTags);
    printf("FormatLen %d\r\n", pHead->fmt_length);
    printf("AudioFormat %d\r\n", pHead->audio_format);
    printf("ChannelNum %d\r\n", pHead->num_channels);
    printf("SampleRate %d\r\n", pHead->sample_rate);
    printf("ByteRate %d\r\n", pHead->byte_rate);
    printf("BlockAlign %d\r\n", pHead->block_align);
    printf("BitPerSample %d\r\n", pHead->bits_per_sample);
    memcpy(acTags, pHead->data_tag, sizeof(pHead->data_tag));
    acTags[sizeof(pHead->data_tag)] = '\0';
    printf("DataTag %s\r\n", acTags);
    printf("DataLen %d\r\n", pHead->data_length);
}

int wavFileHeadCHeck(struct wavfile_header *pHead)
{
    char acRiffTags[8];
    char acWaveTags[8];
    char acFmtTags[8];
    char acDataTags[8];
    memcpy(acRiffTags, "RIFF", 4);
    memcpy(acWaveTags, "WAVE", 4);
    memcpy(acFmtTags, "fmt ", 4);
    memcpy(acDataTags, "data", 4);

    if (0 != memcmp(pHead->riff_tag, "RIFF", 4))
    {
        return -1;
    }

    if (0 != memcmp(pHead->wave_tag, "WAVE", 4))
    {
        return -2;
    }

    if (0 != memcmp(pHead->fmt_tag, "fmt", 3))
    {
        return -3;
    }

    if (0 != memcmp(pHead->data_tag, "data", 4))
    {
        return -4;
    }

    return 0;
}

/************************************************************
  Function   : wavfile_write_open()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/26, create

************************************************************/
FILE *wavfile_write_open(const char *filename, int chans, int sample_rate)
{
    //int samples_per_second = WAVFILE_SAMPLES_PER_SECOND;
    int bits_per_sample = 16;
    FILE *fp = NULL;
    struct wavfile_header header;
    memcpy(header.riff_tag, "RIFF", 4);
    memcpy(header.wave_tag, "WAVE", 4);
    memcpy(header.fmt_tag, "fmt ", 4);
    memcpy(header.data_tag, "data", 4);
    header.riff_length = 0;
    header.fmt_length = 16;
    header.audio_format = 1;
    header.num_channels = chans;
    header.sample_rate = sample_rate;
    header.block_align = bits_per_sample;
    header.byte_rate = sample_rate * header.block_align;
    header.bits_per_sample = bits_per_sample;
    header.data_length = 0;
    fp = fopen(filename, "wb+");

    if (!fp)
    {
        return 0;
    }

    fwrite(&header, sizeof(header), 1, fp);
    fflush(fp);
    return fp;
}

/************************************************************
  Function   : wavfile_read_open()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/26, create

************************************************************/
FILE *wavfile_read_open(const char *filename , struct wavfile_header *header)
{
    FILE *fp = NULL;
    size_t size = 0;
    int res;
    int iOffset;
    //char acFact[4] = {0x66, 0x61, 0x73, 0x74};
    char acFact[4] = {0x64, 0x61, 0x74, 0x61};

    if (header)
    {
        fp = fopen(filename, "rb+");

        if (NULL == fp)
        {
            return 0;
        }

        size = fread(header, sizeof(struct wavfile_header), 1, fp);

        if (size != 1)
        {
            printf("read head error\n");
            fclose(fp);
            return NULL;
        }

        if (header->fmt_length != 16)
        {
            iOffset = 20 + header->fmt_length;
            fseek(fp, iOffset, SEEK_SET);
            size = fread(header->data_tag, 8, 1, fp);

            while (0 != memcmp(header->data_tag, acFact, 4))
            {
                iOffset = iOffset + 8 + header->data_length;
                fseek(fp, iOffset, SEEK_SET);
                size = fread(header->data_tag, 8, 1, fp);
            }
        }
    }
    else
    {
        return NULL;
    }

    res = wavFileHeadCHeck(header);

    if (0 != res)
    {
        printf("wave head check %d\r\n", res);
        wavFileInfoShow(header);
        fclose(fp);
        return NULL;
    }

    return fp;
}

/************************************************************
  Function   : wavfile_read()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/26, create

************************************************************/
int wavfile_read(FILE *fp, short data[], int length)
{
    return fread(data, sizeof(short), length, fp);
}

/************************************************************
  Function   : wavfile_write()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/26, create

************************************************************/
int wavfile_write(FILE *fp, short data[], int length)
{
    return fwrite(data, sizeof(short), length, fp);
}

/************************************************************
  Function   : wavfile_write_close()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/26, create

************************************************************/
void wavfile_write_close(FILE *fp)
{
    int file_length = 0;
    int riff_length = 0;
    int data_length = 0;

    if (fp)
    {
        file_length = ftell(fp);
        data_length = file_length - sizeof(struct wavfile_header);
        riff_length = file_length - 8;
        fseek(fp, sizeof(struct wavfile_header) - sizeof(int), SEEK_SET);
        fwrite(&data_length, sizeof(data_length), 1, fp);
        fseek(fp, 4, SEEK_SET);
        fwrite(&riff_length, sizeof(riff_length), 1, fp);
        fclose(fp);
    }
}

/************************************************************
  Function   : wavfile_read_pos()

  Description: read the data from specified postion
               and channel
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/26, create

************************************************************/
static short wavfile_read_pos(FILE *fp, int channel_num, int channel_index, int pos)
{
    short value = 0;
    int size = 0;
    int index = sizeof(struct wavfile_header) + (pos * channel_num + channel_index) * sizeof(short);
    fseek(fp, index, SEEK_SET);
    size = fread(&value, sizeof(short), 1, fp);

    if (size != sizeof(short))
    {
        printf("Read fail.\n");
    }

    return value;
}

/************************************************************
  Function   : wavfile_read_channel()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/26, create

************************************************************/
void wavfile_read_channel(FILE *fp, short data[], int channel_num, int channel_index, int start_pos, int length)
{
    int i = 0;

    for (; i < length; ++i)
    {
        data[i] = wavfile_read_pos(fp, channel_num, channel_index, start_pos + i);
    }
}

