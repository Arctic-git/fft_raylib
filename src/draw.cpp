#include "draw.h"
#include "raymath.h"
#include "rlFixes.h"
#include "rlgl.h"
#include <algorithm>

#define WAVE_WIDTH_MAX (1024 * 16)
uint8_t lwmin[WAVE_WIDTH_MAX];
uint8_t lwmax[WAVE_WIDTH_MAX];

void wave_line(Rectangle b, uint8_t* l, int samples, int wavebins, bool wave_fill, bool wave_outline) {

    int rpos = 0;
    int error_acc = 0;

    for (int i = 0; i < wavebins; i++) {
        int cperbin = (samples + error_acc) / wavebins;
        error_acc += (samples - cperbin * wavebins);

        lwmin[i] = 255;
        lwmax[i] = 0;
        for (int s = 0; s < cperbin; s++) {
            lwmax[i] = std::max(lwmax[i], l[rpos]);
            lwmin[i] = std::min(lwmin[i], l[rpos]);
            rpos++;
        }
    }

    if (wave_fill) {
        rlBegin(RL_QUADS);
        for (int i = 0; i < wavebins - 1; i++) {
            Vector2 v1 = {
                b.x + (float)i / (wavebins - 1) * b.width,
                b.y + b.height * ((float)lwmin[i] / 255),
            };
            Vector2 v2 = {
                (float)i / (wavebins - 1) * b.width,
                b.y + b.height * ((float)lwmax[i] / 255),
            };
            Vector2 v3 = {
                (float)(i + 1) / (wavebins - 1) * b.width,
                b.y + b.height * ((float)lwmax[i + 1] / 255),
            };
            Vector2 v4 = {
                (float)(i + 1) / (wavebins - 1) * b.width,
                b.y + b.height * ((float)lwmin[i + 1] / 255),
            };

            Color color = WHITE;

            rlColor4ub(color.r, color.g, color.b, color.a);
            rlVertex2f(v1.x, v1.y);
            rlVertex2f(v2.x, v2.y);
            rlVertex2f(v3.x, v3.y);
            rlVertex2f(v4.x, v4.y);
        }
        rlEnd();
    }

    if (wave_outline) {
        for (int i = 0; i < wavebins - 1; i++) {
            Vector2 v_1 = {
                b.x + (float)i / (wavebins - 1) * b.width,
                b.y + b.height * ((float)(lwmin[i]) / 255),
            };
            Vector2 v_2 = {
                b.x + (float)(i + 1) / (wavebins - 1) * b.width,
                b.y + b.height * ((float)(lwmin[i + 1]) / 255),
            };

            DrawLineEx(v_1, v_2, 2, WHITE);
            // DrawCircleV(v_1, 1, WHITE);
        }
        for (int i = 0; i < wavebins - 1; i++) {
            Vector2 v_1 = {
                b.x + (float)i / (wavebins - 1) * b.width,
                b.y + b.height * ((float)(lwmax[i]) / 255),
            };
            Vector2 v_2 = {
                b.x + (float)(i + 1) / (wavebins - 1) * b.width,
                b.y + b.height * ((float)(lwmax[i + 1]) / 255),
            };

            DrawLineEx(v_1, v_2, 2, WHITE);
            // DrawCircleV(v_1, 1, WHITE);
        }
    }
}