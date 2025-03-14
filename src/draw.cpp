#include "draw.h"
#include "FftPostprocessor.h"
#include "raymath.h"
#include "rlFixes.h"
#include "rlgl.h"
#include "tinycolormap.hpp"
#include <algorithm>
#include <assert.h>
#include <filesystem>

#define WAVE_BINS_MAX (1024 * 16)
float lwmin_buf[WAVE_BINS_MAX];
float lwmax_buf[WAVE_BINS_MAX];

float draw_line_width = 1.5;
namespace fs = std::filesystem;
extern fs::path path_res;

void wave_line(Rectangle b, float* l, int samples, int wavebins, bool wave_fill, bool wave_outline) {
    int rpos = 0;
    int error_acc = 0;

    float* lwmax = lwmax_buf;
    float* lwmin = lwmin_buf;

    if (samples > wavebins) {
        for (int i = 0; i < wavebins; i++) {
            int cperbin = (samples + error_acc) / wavebins;
            error_acc += (samples - cperbin * wavebins);

            lwmin[i] = 1000;
            lwmax[i] = -1000;
            for (int s = 0; s < cperbin; s++) {
                lwmax[i] = std::max(lwmax[i], l[rpos]);
                lwmin[i] = std::min(lwmin[i], l[rpos]);
                rpos++;
            }
        }
    } else {
        lwmax = l;
        lwmin = l;
        wavebins = samples;
    }

    for (int i = 0; i < wavebins; i++) {
        lwmin[i] = Clamp(lwmin[i], -1, 1);
        lwmax[i] = Clamp(lwmax[i], -1, 1);
    }

    if (wave_fill) {
        rlBegin(RL_QUADS);
        for (int i = 0; i < wavebins - 1; i++) {
            Vector2 v1 = {
                b.x + b.width * (1 - (float)i / (wavebins - 1)),
                b.y + b.height / 2,
            };
            Vector2 v2 = {
                b.x + b.width * (1 - (float)i / (wavebins - 1)),
                b.y + b.height / 2 - b.height / 2 * lwmax[i],
            };
            Vector2 v3 = {
                b.x + b.width * (1 - (float)(i + 1) / (wavebins - 1)),
                b.y + b.height / 2 - b.height / 2 * lwmax[i + 1],
            };
            Vector2 v4 = {
                b.x + b.width * (1 - (float)(i + 1) / (wavebins - 1)),
                b.y + b.height / 2,
            };

            Color color = WHITE;

            rlColor4ub(color.r, color.g, color.b, 50);
            rlVertex2f(v1.x, v1.y);
            rlColor4ub(color.r, color.g, color.b, 50 + std::abs(lwmax[i]) * 100);
            rlVertex2f(v2.x, v2.y);
            rlColor4ub(color.r, color.g, color.b, 50 + std::abs(lwmax[i + 1]) * 100);
            rlVertex2f(v3.x, v3.y);
            rlColor4ub(color.r, color.g, color.b, 50);
            rlVertex2f(v4.x, v4.y);
        }
        rlEnd();
    }

    if (wave_fill) {
        rlBegin(RL_QUADS);
        for (int i = 0; i < wavebins - 1; i++) {
            Vector2 v1 = {
                b.x + b.width * (1 - (float)i / (wavebins - 1)),
                b.y + b.height / 2 - b.height / 2 * lwmin[i],
            };
            Vector2 v2 = {
                b.x + b.width * (1 - (float)i / (wavebins - 1)),
                b.y + b.height / 2 - b.height / 2 * lwmax[i],
            };
            Vector2 v3 = {
                b.x + b.width * (1 - (float)(i + 1) / (wavebins - 1)),
                b.y + b.height / 2 - b.height / 2 * lwmax[i + 1],
            };
            Vector2 v4 = {
                b.x + b.width * (1 - (float)(i + 1) / (wavebins - 1)),
                b.y + b.height / 2 - b.height / 2 * lwmin[i + 1],
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
                b.x + b.width * (1 - (float)i / (wavebins - 1)),
                b.y + b.height / 2 - b.height / 2 * lwmin[i],
            };
            Vector2 v_2 = {
                b.x + b.width * (1 - (float)(i + 1) / (wavebins - 1)),
                b.y + b.height / 2 - b.height / 2 * lwmin[i + 1],
            };

            DrawLineExQuads(v_1, v_2, draw_line_width, WHITE);
            // DrawCircleV(v_1, 1, WHITE);
        }
        for (int i = 0; i < wavebins - 1; i++) {
            Vector2 v_1 = {
                b.x + b.width * (1 - (float)i / (wavebins - 1)),
                b.y + b.height / 2 - b.height / 2 * lwmax[i],
            };
            Vector2 v_2 = {
                b.x + b.width * (1 - (float)(i + 1) / (wavebins - 1)),
                b.y + b.height / 2 - b.height / 2 * lwmax[i + 1],
            };

            DrawLineExQuads(v_1, v_2, draw_line_width, WHITE);
            // DrawCircleV(v_1, 1, WHITE);
        }
    }
}

