/*******************************************************************************************
 *
 *   raylib-extras [ImGui] example - Docking example
 *
 *	This is an example of using the ImGui docking features that are part of docking branch
 *	You must replace the default imgui with the code from the docking branch for this to work
 *	https://github.com/ocornut/imgui/tree/docking
 *
 *   Copyright (c) 2024 Jeffery Myers
 *
 ********************************************************************************************/

#include "AudioSourcePA.h"
#include "FftPostprocessor.h"
#include "FftProcessor.h"
#include "draw.h"
#include "imgui.h"
#include "imgui_menu.h"
#include "implot.h"
#include "perfMon.h"
#include "raylib.h"
#include "raymath.h"
#include "rlFixes.h"
#include "rlImGui.h"
#include "rlgl.h"
#include <algorithm>
#include <filesystem>
#include <print>

#define SP_RECT(b) b.x, b.y, b.width, b.height
static int screenWidth = 1000;
static int screenHeight = 1000;
int target_fps = 80;
#define SP_COLOR(c) c.r, c.g, c.b, c.a

#define WAVE_SAMPLES_MAX (1024 * 64)
int wave_samples = 2048;
int samplerate = 44100;

namespace fs = std::filesystem;
fs::path path_res;

extern int rlDrawRenderBatch_cnt;
extern int rlDrawRenderBatch_drawCounter;
extern int rlDrawRenderBatch_vertex_cnt;

static Color ImColor_to_Color(ImColor c) {
    return {
        (uint8_t)(c.Value.x * 255),
        (uint8_t)(c.Value.y * 255),
        (uint8_t)(c.Value.z * 255),
        (uint8_t)(c.Value.w * 255),
    };
}

static std::string system_capture(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string std_output;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        std_output += buffer.data();
    }

    int exit_status = pclose(pipe);
    if (exit_status)
        throw std::system_error(exit_status, std::generic_category(), "HwManagerBeaglewire system_capture '" + cmd + "' error");

    return std_output;
}

