#ifndef __AUDIOSOURCEPA_H__
#define __AUDIOSOURCEPA_H__

#include "Ringbuffer.h"
#include "portaudio.h"
#include <vector>

typedef struct paAudioConfig_s {
    bool enableLoopback = 0;
    float gainLoopback = 0.8;
} paAudioConfig_t;

typedef struct {
    Ringbuffer* ringbuffer;
    // sf::Clock* fpsCounterClock;
    // sf::Time* lastTime;
    size_t* callbackCnt;
    paAudioConfig_t* config;
    int channelsIn;
} paUserData_t;

class AudioSourcePA {
public:
    AudioSourcePA(Ringbuffer* ringbuffer, int samplerate);
    ~AudioSourcePA();
    float getFps();
    const char* getInfo();
    int scanDevices();
    int openDevice(int input, int output, int samplerate);
    int openDevice(const char* nameInput, const char* nameOutput, int samplerate);
    std::vector<std::string> getDeviceNames();
    std::vector<std::string> getInDeviceNames();
    std::vector<std::string> getOutDeviceNames();
    void getCurrentDevice(int* input, int* output);

    paAudioConfig_t config;

private:
    PaStream* stream;
    // sf::Clock fpsCounterClock;
    // sf::Time lastTime;
    std::string infoStr;
    size_t callbackCnt;
    paUserData_t userData;
    std::vector<std::string> availableInDevices;
    std::vector<std::string> availableOutDevices;
    std::vector<std::string> availableDevices;
    int currInput;
    int currOutput;
};

#endif // __AUDIOSOURCEPA_H__