Color lerp_color(Color c0, Color c1, float x) {
    Color c = {
        .r = (unsigned char)(c0.r * (1 - x) + c1.r * (x)),
        .g = (unsigned char)(c0.g * (1 - x) + c1.g * (x)),
        .b = (unsigned char)(c0.b * (1 - x) + c1.b * (x)),
        .a = (unsigned char)(c0.a * (1 - x) + c1.a * (x)),
    };
    return c;
}

void wave_scrolltexture::draw(Rectangle b, float* l, int samples, bool scroll) {
    if (b.width <= 0 || b.height <= 0) return;

    if (!shader.id) {
        shader = LoadShader(NULL, fs::path(path_res).append("scroll.fs").string().c_str());
        xScrollOffs_location = GetShaderLocation(shader, "xScrollOffs");
        yScrollOffs_location = GetShaderLocation(shader, "yScrollOffs");
    }

    int width = std::ceilf(b.width);
    int height = std::ceilf(b.height);
    if (!img.data || width != img.width || height != img.height) {
        if (!img.data) {
            // create image
            img.data = (uint8_t*)malloc(width * height * 3);
            img.width = width;
            img.height = height;
        } else {
            int old_width = img.width;
            ImageResize(&img, width, height);
            x = ceil((float)width / old_width * x);
        }

        data_update = (uint8_t*)realloc(data_update, height * 3);
        // memset(data, 155, texture.width * texture.height * 3);

        UnloadTexture(texture);
        texture = LoadTextureFromImage(img);
        SetTextureFilter(texture, TEXTURE_FILTER_POINT);
        // SetTextureWrap(texture, TEXTURE_WRAP_CLAMP);
    }

    float max = l[0];
    float min = l[0];
    float squaresum = 0;
    for (int i = 0; i < samples; i++) {
        float s = l[i];
        if (s > max) max = s;
        if (s < min) min = s;
        squaresum += s * s;
    }
    float rms = sqrtf(squaresum / samples);
    min = Clamp(min, -1, 1);
    max = Clamp(max, -1, 1);
    rms = Clamp(rms, -1, 1);

    // Ableton meter colors 0 to 1
    Color c0rms = {114, 248, 119, 255};
    Color c1rms = {207, 247, 122, 255};
    Color c0 = {64, 136, 74, 255};
    Color c1 = {118, 137, 75, 255};
    // above 1
    Color c2rms = {234, 53, 39, 255};
    Color c3rms = {118, 137, 75, 255};
    Color c2 = {138, 116, 43, 255};
    Color c3 = {129, 35, 29, 255};

    x = (x + 1) % texture.width;
    for (int y = 0; y < texture.height; y++) {
        float y_rel = (1 - (float)y / (texture.height - 1)) * 2 - 1; //-1 to 1
        int r = 0, g = 0, b = 0;
        Color c = {};

        if (y_rel <= max && y_rel >= min) {
            c = lerp_color(c0, c1, std::abs(y_rel));
        }
        if (std::abs(y_rel) <= rms) {
            c = lerp_color(c0rms, c1rms, std::abs(y_rel));
        }

        ((uint8_t*)img.data)[(y * texture.width + x) * 3 + 0] = c.r;
        ((uint8_t*)img.data)[(y * texture.width + x) * 3 + 1] = c.g;
        ((uint8_t*)img.data)[(y * texture.width + x) * 3 + 2] = c.b;
        data_update[y * 3 + 0] = c.r;
        data_update[y * 3 + 1] = c.g;
        data_update[y * 3 + 2] = c.b;
    }

    // UpdateTexture(texture, data);
    UpdateTextureRec(texture, {(float)x, 0.0f, 1.0f, (float)texture.height}, data_update);

    if (IsShaderValid(shader)) {
        float xScrollOffs = scroll ? (float)(x + 1) / texture.width : 0;
        SetShaderValue(shader, xScrollOffs_location, (void*)&xScrollOffs, SHADER_UNIFORM_FLOAT);
        BeginShaderMode(shader);
    }
    DrawTexturePro(texture, {0, 0, (float)texture.width, (float)texture.height}, b, {0, 0}, 0, WHITE);
    EndShaderMode();
}

