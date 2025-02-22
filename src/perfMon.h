#ifndef __PERFMON_H__
#define __PERFMON_H__

#include <chrono>
#include <map>
#include <mutex>
#include <stdint.h>
#include <string>
#include <string_view>
#include <vector>

typedef struct loopMeasurement_s {
    bool valid;
    float min_s;
    float max_s;
    float accum_s;
    int samples;
    float avg_s;
} loopMeasurement_t;

class PerfMon {

    static std::map<std::string, loopMeasurement_t> measurements;
    static std::mutex mtx;

    loopMeasurement_t* measurement;
    std::string name;
    std::chrono::time_point<std::chrono::high_resolution_clock> last;
    bool lastValid = false;

public:
    PerfMon(std::string name);
    float sample();
    void sample_begin();
    float sample_end();

    static loopMeasurement_t get(std::string name);
    static std::map<std::string, loopMeasurement_t> getAll();
    static void print();
};

#endif // __PERFMON_H__