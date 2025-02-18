#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include <mutex>

class Sample {
public:
    float left;
    float right;
};

class Ringbuffer {

public:
    Ringbuffer(size_t);
    ~Ringbuffer();

    void write(const Sample* samples, size_t count, float gain = 1);
    void writeFromInterleaved16(const int16_t* samples, size_t count, float gain = 1);
    void writeFromInterleavedFloat(const float* samples, size_t count, float gain = 1);
    void writeFromMono16(const int16_t* samples, size_t count, float gain = 1);
    void writeFromMonoFloat(const float* samples, size_t count, float gain = 1);
    void get(Sample* samples, size_t count);
    int getlr(float* l, float* r, size_t count);
    int getlri8(int8_t* l, int8_t* r, size_t count);
    int getlru8(uint8_t* l, uint8_t* r, size_t count);

private:
    int lastHead;
    Sample* buffer;
    size_t bufMaxCnt;
    int head;
    std::mutex mutex;
};

// class StereoRingbuffer {
//
// public:
//	StereoRingbuffer(size_t);
//	~StereoRingbuffer();
//
//	Ringbuffer left;
//	Ringbuffer right;
// };

#endif // __RINGBUFFER_H__