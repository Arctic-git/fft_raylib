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
#include <vector>

float xToFreq(float x, float minFreq, float maxFreq, int logspacing);
float freqToX(float freq, float minFreq, float maxFreq, int logspacing);

class FftPostprocessor {
public:
    FftPostprocessor();
    ~FftPostprocessor();
    void allocate(int samplerate, int inputSize, int bins = 0);
    void deallocate();
    void process(float* input, int inputSize, int bins, int samplerate);
    float* getOutput();
    size_t getOutputSize();

    struct {

        struct {
            float alphaUp = 1;
            float alphaDn = 0.4;
            float decay = -0.005f;
            float minDbClamp = -0;
            int blurringPasses = 0;
        } smoothing;

        struct {
            bool logbinning = true;
            bool avgmode = true;
            float minFreq = 22;
            float maxFreq = 22050;
            int notebased = 0;
            bool fold = false;
        } binning;

        struct {
            int removeBaselineOffset = 0;
        } folding;

        struct {
            int mag2db = 1;
        } scaling;

    } config;

private:
    std::vector<float> binned;
    std::vector<float> folded;
    std::vector<float> blurred;
    std::vector<float> blurredWork;
    std::vector<float> blurredSmoothed;
    std::vector<float> blurredSmoothedDecayed;
    std::vector<float> blurredSmoothedDecayedScaled;

    int bins=-1;
    int inputSize=-1;
    int outputSize;
    int fold;
};

#endif /* FftPostprocessor_hpp */
