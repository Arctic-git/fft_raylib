#ifndef __FFTPROCESSOR_H__
#define __FFTPROCESSOR_H__

#include "kiss_fftr.h"

class FftProcessor {
public:
    FftProcessor(int N, int paddingFactor = 1);
    ~FftProcessor();

    void allocate(int N, int paddingFactor = 1);
    void deallocate();

    void updateWindow(int windowFunction);
    void process(float* input);
    void process(float* inputLeft, float* inputRight);

    const char* getWindowName();
    float* getTimeWindowed();
    size_t getTimeSize();
    float* getOutput();
    size_t getOutputSize();

    struct {

        float slope = 3;
        int dbScale = 0;

    } config;

    float* soundWindowedDebug;
    float* soundWindowedDebugTmp;

private:
    int N;
    int NwoPadding;
    int currWindow = -1;

    float* fftWindow;
    float fftWindowSum;
    float* soundWindowed;
    float* fftResult;
    float* fftResultTmp;

    kiss_fftr_cfg fftCfg;
    kiss_fft_cpx* fftCpxData;
    float* slopeFactors;
    int slopFactorsSetTo;
};

#endif // __FFTPROCESSOR_H__