void xy_line(Rectangle b, float* l, float* r, int samples, Color c) {
    float alpha = c.a;

    for (int i = 0; i < samples - 1; i++) {

        Vector2 s_1 = {
            Clamp(l[i], -1, 1),
            Clamp(r[i], -1, 1),
        };
        Vector2 s_2 = {
            Clamp(l[i + 1], -1, 1),
            Clamp(r[i + 1], -1, 1),
        };
        Vector2 v_1 = {
            b.x + b.width / 2 + b.width / 2 * s_1.x,
            b.y + b.height / 2 - b.height / 2 * s_1.y,
        };
        Vector2 v_2 = {
            b.x + b.width / 2 + b.width / 2 * s_2.x,
            b.y + b.height / 2 - b.height / 2 * s_2.y,
        };

        // if (c.a == 254) {
        float len = Vector2Length({s_2.x - s_1.x, s_2.y - s_1.y});
        c.a = alpha * Clamp(1 - len, 0, 1);
        // }

        // DrawLineEx(v_1, v_2, draw_line_width, WHITE); //6 vertecies per line
        DrawLineExQuads(v_1, v_2, draw_line_width, c); // 4 vertecies per line
        // DrawPixelV(v_1, WHITE);
        // DrawCircleV(v_1, 1, WHITE);
    }
}

void xy_osc(Rectangle b, float* l, float* r, int samples, Color c, float iSize, float iIntensity, float iLenDarken, float iSigma) {
    float alpha = c.a;
    float fcolor[] = {c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f};

    static Shader shader = {};
    if (!IsShaderValid(shader)) {
        UnloadShader(shader);
        shader = LoadShader(NULL, fs::path(path_res).append("oscline.fs").string().c_str());
    }

    if (IsShaderValid(shader)) {
        SetShaderValue(shader, GetShaderLocation(shader, "iSize"), (void*)&iSize, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "iIntensity"), (void*)&iIntensity, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "iSigma"), (void*)&iSigma, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "iLenDarken"), (void*)&iLenDarken, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "iColor"), (void*)&fcolor, SHADER_UNIFORM_VEC4);
        BeginShaderMode(shader);
    }

    BeginBlendMode(BLEND_ADDITIVE);

    for (int i = 0; i < samples - 1; i++) {

        Vector2 s_1 = {
            Clamp(l[i], -1, 1),
            Clamp(r[i], -1, 1),
        };
        Vector2 s_2 = {
            Clamp(l[i + 1], -1, 1),
            Clamp(r[i + 1], -1, 1),
        };
        Vector2 v_1 = {
            b.x + b.width / 2 + b.width / 2 * s_1.x,
            b.y + b.height / 2 - b.height / 2 * s_1.y,
        };
        Vector2 v_2 = {
            b.x + b.width / 2 + b.width / 2 * s_2.x,
            b.y + b.height / 2 - b.height / 2 * s_2.y,
        };

        // if (c.a == 254) {
        float len = Vector2Length({s_2.x - s_1.x, s_2.y - s_1.y});
        c.a = alpha * Clamp(1 - len, 0, 1);
        // }

        DrawLineExQuadsTexturecoordsLengthembed(v_1, v_2, iSize * 2, c); // 4 vertecies per line
    }

    EndBlendMode();
    EndShaderMode();
}

