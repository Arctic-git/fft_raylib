#include "rlFixes.h"
#include "raymath.h"
#include "rlgl.h"

#define SUPPORT_QUADS_DRAW_MODE 1

// Draw a color-filled rectangle with pro parameters
// mod: added texturecoords for triangle draw mode
void DrawRectanglePro2(Rectangle rec, Vector2 origin, float rotation, Color color) {
    static Texture2D texShapes = {1, 1, 1, 1, 7};
    Vector2 topLeft = {0};
    Vector2 topRight = {0};
    Vector2 bottomLeft = {0};
    Vector2 bottomRight = {0};

    // Only calculate rotation if needed
    if (rotation == 0.0f) {
        float x = rec.x - origin.x;
        float y = rec.y - origin.y;
        topLeft = (Vector2){x, y};
        topRight = (Vector2){x + rec.width, y};
        bottomLeft = (Vector2){x, y + rec.height};
        bottomRight = (Vector2){x + rec.width, y + rec.height};
    } else {
        float sinRotation = sinf(rotation * DEG2RAD);
        float cosRotation = cosf(rotation * DEG2RAD);
        float x = rec.x;
        float y = rec.y;
        float dx = -origin.x;
        float dy = -origin.y;

        topLeft.x = x + dx * cosRotation - dy * sinRotation;
        topLeft.y = y + dx * sinRotation + dy * cosRotation;

        topRight.x = x + (dx + rec.width) * cosRotation - dy * sinRotation;
        topRight.y = y + (dx + rec.width) * sinRotation + dy * cosRotation;

        bottomLeft.x = x + dx * cosRotation - (dy + rec.height) * sinRotation;
        bottomLeft.y = y + dx * sinRotation + (dy + rec.height) * cosRotation;

        bottomRight.x = x + (dx + rec.width) * cosRotation - (dy + rec.height) * sinRotation;
        bottomRight.y = y + (dx + rec.width) * sinRotation + (dy + rec.height) * cosRotation;
    }

#if defined(SUPPORT_QUADS_DRAW_MODE)
    rlSetTexture(GetShapesTexture().id);
    Rectangle shapeRect = GetShapesTextureRectangle();

    rlBegin(RL_QUADS);

    rlNormal3f(0.0f, 0.0f, 1.0f);
    rlColor4ub(color.r, color.g, color.b, color.a);

    rlTexCoord2f(shapeRect.x / texShapes.width, shapeRect.y / texShapes.height);
    rlVertex2f(topLeft.x, topLeft.y);

    rlTexCoord2f(shapeRect.x / texShapes.width, (shapeRect.y + shapeRect.height) / texShapes.height);
    rlVertex2f(bottomLeft.x, bottomLeft.y);

    rlTexCoord2f((shapeRect.x + shapeRect.width) / texShapes.width, (shapeRect.y + shapeRect.height) / texShapes.height);
    rlVertex2f(bottomRight.x, bottomRight.y);

    rlTexCoord2f((shapeRect.x + shapeRect.width) / texShapes.width, shapeRect.y / texShapes.height);
    rlVertex2f(topRight.x, topRight.y);

    rlEnd();

    rlSetTexture(0);
#else
    rlSetTexture(GetShapesTexture().id);
    Rectangle shapeRect = GetShapesTextureRectangle();

    rlBegin(RL_TRIANGLES);

    rlColor4ub(color.r, color.g, color.b, color.a);

    rlTexCoord2f(shapeRect.x / texShapes.width, shapeRect.y / texShapes.height);
    rlVertex2f(topLeft.x, topLeft.y);
    rlTexCoord2f(shapeRect.x / texShapes.width, (shapeRect.y + shapeRect.height) / texShapes.height);
    rlVertex2f(bottomLeft.x, bottomLeft.y);
    rlTexCoord2f((shapeRect.x + shapeRect.width) / texShapes.width, shapeRect.y / texShapes.height);
    rlVertex2f(topRight.x, topRight.y);

    rlVertex2f(topRight.x, topRight.y);
    rlTexCoord2f(shapeRect.x / texShapes.width, (shapeRect.y + shapeRect.height) / texShapes.height);
    rlVertex2f(bottomLeft.x, bottomLeft.y);
    rlTexCoord2f((shapeRect.x + shapeRect.width) / texShapes.width, (shapeRect.y + shapeRect.height) / texShapes.height);
    rlVertex2f(bottomRight.x, bottomRight.y);

    rlEnd();
    rlSetTexture(0);
#endif
}

