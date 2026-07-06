#include <cmath>
#include <iostream>
#include "../libs/raylib-6.0/src/raylib.h"

const float BASE_GRID_SCALE = 100.0f;

inline Vector2 WorldToScreen(Vector2 worldPos, Vector2 screenCenter, Vector2 offset, float gridScale)
{
    float zoom = gridScale / BASE_GRID_SCALE;
    return Vector2{
        screenCenter.x + offset.x + worldPos.x * zoom,
        screenCenter.y + offset.y + worldPos.y * zoom};
}

inline Vector2 ScreenToWorld(Vector2 screenPos, Vector2 screenCenter, Vector2 offset, float gridScale)
{
    float zoom = gridScale / BASE_GRID_SCALE;
    return Vector2{
        (screenPos.x - screenCenter.x - offset.x) / zoom,
        (screenPos.y - screenCenter.y - offset.y) / zoom};
}

inline void drawGrid(float scale, unsigned int thickness, Vector2 screenRes, Vector2 offset, Color color)
{
    Vector2 center = {screenRes.x / 2.0f, screenRes.y / 2.0f};

    float startX = fmodf(center.x + offset.x, scale);
    if (startX < 0)
        startX += scale;

    for (float x = startX; x < screenRes.x; x += scale)
    {
        DrawLineEx(Vector2{x, 0}, Vector2{x, screenRes.y}, thickness, color);
    }

    float startY = fmodf(center.y + offset.y, scale);
    if (startY < 0)
        startY += scale;

    for (float y = startY; y < screenRes.y; y += scale)
    {
        DrawLineEx(Vector2{0, y}, Vector2{screenRes.x, y}, thickness, color);
    }
}

inline bool OnScreenResChange(Vector2 &beforeScreenRes)
{
    if (beforeScreenRes.x != static_cast<float>(GetRenderWidth()) || beforeScreenRes.y != static_cast<float>(GetRenderHeight()))
    {
        beforeScreenRes = Vector2{static_cast<float>(GetRenderWidth()), static_cast<float>(GetRenderHeight())};
        return true;
    }
    return false;
}