// 0/6: RED, 1/6: YELLOW, 2/6: GREEN, 3/6: CYAN, 4/6: BLUE, 5/6: PURPLE, 6/6: RED
uint32_t HSVtoHEX(float hue, float sat, float value, int gamma2) {
    float pr = 0;
    float pg = 0;
    float pb = 0;
    short ora = 0;
    short og = 0;
    short ob = 0;

    float ro = fmod(hue * 6, 6.);

    float avg = 0;

    ro = fmod(ro + 6 + 1, 6); // Hue was 60* off...

    if (ro < 1) // yellow->red
    {
        pr = 1;
        pg = 1. - ro;
    } else if (ro < 2) {
        pr = 1;
        pb = ro - 1.;
    } else if (ro < 3) {
        pr = 3. - ro;
        pb = 1;
    } else if (ro < 4) {
        pb = 1;
        pg = ro - 3;
    } else if (ro < 5) {
        pb = 5 - ro;
        pg = 1;
    } else {
        pg = 1;
        pr = ro - 5;
    }

    // Actually, above math is backwards, oops!
    pr *= value;
    pg *= value;
    pb *= value;

    avg += pr;
    avg += pg;
    avg += pb;

    pr = pr * sat + avg * (1. - sat);
    pg = pg * sat + avg * (1. - sat);
    pb = pb * sat + avg * (1. - sat);

    if (gamma2) {
        pr = pr * pr;
        pg = pg * pg;
        pb = pb * pb;
    }

    ora = pr * 255;
    og = pb * 255;
    ob = pg * 255;

    if (ora < 0) ora = 0;
    if (ora > 255) ora = 255;
    if (og < 0) og = 0;
    if (og > 255) og = 255;
    if (ob < 0) ob = 0;
    if (ob > 255) ob = 255;

    return (ob << 16) | (og << 8) | ora;
}

// better yellow
uint32_t CCtoHEX(float note, float sat, float value, int gamma2) {
    float hue = 0.0;
    note = fmodf(note, 1.0);
    note *= 12;
    if (note < 4) {
        // Needs to be YELLOW->RED
        hue = (4 - note) / 24.0;
    } else if (note < 8) {
        //            [4]  [8]
        // Needs to be RED->BLUE
        hue = (4 - note) / 12.0;
    } else {
        //             [8] [12]
        // Needs to be BLUE->YELLOW
        hue = (12 - note) / 8.0 + 1. / 6.;
    }
    return HSVtoHEX(hue, sat, value, gamma2);
}

Color hsv(float H, float S, float V) {
    uint32_t rgb = CCtoHEX(1 - H / 360.0 + 2.0 / 6.0, S, V, 0);
    return Color(rgb & 0xff, (rgb >> 8) & 0xff, (rgb >> 16) & 0xff, 255);
}
Color freqToColor(float freq, float sat = 1, float val = 1) {
    float note = 12 * log2f(freq / 440.0); // + 69;
    note = fmodf(note, 12);
    //    return hsv(midiNote/12*360, 0.85, 0.85);
    //    uint32_t rgb = CCtoHEX((midiNote+3)/12, 0.9, 0.8);
    //    return Color(rgb&0xff, (rgb>>8)&0xff, (rgb>>16)&0xff);
    return hsv(note / 12 * 360, sat, val);
}
Color fftColor(float relativex, float freq, int colormode, float sat = 1, float val = 1) {
    if (colormode == 0)
        return hsv(relativex * 360, sat, val);
    else
        return freqToColor(freq, sat, val);
}

