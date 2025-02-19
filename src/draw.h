#ifndef __DRAW_H__
#define __DRAW_H__

#include "raylib.h"
#include <stdint.h>

void wave_line(Rectangle b, float* l, int samples, int wavebins, bool wave_fill, bool wave_outline);
void fft_conti(Rectangle b, float* f, int samples, bool wave_fill, bool wave_outline, bool logspacing, int colormode, float min=-66, float max=-12);

extern float draw_line_width;

#endif // __DRAW_H__