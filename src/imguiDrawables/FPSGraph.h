#ifndef __FPSGRAPH_H__
#define __FPSGRAPH_H__
#include <string>
#include <vector>

using namespace std;

class NumberGraph {

public:
    NumberGraph(const char*name = "NumberGraph", float min = 0, float max = 1000, const char* format = "%4.0f");
    ~NumberGraph();
    void Draw(float valCurrent);
    
private:
    string formatString;
    const char* name;
    float min, max;
    float avg;
    bool avg_valid = false;
    vector<float> valHist;
};

class FrametimeGraph : public NumberGraph {
    
public:
    FrametimeGraph(const char*name = "FrametimeGraph", float min = 8, float max = 32, const char* format = "%4.2fms");

};

class FPSGraph : public NumberGraph {
    
public:
    FPSGraph(const char*name = "FPSGraph", float min = 20, float max = 70, const char* format = "%3.2ffps");

};

#endif
