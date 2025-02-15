#include "Ringbuffer.h"
#include <stdio.h>

Ringbuffer::Ringbuffer(size_t cnt) {
    bufMaxCnt = cnt;
    buffer = new Sample[cnt];
    for (int i = 0; i < cnt; i++) {
        buffer[i].left = 0;
        buffer[i].right = 0;
    }
    lastHead = head = 0;
}

Ringbuffer::~Ringbuffer() {
    delete[] buffer;
}

void Ringbuffer::write(const Sample* samples, size_t count) {
    mutex.lock();

    if (count > bufMaxCnt) {
        int tooMany = count - bufMaxCnt;
        samples += tooMany;
        count -= tooMany;
    }

    for (int i = 0; i < count; i++) {
        buffer[head] = samples[i];
        head = (head + 1) % bufMaxCnt;
    }

    mutex.unlock();
}

// count = number of STEREO samples
void Ringbuffer::writeFromInterleaved16(const int16_t* samples, size_t count) {
    mutex.lock();

    for (int i = 0; i < count; i++) {
        buffer[head].left = float(samples[i * 2]) / 32768.0;
        buffer[head].right = float(samples[i * 2 + 1]) / 32768.0;
        head = (head + 1) % bufMaxCnt;
    }

    mutex.unlock();
}

void Ringbuffer::writeFromInterleavedFloat(const float* samples, size_t count) {
    mutex.lock();

    for (int i = 0; i < count; i++) {
        buffer[head].left = float(samples[i * 2]);
        buffer[head].right = float(samples[i * 2 + 1]);
        head = (head + 1) % bufMaxCnt;
    }

    mutex.unlock();
}

void Ringbuffer::writeFromMono16(const int16_t* samples, size_t count) {
    mutex.lock();

    for (int i = 0; i < count; i++) {
        buffer[head].left = float(samples[i]) / 32768.0;
        buffer[head].right = float(samples[i]) / 32768.0;
        head = (head + 1) % bufMaxCnt;
    }

    mutex.unlock();
}

void Ringbuffer::writeFromMonoFloat(const float* samples, size_t count) {
    mutex.lock();

    for (int i = 0; i < count; i++) {
        buffer[head].left = float(samples[i]);
        buffer[head].right = float(samples[i]);
        head = (head + 1) % bufMaxCnt;
    }

    mutex.unlock();
}

void Ringbuffer::get(Sample* samples, size_t count) {
    mutex.lock();

    if (count > bufMaxCnt) return;

    int readhead = head;
    for (int i = 0; i < count; i++) {
        readhead = (readhead - 1 + bufMaxCnt) % bufMaxCnt;
        samples[i] = buffer[readhead];
    }

    mutex.unlock();
}

int Ringbuffer::getlr(float* l, float* r, size_t count) {
    mutex.lock();

    if (count > bufMaxCnt) count = bufMaxCnt;

    int freshCnt = (head - lastHead + bufMaxCnt) % bufMaxCnt;
    if (freshCnt > count) freshCnt = count;
    lastHead = head;

    int readhead = head;
    for (int i = 0; i < count; i++) {
        readhead = (readhead - 1 + bufMaxCnt) % bufMaxCnt;
        l[i] = buffer[readhead].left;
        r[i] = buffer[readhead].right;
    }

    mutex.unlock();

    return freshCnt;
}

static inline int8_t ftoi8(float in) {
    int in_i = in * 127;
    if (in_i > 127)
        in_i = 127;
    if (in_i < -128)
        in_i = -128;

    return in_i;
}

static inline uint8_t ftou8(float in) {
    int in_i = 128 + in * 127;
    if (in_i > 255)
        in_i = 255;
    if (in_i < 0)
        in_i = 0;

    return in_i;
}

int Ringbuffer::getlri8(int8_t* l, int8_t* r, size_t count) {
    mutex.lock();

    if (count > bufMaxCnt) count = bufMaxCnt;

    int freshCnt = (head - lastHead + bufMaxCnt) % bufMaxCnt;
    if (freshCnt > count) freshCnt = count;
    lastHead = head;

    int readhead = head;
    for (int i = 0; i < count; i++) {
        readhead = (readhead - 1 + bufMaxCnt) % bufMaxCnt;
        l[i] = ftoi8(buffer[readhead].left);
        r[i] = ftoi8(buffer[readhead].right);
    }

    mutex.unlock();

    return freshCnt;
}

int Ringbuffer::getlru8(uint8_t* l, uint8_t* r, size_t count) {
    mutex.lock();

    if (count > bufMaxCnt) count = bufMaxCnt;

    int freshCnt = (head - lastHead + bufMaxCnt) % bufMaxCnt;
    if (freshCnt > count) freshCnt = count;
    lastHead = head;

    int readhead = head;
    for (int i = 0; i < count; i++) {
        readhead = (readhead - 1 + bufMaxCnt) % bufMaxCnt;
        l[i] = ftou8(buffer[readhead].left);
        r[i] = ftou8(buffer[readhead].right);
    }

    mutex.unlock();

    return freshCnt;
}

// Ringbuffer::Ringbuffer(size_t len){
//	buflen = len;
//	buffer = new float[buflen];
//	head = 0;
// }
//
//
// Ringbuffer::~Ringbuffer(){
//	delete[] buffer;
// }
//
//
// void Ringbuffer::write(const float* samples, size_t count){
//	mutex.lock();
//
//	if (count > buflen) {
//		int tooMany = count - buflen;
//		samples += tooMany;
//		count -= tooMany;
//	}
//
//	for (int i = 0; i < count; i++) {
//		buffer[head] = samples[i];
//		head = (head + 1) % buflen;
//	}
//
//	mutex.unlock();
// }
//
//
//
// void Ringbuffer::get(float* samples, size_t count) {
//	mutex.lock();
//
//	if (count > buflen) return;
//
//	int readhead = head;
//	for (int i = 0; i < count; i++) {
//		readhead = (readhead - 1 + buflen) % buflen;
//		samples[i] = buffer[readhead];
//	}
//
//	mutex.unlock();
// }
//
//
// size_t Ringbuffer::getLen() {
//	return buflen;
// }
