#ifndef __DRAW_H__
#define __DRAW_H__

#include "raylib.h"
#include <stdint.h>

extern const char* colorscale_names[19];

void wave_line(Rectangle b, float* l, int samples, int wavebins, bool wave_fill, bool wave_outline);

class wave_scrolltexture {
public:
    wave_scrolltexture() = default;
    void draw(Rectangle b, float* l, int samples, bool scroll);

private:
    int x = -1;
    uint8_t* data_update = 0;
    Shader shader = {};
    int xScrollOffs_location = 0, yScrollOffs_location = 0;
    Image img = {.data = 0, .width=0, .height=0, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8};
    Texture2D texture;
};

class fft_scrolltexture {
public:
    fft_scrolltexture() = default;
    void draw(Rectangle b, float* f, int samples, float minFreq, float maxFreq, bool logspacing, int colorscale, bool scroll, float min = -66, float max = -12, float y_multi = 1);

private:
    int y = 1;
    uint8_t* data_update = 0;
    Shader shader = {};
    int xScrollOffs_location = 0, yScrollOffs_location = 0;
    Image img = {.data = 0, .width=0, .height=0, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8};
    Texture2D texture;
};

void xy_line(Rectangle b, float* l, float* r, int samples, Color c);
void xy_osc(Rectangle b, float* l, float* r, int samples, Color c, float iSize, float iIntensity, float iLenDarken);


// void fft_conti(Rectangle b, float* f, int samples, bool wave_fill, bool wave_outline, int colormode, float min = -66, float max = -12);
void fft_conti2(Rectangle b, float* f, int samples, float minFreq, float maxFreq, bool logspacing, bool wave_fill, bool wave_outline, int colormode, float min = -66, float max = -12);

extern float draw_line_width;

#endif // __DRAW_H__