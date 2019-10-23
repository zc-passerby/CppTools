//
// Created by Administrator on 2019/8/23.
//

#include "waveformat.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void write_intel_ushort(int in, FILE *fp) {
    unsigned char szTemp[SIZE_SHORT];
    szTemp[0] = in & 0xff;
    szTemp[1] = in >> 8;
    fwrite(szTemp, sizeof(*szTemp), SIZE_SHORT, fp);
}

void write_intel_uint(unsigned int in, FILE *fp) {
    unsigned char szTemp[SIZE_INT];
    szTemp[0] = in & 0xff;
    in >>= 8;
    szTemp[1] = in & 0xff;
    in >>= 8;
    szTemp[2] = in & 0xff;
    in >>= 8;
    szTemp[3] = in & 0xff;
    fwrite(szTemp, sizeof(*szTemp), SIZE_INT, fp);
}

unsigned short read_intel_ushort(FILE *fp) {
    unsigned short usCx;
    unsigned char szTemp[SIZE_SHORT];
    fread(szTemp, sizeof(*szTemp), SIZE_SHORT, fp);
    usCx = szTemp[0] | (szTemp[1] * 256);
    return usCx;
}

unsigned int read_intel_uint(FILE *fp) {
    unsigned int ulCx;
    unsigned char szTemp[SIZE_INT];
    fread(szTemp, sizeof(*szTemp), SIZE_INT, fp);
    ulCx = (unsigned int) szTemp[0];
    ulCx |= (unsigned int) szTemp[1] << 8;
    ulCx |= (unsigned int) szTemp[2] << 16;
    ulCx |= (unsigned int) szTemp[3] << 24;
    return ulCx;
}