Vector2 GetMousePositionRelativeTo(Rectangle b) {
    Vector2 mp = GetMousePosition();
    return {(mp.x - b.x) / b.width, (mp.y - b.y) / b.height};
}

static const char* midiToNote(float midi) {
    const char* notes[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "H"};
    return notes[int(roundf(midi)) % 12];
}

static int midiToNoteOctave(float midi) {
    return floorf(roundf(midi) / 12) - 1;
}

static float freqToMidi(float freq) {
    return 12.0 * log2f(freq / 440.0) + 69.0;
}

static float midiToFreq(float midi) {
    return powf(2, (midi - 69) / 12.0) * 440.0;
}

static const char* freqToNote(float freq) {
    float midiNote = freqToMidi(freq);
    return midiToNote(midiNote);
}

static Color heatmap_white(float value) {
    return hsv(0, 0, value);
}

static Color heatmap_jet(float value) { // jet (blue ... red)
    return hsv(240 - value * 240, 1, 1);
}

static Color heatmap_blackjet(float value) { // jet (black blue ... red)
    float u = 1.0 / 6;

    if (value < u)
        return hsv(240, 1, value / u);
    else
        return hsv(240 - ((value - u) / (1 - u)) * 240, 1, 1);
}

static Color heatmap_gqrx(float value) {
    // level 0: black background
    float i = value * 255;
    if (i < 20)
        return Color(0, 0, 0, 255);
    // level 1: black -> blue
    else if ((i >= 20) && (i < 70))
        return Color(0, 0, 140 * (i - 20) / 50, 255);
    // level 2: blue -> light-blue / greenish
    else if ((i >= 70) && (i < 100))
        return Color(60 * (i - 70) / 30, 125 * (i - 70) / 30, 115 * (i - 70) / 30 + 140, 255);
    // level 3: light blue -> yellow
    else if ((i >= 100) && (i < 150))
        return Color(195 * (i - 100) / 50 + 60, 130 * (i - 100) / 50 + 125, 255 - (255 * (i - 100) / 50), 255);
    // level 4: yellow -> red
    else if ((i >= 150) && (i < 250))
        return Color(255, 255 - 255 * (i - 150) / 100, 0, 255);
    // level 5: red -> white
    else if (i >= 250)
        return Color(255, 255 * (i - 250) / 5, 255 * (i - 250) / 5, 255);
    else
        return Color(0, 0, 0, 255);
}

Color heatmap(float value, int colormode) {
    if (colormode == 0)
        return heatmap_white(value);
    else if (colormode == 1)
        return heatmap_jet(value); // jet
    else if (colormode == 2)
        return heatmap_blackjet(value);
    else if (colormode == 3)
        return heatmap_gqrx(value);
    else {
        const auto color = tinycolormap::GetColor(value, (tinycolormap::ColormapType)(colormode - 4)); // 13 more
        return Color(color.ri(), color.gi(), color.bi(), 255);
    }
}

const char* colorscale_names[19] = {"my_white", "my_jet", "my_blackjet", "gqrx", "Parula", "Heat", "Jet", "Turbo", "Hot", "Gray", "Magma", "Inferno", "Plasma", "Viridis", "Cividis", "Github", "Cubehelix", "MyGqrxt", "MyGqrxt2"};

