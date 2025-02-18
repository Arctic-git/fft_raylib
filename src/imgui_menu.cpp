#include "imgui_menu.h"
#include "imgui.h"
#include "imguiDrawables/FPSGraph.h"
#include "raylib.h"

void draw_window() {
    if (ImGui::TreeNodeEx("Window", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed)) {

        int vsync = IsWindowState(FLAG_VSYNC_HINT);
        if (ImGui::Checkbox("vsync", (bool*)&vsync)) {
            if (vsync)
                SetWindowState(FLAG_VSYNC_HINT);
            else
                ClearWindowState(FLAG_VSYNC_HINT);
        }
        if (ImGui::Button("ToggleFullscreen"))
            ToggleFullscreen();
        if (ImGui::Button("ToggleBorderlessWindowed"))
            ToggleBorderlessWindowed();
        if (ImGui::Button("MaximizeWindow"))
            MaximizeWindow();
        int resizable = IsWindowState(FLAG_WINDOW_RESIZABLE);
        if (ImGui::Checkbox("resizable", (bool*)&resizable)) {
            if (resizable)
                SetWindowState(FLAG_WINDOW_RESIZABLE);
            else
                ClearWindowState(FLAG_WINDOW_RESIZABLE);
        }
        int undecorated = IsWindowState(FLAG_WINDOW_UNDECORATED);
        if (ImGui::Checkbox("undecorated", (bool*)&undecorated)) {
            if (undecorated)
                SetWindowState(FLAG_WINDOW_UNDECORATED);
            else
                ClearWindowState(FLAG_WINDOW_UNDECORATED);
        }
        int topmost = IsWindowState(FLAG_WINDOW_TOPMOST);
        if (ImGui::Checkbox("topmost", (bool*)&topmost)) {
            if (topmost)
                SetWindowState(FLAG_WINDOW_TOPMOST);
            else
                ClearWindowState(FLAG_WINDOW_TOPMOST);
        }
        int always_run = IsWindowState(FLAG_WINDOW_ALWAYS_RUN);
        if (ImGui::Checkbox("always_run", (bool*)&always_run)) {
            if (always_run)
                SetWindowState(FLAG_WINDOW_ALWAYS_RUN);
            else
                ClearWindowState(FLAG_WINDOW_ALWAYS_RUN);
        }

        const char* items[] = {"LOG_ALL", "LOG_TRACE", "LOG_DEBUG", "LOG_INFO", "LOG_WARNING", "LOG_ERROR", "LOG_FATAL", "LOG_NONE"};
        static int item_current = LOG_INFO;
        if (ImGui::Combo("combo", &item_current, items, IM_ARRAYSIZE(items))) {
            SetTraceLogLevel(item_current);
        }

        Vector2 pos = GetWindowPosition();
        if (ImGui::DragFloat2("pos", (float*)&pos)) {
            SetWindowPosition(pos.x, pos.y);
        }
        ImGui::Button("Drag Me");
        if (ImGui::IsItemActive()){
            pos.x += ImGui::GetIO().MouseDelta.x;
            pos.y += ImGui::GetIO().MouseDelta.y;
            SetWindowPosition(pos.x, pos.y);
        }

        static int target_fps = 60;
        if (ImGui::SliderInt("Target Fps", &target_fps, 0, 120))
            SetTargetFPS(target_fps);

        ImGui::TreePop();
    }
}

void draw_perf(int freshSamples) {
    if (ImGui::TreeNodeEx("Performance", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed)) {
        static FrametimeGraph frametimegraphmain("Main Frametime", 5, 32);
        frametimegraphmain.Draw(GetFrameTime() * 1000);
        static NumberGraph freshsamplesgraph("Fresh samples", 0, 2000);
        freshsamplesgraph.Draw(freshSamples);
        ImGui::TreePop();
    }
}

void draw_audiosource(AudioSourcePA& audioSource) {
    static int selectedInputDevice = -1;
    static int selectedOutputDevice = -1;
    if (selectedInputDevice == -1)
        audioSource.getCurrentDevice(&selectedInputDevice, &selectedOutputDevice);

    if (ImGui::TreeNodeEx("audioSoruce", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed)) {
        ImGui::Text("%s", audioSource.getInfo());
        auto deviceNames = audioSource.getDeviceNames();
        if (ImGui::Button("scanDevices")) {
            audioSource.scanDevices();
        }
        ImGui::SameLine();
        if (ImGui::Button("openDevice")) {
            audioSource.openDevice(selectedInputDevice, selectedOutputDevice, 44100);
        }
        ImGui::SameLine();
        ImGui::Checkbox("enableLoopback", (bool*)&audioSource.config.enableLoopback);
        if (ImGui::BeginCombo("input", deviceNames[selectedInputDevice].c_str())) {
            for (int n = 0; n < deviceNames.size(); n++) {
                const bool is_selected = (selectedInputDevice == n);
                if (ImGui::Selectable(deviceNames[n].c_str(), is_selected)) {
                    selectedInputDevice = n;
                    audioSource.openDevice(selectedInputDevice, selectedOutputDevice, 44100);
                }

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (ImGui::BeginCombo("output", deviceNames[selectedOutputDevice].c_str())) {
            for (int n = 0; n < deviceNames.size(); n++) {
                const bool is_selected = (selectedOutputDevice == n);
                if (ImGui::Selectable(deviceNames[n].c_str(), is_selected)) {
                    selectedOutputDevice = n;
                    audioSource.openDevice(selectedInputDevice, selectedOutputDevice, 44100);
                }

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::SliderFloat("gain", &audioSource.config.gain, 0, 30, "%.1f", ImGuiSliderFlags_Logarithmic);
        ImGui::SliderFloat("gainLoopback", &audioSource.config.gainLoopback, 0, 2, "%.1f", ImGuiSliderFlags_Logarithmic);
        ImGui::TreePop();
    }
}