int main(int argc, char* argv[]) {
#ifdef PATH_RESOURCES_REL
    path_res = fs::path(GetApplicationDirectory()).append(PATH_RESOURCES_REL);
#else
    path_res = PATH_RESOURCES_DEVEL_ABS;
#endif
    TraceLog(LOG_INFO, "Resource Path: '%s'", path_res.string().c_str());
    if (!DirectoryExists(path_res.string().c_str())) {
        TraceLog(LOG_ERROR, "Resource path does not exist! '%s'", path_res.string().c_str());
    }

    // Image icon = LoadImage(fs::path(path_res).append("icon3.png").string().c_str());
    // SetWindowIcon(icon);s

    fs::path path_settings;
#if defined(PATH_SETTINGS_BASH)
    path_settings = system_capture("bash -c \"echo -n " + std::string(PATH_SETTINGS_BASH) + "\"");
    MakeDirectory(path_settings.string().c_str());
#else
    path_settings = fs::path(GetApplicationDirectory());
#endif
    TraceLog(LOG_INFO, "path_settings '%s'", path_settings.string().c_str());

    // copy default settings from resources
    fs::path path_imgui = fs::path(path_settings).append("imgui.ini").string().c_str();
    std::string imgui_path_str = path_imgui.string();
    TraceLog(LOG_INFO, "path_imgui '%s'", path_imgui.string().c_str());
    if (!FileExists(path_imgui.string().c_str())) {
        fs::copy(fs::path(path_res).append("imgui_default.ini"), path_imgui);
        TraceLog(LOG_INFO, "copied imgui");
    }

    int fftp_window = 1;
    int fftp_padding = 32768 / 2;

    Ringbuffer soundbuffer(1024 * 256);
    AudioSourcePA audioSource(&soundbuffer, samplerate);
    FftProcessor fftProcessor(wave_samples, std::max(fftp_padding / wave_samples, 1));
    fftProcessor.updateWindow(fftp_window);
    FftPostprocessor fftPostprocessorConti;
    fftPostprocessorConti.config.smoothing.alphaDn = fftPostprocessorConti.config.smoothing.alphaUp = 0.2;
    FftPostprocessor fftPostprocessorScroll;
    fftPostprocessorScroll.config.smoothing.alphaDn = fftPostprocessorScroll.config.smoothing.alphaUp = 1;
    fftPostprocessorScroll.config.scaling.mag2db = false;
    FftPostprocessor* pps[] = {&fftPostprocessorConti, &fftPostprocessorScroll};
    const char* ppNames[] = {"+fftPostprocessorConti", "+fftPostprocessorScroll"};

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE); // | FLAG_WINDOW_UNDECORATED); //| FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "fft_raylib");
    // ClearWindowState(FLAG_WINDOW_RESIZABLE);
    // SetWindowState(FLAG_WINDOW_RESIZABLE);
    // std::print("{} {} {}\n", GetCurrentMonitor(), GetMonitorPosition(GetCurrentMonitor()).x, GetMonitorPosition(GetCurrentMonitor()).y);

    SetWindowPosition(GetMonitorPosition(GetCurrentMonitor()).x, GetMonitorPosition(GetCurrentMonitor()).y + 31);

    target_fps = GetMonitorRefreshRate(GetCurrentMonitor());
    SetTargetFPS(target_fps);
    rlImGuiSetup(true);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::GetIO().IniFilename = imgui_path_str.c_str(); // has to stay in scope
    ImPlot::CreateContext();

    Shader shader = LoadShader(fs::path(path_res).append("custom.vs").string().c_str(), fs::path(path_res).append("custom.fs").string().c_str());

    // remove text optimisation
    Texture2D texture = {1, 1, 1, 1, 7};
    SetShapesTexture(texture, (Rectangle){0.0f, 0.0f, 1.0f, 1.0f});

    // wave texture
    // Image im_w = {
    //     .data = l,
    //     .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
    //     .height = 1,
    //     .width = wave_samples,
    //     .mipmaps = 1,
    // };
    // Texture2D texture_w = LoadTextureFromImage(im_w); // Load blank texture to fill on shader
    // SetTextureFilter(texture_w, TEXTURE_FILTER_BILINEAR);
    // SetTextureWrap(texture_w, TEXTURE_WRAP_CLAMP);

    float iTime = 0;
    bool pause = false;
    bool settings = false;
    bool wave = true;
    bool wavewindowed = false;
    bool wavescroll = true;
    bool wavescroll_scroll = true;
    int wavescroll_colorscale = 7;
    bool wave_outline = true;
    bool wave_fill = true;
    bool xy = true;
    bool fft = true;
    int fft_colormode = 0;
    bool fftscroll = true;
    bool fftscroll_dpi = false;
    float fftscroll_ymulti = 1;
    int stereo_mode = 0;
    ImVec2 gui_padding{8, 8};
    float gui_seperator = 2;

    ImColor xy_color = {SP_COLOR(WHITE)};
