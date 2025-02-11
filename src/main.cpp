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

#define SAMPLERATE (44100)

int main(int argc, char* argv[]) {
    int screenWidth = 1280;
    int screenHeight = 800;

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "raylib-Extras [ImGui] example - Docking");
    SetTargetFPS(30);
    rlImGuiSetup(true);

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    Ringbuffer soundbuffer(65536);
    AudioSourcePA audioSource(&soundbuffer, SAMPLERATE);

    while (!WindowShouldClose() && !(IsKeyDown(KEY_W) && (IsKeyDown(KEY_LEFT_SUPER) | IsKeyDown(KEY_LEFT_CONTROL)))) {
        BeginDrawing();
        rlImGuiBegin();

        ClearBackground(DARKGRAY);
        DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2, GetScreenHeight() * 0.45f, DARKGREEN);

        ImGui::DockSpaceOverViewport(0, NULL, ImGuiDockNodeFlags_PassthruCentralNode);

        static bool showDemoWindow = true;
        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);

        if (ImGui::Begin("Test Window")) {
            ImGui::TextUnformatted("Another window");
        }
        ImGui::End();

        rlImGuiEnd();
        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}