waveFormat readWaveHeader(FILE *fp) {
    size_t nReadCount;  // fread函数读取的数据长度
    unsigned int unFmtSize; // "fmt " Chunk数据长度
    unsigned short usAudioFormat; // wav 文件格式
    unsigned int unSkipSize;  // 需要跳过的长度
    unsigned int unChunkSize;  // 文件长度(WAVE文件的大小, 不含前8个字节)
    unsigned int unSampleRate; // 采样率(每秒样本数), 表示每个通道的播放速度
    unsigned int unBytesPerSec; // 波形音频数据传输速率, 其值为:通道数*每秒数据位数*每样本的数据位数/8
    unsigned short usChannels; // 通道数(单声道为1, 双声道为2)
    unsigned short usBlockAlign; // 每样本的数据位数(按字节算), 其值为:通道数*每样本的数据位值/8
    unsigned short usBitsPerSample; // 每样本的数据位数, 表示每个声道中各个样本的数据位数
    unsigned int unDataSize; // 真实的语音数据长度
    unsigned char szTemp[SIZE_ID];    // 用于存放读取到的wav头中ID的值

    waveFormat fmt;

    // 读取RIFF头，用于判断是否为wav文件
    nReadCount = fread(szTemp, sizeof(*szTemp), SIZE_ID, fp);
    if (nReadCount == 0) { // 读取失败或读到文件尾
        fprintf(stderr, "read file failed!\n");
        fmt.parse_result = WAVE_PARSE_RESULT_READ_FILE_FAILED;
        return fmt;
    }
    if (memcmp(szTemp, "RIFF", (size_t) SIZE_ID) != 0) { // 文件没有RIFF头
        fprintf(stderr, "File is not WAVE format!\\n");
        fmt.parse_result = WAVE_PARSE_RESULT_NOT_WAVE_FILE;
        return fmt;
    }

    // 读取文件长度(WAVE文件的大小, 不含前8个字节)
    unChunkSize = read_intel_uint(fp);

    // 读取WAVE标志，用于判断是否为wav文件
    nReadCount = fread(szTemp, sizeof(*szTemp), SIZE_ID, fp);
    if (nReadCount == 0) { // 读取失败或读到文件尾
        fprintf(stderr, "read file failed!\n");
        fmt.parse_result = WAVE_PARSE_RESULT_READ_FILE_FAILED;
        return fmt;
    }
    if (memcmp(szTemp, "WAVE", (size_t) SIZE_ID) != 0) { // 文件没有WAVE标志
        fprintf(stderr, "File is not WAVE format!\\n");
        fmt.parse_result = WAVE_PARSE_RESULT_NOT_WAVE_FILE;
        return fmt;
    }

    // 读取后续所有Chunk, 需要处理特殊的LIST和fact块, 读到"data"为止, 并将fp指向到真正的语音数据位置
    while (1) {
        // 先读取ChunkId
        nReadCount = fread(szTemp, sizeof(*szTemp), SIZE_ID, fp);
        if (nReadCount == 0) { // 读取失败或读到文件尾
            fprintf(stderr, "read file failed!\n");
            fmt.parse_result = WAVE_PARSE_RESULT_READ_FILE_FAILED;
            return fmt;
        }
        // 判断ChunkId, 只处理"fmt "和"data", 其余Chunk则跳过
        if (memcmp(szTemp, "LIST", (size_t) SIZE_ID) == 0 ||
            memcmp(szTemp, "fact", (size_t) SIZE_ID) == 0) { // 读到"LIST"、"fact"标志，跳过这部分数据
            // 读取"LIST"、"fact" Chunk的数据长度跳过它
            unSkipSize = read_intel_uint(fp);
            if (unSkipSize != 0) // 长度不为0, 跳过
                fseek(fp, unSkipSize, SEEK_CUR);
        } else if (memcmp(szTemp, "fmt ", (size_t) SIZE_ID) == 0) { // 读到"fmt "标志，进行解析处理
            // "fmt " Chunk长度
            unFmtSize = read_intel_uint(fp);
            // wav文件格式
            usAudioFormat = read_intel_ushort(fp);
            if (WAVE_FORMAT_PCM2 != usAudioFormat) {
                fprintf(stderr, "Error! Unsupported WAVE File format.\n");
                fmt.parse_result = WAVE_PARSE_RESULT_NOT_SUPPORT_FORMAT;
                return fmt;
            }
            unFmtSize -= SIZE_SHORT;
            // 读取通道数
            usChannels = read_intel_ushort(fp);
            unFmtSize -= SIZE_SHORT;
            // 读取采样率
            unSampleRate = read_intel_uint(fp);
            unFmtSize -= SIZE_INT;
            // 读取其他数据
            unBytesPerSec = read_intel_uint(fp);
            usBlockAlign = read_intel_ushort(fp);
            usBitsPerSample = read_intel_ushort(fp);
            unFmtSize -= SIZE_INT + SIZE_SHORT + SIZE_SHORT;
            if (unFmtSize != 0)
                fseek(fp, unFmtSize, SEEK_CUR);
        } else if (memcmp(szTemp, "data", (size_t) SIZE_ID) == 0) { // 读到"data"标志
            unDataSize = read_intel_uint(fp);
            break;
        } else { // 读到其他标志，跳过
            unSkipSize = read_intel_uint(fp);
            if (unSkipSize != 0)
                fseek(fp, unSkipSize, SEEK_CUR);
        }
    }
    fmt.audio_format = usAudioFormat;
    fmt.channels = usChannels;
    fmt.sample_rate = unSampleRate;
    fmt.bits_per_sample = usBitsPerSample;
    fmt.block_align = usBlockAlign;
    fmt.data_size = unDataSize;
    return fmt;
}

void writeWaveHeader(waveFormat fmt, FILE *fp) {
    unsigned int unFmtSize = SIZE_SHORT * 2 + SIZE_INT * 2 + SIZE_SHORT * 2;

    fseek(fp, 0, SEEK_SET);
    fwrite("RIFF", sizeof(char), SIZE_ID, fp);
    write_intel_uint(fmt.data_size * fmt.block_align + 36, fp);
    fwrite("WAVE", sizeof(char), SIZE_ID, fp);
    fwrite("fmt ", sizeof(char), SIZE_ID, fp);
    write_intel_uint(unFmtSize, fp);
    write_intel_ushort(fmt.audio_format, fp);
    write_intel_ushort(fmt.channels, fp);
    write_intel_uint(fmt.sample_rate, fp);
    write_intel_uint(fmt.channels * fmt.sample_rate * fmt.bits_per_sample / BITS_PER_BYTE, fp);
    write_intel_ushort(fmt.block_align, fp);
    write_intel_ushort(fmt.bits_per_sample, fp);
    fwrite("data", sizeof(char), SIZE_ID, fp);
    write_intel_uint(fmt.data_size, fp);
}