// Draw a line defining thickness

// mod use quads and texture coordinates
void DrawLineEx2(Vector2 startPos, Vector2 endPos, float thick, Color color) {
    Vector2 delta = {endPos.x - startPos.x, endPos.y - startPos.y};
    float length = sqrtf(delta.x * delta.x + delta.y * delta.y);

    if ((length > 0) && (thick > 0)) {
        float scale = thick / (2 * length);

        Vector2 radius = {-scale * delta.y, scale * delta.x};
        Vector2 strip[4] = {
            {startPos.x - radius.x, startPos.y - radius.y},
            {startPos.x + radius.x, startPos.y + radius.y},
            {endPos.x - radius.x, endPos.y - radius.y},
            {endPos.x + radius.x, endPos.y + radius.y}};

        rlSetTexture(GetShapesTexture().id);
        Rectangle shapeRect = GetShapesTextureRectangle();
        Texture2D texShapes = {1, 1, 1, 1, 7};

        rlBegin(RL_QUADS);
        rlNormal3f(0.0f, 0.0f, 1.0f);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlTexCoord2f(shapeRect.x / texShapes.width, shapeRect.y / texShapes.height);
        rlVertex2f(strip[0].x, strip[0].y);
        rlTexCoord2f(shapeRect.x / texShapes.width, (shapeRect.y + shapeRect.height) / texShapes.height);
        rlVertex2f(strip[1].x, strip[1].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width) / texShapes.width, (shapeRect.y + shapeRect.height) / texShapes.height);
        rlVertex2f(strip[3].x, strip[3].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width) / texShapes.width, shapeRect.y / texShapes.height);
        rlVertex2f(strip[2].x, strip[2].y);
        rlEnd();
        rlSetTexture(0);
    }
}

void DrawLineExDualcolor(Vector2 startPos, Vector2 endPos, float thick, Color color, Color color2) {
    Vector2 delta = {endPos.x - startPos.x, endPos.y - startPos.y};
    float length = sqrtf(delta.x * delta.x + delta.y * delta.y);

    if ((length > 0) && (thick > 0)) {
        float scale = thick / (2 * length);

        Vector2 radius = {-scale * delta.y, scale * delta.x};
        Vector2 strip[4] = {
            {startPos.x - radius.x, startPos.y - radius.y},
            {startPos.x + radius.x, startPos.y + radius.y},
            {endPos.x - radius.x, endPos.y - radius.y},
            {endPos.x + radius.x, endPos.y + radius.y}};

        rlBegin(RL_QUADS);
        rlNormal3f(0.0f, 0.0f, 1.0f);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(strip[0].x, strip[0].y);
        rlVertex2f(strip[1].x, strip[1].y);
        rlColor4ub(color2.r, color2.g, color2.b, color2.a);
        rlVertex2f(strip[3].x, strip[3].y);
        rlVertex2f(strip[2].x, strip[2].y);
        rlEnd();
    }
}

// expand all corners outwards
// x---------x
// |  -----  |
// x---------x
void DrawLineEx3(Vector2 startPos, Vector2 endPos, float thick, Color color) {
    Vector2 delta = {endPos.x - startPos.x, endPos.y - startPos.y};
    float length = sqrtf(delta.x * delta.x + delta.y * delta.y);

    if ((length > 0) && (thick > 0)) {
        float scale = thick / (2 * length);

        startPos.x -= delta.x * scale;
        startPos.y -= delta.y * scale;
        endPos.x += delta.x * scale;
        endPos.y += delta.y * scale;

        DrawLineEx2(startPos, endPos, thick, color);
    }
}
