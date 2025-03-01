#ifndef __RLFIXES_H__
#define __RLFIXES_H__

#if defined(__cplusplus)
extern "C" { // Prevents name mangling of functions
#endif

#include "raylib.h"

extern bool DrawLineExQuadsDc_expand;

void DrawRectanglePro2(Rectangle rec, Vector2 origin, float rotation, Color color);
void DrawLineExQuads(Vector2 startPos, Vector2 endPos, float thick, Color color);
void DrawLineExQuadsDc(Vector2 startPos, Vector2 endPos, float thick, Color color, Color color2);
void DrawLineExQuadsTexturecoordsLengthembed(Vector2 startPos, Vector2 endPos, float thick, Color color);

#if defined(__cplusplus)
}
#endif

#endif // __RLFIXES_H__