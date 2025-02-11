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

    void write(const Sample* samples, size_t count);
    void writeFromInterleaved16(const int16_t* samples, size_t count);
    void writeFromInterleavedFloat(const float* samples, size_t count);
    void writeFromMono16(const int16_t* samples, size_t count);
    void writeFromMonoFloat(const float* samples, size_t count);
    void get(Sample* samples, size_t count);
    int getlr(float* l, float* r, size_t count);

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