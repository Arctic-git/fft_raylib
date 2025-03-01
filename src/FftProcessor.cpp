#include "FftProcessor.h"
#include <math.h>

double cheby_poly(int n, double x) {
    double res;
    if (fabs(x) <= 1)
        res = cos(n * acos(x));
    else
        res = cosh(n * acosh(x));
    return res;
}
void cheby_win(float* out, int N, float atten) {
    int nn, i;
    double M, n, sum = 0, max = 0;
    double tg = pow(10, atten / 20); /* 1/r term [2], 10^gamma [2] */
    double x0 = cosh((1.0 / (N - 1)) * acosh(tg));
    double PI = 3.14159265359;
    M = (N - 1) / 2;
    if (N % 2 == 0) M = M + 0.5; /* handle even length windows */
    for (nn = 0; nn < (N / 2 + 1); nn++) {
        n = nn - M;
        sum = 0;
        for (i = 1; i <= M; i++) {
            sum += cheby_poly(N - 1, x0 * cos(PI * i / N)) * cos(2.0 * n * PI * i / N);
        }
        out[nn] = tg + 2 * sum;
        out[N - nn - 1] = out[nn];
        if (out[nn] > max) max = out[nn];
    }
    for (nn = 0; nn < N; nn++)
        out[nn] /= max; /* normalise everything */
}

double bessi0(double x) {
    double ax, ans;
    double y;

    if ((ax = fabs(x)) < 3.75) {
        y = x / 3.75, y = y * y;
        ans = 1.0 + y * (3.5156229 + y * (3.0899424 + y * (1.2067492 + y * (0.2659732 + y * (0.360768e-1 + y * 0.45813e-2)))));
    } else {
        y = 3.75 / ax;
        ans = (exp(ax) / sqrt(ax)) * (0.39894228 + y * (0.1328592e-1 + y * (0.225319e-2 + y * (-0.157565e-2 + y * (0.916281e-2 + y * (-0.2057706e-1 + y * (0.2635537e-1 + y * (-0.1647633e-1 + y * 0.392377e-2))))))));
    }
    return ans;
}
void kaiser_win(float* out, int N, float beta) {
    for (int i = 0; i < N; i++) {
        out[i] = bessi0(beta * sqrt(1. - (2. * i / (N - 1.) - 1.) * (2. * i / (N - 1.) - 1.))) / bessi0(beta);
    }
}

void hann_win(float* out, int N) {
    for (int i = 0; i < N; i++) {
        out[i] = 0.5f * (1 - cos((6.2831853072f * i) / (N - 1))); // Hann
    }
}

void hamming_win(float* out, int N) {
    for (int i = 0; i < N; i++) {
        out[i] = 0.54 - 0.46f * cos((6.2831853072f * i) / (N - 1));
    }
}

void rect_win(float* out, int N) {
    for (int i = 0; i < N; i++) {
        out[i] = 1;
    }
}

FftProcessor::FftProcessor(int N, int paddingFactor) {
    allocate(N, paddingFactor);
    updateWindow(1);
}

FftProcessor::~FftProcessor() {
    deallocate();
}

void FftProcessor::allocate(int N, int paddingFactor) {
    this->NwoPadding = N;
    N *= paddingFactor;
    this->N = N;

    fftWindow = new float[NwoPadding];
    soundWindowed = new float[N]{};
    fftResult = new float[N / 2 + 1];
    fftResultTmp = new float[N / 2 + 1];

    fftCfg = kiss_fftr_alloc(N, 0, NULL, NULL);
    fftCpxData = new kiss_fft_cpx[(N / 2 + 1)];

    slopFactorsSetTo = -1;
    slopeFactors = new float[N / 2 + 1];

    // soundWindowedDebugTmp = new float[NwoPadding];
    // soundWindowedDebug = &soundWindowed[((N - NwoPadding) / 2)];
}
void FftProcessor::deallocate() {
    // delete[] soundWindowedDebugTmp;
    delete[] slopeFactors;
    delete[] fftCpxData;
    kiss_fftr_free(fftCfg);
    delete[] fftResultTmp;
    delete[] fftResult;
    delete[] soundWindowed;
    delete[] fftWindow;
    currWindow = -1;
}

