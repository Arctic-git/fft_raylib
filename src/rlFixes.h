#ifndef __RLFIXES_H__
#define __RLFIXES_H__

#if defined(__cplusplus)
extern "C" { // Prevents name mangling of functions
#endif

#include "raylib.h"

void DrawRectanglePro2(Rectangle rec, Vector2 origin, float rotation, Color color);
void DrawLineExDualcolor(Vector2 startPos, Vector2 endPos, float thick, Color color, Color color2);
void DrawLineEx2(Vector2 startPos, Vector2 endPos, float thick, Color color);
void DrawLineEx3(Vector2 startPos, Vector2 endPos, float thick, Color color);

#if defined(__cplusplus)
}
#endif

#endif // __RLFIXES_H__