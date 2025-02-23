//
//  FftPostprocessor.cpp
//  sfml_neu
//
//  Created by Sebastian Sinn on 10.04.20.
//  Copyright © 2020 Sebastian Sinn. All rights reserved.
//

#include "FftPostprocessor.h"
#include "analyze.h"
#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <string.h>

// I LOVE YOU TWO
float xToFreq(float x, float minFreq, float maxFreq, int logspacing) { // x[0,1] -> freq[minFreq,maxFreq]
    float freq;
    if (logspacing == 0) {
        freq = minFreq + (maxFreq - minFreq) * x;
    } else {
        freq = minFreq * pow(maxFreq / minFreq, x);
    }
    return freq;
}
float freqToX(float freq, float minFreq, float maxFreq, int logspacing) { // freq[minFreq,maxFreq] -> x[0,1]
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
        float freqLeft = xToFreq(float(i) / outputSize, minFreq, maxFreq, logspacing);
        float freqRight = xToFreq(float(i + 1) / outputSize, minFreq, maxFreq, logspacing);

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

static void bin_lerp(const float* input, int inputSize, float* output, int outputSize, float samplerate, float minFreq, float maxFreq, int logspacing, int db_correct_lerp) {
    float bw = float(samplerate / 2) / (inputSize - 1);
    bool lerp = true;
    bool bin_avgmode = true;

    for (int x = 0; x < outputSize; x++) {

        // try average, if not enough samples lerp
        float x_rel_m1 = (float)(x - 0.5) / (outputSize - 1);
        float x_rel_1 = (float)(x + 0.5) / (outputSize - 1);
        x_rel_m1 = std::clamp(x_rel_m1, 0.0f, 1.0f);
        x_rel_1 = std::clamp(x_rel_1, 0.0f, 1.0f);

        int bin = std::round(xToFreq(x_rel_m1, minFreq, maxFreq, logspacing) / bw);
        int bin_1 = std::round(xToFreq(x_rel_1, minFreq, maxFreq, logspacing) / bw);
        int num_bins = bin_1 - bin + 1;

        float f_interp = 0;
        if (lerp && num_bins <= 2) {
            float x_rel = (float)(x) / (outputSize - 1);
            int bin = std::floor(xToFreq(x_rel, minFreq, maxFreq, logspacing) / bw);
            float x_rel_left = freqToX(bin * bw, minFreq, maxFreq, logspacing);
            float x_rel_right = freqToX((bin + 1) * bw, minFreq, maxFreq, logspacing);
            float f_left = input[bin];
            float f_right = input[bin + 1];

            // lerp
            float a = (x_rel - x_rel_left) / (x_rel_right - x_rel_left);

            if (db_correct_lerp && f_left > 1e-5) { // above -100 dBfs
                // f_interp = powf(10, log10f(f_left) * (1 - a) + log10f(f_right) * a);
                // f_interp = powf(10, log10f(f_left) + log10f(f_right/f_left) * a);
                // f_interp = powf(10, log10f(f_left) + log10f(powf(f_right / f_left, a)));
                // f_interp = powf(10, log10f(f_left*powf(f_right / f_left, a)));
                f_interp = f_left * powf(f_right / f_left, a);
            } else {
                // f_interp = f_left * (1 - a) + f_right * a;
                f_interp = f_left + (f_right - f_left) * a;
            }
        } else {
            float fn_max = input[bin];
            float fn_avg = 0;
            for (int i = 0; i < num_bins; i++) {
                float val = input[bin + i];
                if (val > fn_max)
                    fn_max = val;
                fn_avg += val / num_bins;
            }
            f_interp = fn_max;
            if (bin_avgmode)
                f_interp = fn_avg;
        }

        output[x] = f_interp;
    }
}
FftPostprocessor::FftPostprocessor() {
}
FftPostprocessor::~FftPostprocessor() {
}

void FftPostprocessor::allocate(int samplerate, int inputSize, int bins) {

    if (bins <= 0)
        bins = inputSize;

    outputSize = bins;

    if (config.binning.notebased && fold)
        outputSize = 12;

    if (config.binning.notebased == 1) {
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

        float rootNote = config.binning.minFreq;
        float maxNote = config.binning.minFreq + bins; // ignore maxFreq
        config.binning.minFreq = noteToFreq(rootNote - 0.5);
        config.binning.maxFreq = noteToFreq(maxNote - 0.5);
        printf("noteToFreq(rootNote) %f, noteToFreq(maxNote) %f\n", noteToFreq(rootNote), noteToFreq(maxNote));
    } else if (config.binning.notebased == 2) {
        float rootNote = config.binning.minFreq;
        float maxNote = config.binning.minFreq + bins; // ignore maxFreq
        config.binning.minFreq = noteToFreq(rootNote - 0.25);
        config.binning.maxFreq = noteToFreq(maxNote - 0.25);

        bins *= 2;
        outputSize *= 2;
        printf("222 noteToFreq(rootNote) %f, noteToFreq(maxNote) %f\n", noteToFreq(rootNote), noteToFreq(maxNote));
    }

    if (this->inputSize != inputSize || binned.size() != bins || folded.size() != outputSize) {

        binned.resize(bins);
        folded.resize(outputSize);
        blurred.resize(outputSize);
        blurredWork.resize(outputSize);
        blurredSmoothed.resize(outputSize);
        blurredSmoothedDecayed.resize(outputSize);
        blurredSmoothedDecayedScaled.resize(outputSize);

        for (int i = 0; i < bins; i++) {
            binned[i] = 0;
        }
        for (int i = 0; i < outputSize; i++) {
            folded[i] = blurred[i] = blurredSmoothed[i] = blurredSmoothedDecayed[i] = blurredSmoothedDecayedScaled[i] = config.smoothing.minDbClamp;
        }
    }

    this->bins = bins;
    this->inputSize = inputSize;
}

void FftPostprocessor::process(float* input, int inputSize, int bins, int samplerate) {
    allocate(samplerate, inputSize, bins);
    // printf("%d %d %d\n", inputSize, bins, outputSize);
    // printf("%f\n", input[0]);

    if (inputSize != outputSize) {
        // bin(input, inputSize, binned.data(), bins, samplerate, config.binning.minFreq, config.binning.maxFreq, config.binning.logbinning);
        bin_lerp(input, inputSize, binned.data(), binned.size(), samplerate, config.binning.minFreq, config.binning.maxFreq, config.binning.logbinning, config.scaling.mag2db);
    } else {
        memcpy(binned.data(), input, inputSize * sizeof(float));
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
            arrAddScalar(folded.data(), folded.size(), -calcMin(folded.data(), folded.size()));
        }

    } else {
        memcpy(folded.data(), binned.data(), outputSize * sizeof(float));
    }

    blur(blurredWork.data(), folded.data(), blurred.data(), outputSize, config.smoothing.blurringPasses);
    smooth(blurred.data(), outputSize, blurredSmoothed.data(), config.smoothing.alphaUp, config.smoothing.alphaDn);
    decay(blurredSmoothed.data(), outputSize, blurredSmoothedDecayed.data(), config.smoothing.decay);

    if (config.scaling.mag2db) {
        for (int i = 0; i < outputSize; i++) {
            blurredSmoothedDecayedScaled[i] = 20 * log10f(blurredSmoothedDecayed[i]);
        }
    } else {
        memcpy(blurredSmoothedDecayedScaled.data(), blurredSmoothedDecayed.data(), outputSize * sizeof(float));
    }
}

float* FftPostprocessor::getOutput() {
    return blurredSmoothedDecayedScaled.data();
}

size_t FftPostprocessor::getOutputSize() {
    return outputSize;
}
