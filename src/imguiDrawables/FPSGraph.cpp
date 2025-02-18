#include "FPSGraph.h"
#include <imgui.h>

NumberGraph::NumberGraph(const char* name, float min, float max, const char* format) {
    this->name = name;
    this->min = min;
    this->max = max;
    this->avg = 0;
    this->formatString = "last " + std::string(format) + ", avg " + std::string(format);
    valHist.resize(101, 0);
}

NumberGraph::~NumberGraph() {
}

void NumberGraph::Draw(float valCurrent) {

    for (size_t i = 1; i < valHist.size(); i++) {
        valHist[i - 1] = valHist[i];
    }
    valHist[valHist.size() - 1] = valCurrent;

    if (avg_valid) {
        avg = avg * 0.98 + 0.02 * valCurrent;
    } else {
        avg = valCurrent;
        avg_valid = true;
    }
    // ImGui::Begin("NumberGraph");

    char str[64];
    snprintf(str, sizeof(str), formatString.c_str(), valCurrent, avg);
    ImGui::PlotHistogram(name, &valHist[0], valHist.size(), 0, str, min, max, ImVec2(ImGui::GetFontSize() * 16, ImGui::GetFontSize() * 6));

    // ImGui::End();
}

FrametimeGraph::FrametimeGraph(const char* name, float min, float max, const char* formatString) : NumberGraph(name, min, max, formatString) {
}

FPSGraph::FPSGraph(const char* name, float min, float max, const char* formatString) : NumberGraph(name, min, max, formatString) {
}