void draw_mouse_overlay(Rectangle b, float minFreq, float maxFreq, bool logspacing) {
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 mpr = GetMousePositionRelativeTo(b);
        if (mpr.x > 0 && mpr.x < 1 && mpr.y > 0 && mpr.y < 1) {
            float freq = xToFreq(mpr.x, minFreq, maxFreq, logspacing);
            // float amplitude = min + (1 - mpr.y) * (max - min); // data[int(round(bin))];
            float midi = freqToMidi(freq);
            float midiH = roundf(midi) + 0.5;
            float midiL = roundf(midi) - 0.5;

            float px_midiH = freqToX(midiToFreq(midiH), minFreq, maxFreq, logspacing) * b.width + b.x;
            float px_midiL = freqToX(midiToFreq(midiL), minFreq, maxFreq, logspacing) * b.width + b.x;
            DrawRectangle(px_midiL, b.y, px_midiH - px_midiL, b.height, {255, 255, 255, 32});
            DrawLineEx({px_midiL, b.y}, {px_midiL, b.y + b.height}, draw_line_width, WHITE);
            DrawLineEx({px_midiH, b.y}, {px_midiH, b.y + b.height}, draw_line_width, WHITE);

            const char* note = midiToNote(midi);
            int oct = midiToNoteOctave(midi);

            int font_size = 20;
            int font_spacing = 2;
            // const char* text = TextFormat("%.0f Hz %-2.2s%d", freq, note, oct);
            // const char* text = TextFormat("%-2.2s%d %.0f Hz", note, oct, freq);
            const char* text = TextFormat("%.0f Hz\n%-2.2s%d", freq, note, oct);
            Vector2 tb = MeasureTextEx(GetFontDefault(), text, font_size, font_spacing);
            Vector2 pos = Vector2Add(GetMousePosition(), (Vector2){-44, -48});

            DrawRectangleV(pos, tb, {0, 0, 0, 128});
            DrawTextEx(GetFontDefault(), text, pos, font_size, font_spacing, WHITE);
        }
    }
}

// void fft_conti(Rectangle b, float* f, int samples, bool wave_fill, bool wave_outline, bool logspacing, int colormode, float min, float max) {
//     float bw = float(44100 / 2) / (samples - 1);

//     if (wave_fill) {
//         rlBegin(RL_QUADS);
//         for (int i = 0; i < samples - 1; i++) {
//             float fn = (f[i] - min) / (max - min);       // convert range to [0,1]
//             float fn_1 = (f[i + 1] - min) / (max - min); // convert range to [0,1]

//             fn = std::max(0.f, fn);
//             fn_1 = std::max(0.f, fn_1);
//             // fn = std::min(1.f, fn);
//             // fn_1 = std::min(1.f, fn_1);

//             float relativex = freqToX(i * bw, 22, 22050, logspacing);
//             float relativex_1 = freqToX((i + 1) * bw, 22, 22050, logspacing);
//             if (relativex_1 < 0)
//                 continue;

//             Vector2 v1 = {
//                 b.x + b.width * relativex,
//                 b.y + b.height,
//             };
//             Vector2 v2 = {
//                 b.x + b.width * relativex,
//                 b.y + b.height - b.height * fn,
//             };
//             Vector2 v3 = {
//                 b.x + b.width * relativex_1,
//                 b.y + b.height - b.height * fn_1,
//             };
//             Vector2 v4 = {
//                 b.x + b.width * relativex_1,
//                 b.y + b.height,
//             };

//             // Color color = WHITE;
//             Color color = fftColor(relativex_1, (i + 1) * bw, colormode);
//             rlColor4ub(color.r, color.g, color.b, std::min(255.0f, std::max(0.0f, (-255 + 255 * fn_1)))); //-255 + 255*fn_1
//             rlVertex2f(v4.x, v4.y);
//             rlColor4ub(color.r, color.g, color.b, std::min(255.0f, 255 * (fn_1)));
//             rlVertex2f(v3.x, v3.y);
//             color = fftColor(relativex, i * bw, colormode);
//             rlColor4ub(color.r, color.g, color.b, std::min(255.0f, 255 * (fn)));
//             rlVertex2f(v2.x, v2.y);
//             rlColor4ub(color.r, color.g, color.b, std::min(255.0f, std::max(0.0f, (-255 + 255 * fn))));
//             rlVertex2f(v1.x, v1.y);
//         }
//         rlEnd();
//     }

