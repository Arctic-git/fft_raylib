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
#include "rlFixes.h"
#include "rlImGui.h"
#include "rlgl.h"
#include <chrono>
#include <filesystem>
#include <print>
#include <thread>

#define SAMPLERATE (44100)
#define WAVE_WIDTH (1024 * 2)

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    int screenWidth = 800;
    int screenHeight = 600;

    Ringbuffer soundbuffer(1024 * 256);
    AudioSourcePA audioSource(&soundbuffer, SAMPLERATE);
    FftProcessor fftProcessor(WAVE_WIDTH, std::max(32768 / 2, WAVE_WIDTH) / WAVE_WIDTH);
    fftProcessor.updateWindow(1);
    FftPostprocessor fftPostprocessorConti(SAMPLERATE, fftProcessor.getOutputSize());
    FftPostprocessor* pps[] = {&fftPostprocessorConti};
    const char* ppNames[] = {"+fftPostprocessorConti"};

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE); // | FLAG_WINDOW_UNDECORATED); //| FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "fft_raylib");
    // ClearWindowState(FLAG_WINDOW_RESIZABLE);
    // SetWindowState(FLAG_WINDOW_RESIZABLE);
    // std::print("{} {} {}\n", GetCurrentMonitor(), GetMonitorPosition(GetCurrentMonitor()).x, GetMonitorPosition(GetCurrentMonitor()).y);
    SetWindowPosition(GetMonitorPosition(GetCurrentMonitor()).x, GetMonitorPosition(GetCurrentMonitor()).y);

    SetTargetFPS(60);
    rlImGuiSetup(true);
    ImPlot::CreateContext();

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    Shader shader = LoadShader(PATH_RESOURCES "custom.vs", PATH_RESOURCES "custom.fs");

    // remove text optimisation
    Texture2D texture = {1, 1, 1, 1, 7};
    SetShapesTexture(texture, (Rectangle){0.0f, 0.0f, 1.0f, 1.0f});

    float l[WAVE_WIDTH];
    float r[WAVE_WIDTH];

    for (int i = 0; i < WAVE_WIDTH; i++) {
        l[i] = i * 255 / (WAVE_WIDTH - 1);
    }

    // Image imBlank = GenImageColor(1, 1, BLANK);
    Image im_w = {
        .data = l,
        .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
        .height = 1,
        .width = WAVE_WIDTH,
        .mipmaps = 1,
    };
    Texture2D texture_w = LoadTextureFromImage(im_w); // Load blank texture to fill on shader
    SetTextureFilter(texture_w, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(texture_w, TEXTURE_WRAP_CLAMP);
    float iTime = 0;
    bool pause = false;
    bool settings = false;

    bool wave = true;
    bool wave_outline = true;
    bool wave_fill = true;
    bool xy = true;
    bool fft = true;
    bool fft_logspacing = true;
    int fft_colormode = 0;

    while (!WindowShouldClose() && !(IsKeyDown(KEY_W) && (IsKeyDown(KEY_LEFT_SUPER) | IsKeyDown(KEY_LEFT_CONTROL)))) {
        if (IsKeyPressed(KEY_L)) audioSource.config.enableLoopback ^= 1;
        if (IsKeyPressed(KEY_P)) pause ^= 1;
        if (IsKeyPressed(KEY_S)) settings ^= 1;

        iTime += GetFrameTime();

        BeginDrawing();

        // ClearBackground(DARKGRAY);
        // DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2, GetScreenHeight() * 0.45f, DARKGREEN);
        ClearBackground(BLACK);

        float w = GetScreenWidth();
        float h = GetScreenHeight();
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

        int freshSamples = 0;
        if (!pause) {
            freshSamples = soundbuffer.getlr(l, r, WAVE_WIDTH);
        }

        // UpdateTexture(texture_w, lu8);
        // Shader shader_wave = LoadShader(0, PATH_RESOURCES "wave.fs");
        // if (IsShaderValid(shader_wave)) {
        //     BeginShaderMode(shader_wave);
        //     DrawTexturePro(texture_w, (Rectangle){0, 0, WAVE_WIDTH, 1}, (Rectangle){0, 0, w, h / 2}, (Vector2){0, 0}, 0, WHITE);
        //     EndShaderMode();
        // }
        // UnloadShader(shader_wave);

        if (wave) {
            rlDisableBackfaceCulling();
            wave_line({0, 0, w, h / 4}, l, WAVE_WIDTH, w, wave_fill, wave_outline);
            wave_line({0, h / 8, w, h / 4}, r, WAVE_WIDTH, w, wave_fill, wave_outline);
        }

        if (xy) {
            float gain = 1;
            for (int i = 0; i < WAVE_WIDTH - 1; i++) {
                Vector2 v_1 = {
                    w / 2 + h / 2 * ((float)l[i]) * gain,
                    h / 2 + h / 2 * ((float)r[i]) * gain,
                };
                Vector2 v_2 = {
                    w / 2 + h / 2 * ((float)l[i + 1]) * gain,
                    h / 2 + h / 2 * ((float)r[i + 1]) * gain,
                };
                DrawLineEx(v_1, v_2, draw_line_width, WHITE);
                // DrawCircleV(v_1, 1, WHITE);
            }
        }

        if (fft) {
            fftProcessor.process(l, r);
            fftPostprocessorConti.process(fftProcessor.getOutput());
            fft_conti({0, h / 2, w, h / 2}, fftPostprocessorConti.getOutput(), fftPostprocessorConti.getOutputSize(), wave_fill, wave_outline, fft_logspacing, fft_colormode);
        }

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

        if (settings) {
            rlImGuiBegin();
            ImGui::DockSpaceOverViewport(0, NULL, ImGuiDockNodeFlags_PassthruCentralNode);
            static bool imgui_ShowDemoWindow = 0, implot_ShowDemoWindow = 0;
            if (imgui_ShowDemoWindow)
                ImGui::ShowDemoWindow(&imgui_ShowDemoWindow);
            if (implot_ShowDemoWindow)
                ImPlot::ShowDemoWindow(&implot_ShowDemoWindow);

            if (ImGui::Begin("Settings")) {
                draw_audiosource(audioSource);
                if (ImGui::TreeNodeEx("Drawing", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed)) {
                    ImGui::Checkbox("pause", &pause);
                    ImGui::Checkbox("wave", &wave);
                    ImGui::Checkbox("wave_outline", &wave_outline);
                    ImGui::SameLine();
                    ImGui::Checkbox("wave_fill", &wave_fill);
                    ImGui::Checkbox("xy", &xy);
                    ImGui::Checkbox("fft", &fft);
                    ImGui::Checkbox("fft_logspacing", &fft_logspacing);
                    ImGui::SliderInt("fft_colormode", &fft_colormode, 0, 1);
                    ImGui::SliderFloat("draw_line_width", &draw_line_width, 0.1, 8, "%.1f", ImGuiSliderFlags_Logarithmic);

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
                draw_window();
                draw_perf(freshSamples);

                ImGui::Separator();
                ImGui::Checkbox("ImGui::ShowDemoWindow", &imgui_ShowDemoWindow);
                ImGui::Checkbox("ImPlot::ShowDemoWindow", &implot_ShowDemoWindow);
            }
            ImGui::End();
            rlImGuiEnd();
        }

        DrawFPS(10, 10);

        EndDrawing();
    }

    ImPlot::DestroyContext();
    rlImGuiShutdown();
    CloseWindow();

    return 0;
}