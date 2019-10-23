//
// Created by Passerby on 2019/8/23.
//

#ifndef CPPTOOLS_WAVEFORMAT_H
#define CPPTOOLS_WAVEFORMAT_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct waveFormat {
    unsigned int audio_format;
    unsigned int sample_rate;
    unsigned short channels;    // 通道数
    unsigned short bits_per_sample;
    unsigned short block_align;
    unsigned int data_size;

    int parse_result; // 读取文件结果
} waveFormat;

#define WAVE_FORMAT_UNKNOWN             (0x0000)
#define WAVE_FORMAT_PCM2                (0x0001)
#define WAVE_FORMAT_ADPCM               (0x0002)
#define WAVE_FORMAT_ALAW                (0x0006)
#define WAVE_FORMAT_MULAW               (0x0007)
#define WAVE_FORMAT_OKI_ADPCM           (0x0010)
#define WAVE_FORMAT_IMA_ADPCM           (0x0011)
#define WAVE_FORMAT_DIGISTD             (0x0015)
#define WAVE_FORMAT_DIGIFIX             (0x0016)
#define IBM_FORMAT_MULAW                (0x0101)
#define IBM_FORMAT_ALAW                 (0x0102)
#define IBM_FORMAT_ADPCM                (0x0103)

#define SIZE_ID         4
#define SIZE_INT        4
#define SIZE_SHORT      2
#define BITS_PER_BYTE   8

#define WAVE_PARSE_RESULT_SUCCESS                   0
#define WAVE_PARSE_RESULT_READ_FILE_FAILED          1
#define WAVE_PARSE_RESULT_NOT_WAVE_FILE             2
#define WAVE_PARSE_RESULT_NOT_SUPPORT_FORMAT        3

waveFormat readWaveHeader(FILE *fp);

void writeWaveHeader(waveFormat fmt, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif //CPPTOOLS_WAVEFORMAT_H
