/*
A simple sound library for CSE 20211 by Douglas Thain
For course assignments, you should not change this file.
For complete documentation, see:
http://www.nd.edu/~dthain/courses/cse20211/fall2013/wavfile
*/

#ifndef _WAVFILE_H_
#define _WAVFILE_H_
#include<stdio.h>
#define WAVFILE_SAMPLES_PER_SECOND  (16000)

struct wavfile_header
{
    char riff_tag[4];
    int riff_length;
    char wave_tag[4];
    char fmt_tag[4];
    int fmt_length;
    short audio_format;
    short num_channels;
    int sample_rate;
    int byte_rate;
    short block_align;
    short bits_per_sample;
    char data_tag[4];
    int data_length;
};

void wavFileInfoShow(struct wavfile_header *pHead);

/*
    open an audio file with write flag
    and write necessary wav header to it;

    input   :
    filename: string of the file name
    output  :
    the file pointer of the audio file
*/
FILE *wavfile_write_open(const char *filename, int chans, int sample_rate);

/*
    write data to the audio file
    input   :
    file    : file pointer of the FILE
    data    : the array of data that need to be written to the file
    length  : number of short type numbers that need to be written
*/
int wavfile_write(FILE *file, short data[], int length);

/*
    read data from file
    input   :
    file    :
    file    : file pointer of the file
    data    : the array to store data.
    length  : the number of short type numbers that need to be fetched.
    output
    the number of bytes that the function fetched fromt the file.
*/
int wavfile_read(FILE *file, short data[], int length);

/*
    close the audio file and write the length to the header
*/
void wavfile_write_close(FILE *file);

/*
    open an audio file for reading and get the header from file
*/
FILE *wavfile_read_open(const char *filename , struct wavfile_header *header);

/*
    read the data of an channel with the specified postion and length
*/
void wavfile_read_channel(FILE *file, short data[], int channel_num, int channel_index, int start_pos, int length);

#endif