//     if (wave_outline) {
//         for (int i = 0; i < samples - 1; i++) {
//             float fn = (f[i] - min) / (max - min);       // convert range to [0,1]
//             float fn_1 = (f[i + 1] - min) / (max - min); // convert range to [0,1]
//             fn = std::max(-8.f / b.height, fn);
//             fn_1 = std::max(-8.f / b.height, fn_1); // line can be 8 px below (linewidth/2)

//             float relativex = freqToX(i * bw, 22, 22050, logspacing);
//             float relativex_1 = freqToX((i + 1) * bw, 22, 22050, logspacing);
//             if (relativex_1 < 0)
//                 continue;

//             Vector2 v_1 = {
//                 b.x + b.width * relativex,
//                 b.y + b.height - b.height * fn,
//             };
//             Vector2 v_2 = {
//                 b.x + b.width * relativex_1,
//                 b.y + b.height - b.height * fn_1,
//             };

//             // Color color = BLACK;
//             // Color color = hsv(relativex * 360.0, 0.8, 1);
//             Color color = fftColor(relativex, (i)*bw, colormode, 0.8);
//             Color color_1 = fftColor(relativex_1, (i + 1) * bw, colormode, 0.8);
//             DrawLineExDualcolor(v_1, v_2, draw_line_width, color, color_1);
//             // DrawCircleV(v_1, 1, WHITE);
//         }

//         // // hides line on silence
//         // DrawLineEx({b.x, b.y + b.height + 1}, {b.x + b.width, b.y + b.height + 1}, 2, BLACK);
//     }

//     draw_mouse_overlay(b, logspacing);
// }

void fft_conti2(Rectangle b, float* f, int samples, float minFreq, float maxFreq, bool logspacing, bool wave_fill, bool wave_outline, int colormode, float min, float max) {

    if (wave_fill) {
        rlBegin(RL_QUADS);
        for (int i = 0; i < samples - 1; i++) {
            float fn = (f[i] - min) / (max - min);       // convert range to [0,1]
            float fn_1 = (f[i + 1] - min) / (max - min); // convert range to [0,1]

            fn = std::max(0.f, fn);
            fn_1 = std::max(0.f, fn_1);
            // fn = std::min(1.f, fn);
            // fn_1 = std::min(1.f, fn_1);

            float relativex = (float)(i) / (samples - 1);
            float relativex_1 = (float)(i + 1) / (samples - 1);

            Vector2 v1 = {
                b.x + b.width * relativex,
                b.y + b.height,
            };
            Vector2 v2 = {
                b.x + b.width * relativex,
                b.y + b.height - b.height * fn,
            };
            Vector2 v3 = {
                b.x + b.width * relativex_1,
                b.y + b.height - b.height * fn_1,
            };
            Vector2 v4 = {
                b.x + b.width * relativex_1,
                b.y + b.height,
            };

            // Color color = WHITE;
            Color color = fftColor(relativex_1, 0, colormode);
            rlColor4ub(color.r, color.g, color.b, std::min(255.0f, std::max(0.0f, (-255 + 255 * fn_1)))); //-255 + 255*fn_1
            rlVertex2f(v4.x, v4.y);
            rlColor4ub(color.r, color.g, color.b, std::min(255.0f, 255 * (fn_1)));
            rlVertex2f(v3.x, v3.y);
            color = fftColor(relativex, 0, colormode);
            rlColor4ub(color.r, color.g, color.b, std::min(255.0f, 255 * (fn)));
            rlVertex2f(v2.x, v2.y);
            rlColor4ub(color.r, color.g, color.b, std::min(255.0f, std::max(0.0f, (-255 + 255 * fn))));
            rlVertex2f(v1.x, v1.y);
        }
        rlEnd();
    }

    if (wave_outline) {
        for (int i = 0; i < samples - 1; i++) {
            float fn = (f[i] - min) / (max - min);       // convert range to [0,1]
            float fn_1 = (f[i + 1] - min) / (max - min); // convert range to [0,1]
            fn = std::max(-8.f / b.height, fn);
            fn_1 = std::max(-8.f / b.height, fn_1); // line can be 8 px below (linewidth/2)

            float relativex = (float)(i) / (samples - 1);
            float relativex_1 = (float)(i + 1) / (samples - 1);

            Vector2 v_1 = {
                b.x + b.width * relativex,
                b.y + b.height - b.height * fn,
            };
            Vector2 v_2 = {
                b.x + b.width * relativex_1,
                b.y + b.height - b.height * fn_1,
            };

            // Color color = BLACK;
            // Color color = hsv(relativex * 360.0, 0.8, 1);
            Color color = fftColor(relativex, 0, colormode, 0.8);
            Color color_1 = fftColor(relativex_1, 0, colormode, 0.8);
            DrawLineExQuadsDc(v_1, v_2, draw_line_width, color, color_1);
            // DrawCircleV(v_1, 1, WHITE);
        }

        // // hides line on silence
        // DrawLineEx({b.x, b.y + b.height + 1}, {b.x + b.width, b.y + b.height + 1}, 2, BLACK);
    }

    draw_mouse_overlay(b, minFreq, maxFreq, logspacing);
}

