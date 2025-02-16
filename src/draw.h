#ifndef __DRAW_H__
#define __DRAW_H__

#include "raylib.h"
#include <stdint.h>

void wave_line(Rectangle b, uint8_t* l, int samples, int wavebins, bool wave_fill, bool wave_outline);

#endif // __DRAW_H__