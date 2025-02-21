#include "imgui_menu.h"
#include "imgui.h"
#include "imguiDrawables/FPSGraph.h"
#include "raylib.h"

#define SP_V2(v) v.x, v.y

extern int target_fps;

void static_checkbox(const char* label, bool val) {
    ImGui::Checkbox(label, &val);
}

void draw_window(int argc, char* argv[]) {

    if (ImGui::TreeNodeEx("Window", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed)) {

        ImGui::SeparatorText("Executable");

        ImGui::Text("%s", GetWorkingDirectory());
        ImGui::Text("%s", GetApplicationDirectory());
        for (int i = 0; i < argc; i++) {
            ImGui::Text("%s", argv[i]);
        }

        ImGui::SeparatorText("Monitors");

        int monitors = GetMonitorCount();
        int cur = GetCurrentMonitor();
        for (int i = 0; i < monitors; i++) {
            if (ImGui::TreeNode(TextFormat("%s %s", GetMonitorName(i), cur == i ? "(current)" : ""))) {
                ImGui::BulletText("%.1f %.1f %dx%d %dx%d %d", SP_V2(GetMonitorPosition(i)), GetMonitorWidth(i), GetMonitorHeight(i), GetMonitorPhysicalWidth(i), GetMonitorPhysicalHeight(i), GetMonitorRefreshRate(i));
                ImGui::TreePop();
            }
        }

        ImGui::SeparatorText("Screen");
        ImGui::Text("size %d x %d", GetScreenWidth(), GetScreenHeight());
        ImGui::Text("render size %d x %d", GetRenderWidth(), GetRenderHeight());

        ImGui::SeparatorText("Window");
        ImGui::Text("scale dpi %.1f x %.1f", SP_V2(GetWindowScaleDPI()));

        static_checkbox("IsWindowFullscreen", IsWindowFullscreen());
        ImGui::SameLine();
        static_checkbox("IsWindowMinimized", IsWindowMinimized());
        ImGui::SameLine();
        static_checkbox("IsWindowMaximized", IsWindowMaximized());
        static_checkbox("IsWindowHidden", IsWindowHidden());
        ImGui::SameLine();
        static_checkbox("IsWindowFocused", IsWindowFocused());
        ImGui::SameLine();
        static_checkbox("IsWindowResized", IsWindowResized());

        ImGui::Separator();

        if (ImGui::Button("ToggleFullscreen")) ToggleFullscreen();
        ImGui::SameLine();
        if (ImGui::Button("ToggleBorderlessWindowed")) ToggleBorderlessWindowed();
        if (ImGui::Button("MaximizeWindow")) MaximizeWindow();
        ImGui::SameLine();
        if (ImGui::Button("RestoreWindow")) RestoreWindow();

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

        Vector2 pos = GetWindowPosition();
        if (ImGui::DragFloat2("pos", (float*)&pos)) {
            SetWindowPosition(pos.x, pos.y);
        }
        ImGui::Button("Drag Me");
        Vector2 size = {(float)GetScreenWidth(), (float)GetScreenHeight()};
        if (ImGui::DragFloat2("size", (float*)&size)) {
            SetWindowSize(size.x, size.y);
        }
        if (ImGui::IsItemActive()) {
            pos.x += ImGui::GetIO().MouseDelta.x;
            pos.y += ImGui::GetIO().MouseDelta.y;
            SetWindowPosition(pos.x, pos.y);
        }

        int vsync = IsWindowState(FLAG_VSYNC_HINT);
        if (ImGui::Checkbox("vsync", (bool*)&vsync)) {
            if (vsync)
                SetWindowState(FLAG_VSYNC_HINT);
            else
                ClearWindowState(FLAG_VSYNC_HINT);
        }
        if (ImGui::SliderInt("Target Fps", &target_fps, 0, 120))
            SetTargetFPS(target_fps);

        static float opacity = 1;
        if (ImGui::SliderFloat("opacity", &opacity, 0, 1)) {
            SetWindowOpacity(opacity);
        }

        const char* items[] = {"LOG_ALL", "LOG_TRACE", "LOG_DEBUG", "LOG_INFO", "LOG_WARNING", "LOG_ERROR", "LOG_FATAL", "LOG_NONE"};
        static int item_current = LOG_INFO;
        if (ImGui::Combo("LogLevel", &item_current, items, IM_ARRAYSIZE(items))) {
            SetTraceLogLevel(item_current);
        }

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
        auto devices = audioSource.getDevices();
        if (ImGui::Button("scanDevices")) {
            audioSource.scanDevices();
        }
        ImGui::SameLine();
        if (ImGui::Button("openDevice")) {
            audioSource.openDevice(selectedInputDevice, selectedOutputDevice, 44100);
        }
        ImGui::SameLine();
        ImGui::Checkbox("enableLoopback", (bool*)&audioSource.config.enableLoopback);
        if (ImGui::BeginCombo("input", devices[selectedInputDevice].name.c_str())) {
            for (int n = 0; n < devices.size(); n++) {
                if (devices[n].maxInputChannels == 0) continue;
                const bool is_selected = (selectedInputDevice == n);
                if (ImGui::Selectable(devices[n].name.c_str(), is_selected)) {
                    selectedInputDevice = n;
                    audioSource.openDevice(selectedInputDevice, selectedOutputDevice, 44100);
                }

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (ImGui::BeginCombo("output", devices[selectedOutputDevice].name.c_str())) {
            for (int n = 0; n < devices.size(); n++) {
                if (devices[n].maxOutputChannels == 0) continue;
                const bool is_selected = (selectedOutputDevice == n);
                if (ImGui::Selectable(devices[n].name.c_str(), is_selected)) {
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
        ImGui::SliderFloat("gainLoopback", &audioSource.config.gainLoopback, 0.01, 2, "%.2f", ImGuiSliderFlags_Logarithmic);
        ImGui::TreePop();
    }
}