void fft_scrolltexture::draw(Rectangle b, float* f, int samples, float minFreq, float maxFreq, bool logspacing, int colorscale, bool scroll, float min, float max, float y_multi) {
    if (b.width <= 0 || b.height <= 0) return;

    if (!shader.id) {
        shader = LoadShader(NULL, fs::path(path_res).append("scroll.fs").string().c_str());
        xScrollOffs_location = GetShaderLocation(shader, "xScrollOffs");
        yScrollOffs_location = GetShaderLocation(shader, "yScrollOffs");
    }

    // image width should be scaled with GetWindowScaleDPI() to get maximum resolution. but makes scroll slower ofc.
    // int width = std::ceilf(b.width);
    int width = samples;
    int height = std::ceil(b.height * y_multi);
    if (!img.data || width != img.width || height != img.height) {
        if (!img.data) {
            // create image
            img.data = (uint8_t*)malloc(width * height * 3);
            img.width = width;
            img.height = height;
        } else {
            int old_height = img.height;
            ImageResize(&img, width, height);
            y = ceil((float)height / old_height * y);
        }

        data_update = (uint8_t*)realloc(data_update, width * 3);
        // memset(data, 155, texture.width * texture.height * 3);

        UnloadTexture(texture);
        texture = LoadTextureFromImage(img);
        SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
        // SetTextureWrap(texture, TEXTURE_WRAP_CLAMP);
    }

    y = (y - 1 + texture.height) % texture.height;

    for (int x = 0; x < texture.width; x++) {
        // int bin = roundf((float)x / (texture.width - 1) * (samples - 1)); // needed if samples != texture width
        int bin = x;
        float fn = (f[bin] - min) / (max - min); // convert range to [0,1]
        // printf("%f\n", f[x]);

        Color c = heatmap(fn, colorscale);
        ((uint8_t*)img.data)[(y * texture.width + x) * 3 + 0] = c.r; // whole image needed for resizing
        ((uint8_t*)img.data)[(y * texture.width + x) * 3 + 1] = c.g;
        ((uint8_t*)img.data)[(y * texture.width + x) * 3 + 2] = c.b;
        data_update[x * 3 + 0] = c.r; // efficient transfer of only one line
        data_update[x * 3 + 1] = c.g;
        data_update[x * 3 + 2] = c.b;
    }

    // UpdateTexture(texture, data);
    UpdateTextureRec(texture, {0.0f, (float)y, (float)texture.width, 1.0f}, data_update);

    if (IsShaderValid(shader)) {
        float yScrollOffs = scroll ? (float)(y) / texture.height : 0;
        SetShaderValue(shader, yScrollOffs_location, (void*)&yScrollOffs, SHADER_UNIFORM_FLOAT);
        BeginShaderMode(shader);
    }
    DrawTexturePro(texture, {0, 0, (float)texture.width, (float)texture.height}, b, {0, 0}, 0, WHITE);
    EndShaderMode();

    draw_mouse_overlay(b, minFreq, maxFreq, logspacing);
}