//
//  FftPostprocessor.cpp
//  sfml_neu
//
//  Created by Sebastian Sinn on 10.04.20.
//  Copyright © 2020 Sebastian Sinn. All rights reserved.
//

#include "FftPostprocessor.h"
#include "analyze.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

// I LOVE YOU TWO
float FftPostprocessor::xToFreq(float x, float minFreq, float maxFreq, int logspacing) { // x[0,1] -> freq[minFreq,maxFreq]
    float freq;
    if (logspacing == 0) {
        freq = minFreq + (maxFreq - minFreq) * x;
    } else {
        freq = minFreq * pow(maxFreq / minFreq, x);
    }
    return freq;
}
float FftPostprocessor::freqToX(float freq, float minFreq, float maxFreq, int logspacing) { // freq[minFreq,maxFreq] -> x[0,1]
    float x;
    if (logspacing == 0) {
        x = (freq - minFreq) / (maxFreq - minFreq);
    } else {
        x = log(freq / minFreq) / log(maxFreq / minFreq);
    }
    return x;
}
static float noteToFreq(float note) {
    return 440.0 * pow(2, (note - 69.0) / 12.0);
}

static void blur(float* work, const float* input, float* blurred, int count, int passes) {
    memcpy(work, input, count * sizeof(float));

    if (passes > 0) {
        for (int p = 0; p < passes; p++) {
            for (int i = 0; i < count; i++) {
                float right = work[(i + count + 1) % count];
                float left = work[(i + count - 1) % count];
                blurred[i] = (work[i] * 2 + right + left) / 4;
            }
            memcpy(work, blurred, count * sizeof(float));
        }
    } else {
        memcpy(blurred, input, count * sizeof(float));
    }
}

static void smooth(const float* input, int count, float* smooth, float alphaUp, float alphaDn) {
    for (int i = 0; i < count; i++) {
        float inputValue = input[i];

        //        if (!isfinite(inputValue) || inputValue < minDbClamp) inputValue = minDbClamp;

        if (inputValue > smooth[i])
            smooth[i] = inputValue * alphaUp + smooth[i] * (1 - alphaUp);
        else
            smooth[i] = inputValue * alphaDn + smooth[i] * (1 - alphaDn);

        if (smooth[i] < 0.00001) smooth[i] = 0; // avoid denormalized floats (<-100dB)
    }
}

static void decay(const float* input, int count, float* decayed, float decay) {
    for (int i = 0; i < count; i++) {
        if (decay < 0 || input[i] > (decayed[i] - decay))
            decayed[i] = input[i];
        else
            decayed[i] -= decay;
    }
}

static void bin(const float* input, int inputSize, float* output, int outputSize, float samplerate, float minFreq, float maxFreq, int logspacing) {
    float bw = float(samplerate / 2) / (inputSize - 1);

    for (int i = 0; i < outputSize; i++) {
        float freqLeft = FftPostprocessor::xToFreq(float(i) / outputSize, minFreq, maxFreq, logspacing);
        float freqRight = FftPostprocessor::xToFreq(float(i + 1) / outputSize, minFreq, maxFreq, logspacing);

        float binLeft = freqLeft / bw;
        float binRight = freqRight / bw;
        int binLeftInt = int(roundf(binLeft));
        int binRightInt = int(roundf(binRight));

        int avgcnt = 0;
        float avgVal = 0;
        float maxVal = -200;
        for (int b = binLeftInt; b <= binRightInt; b++) {
            float val = input[b];
            if (val > maxVal) maxVal = val;
            avgVal += val;
            avgcnt++;
        }
        avgVal = avgVal / float(avgcnt);

        //        output[i] = ( avgVal + maxVal ) / 2.0;
        output[i] = avgVal;
    }
}

FftPostprocessor::FftPostprocessor(int samplerate, int inputSize, int bins, float minFreq, float maxFreq, int notebased, int fold) {
    allocate(samplerate, inputSize, bins, minFreq, maxFreq, notebased, fold);
}
FftPostprocessor::~FftPostprocessor() {
}

