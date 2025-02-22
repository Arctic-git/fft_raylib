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
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"
#include "rlgl.h"
#include <algorithm>
#include <filesystem>
#include <print>

#define SAMPLERATE (44100)
#define SP_RECT(b) b.x, b.y, b.width, b.height
static int screenWidth = 1200;
static int screenHeight = 1000;
int target_fps = 80;

#define WAVE_SAMPLES_MAX (1024 * 64)
int wave_samples = 2048;

namespace fs = std::filesystem;
fs::path path_res;

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
    path_res = fs::path(GetApplicationDirectory()).append(PATH_RESOURCES_REL);
    TraceLog(LOG_INFO, "Resource Path: '%s'", path_res.string().c_str());
    if (!DirectoryExists(path_res.c_str())) {
        TraceLog(LOG_ERROR, "Resource path does not exist! '%s'", path_res.c_str());
    }

    // Image icon = LoadImage(fs::path(path_res).append("icon3.png").c_str());
    // SetWindowIcon(icon);s

    fs::path path_settings;
#if defined(PATH_SETTINGS_BASH)
    path_settings = system_capture("bash -c \"echo -n " + std::string(PATH_SETTINGS_BASH) + "\"");
    MakeDirectory(path_settings.c_str());
#else
    path_settings = "."
