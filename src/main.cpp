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
#define SP_RECT(b) b.x, b.y, b.width, b.height

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    int screenWidth = 800;
    int screenHeight = 600;

    Ringbuffer soundbuffer(1024 * 256);
    AudioSourcePA audioSource(&soundbuffer, SAMPLERATE);
    FftProcessor fftProcessor(WAVE_WIDTH, std::max(32768 / 2, WAVE_WIDTH) / WAVE_WIDTH);
    fftProcessor.updateWindow(1);
    FftPostprocessor fftPostprocessorConti(SAMPLERATE, fftProcessor.getOutputSize());
    fftPostprocessorConti.config.smoothing.alphaDn = fftPostprocessorConti.config.smoothing.alphaUp = 0.2;
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
    bool windowed = false;
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

        int freshSamples = 0;
        if (!pause) {
            freshSamples = soundbuffer.getlr(l, r, WAVE_WIDTH);
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
        //     DrawTexturePro(texture_w, (Rectangle){0, 0, WAVE_WIDTH, 1}, (Rectangle){0, 0, w, h / 2}, (Vector2){0, 0}, 0, WHITE);
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
            wave_line(b1, l, WAVE_WIDTH, b1.width, wave_fill, wave_outline);
            BeginScissorMode(SP_RECT(b2));
            wave_line(b2, r, WAVE_WIDTH, b2.width, wave_fill, wave_outline);
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
            for (int i = 0; i < WAVE_WIDTH - 1; i++) {
                Vector2 v_1 = {
                    b.x + b.width / 2 + b.width / 2 * l[i],
                    b.y + b.height / 2 + b.height / 2 * r[i],
                };
                Vector2 v_2 = {
                    b.x + b.width / 2 + b.width / 2 * l[i + 1],
                    b.y + b.height / 2 + b.height / 2 * r[i + 1],
                };
                DrawLineEx(v_1, v_2, draw_line_width, WHITE);
                // DrawCircleV(v_1, 1, WHITE);
            }
            EndScissorMode();
            ImGui::End();
        }

        if (fft) {
            ImGui::SetNextWindowBgAlpha(0.0f);
            ImGui::Begin("fft", &fft, 0);
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
            Rectangle b = {canvas_p0.x, canvas_p0.y, canvas_sz.x, canvas_sz.y};

            fftProcessor.process(l, r);
            fftPostprocessorConti.process(fftProcessor.getOutput());
            float fft_min = -66, fft_max = -12;
            if (!fftPostprocessorConti.config.scaling.mag2db) {
                fft_min = powf(10, fft_min / 20);
                fft_max = powf(10, fft_max / 20) / 2;
            }
            BeginScissorMode(SP_RECT(b));
            fft_conti(b, fftPostprocessorConti.getOutput(), fftPostprocessorConti.getOutputSize(), wave_fill, wave_outline, fft_logspacing, fft_colormode, fft_min, fft_max);
            EndScissorMode();
            ImGui::End();
        }

        if (windowed) {
            ImGui::SetNextWindowBgAlpha(0.0f);
            ImGui::Begin("windowed", &windowed, 0);
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
            Rectangle b = {canvas_p0.x, canvas_p0.y, canvas_sz.x, canvas_sz.y};

            rlDisableBackfaceCulling();
            BeginScissorMode(SP_RECT(b));
            wave_line(b, fftProcessor.getTimeWindowed(), fftProcessor.getTimeSize(), b.width, wave_fill, wave_outline);
            EndScissorMode();
            ImGui::End();
        }

        if (settings) {
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
                    ImGui::Checkbox("windowed", &windowed);
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
                draw_window(argc, argv);
                draw_perf(freshSamples);

                ImGui::Separator();
                ImGui::Checkbox("ImGui::ShowDemoWindow", &imgui_ShowDemoWindow);
                ImGui::Checkbox("ImPlot::ShowDemoWindow", &implot_ShowDemoWindow);
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