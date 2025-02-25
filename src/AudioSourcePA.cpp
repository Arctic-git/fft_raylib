#include "AudioSourcePA.h"

#define PA_SAMPLE_TYPE paFloat32

static int soundCallback(const void* inputBuffer, void* outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void* userData) {
    float* out = (float*)outputBuffer;
    const float* in = (const float*)inputBuffer;
    unsigned int i;
    paUserData_t* userdata = (paUserData_t*)userData;
    (void)timeInfo; /* Prevent unused variable warnings. */
    (void)statusFlags;

    // *userdata->lastTime = *userdata->lastTime*0.9f + 0.1f*userdata->fpsCounterClock->restart();
    (*userdata->callbackCnt)++;

    if (inputBuffer == NULL) {
        for (i = 0; i < framesPerBuffer; i++) {
            *out++ = 0; /* left - silent */
            *out++ = 0; /* right - silent */
        }
    } else {
        if (userdata->channelsIn == 2)
            userdata->ringbuffer->writeFromInterleavedFloat(in, framesPerBuffer, userdata->config->gain);
        else
            userdata->ringbuffer->writeFromMonoFloat(in, framesPerBuffer, userdata->config->gain);

        if (userdata->config->enableLoopback) {
            //			memcpy(out, in, framesPerBuffer * sizeof(float) * 2);
            for (i = 0; i < framesPerBuffer; i++) {
                *out++ = (*in++) * userdata->config->gainLoopback;
                if (userdata->channelsIn == 2)
                    *out++ = (*in++) * userdata->config->gainLoopback;
                else
                    *out++ = *out;
            }
        } else {
            for (i = 0; i < framesPerBuffer; i++) {
                *out++ = 0; /* left - silent */
                *out++ = 0; /* right - silent */
            }
        }
    }

    return paContinue;
}

AudioSourcePA::AudioSourcePA(Ringbuffer* ringbuffer, int samplerate) {
    stream = nullptr;

    PaError err;
    userData.ringbuffer = ringbuffer;
    // userData.fpsCounterClock = &fpsCounterClock;
    // userData.lastTime = &lastTime;
    userData.callbackCnt = &callbackCnt;
    userData.config = &config;

    err = Pa_Initialize();
    if (err != paNoError) goto error;

    scanDevices();
    err = openDevice(-1, -1, samplerate);
    if (err != paNoError) goto error;

    return;

error:
    // Pa_Terminate();
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));

    return;
}

AudioSourcePA::~AudioSourcePA() {
    PaError err = Pa_CloseStream(stream);
    if (err != paNoError) goto error;

    Pa_Terminate();
    return;

error:
    Pa_Terminate();
    fprintf(stderr, "An error occured while using the portaudio stream (Pa_CloseStream)\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    return;
}

int AudioSourcePA::openDevice(int input, int output, int samplerate) {
    PaStreamParameters inputParameters, outputParameters;

    if (Pa_IsStreamActive(stream)) Pa_CloseStream(stream);
    infoStr = "closed";
    currInput = currOutput = -1;

    if (input == -1) input = Pa_GetDefaultInputDevice();
    if (output == -1) output = Pa_GetDefaultOutputDevice();
    currInput = input;
    currOutput = output;

    inputParameters.device = input;
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default input device.\n");
        return -1;
    }
    inputParameters.channelCount = 2; /* stereo input */
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    outputParameters.device = output; /* default output device */
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default output device.\n");
        return -1;
    }
    outputParameters.channelCount = 2; /* stereo output */
    outputParameters.sampleFormat = PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    userData.channelsIn = inputParameters.channelCount;
    PaError err = Pa_OpenStream(
        &stream,
        &inputParameters,
        &outputParameters,
        samplerate,
        paFramesPerBufferUnspecified, // default for best latency
        0, /* paClipOff, */           /* we won't output out of range samples so don't bother clipping them */
        soundCallback,
        &userData);

    if (err == -9998) {
        fprintf(stderr, "Error during Pa_OpenStream retrying mono\n");

        inputParameters.channelCount = 1;
        userData.channelsIn = inputParameters.channelCount;
        err = Pa_OpenStream(
            &stream,
            &inputParameters,
            &outputParameters,
            samplerate,
            paFramesPerBufferUnspecified, // default for best latency
            0, /* paClipOff, */           /* we won't output out of range samples so don't bother clipping them */
            soundCallback,
            &userData);
    }

    if (err != paNoError) {
        fprintf(stderr, "Error during Pa_OpenStream %d %s\n", err, Pa_GetErrorText(err));
        return err;
    }

    const PaStreamInfo *streamInfo = Pa_GetStreamInfo(stream);
    if(streamInfo){
        printf("Input %.2f %.2f seconds\n", inputParameters.suggestedLatency, streamInfo->inputLatency);
        printf("Output %.2f %.2f seconds\n", outputParameters.suggestedLatency, streamInfo->outputLatency);
        printf("Sample Rate: %f Hz\n", streamInfo->sampleRate);
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "Error during Pa_StartStream\n");
        return err;
    }

    infoStr = "in: " + std::string(Pa_GetDeviceInfo(inputParameters.device)->name) + "  out: " + Pa_GetDeviceInfo(outputParameters.device)->name + "\n" + std::to_string(streamInfo->sampleRate) + " "+ std::to_string(streamInfo->inputLatency) + " " + std::to_string(streamInfo->outputLatency);

    printf("opened %s\n", infoStr.c_str());

    return 0;
}

int AudioSourcePA::openDevice(const char* nameInput, const char* nameOutput, int samplerate) {
    int input = -1, output = -1;

    int num = 0;
    for (std::string name : availableDeviceNames) {
        if (name == std::string(nameInput)) {
            input = num;
        }
        if (name == std::string(nameOutput)) {
            output = num;
        }
    }
    return openDevice(input, output, samplerate);
}

float AudioSourcePA::getFps() {
    // return 1.0 / lastTime.asSeconds();
    return 1;
}

const char* AudioSourcePA::getInfo() {
    return infoStr.c_str();
}

int AudioSourcePA::scanDevices() {
    Pa_Terminate();
    Pa_Initialize();
    infoStr = "closed";

    availableDeviceNames.clear();
    availableDevices.clear();

    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        printf("ERROR: Pa_GetDeviceCount returned 0x%x\n", numDevices);
        return -1;
    }

    printf("Number of devices = %d\n", numDevices);
    for (int i = 0; i < numDevices; i++) {
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
        std::string name = "#" + std::to_string(i) + " " + std::to_string(deviceInfo->maxInputChannels) + "/" + std::to_string(deviceInfo->maxOutputChannels) + " " + std::string(deviceInfo->name);
        printf("%s, in %d, out %d ", name.c_str(), deviceInfo->maxInputChannels, deviceInfo->maxOutputChannels);
        if (i == Pa_GetDefaultOutputDevice()) printf("Default Output ");
        if (i == Pa_GetDefaultInputDevice()) printf("Default Input ");
        printf("\n");

        availableDeviceNames.push_back(name);
        availableDevices.push_back({name, deviceInfo->maxInputChannels, deviceInfo->maxOutputChannels});
    }

    return 0;
}

std::vector<std::string> AudioSourcePA::getDeviceNames() {
    return availableDeviceNames;
}

std::vector<device_t> AudioSourcePA::getDevices() {
    return availableDevices;
}


void AudioSourcePA::getCurrentDevice(int* input, int* output) {
    *input = currInput;
    *output = currOutput;
}