#endif
    TraceLog(LOG_INFO, "path_settings '%s'", path_settings.c_str());

    // copy default settings from resources
    fs::path path_imgui = fs::path(path_settings).append("imgui.ini").c_str();
    if (!FileExists(path_imgui.c_str())) {
        fs::copy(fs::path(path_res).append("imgui_default.ini"), path_imgui);
    }

    int fftp_window = 1;
    int fftp_padding = 32768 / 2;

    Ringbuffer soundbuffer(1024 * 256);
    AudioSourcePA audioSource(&soundbuffer, SAMPLERATE);
    FftProcessor fftProcessor(wave_samples, std::max(fftp_padding / wave_samples, 1));
    fftProcessor.updateWindow(fftp_window);
    FftPostprocessor fftPostprocessorConti(SAMPLERATE, fftProcessor.getOutputSize());
    fftPostprocessorConti.config.smoothing.alphaDn = fftPostprocessorConti.config.smoothing.alphaUp = 0.2;
    FftPostprocessor fftPostprocessorScroll(SAMPLERATE, fftProcessor.getOutputSize());
    fftPostprocessorScroll.config.smoothing.alphaDn = fftPostprocessorScroll.config.smoothing.alphaUp = 1;
    fftPostprocessorScroll.config.scaling.mag2db = false;
    FftPostprocessor* pps[] = {&fftPostprocessorConti, &fftPostprocessorScroll};
    const char* ppNames[] = {"+fftPostprocessorConti", "+fftPostprocessorScroll"};

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE); // | FLAG_WINDOW_UNDECORATED); //| FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "fft_raylib");
    // ClearWindowState(FLAG_WINDOW_RESIZABLE);
    // SetWindowState(FLAG_WINDOW_RESIZABLE);
    // std::print("{} {} {}\n", GetCurrentMonitor(), GetMonitorPosition(GetCurrentMonitor()).x, GetMonitorPosition(GetCurrentMonitor()).y);
    SetWindowPosition(GetMonitorPosition(GetCurrentMonitor()).x, GetMonitorPosition(GetCurrentMonitor()).y);

    target_fps = GetMonitorRefreshRate(GetCurrentMonitor());
    SetTargetFPS(target_fps);
    rlImGuiSetup(true);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::GetIO().IniFilename = path_imgui.c_str();
    ImPlot::CreateContext();

    Shader shader = LoadShader(fs::path(path_res).append("custom.vs").c_str(), fs::path(path_res).append("custom.fs").c_str());

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
    bool fft_logspacing = true;
    int fft_colormode = 0;
    bool fftscroll = true;
    bool fftscroll_lerp = true;
    bool fftscroll_avg = true;
    int stereo_mode = 0;
    ImVec2 gui_padding{8, 8};
    float gui_seperator = 2;

    float fft_min = -66, fft_max = -12;

    float l[WAVE_SAMPLES_MAX], r[WAVE_SAMPLES_MAX], lr[WAVE_SAMPLES_MAX];

    wave_scrolltexture wave_scrolltexture1;
    fft_scrolltexture fft_scrolltexture1;

    while (!WindowShouldClose() && !(IsKeyDown(KEY_W) && (IsKeyDown(KEY_LEFT_SUPER) | IsKeyDown(KEY_LEFT_CONTROL)))) {
        if (IsKeyPressed(KEY_L)) audioSource.config.enableLoopback ^= 1;
        if (IsKeyPressed(KEY_P)) pause ^= 1;
        if (IsKeyPressed(KEY_S)) settings ^= 1;
        if (IsKeyPressed(KEY_D)) {
            ImGui::LoadIniSettingsFromDisk(fs::path(path_res).append("imgui_default.ini").c_str());
        }

        if (IsWindowHidden()) {
            WaitTime(4);
        }

        int freshSamples = 0;
        if (!pause) {
            freshSamples = soundbuffer.getlr(l, r, wave_samples);
        }

        iTime += GetFrameTime();
        BeginDrawing();
        ClearBackground(BLACK);
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
        // DrawLineEx({150, 150}, {250, 150}, 5, WHITE);

        rlImGuiBegin();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, gui_padding);          // default is 8,8
        ImGui::PushStyleVar(ImGuiStyleVar_DockingSeparatorSize, gui_seperator); // default is 2
        ImGui::DockSpaceOverViewport(0, NULL, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar);

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
            BeginScissorMode(SP_RECT(b1));
            wave_line(b1, l, wave_samples, b1.width, wave_fill, wave_outline);
            BeginScissorMode(SP_RECT(b2));
            wave_line(b2, r, wave_samples, b2.width, wave_fill, wave_outline);
            EndScissorMode();
            ImGui::End();
        }

        if (xy) {
            ImGui::SetNextWindowBgAlpha(0.0f);
            ImGui::Begin("xy", &xy, 0);
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
            Rectangle b = {canvas_p0.x, canvas_p0.y, canvas_sz.x, canvas_sz.y};
            BeginScissorMode(SP_RECT(b));
            xy_line(b, l, r, wave_samples);
            EndScissorMode();
            ImGui::End();
        }

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

        if (fft) {
            ImGui::SetNextWindowBgAlpha(0.0f);
            ImGui::Begin("fft", &fft, 0);
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
            Rectangle b = {canvas_p0.x, canvas_p0.y, canvas_sz.x, canvas_sz.y};

            fftPostprocessorConti.process(fftProcessor.getOutput());
            float my_fft_min = fft_min, my_fft_max = fft_max;
            if (!fftPostprocessorConti.config.scaling.mag2db) {
                my_fft_min = powf(10, fft_min / 20);
                my_fft_max = powf(10, fft_max / 20) / 2;
            }

            BeginScissorMode(SP_RECT(b));
            fft_conti(b, fftPostprocessorConti.getOutput(), fftPostprocessorConti.getOutputSize(), wave_fill, wave_outline, fft_logspacing, fft_colormode, my_fft_min, my_fft_max);
            EndScissorMode();
            ImGui::End();
        }

        if (fftscroll) {
            ImGui::SetNextWindowBgAlpha(0.0f);
            ImGui::Begin("fftscroll", &fftscroll, 0);
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
            Rectangle b = {canvas_p0.x, canvas_p0.y, canvas_sz.x, canvas_sz.y};

            fftPostprocessorScroll.process(fftProcessor.getOutput());
            float my_fft_min = fft_min, my_fft_max = fft_max;
            if (!fftPostprocessorScroll.config.scaling.mag2db) {
                my_fft_min = powf(10, fft_min / 20);
                my_fft_max = powf(10, fft_max / 20) / 2;
            }

            // BeginScissorMode(SP_RECT(b));
            fft_scrolltexture1.draw(b, fftPostprocessorScroll.getOutput(), fftPostprocessorScroll.getOutputSize(), fft_logspacing, wavescroll_colorscale, fftscroll_lerp, wavescroll_scroll, fftscroll_avg, my_fft_min, my_fft_max);
            // EndScissorMode();
            ImGui::End();
        }

        if (wavescroll) {
            ImGui::SetNextWindowBgAlpha(0.0f);
            ImGui::Begin("wavescroll", &xy, 0);
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
            Rectangle b = {canvas_p0.x, canvas_p0.y, canvas_sz.x, canvas_sz.y};
            // BeginScissorMode(SP_RECT(b));
            wave_scrolltexture1.draw(b, l, wave_samples, wavescroll_scroll);
            // EndScissorMode();
            ImGui::End();
        }

        if (wavewindowed) {
            ImGui::SetNextWindowBgAlpha(0.0f);
            ImGui::Begin("wavewindowed", &wavewindowed, 0);
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
            Rectangle b = {canvas_p0.x, canvas_p0.y, canvas_sz.x, canvas_sz.y};

            rlDisableBackfaceCulling();
            BeginScissorMode(SP_RECT(b));
            wave_line(b, fftProcessor.getTimeWindowed(), fftProcessor.getTimeSize(), b.width, wave_fill, wave_outline);
            EndScissorMode();
            ImGui::End();
        }
        ImGui::PopStyleVar(2);

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
                    ImGui::Checkbox("pause", &pause);
                    ImGui::Checkbox("wave", &wave);
                    ImGui::SameLine();
                    ImGui::Checkbox("wave_outline", &wave_outline);
                    ImGui::SameLine();
                    ImGui::Checkbox("wave_fill", &wave_fill);
                    ImGui::Checkbox("wavewindowed", &wavewindowed);
                    ImGui::Checkbox("wavescroll", &wavescroll);
                    ImGui::SameLine();
                    ImGui::Checkbox("wavescroll_scroll", &wavescroll_scroll);
                    ImGui::Checkbox("xy", &xy);
                    ImGui::Checkbox("fft", &fft);
                    ImGui::Checkbox("fftscroll", &fftscroll);
                    ImGui::SameLine();
                    ImGui::Checkbox("fftscroll_lerp", &fftscroll_lerp);
                    ImGui::SameLine();
                    ImGui::Checkbox("fftscroll_avg", &fftscroll_avg);
                    ImGui::Checkbox("fft_logspacing", &fft_logspacing);
                    ImGui::SliderInt("fft_colormode", &fft_colormode, 0, 1);

                    float minmax[2] = {fft_min, fft_max};
                    ImGui::SliderFloat2("fft_min/max", (float*)&minmax, -100, 0, "%.0f");
                    fft_min = minmax[0];
                    fft_max = minmax[1];

                    // ImGui::SliderFloat("fft_max", &fft_max, -100, 0, "%.0f");
                    // ImGui::SliderInt("wavescroll_colorscale", &wavescroll_colorscale, 0, 14);
                    ImGui::SliderInt("wavescroll_colorscale", &wavescroll_colorscale, 0, IM_ARRAYSIZE(colorscale_names) - 1, colorscale_names[wavescroll_colorscale]);

                    ImGui::SliderFloat("draw_line_width", &draw_line_width, 0.1, 8, "%.1f", ImGuiSliderFlags_Logarithmic);

                    ImGui::SliderFloat("gui_padding", &gui_padding.x, 0.0f, 20.0f, "%.0f");
                    gui_padding.y = gui_padding.x;
                    ImGui::SliderFloat("gui_seperator", &gui_seperator, 0.0f, 6.0f, "%.0f");

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

                            TraceLog(LOG_INFO, "reload fftpp %d %d", SAMPLERATE, fftProcessor.getOutputSize());
                            fftPostprocessorConti.deallocate();
                            fftPostprocessorScroll.deallocate();
                            fftPostprocessorConti.allocate(SAMPLERATE, fftProcessor.getOutputSize());
                            fftPostprocessorScroll.allocate(SAMPLERATE, fftProcessor.getOutputSize());
                        }

                        ImGui::SliderInt("stereo_mode", &stereo_mode, 0, 2, (const char*[]){"fft(l)+fft(r)", "fft(l+r)", "fft(l-r)"}[stereo_mode]);

                        ImGui::SliderFloat("slope", &fftProcessor.config.slope, 0, 6, "%.0f", ImGuiSliderFlags_None);
                        ImGui::TreePop();
                    }
                    int ppNr = 0;
                    for (FftPostprocessor* pp : pps) {
                        ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_None;
                        if (ppNames[ppNr][0] == '+') flag = ImGuiTreeNodeFlags_DefaultOpen;
                        if (ImGui::TreeNodeEx(ppNames[ppNr] + 1, flag)) {
                            ImGui::Text("OutputSize %d", (int)pp->getOutputSize());
                            ImGui::SliderFloat("alphaUp", &pp->config.smoothing.alphaUp, 0, 1, "%.1f", ImGuiSliderFlags_None);
                            ImGui::SliderFloat("alphaDn", &pp->config.smoothing.alphaDn, 0, 1, "%.1f", ImGuiSliderFlags_None);
                            ImGui::SliderFloat("minDbClamp", &pp->config.smoothing.minDbClamp, -99, 0, "%.1f", ImGuiSliderFlags_None);
                            ImGui::SliderFloat("decay", &pp->config.smoothing.decay, -0.0001f, 0.005f, "%.4f", ImGuiSliderFlags_None);
                            ImGui::SliderInt("blurringPasses", &pp->config.smoothing.blurringPasses, 0, 20);

                            ImGui::Checkbox("logbinning", (bool*)&pp->config.binning.logbinning);
                            ImGui::Checkbox("removeBaselineOffset", (bool*)&pp->config.folding.removeBaselineOffset);
                            ImGui::Checkbox("mag2db", (bool*)&pp->config.scaling.mag2db);

                            ImGui::TreePop();
                        }
                        ppNr++;
                    }
                    ImGui::TreePop();
                }
                draw_window(argc, argv);
                draw_perf(freshSamples);

                ImGui::Separator();
                ImGui::Checkbox("ImGui::ShowDemoWindow", &imgui_ShowDemoWindow);
                ImGui::Checkbox("ImPlot::ShowDemoWindow", &implot_ShowDemoWindow);
                ImGui::Checkbox("ImGui::ShowStyleEditor", &imgui_ShowStyleEditor);
            }
            ImGui::End();
        }

        rlImGuiEnd();

        if (settings)
            DrawFPS(10, 10);

        EndDrawing();
    }

    ImPlot::DestroyContext();
    rlImGuiShutdown();
    CloseWindow();

    return 0;
}