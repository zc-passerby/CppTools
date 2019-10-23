#include "waveformat.h"

#include <stdio.h>

const int g_nFrameLen = 20; // 20ms

int main(int argc, char *argv[]) {
    char *szFileName = NULL;
    FILE *fp = NULL;
    int nBytesPerFrame;
    int nExtraLen = 0;
    int nFrameCount;
    char *szWavBuffer = NULL;
    int nSplit

    if (argc != 2)
    {
        printf("Usage: wavFile\n");
        return -1;
    }

    szFileName = argv[1];
    fp = open(szFileName, "rb");
    waveFormat fmt = readWaveHeader(fp);
    nBytesPerFrame = fmt.sample_rate * g_nFrameLen * fmt.channels * fmt.bits_per_sample / 8;
    if (fmt.data_size % nBytesPerFrame > 0)
        nExtraLen = 1;
    szWavBuffer = (char *)malloc(nBytesPerFrame * sizeof(char));
    memset(szWavBuffer, 0, nBytesPerFrame * sizeof(char));
    nFrameCount = fmt.data_size / nBytesPerFrame + nExtraLen;

    while (fread(szWavBuffer, sizeof(char), nBytesPerFrame, fp) == nBytesPerFrame) {
        ;
    }
}