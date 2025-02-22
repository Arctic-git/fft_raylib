#include "perfMon.h"
#include <algorithm>
#include <math.h>

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::seconds;

std::map<std::string, loopMeasurement_t> PerfMon::measurements;
std::mutex PerfMon::mtx;

PerfMon::PerfMon(std::string name) : name(name) {
    std::lock_guard<std::mutex> lck(mtx);
    measurements[name.c_str()] = {};
    measurement = &measurements[name.c_str()];
}

float PerfMon::sample() {
    auto now = high_resolution_clock::now();
    float diff_s = NAN;
    if (lastValid) {
        diff_s = duration_cast<std::chrono::duration<float>>(now - last).count();

        {
            std::lock_guard<std::mutex> lck(mtx);
            if (measurement->valid) {
                measurement->min_s = std::min(diff_s, measurement->min_s);
                measurement->max_s = std::max(diff_s, measurement->max_s);
                measurement->accum_s += diff_s;
                measurement->samples += 1;
            } else {
                measurement->valid = true;
                measurement->min_s = diff_s;
                measurement->max_s = diff_s;
                measurement->accum_s = diff_s;
                measurement->samples = 1;
            }
        }
    }
    lastValid = true;
    last = now;
    return diff_s;
}

void PerfMon::sample_begin() {
    last = high_resolution_clock::now();
    lastValid = true;
}
float PerfMon::sample_end() {
    return sample();
}

loopMeasurement_t PerfMon::get(std::string name) {
    std::lock_guard<std::mutex> lck(mtx);
    measurements.at(name).valid = false;
    measurements.at(name).avg_s = measurements.at(name).accum_s / measurements.at(name).samples;
    return measurements.at(name);
}

std::map<std::string, loopMeasurement_t> PerfMon::getAll() {
    std::lock_guard<std::mutex> lck(mtx);
    for (auto& pair : measurements) {
        if(pair.second.valid){
            pair.second.valid = false;
            pair.second.avg_s = pair.second.accum_s / pair.second.samples;
        }
    }
    return measurements;
}


void PerfMon::print() {
    auto map = PerfMon::getAll();

    printf("name                         min    max    avg     n   \n");


    for (const auto& pair : map) {
        const loopMeasurement_t& m = pair.second;
        if(m.samples)
            printf("%-25s  %5.1f  %5.1f  %5.1f  %4d\n", pair.first.c_str(), m.min_s * 1000, m.max_s * 1000, m.avg_s * 1000, m.samples);
        else
            printf("%-25s  -\n", pair.first.c_str());
    }
    printf("\n");
}
