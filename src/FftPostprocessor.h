//
//  FftPostprocessor.hpp
//  sfml_neu
//
//  Created by Sebastian Sinn on 10.04.20.
//  Copyright © 2020 Sebastian Sinn. All rights reserved.
//

#ifndef FftPostprocessor_h
#define FftPostprocessor_h

#include <stddef.h>

class FftPostprocessor {
public:
    FftPostprocessor(int samplerate, int inputSize, int bins = 0, float minFreq = 22, float maxFreq = 22050, int notebased = 0, int fold = 0);
    ~FftPostprocessor();
    void allocate(int samplerate, int inputSize, int bins = 0, float minFreq = 22, float maxFreq = 22050, int notebased = 0, int fold = 0);
    void deallocate();
    void process(float* input);
    float* getOutput();
    size_t getOutputSize();
    static float xToFreq(float x, float minFreq, float maxFreq, int logspacing);
    static float freqToX(float freq, float minFreq, float maxFreq, int logspacing);

    struct {

        struct {
            float alphaUp = 1;
            float alphaDn = 0.4;
            float minDbClamp = -0;
            float decay = -0.005f;
            int blurringPasses = 0;
        } smoothing;

        struct {
            int logbinning = 1;
        } binning;

        struct {
            int removeBaselineOffset = 0;
        } folding;

        struct {
            int mag2db = 1;
        } scaling;

    } config;

private:
    float* binned;
    float* folded;
    float* blurred;
    float* blurredWork;
    float* blurredSmoothed;
    float* blurredSmoothedDecayed;
    float* blurredSmoothedDecayedScaled;
    int inputSize;
    int bins;
    int outputSize;
    float minFreq;
    float maxFreq;
    int fold;
    int samplerate;
};

#endif /* FftPostprocessor_hpp */
