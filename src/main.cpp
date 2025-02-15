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
#include "imgui.h"
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"
#include "rlgl.h"
#include <chrono>
#include <filesystem>
#include <print>
#include <thread>

#define SAMPLERATE (44100)
#define WAVE_WIDTH 256

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    int screenWidth = 400;
    int screenHeight = 400;

    Ringbuffer soundbuffer(65536);
    AudioSourcePA audioSource(&soundbuffer, SAMPLERATE);

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | 0);
    InitWindow(screenWidth, screenHeight, "raylib-Extras [ImGui] example - Docking");
    SetTargetFPS(60);
    rlImGuiSetup(true);

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    Shader shader = LoadShader(PATH_RESOURCES "custom.vs", PATH_RESOURCES "custom.fs");

    // remove text optimisation
    Texture2D texture = {1, 1, 1, 1, 7};
    SetShapesTexture(texture, (Rectangle){0.0f, 0.0f, 1.0f, 1.0f});

    uint8_t l[WAVE_WIDTH];
    uint8_t r[WAVE_WIDTH];

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

    while (!WindowShouldClose() && !(IsKeyDown(KEY_W) && (IsKeyDown(KEY_LEFT_SUPER) | IsKeyDown(KEY_LEFT_CONTROL)))) {
        if (IsKeyPressed(KEY_L)) audioSource.config.enableLoopback ^= 1;
        if (IsKeyPressed(KEY_P)) pause ^= 1;
        if (IsKeyPressed(KEY_S)) settings ^= 1;
        iTime += GetFrameTime();

        BeginDrawing();

        ClearBackground(DARKGRAY);
        DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2, GetScreenHeight() * 0.45f, DARKGREEN);

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

        if (!pause) {
            soundbuffer.getlru8(l, r, WAVE_WIDTH);
            UpdateTexture(texture_w, l);
        }

        Shader shader_wave = LoadShader(0, PATH_RESOURCES "wave.fs");
        if (IsShaderValid(shader_wave)) {
            BeginShaderMode(shader_wave);
            DrawTexturePro(texture_w, (Rectangle){0, 0, WAVE_WIDTH, 1}, (Rectangle){0, 0, w, h}, (Vector2){0, 0}, 0, WHITE);
            EndShaderMode();
        }
        UnloadShader(shader_wave);

        if (settings) {
            rlImGuiBegin();
            ImGui::DockSpaceOverViewport(0, NULL, ImGuiDockNodeFlags_PassthruCentralNode);
            static bool showDemoWindow = true;
            if (showDemoWindow)
                ImGui::ShowDemoWindow(&showDemoWindow);

            if (ImGui::Begin("Test Window")) {
                ImGui::TextUnformatted("Another window");
            }
            ImGui::End();
            rlImGuiEnd();
        }

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}