void FftPostprocessor::allocate(int samplerate, int inputSize, int bins, float minFreq, float maxFreq, int notebased, int fold) {
    this->samplerate = samplerate;
    this->inputSize = inputSize;
    this->fold = fold;

    if (bins <= 0)
        bins = inputSize;

    if (fold)
        outputSize = 12;
    else
        outputSize = bins;

    if (notebased == 0) {
        this->minFreq = minFreq;
        this->maxFreq = maxFreq;
    } else if (notebased == 1) {
        //        float rootNote = minFreq; // 33 a 55hz
        //        float maxNote = rootNote + bins; //ignore maxFreq
        //
        ////        this->minFreq = (noteToFreq(rootNote-1) + noteToFreq(rootNote))/2;
        ////        this->maxFreq = (noteToFreq(maxNote-1) + noteToFreq(maxNote))/2;
        //
        //        bins*=2; //quatertones
        //        outputSize*=2;
        //        this->minFreq = (noteToFreq(rootNote-1) + noteToFreq(rootNote)*3)/4;
        //        this->maxFreq = (noteToFreq(maxNote-1) + noteToFreq(maxNote)*3)/4;

        //        float rootNote = minFreq;
        //        float maxNote = rootNote + bins -1; //ignore maxFreq
        //        this->minFreq = (noteToFreq(rootNote-1) + noteToFreq(rootNote))/2;
        //        this->maxFreq = (noteToFreq(maxNote+1) + noteToFreq(maxNote))/2;

        float rootNote = minFreq;
        float maxNote = minFreq + bins; // ignore maxFreq
        this->minFreq = noteToFreq(rootNote - 0.5);
        this->maxFreq = noteToFreq(maxNote - 0.5);
        printf("noteToFreq(rootNote) %f, noteToFreq(maxNote) %f\n", noteToFreq(rootNote), noteToFreq(maxNote));
    } else if (notebased == 2) {
        float rootNote = minFreq;
        float maxNote = minFreq + bins; // ignore maxFreq
        this->minFreq = noteToFreq(rootNote - 0.25);
        this->maxFreq = noteToFreq(maxNote - 0.25);

        bins *= 2;
        outputSize *= 2;
        printf("222 noteToFreq(rootNote) %f, noteToFreq(maxNote) %f\n", noteToFreq(rootNote), noteToFreq(maxNote));
    }

    this->bins = bins;
    binned = new float[bins];
    folded = new float[outputSize];
    blurred = new float[outputSize];
    blurredWork = new float[outputSize];
    blurredSmoothed = new float[outputSize];
    blurredSmoothedDecayed = new float[outputSize];
    blurredSmoothedDecayedScaled = new float[outputSize];

    for (int i = 0; i < bins; i++) {
        binned[i] = 0;
    }
    for (int i = 0; i < outputSize; i++) {
        folded[i] = blurred[i] = blurredSmoothed[i] = blurredSmoothedDecayed[i] = blurredSmoothedDecayedScaled[i] = config.smoothing.minDbClamp;
    }
}
void FftPostprocessor::deallocate() {
    delete[] binned;
    delete[] folded;
    delete[] blurred;
    delete[] blurredWork;
    delete[] blurredSmoothed;
    delete[] blurredSmoothedDecayed;
    delete[] blurredSmoothedDecayedScaled;
}

void FftPostprocessor::process(float* input) {

    if (inputSize != outputSize) {
        bin(input, inputSize, binned, bins, samplerate, minFreq, maxFreq, config.binning.logbinning);
    } else {
        memcpy(binned, input, inputSize * sizeof(float));
    }

    if (fold) {
        int avgcnt[24] = {0};
        for (int i = 0; i < outputSize; i++) {
            folded[i] = 0;
        }
        for (int i = 0; i < bins; i++) {
            folded[i % outputSize] += binned[i];
            avgcnt[i % outputSize] += 1;
        }
        for (int i = 0; i < outputSize; i++) {
            folded[i] = folded[i] / avgcnt[i];
        }

        if (config.folding.removeBaselineOffset) {
            arrAddScalar(folded, outputSize, -calcMin(folded, outputSize));
        }

    } else {
        memcpy(folded, binned, outputSize * sizeof(float));
    }

    blur(blurredWork, folded, blurred, outputSize, config.smoothing.blurringPasses);
    smooth(blurred, outputSize, blurredSmoothed, config.smoothing.alphaUp, config.smoothing.alphaDn);
    decay(blurredSmoothed, outputSize, blurredSmoothedDecayed, config.smoothing.decay);

    if (config.scaling.mag2db) {
        for (int i = 0; i < outputSize; i++) {
            blurredSmoothedDecayedScaled[i] = 20 * log10f(blurredSmoothedDecayed[i]);
        }
    } else {
        memcpy(blurredSmoothedDecayedScaled, blurredSmoothedDecayed, outputSize * sizeof(float));
    }
}

float* FftPostprocessor::getOutput() {
    return blurredSmoothedDecayedScaled;
}

size_t FftPostprocessor::getOutputSize() {
    return outputSize;
}