const char* FftProcessor::getWindowName() {
    if (currWindow == 0) {
        return "rect";
    } else if (currWindow == 1) {
        return "hamming";
    } else if (currWindow == 2) {
        return "hann";
    } else if (currWindow == 3) {
        return "kaiser b=9";
    } else if (currWindow == 4) {
        return "cheby atten=100";
    }

    return "invalid";
}

void FftProcessor::updateWindow(int windowFunction) {
    if (currWindow == windowFunction)
        return;

    if (windowFunction == 0) {
        rect_win(fftWindow, NwoPadding);
    } else if (windowFunction == 1) {
        hamming_win(fftWindow, NwoPadding);
    } else if (windowFunction == 2) {
        hann_win(fftWindow, NwoPadding);
    } else if (windowFunction == 3) {
        kaiser_win(fftWindow, NwoPadding, 9);
    } else if (windowFunction == 4) {
        cheby_win(fftWindow, NwoPadding, 100);
    }

    fftWindowSum = 0;
    for (int i = 0; i < NwoPadding; i++) {
        fftWindowSum += (fftWindow[i]);
    }

    currWindow = windowFunction;
}

static void precalculateSlope(float* factors, int N, float slope) {
    for (int i = 0; i < N / 2 + 1; i++) {
        // if (slope == 6) { //'6db' slope
        //     factors[i] = (i + 1) / (sqrt(N / 2) + 1);
        // } else if (slope == 3) { //'3db' slope
        //     factors[i] = sqrtf((i + 1) / (sqrtf(N / 2) + 1));
        // } else if (slope == 0) {
        //     factors[i] = 1;
        // } else {                                                        // slow
        factors[i] = powf((i + 1) / (sqrtf(N / 2) + 1), slope / 6);       // sqrt(N/2) = slope in the middle
        factors[i] = factors[i] / (powf((float)N / 2048, slope / 6 / 2)); // normalize amplitude for num bins
        // }
    }
}

void FftProcessor::process(float* input) {
    int numZeros = N - NwoPadding;

    for (int i = 0; i < NwoPadding; i++) {
        soundWindowed[i + numZeros / 2] = fftWindow[i] * input[i];
    }

    kiss_fftr(fftCfg, soundWindowed, fftCpxData);

    // precalculateSlopex
    if (slopFactorsSetTo != config.slope) {
        precalculateSlope(slopeFactors, N, config.slope);
        slopFactorsSetTo = config.slope;
    }

    for (int i = 0; i < N / 2 + 1; i++) {
        // Scale the magnitude of FFT by window and factor of 2, because we are using half of FFT spectrum.
        float mag = sqrtf(fftCpxData[i].r * fftCpxData[i].r + fftCpxData[i].i * fftCpxData[i].i) * 2.0 / fftWindowSum;

        fftResult[i] = mag * slopeFactors[i];
    }

    if (config.dbScale == 1) {
        for (int i = 0; i < N / 2 + 1; i++) {
            fftResult[i] = 20 * log10f(fftResult[i]);
        }
    }
}

void FftProcessor::process(float* inputLeft, float* inputRight) {
    process(inputLeft);
    memcpy(fftResultTmp, fftResult, (N / 2 + 1) * sizeof(float));
    // memcpy(soundWindowedDebugTmp, soundWindowedDebug, NwoPadding * sizeof(float));
    process(inputRight);

    for (int i = 0; i < N / 2 + 1; i++) {
        fftResult[i] = (fftResult[i] + fftResultTmp[i]) / 2;
    }
}

float* FftProcessor::getTimeWindowed() {
    return soundWindowed + (N - NwoPadding) / 2;
}
size_t FftProcessor::getTimeSize() {
    return NwoPadding;
}

float* FftProcessor::getOutput() {
    return fftResult;
}
size_t FftProcessor::getOutputSize() {
    return N / 2 + 1;
}