#define osc_green Color{48, 255, 76, 255}
    ImColor xy_osc_color = {SP_COLOR(osc_green)};

    bool wave_scissor = false;
    bool wavewindowed_scissor = false;
    bool xy_scissor = false;
    bool fft_scissor = true;

    bool xy_shader = false;
    float osc_iSize = 5;
    float osc_iIntensity = 1;
    float osc_iLenDarken = 1;

    float fft_min = -78, fft_max = -18;

    float l[WAVE_SAMPLES_MAX], r[WAVE_SAMPLES_MAX], lr[WAVE_SAMPLES_MAX];

    int page = 1;

    wave_scrolltexture wave_scrolltexture1;
    fft_scrolltexture fft_scrolltexture1;

    PerfMon pm_calc("calc");
    PerfMon pm_wave("wave");
    PerfMon pm_xy("xy");
    PerfMon pm_fft("fft");
    PerfMon pm_fftscroll("fftscroll");
    PerfMon pm_wavescroll("wavescroll");
    PerfMon pm_imgui("imgui");
    PerfMon pm_swap("swap");

    while (!WindowShouldClose() && !(IsKeyDown(KEY_W) && (IsKeyDown(KEY_LEFT_SUPER) | IsKeyDown(KEY_LEFT_CONTROL)))) {
        if (IsKeyPressed(KEY_L)) audioSource.config.enableLoopback ^= 1;
        if (IsKeyPressed(KEY_P)) pause ^= 1;
        if (IsKeyPressed(KEY_S)) settings ^= 1;
        if (IsKeyPressed(KEY_R)) ImGui::LoadIniSettingsFromDisk(ImGui::GetIO().IniFilename);
        if (IsKeyPressed(KEY_D)) ImGui::LoadIniSettingsFromDisk(fs::path(path_res).append("imgui_default.ini").string().c_str());
        if (IsKeyPressed(KEY_V)) PerfMon::print();
        if (IsKeyPressed(KEY_ONE)) page = 1;
        if (IsKeyPressed(KEY_TWO)) page = 2;

        if (IsWindowHidden()) {
            WaitTime(4);
        }

        int freshSamples = 0;
        if (!pause) {
            freshSamples = soundbuffer.getlr(l, r, wave_samples);
        }

        pm_calc.sample_begin();
        if (fft || fftscroll) {
            if (stereo_mode == 0) {
                fftProcessor.process(l, r);
            } else if (stereo_mode == 1) {
                for (int i = 0; i < wave_samples; i++) {
                    lr[i] = (l[i] + r[i]) / 2;
                }
                fftProcessor.process(lr);
            } else if (stereo_mode == 2) {
                for (int i = 0; i < wave_samples; i++) {
                    lr[i] = (l[i] - r[i]) / 2;
                }
                fftProcessor.process(lr);
            }
        }
        pm_calc.sample_end();

        iTime += GetFrameTime();
        rlDrawRenderBatch_cnt = 0;
        rlDrawRenderBatch_drawCounter = 0;
        rlDrawRenderBatch_vertex_cnt = 0;

        BeginDrawing();
        ClearBackground(BLACK);
        rlImGuiBegin();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, gui_padding);          // default is 8,8
        ImGui::PushStyleVar(ImGuiStyleVar_DockingSeparatorSize, gui_seperator); // default is 2
        ImGui::DockSpaceOverViewport(0, NULL, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar);

        // float w = GetScreenWidth(), h = GetScreenHeight();

        // UnloadShader(shader);
        // shader = LoadShader(PATH_RESOURCES "custom.vs", PATH_RESOURCES "custom.fs");
        // if (IsShaderValid(shader)) {
        //     SetShaderValue(shader, GetShaderLocation(shader, "iTime"), (void*)&iTime, SHADER_UNIFORM_FLOAT);

        //     BeginShaderMode(shader);
        //     DrawText("USING CUSTOM SHADER", 190, 40, 10, RED);
        //     DrawRectangleGradientH(250 - 90, 170, 400, 200, MAROON, GOLD);
        //     DrawRectangleGradientH(0, 0, 200, 100, MAROON, GOLD);
        //     EndShaderMode();
        // }

        // UpdateTexture(texture_w, lu8);
        // Shader shader_wave = LoadShader(0, PATH_RESOURCES "wave.fs");
        // if (IsShaderValid(shader_wave)) {
        //     BeginShaderMode(shader_wave);
        //     DrawTexturePro(texture_w, (Rectangle){0, 0, wave_samples, 1}, (Rectangle){0, 0, w, h / 2}, (Vector2){0, 0}, 0, WHITE);
        //     EndShaderMode();
        // }
        // UnloadShader(shader_wave);

        // UnloadShader(shader);
        // shader = LoadShader(0, PATH_RESOURCES "line.fs");
        // if (IsShaderValid(shader)) {
        //     BeginShaderMode(shader);
        // }

        // // DrawRectangle(200, 0, 200, 100, GOLD);
        // // DrawRectanglePro2({200, 100, 200, 100}, {0,0},  30, GOLD);
        // // DrawLineEx2({0, 0}, {w, h}, 50, WHITE);

        // DrawLineEx3({50, 50}, {100, 100}, 50, WHITE);
        // DrawLineEx3({150, 150}, {250, 150}, 50, WHITE);
        // EndShaderMode();
        // DrawLineEx({50, 50}, {100, 100}, 5, WHITE);

        if (page == 2) {
            DrawLineExQuads({100, 100}, {200, 500}, osc_iSize * 2, WHITE);
            DrawLineExQuads({200, 500}, {300, 100}, osc_iSize * 2, GRAY);

            UnloadShader(shader);
            shader = LoadShader(NULL, fs::path(path_res).append("oscline.fs").string().c_str());
            if (IsShaderValid(shader)) {
                SetShaderValue(shader, GetShaderLocation(shader, "iTime"), (void*)&iTime, SHADER_UNIFORM_FLOAT);
                SetShaderValue(shader, GetShaderLocation(shader, "iSize"), (void*)&osc_iSize, SHADER_UNIFORM_FLOAT);
                SetShaderValue(shader, GetShaderLocation(shader, "iIntensity"), (void*)&osc_iIntensity, SHADER_UNIFORM_FLOAT);
                SetShaderValue(shader, GetShaderLocation(shader, "iLenDarken"), (void*)&osc_iLenDarken, SHADER_UNIFORM_FLOAT);
                SetShaderValue(shader, GetShaderLocation(shader, "iColor"), (void*)&xy_osc_color, SHADER_UNIFORM_VEC4);
                BeginShaderMode(shader);
            }
            DrawLineExQuadsTexturecoordsLengthembed({100 + 300, 100}, {200 + 300, 500}, osc_iSize * 2, WHITE);
            DrawLineExQuadsTexturecoordsLengthembed({200 + 300, 500}, {300 + 300, 100}, osc_iSize * 2, GRAY);
            DrawLineExQuadsTexturecoordsLengthembed({300 + 300, 100}, {300 + 400, 200}, osc_iSize * 2, GRAY);
            EndShaderMode();

        } else if (page == 1) {
            pm_wave.sample_begin();
            if (wave) {
                ImGui::SetNextWindowBgAlpha(0.0f);
                ImGui::Begin("wave", &wave, 0);
                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
                Rectangle b = {canvas_p0.x, canvas_p0.y, canvas_sz.x, canvas_sz.y};
                Rectangle b1 = b;
                b1.height /= 2;
                Rectangle b2 = b;
                b2.height /= 2;
                b2.y += b2.height;

                rlDisableBackfaceCulling();
                if (wave_scissor) BeginScissorMode(SP_RECT(b1));
                wave_line(b1, l, wave_samples, b1.width, wave_fill, wave_outline);
                if (wave_scissor) BeginScissorMode(SP_RECT(b2));
                wave_line(b2, r, wave_samples, b2.width, wave_fill, wave_outline);
                if (wave_scissor) EndScissorMode();
                ImGui::End();
            }
            pm_wave.sample_end();

            if (wavewindowed) {
                ImGui::SetNextWindowBgAlpha(0.0f);
                ImGui::Begin("wavewindowed", &wavewindowed, 0);
                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
                Rectangle b = {canvas_p0.x, canvas_p0.y, canvas_sz.x, canvas_sz.y};

                rlDisableBackfaceCulling();
                if (wavewindowed_scissor) BeginScissorMode(SP_RECT(b));
                wave_line(b, fftProcessor.getTimeWindowed(), fftProcessor.getTimeSize(), b.width, wave_fill, wave_outline);
                if (wavewindowed_scissor) EndScissorMode();
                ImGui::End();
            }

            pm_xy.sample_begin();
            if (xy) {
                ImGui::SetNextWindowBgAlpha(0.0f);
                ImGui::Begin("xy", &xy, 0);
                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
                Rectangle b = {canvas_p0.x, canvas_p0.y, canvas_sz.x, canvas_sz.y};
                if (xy_scissor) BeginScissorMode(SP_RECT(b));
                if (xy_shader)
                    xy_osc(b, l, r, wave_samples, ImColor_to_Color(xy_osc_color), osc_iSize, osc_iIntensity, osc_iLenDarken);
                else
                    xy_line(b, l, r, wave_samples, ImColor_to_Color(xy_color));
                if (xy_scissor) EndScissorMode();
                ImGui::End();
            }
            pm_xy.sample_end();
            pm_fft.sample_begin();
            if (fft) {
                ImGui::SetNextWindowBgAlpha(0.0f);
                ImGui::Begin("fft", &fft, 0);
                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
                Rectangle b = {canvas_p0.x, canvas_p0.y, canvas_sz.x, canvas_sz.y};

                fftPostprocessorConti.process(fftProcessor.getOutput(), fftProcessor.getOutputSize(), std::ceil(b.width), samplerate);
                float my_fft_min = fft_min, my_fft_max = fft_max;
                if (!fftPostprocessorConti.config.scaling.mag2db) {
                    my_fft_min = powf(10, fft_min / 20);
                    my_fft_max = powf(10, fft_max / 20) / 2;
                }

                if (fft_scissor) BeginScissorMode(SP_RECT(b));
                fft_conti2(b, fftPostprocessorConti.getOutput(), fftPostprocessorConti.getOutputSize(), fftPostprocessorConti.config.binning.minFreq, fftPostprocessorConti.config.binning.maxFreq, fftPostprocessorConti.config.binning.logbinning, wave_fill, wave_outline, fft_colormode, my_fft_min, my_fft_max);
                if (fft_scissor) EndScissorMode();
                ImGui::End();
            }
            pm_fft.sample_end();
            pm_fftscroll.sample_begin();
            if (fftscroll) {
                ImGui::SetNextWindowBgAlpha(0.0f);
                ImGui::Begin("fftscroll", &fftscroll, 0);
                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
                Rectangle b = {canvas_p0.x, canvas_p0.y, canvas_sz.x, canvas_sz.y};

                int bins = fftscroll_dpi ? std::ceil(b.width * GetWindowScaleDPI().x) : std::ceil(b.width);
                fftPostprocessorScroll.process(fftProcessor.getOutput(), fftProcessor.getOutputSize(), bins, samplerate);
                float my_fft_min = fft_min, my_fft_max = fft_max;
                if (!fftPostprocessorScroll.config.scaling.mag2db) {
                    my_fft_min = powf(10, fft_min / 20);
                    my_fft_max = powf(10, fft_max / 20) / 2;
                }

                fft_scrolltexture1.draw(b, fftPostprocessorScroll.getOutput(), fftPostprocessorScroll.getOutputSize(), fftPostprocessorConti.config.binning.minFreq, fftPostprocessorConti.config.binning.maxFreq, fftPostprocessorConti.config.binning.logbinning, wavescroll_colorscale, wavescroll_scroll, my_fft_min, my_fft_max, fftscroll_ymulti);
                ImGui::End();
            }
            pm_fftscroll.sample_end();
            pm_wavescroll.sample_begin();
            if (wavescroll) {
                ImGui::SetNextWindowBgAlpha(0.0f);
                ImGui::Begin("wavescroll", &xy, 0);
                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
                Rectangle b = {canvas_p0.x, canvas_p0.y, canvas_sz.x, canvas_sz.y};
                wave_scrolltexture1.draw(b, l, wave_samples, wavescroll_scroll);
                ImGui::End();
            }
            pm_wavescroll.sample_end();
        }

        ImGui::PopStyleVar(2);

        pm_imgui.sample_begin();
        if (settings) {
            static bool imgui_ShowDemoWindow = 0, implot_ShowDemoWindow = 0, imgui_ShowStyleEditor = 0;
            if (imgui_ShowDemoWindow)
                ImGui::ShowDemoWindow(&imgui_ShowDemoWindow);
            if (implot_ShowDemoWindow)
                ImPlot::ShowDemoWindow(&implot_ShowDemoWindow);
            if (imgui_ShowStyleEditor) {
                ImGui::Begin("Dear ImGui Style Editor", &imgui_ShowStyleEditor);
                ImGui::ShowStyleEditor();
                ImGui::End();
            }

            if (ImGui::Begin("Settings")) {

                draw_audiosource(audioSource);
                if (ImGui::TreeNodeEx("Drawing", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed)) {
                    ImGui::Checkbox("pause (p)", &pause);
                    ImGui::Separator();

                    ImGui::Checkbox("wave", &wave);
                    ImGui::SameLine();
                    ImGui::Checkbox("wave_outline", &wave_outline);
                    ImGui::SameLine();
                    ImGui::Checkbox("wave_fill", &wave_fill);
                    ImGui::SameLine();
                    ImGui::Checkbox("expand", &DrawLineExQuadsDc_expand);
                    ImGui::Checkbox("wavewindowed", &wavewindowed);
                    ImGui::Checkbox("wavescroll", &wavescroll);
                    ImGui::SameLine();
                    ImGui::Checkbox("wavescroll_scroll", &wavescroll_scroll);
                    ImGui::Checkbox("xy", &xy);
                    ImGui::SameLine();
                    ImGui::Checkbox("xy_shader", &xy_shader);
                    ImGui::Checkbox("fft", &fft);
                    ImGui::Checkbox("fftscroll", &fftscroll);
                    ImGui::SameLine();
                    ImGui::Checkbox("dpi", &fftscroll_dpi);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(100);
                    ImGui::SliderFloat("ymulti", &fftscroll_ymulti, 0.2, 4, "%.1f", ImGuiSliderFlags_Logarithmic);
                    ImGui::Separator();

                    ImGui::Checkbox("wave_scissor", &wave_scissor);
                    ImGui::Checkbox("wavewindowed_scissor", &wavewindowed_scissor);
                    ImGui::Checkbox("xy_scissor", &xy_scissor);
                    ImGui::Checkbox("fft_scissor", &fft_scissor);
                    ImGui::Separator();

                    ImGui::SliderFloat("gui_padding", &gui_padding.x, 0.0f, 20.0f, "%.0f");
                    gui_padding.y = gui_padding.x;
                    ImGui::SliderFloat("gui_seperator", &gui_seperator, 0.0f, 6.0f, "%.0f");
                    ImGui::SliderFloat("draw_line_width", &draw_line_width, 0.1, 8, "%.1f", ImGuiSliderFlags_Logarithmic);
                    ImGui::Separator();

                    ImGui::SliderInt("fft_colormode", &fft_colormode, 0, 1);
                    ImGui::SliderInt("wavescroll_colorscale", &wavescroll_colorscale, 0, IM_ARRAYSIZE(colorscale_names) - 1, colorscale_names[wavescroll_colorscale]);
                    ImGui::Separator();

                    ImGui::ColorEdit4("xy color", (float*)&xy_color);
                    ImGui::ColorEdit4("xy osc color", (float*)&xy_osc_color);
                    ImGui::SliderFloat("iSize", &osc_iSize, 0, 20, "%.2f", ImGuiSliderFlags_Logarithmic);
                    ImGui::SliderFloat("iIntensity", &osc_iIntensity, 0, 1);
                    ImGui::SliderFloat("iLenDarken", &osc_iLenDarken, 0, 1);
                    ImGui::TreePop();
                }

                if (ImGui::TreeNodeEx("FFT", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed)) {

                    float minmax[2] = {fft_min, fft_max};
                    ImGui::SliderFloat2("fft_min/max", (float*)&minmax, -120, 0, "%.0f");
                    fft_min = minmax[0];
                    fft_max = minmax[1];

                    // ImGui::SliderFloat("fft_max", &fft_max, -100, 0, "%.0f");
                    // ImGui::SliderInt("wavescroll_colorscale", &wavescroll_colorscale, 0, 14);

                    if (ImGui::TreeNodeEx("FftProcessor", ImGuiTreeNodeFlags_DefaultOpen)) {

                        if (ImGui::SliderInt("fftp_window", &fftp_window, 0, 4, fftProcessor.getWindowName())) {
                            fftProcessor.updateWindow(fftp_window);
                        }

                        bool reload_fft = false;
                        int wave_samples_base = log2(wave_samples);
                        if (ImGui::SliderInt("wave_samples", &wave_samples_base, 6, 16, TextFormat("%d", 1 << wave_samples_base))) {
                            wave_samples = 1 << wave_samples_base;
                            reload_fft = true;
                        }
                        int pad_base = log2(fftp_padding);
                        if (ImGui::SliderInt("fftp_padding", &pad_base, 6, 16, TextFormat("%d", 1 << pad_base))) {
                            fftp_padding = 1 << pad_base;
                            reload_fft = true;
                        }
                        if (reload_fft) {
                            TraceLog(LOG_INFO, "reload fftp %d %d", wave_samples, fftp_padding);
                            fftProcessor.deallocate();
                            fftProcessor.allocate(wave_samples, std::max(fftp_padding / wave_samples, 1));
                            fftProcessor.updateWindow(fftp_window);
                        }

                        ImGui::SliderInt("stereo_mode", &stereo_mode, 0, 2, (const char*[]){"fft(l)+fft(r)", "fft(l+r)", "fft(l-r)"}[stereo_mode]);

                        ImGui::SliderFloat("slope", &fftProcessor.config.slope, 0, 6, "%.0f", ImGuiSliderFlags_None);
                        ImGui::TreePop();
                    }

                    ImGui::Separator();

                    static bool copy = false;
                    ImGui::Checkbox("copy fftp config", &copy);

                    int ppNr = 0;
                    for (auto& pp : pps) {
                        ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_None;
                        if (ppNr && copy)
                            pp->config = pps[ppNr - 1]->config;
                        if (ppNames[ppNr][0] == '+') flag = ImGuiTreeNodeFlags_DefaultOpen;
                        if (ImGui::TreeNodeEx(ppNames[ppNr] + 1, flag)) {
                            ImGui::Text("OutputSize %d", (int)pp->getOutputSize());
                            ImGui::SliderFloat2("alphaUp/Down", &pp->config.smoothing.alphaUp, 0, 1, "%.1f", ImGuiSliderFlags_None);
                            ImGui::SliderFloat("decay", &pp->config.smoothing.decay, -0.0001f, 0.005f, "%.4f", ImGuiSliderFlags_None);
                            // ImGui::SliderFloat("minDbClamp", &pp->config.smoothing.minDbClamp, -99, 0, "%.1f", ImGuiSliderFlags_None);
                            ImGui::SliderInt("blurringPasses", &pp->config.smoothing.blurringPasses, 0, 20);

                            ImGui::SliderFloat2("Freq min/max", &pp->config.binning.minFreq, 10, samplerate / 2, "%.0f", ImGuiSliderFlags_Logarithmic);
                            ImGui::Checkbox("bin_logbinning", (bool*)&pp->config.binning.logbinning);
                            ImGui::SameLine();
                            ImGui::Checkbox("bin_avgmode", (bool*)&pp->config.binning.avgmode);
                            ImGui::SameLine();
                            // ImGui::Checkbox("removeBaselineOffset", (bool*)&pp->config.folding.removeBaselineOffset);
                            ImGui::Checkbox("mag2db", (bool*)&pp->config.scaling.mag2db);

                            ImGui::TreePop();
                        }
                        ppNr++;
                    }
                    ImGui::TreePop();
                }
                draw_window(argc, argv);
                if (draw_perf(freshSamples)) {
                    auto measurements = PerfMon::getAll();
                    float m_avg[10];
                    const char* names[10];
                    int cnt = 0;
                    float sum = 0;
                    for (const auto& pair : measurements) {
                        names[cnt] = pair.first.c_str();
                        m_avg[cnt] = pair.second.avg_s * 1000;
                        sum += m_avg[cnt];
                        cnt++;
                    }
                    // names[cnt] = "free";
                    // m_avg[cnt] =  1000/target_fps - sum;
                    // cnt++;

                    if (ImPlot::BeginPlot("PerfMon", ImVec2(350, 250), ImPlotFlags_NoMouseText)) {
                        int flags = ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_Lock;
                        ImPlot::SetupAxes(nullptr, nullptr, flags, flags);
                        ImPlot::SetupAxesLimits(0, 1, 0, 1);
                        ImPlot::PlotPieChart(names, m_avg, cnt, 0.7, 0.5, 0.3, "%.2f", 90, ImPlotPieChartFlags_Normalize);

                        ImPlot::EndPlot();
                    }
                }

                ImGui::Separator();
                ImGui::Checkbox("ImGui::ShowDemoWindow", &imgui_ShowDemoWindow);
                ImGui::Checkbox("ImPlot::ShowDemoWindow", &implot_ShowDemoWindow);
                ImGui::Checkbox("ImGui::ShowStyleEditor", &imgui_ShowStyleEditor);
            }

            ImGui::End();
        }

        if (settings) {
            DrawFPS(10, 10);
            // without imgui drawcalls since it is before rlImGuiEnd
            DrawText(TextFormat("%d %d %.3f", rlDrawRenderBatch_cnt, rlDrawRenderBatch_drawCounter, (float)rlDrawRenderBatch_vertex_cnt / 1000), 100, 10, 20, WHITE);
        }

        rlImGuiEnd();
        pm_imgui.sample_end();

        rlDrawRenderBatchActive();

        pm_swap.sample_begin();
        EndDrawing();
        // SwapScreenBuffer();
        pm_swap.sample_end();
    }

    ImPlot::DestroyContext();
    rlImGuiShutdown();
    CloseWindow();

    return 0;
}