#include "paramwidgets.h"
#include "node.h"
#include "../drawhelpers.h"
#include "../../libs/raylib-6.0/src/raymath.h"

static int draggingParamNodeIdx = -1;
static int draggingParamIdx = -1;
static int draggingVec2Component = -1; // 0 = X, 1 = Y - locked in at drag-start like the color channel below

// Color param popup (Blender-style: swatch button opens a wheel+sliders dialog) - only one open at a time
static int colorPopupNodeIdx = -1;
static int colorPopupParamIdx = -1;
static bool colorPopupUseRGB = false; // false = HSV sliders, true = RGB sliders
static bool draggingColorWheel = false;
static int draggingColorPopupSlider = -1; // 0/1/2 = which of the 3 visible sliders (H/S/V or R/G/B)

void ResetParamInteractionState()
{
    draggingParamNodeIdx = -1;
    draggingParamIdx = -1;
    draggingVec2Component = -1;
    colorPopupNodeIdx = -1;
    colorPopupParamIdx = -1;
    draggingColorWheel = false;
    draggingColorPopupSlider = -1;
}

int DrawNodeParam(NodeParam &p, Vector2 topLeft, float zoom, float rDownCornerX, int paramOffsetY)
{
    float paramWidth = rDownCornerX - 16;

    if (p.type == InputType::Slider)
    {
        Rectangle sliderRect{
            topLeft.x + 8 * zoom,
            topLeft.y + paramOffsetY * zoom,
            paramWidth * zoom,
            14 * zoom};

        p.rect = sliderRect; // stored so HandleParamDragging can hit-test/drag it next frame

        DrawRectangleRounded(sliderRect, 0.3f, 4, Color{20, 20, 28, 255});

        float t = Clamp((p.max > p.min) ? (p.value - p.min) / (p.max - p.min) : 0.0f, 0.0f, 1.0f);
        Rectangle fillRect = sliderRect;
        fillRect.width *= t;
        DrawRectangleRounded(fillRect, 0.3f, 4, Color{80, 120, 200, 255});
        const char *label = TextFormat("%s: %.2f", p.name.c_str(), p.value);
        DrawTextEx(GetFontDefault(), label, Vector2{sliderRect.x + 4, sliderRect.y + 1}, 10.0f * zoom, 1, WHITE);
        return 20;
    }
    else if (p.type == InputType::DragFloat)
    {
        Rectangle sliderRect{
            topLeft.x + 8 * zoom,
            topLeft.y + paramOffsetY * zoom,
            paramWidth * zoom,
            14 * zoom};

        p.rect = sliderRect; // stored so HandleParamDragging can hit-test/drag it next frame

        DrawRectangleRounded(sliderRect, 0.3f, 4, Color{10, 10, 18, 255});

        const char *label = TextFormat("%s: %.2f", p.name.c_str(), p.value);
        DrawTextEx(GetFontDefault(), label, Vector2{sliderRect.x + 4, sliderRect.y + 1}, 10.0f * zoom, 1, WHITE);
        return 20;
    }
    else if (p.type == InputType::Color)
    {
        // just a swatch button here - clicking it opens the full wheel/sliders popup,
        // drawn and handled separately by DrawColorPopup (needs to render on top of every
        // node, not just this one, so it can't live in this per-node draw call)
        Rectangle swatchRect{
            topLeft.x + 8 * zoom,
            topLeft.y + paramOffsetY * zoom,
            paramWidth * zoom,
            14 * zoom};

        p.rect = swatchRect; // stored so HandleParamDragging can hit-test it next frame

        DrawRectangleRounded(swatchRect, 0.3f, 4, p.colorValue);
        DrawRectangleRoundedLinesEx(swatchRect, 0.3f, 4, 1.0f, Color{0, 0, 0, 180});
        DrawTextEx(GetFontDefault(), p.name.c_str(), Vector2{swatchRect.x + 4, swatchRect.y + 1}, 10.0f * zoom, 1, WHITE);
        return 20;
    }
    else if (p.type == InputType::Vec2)
    {
        // name label above, X and Y drag-boxes side by side below - p.rect covers both boxes as
        // one region, HandleParamDragging figures out which half (X vs Y) a click landed on
        DrawTextEx(GetFontDefault(), p.name.c_str(), Vector2{topLeft.x + 8 * zoom, topLeft.y + paramOffsetY * zoom}, 10.0f * zoom, 1, WHITE);

        Rectangle fullRect{
            topLeft.x + 8 * zoom,
            topLeft.y + (paramOffsetY + 14) * zoom,
            paramWidth * zoom,
            14 * zoom};

        p.rect = fullRect;

        float half = fullRect.width / 2.0f;
        Rectangle xRect{fullRect.x, fullRect.y, half - 2 * zoom, fullRect.height};
        Rectangle yRect{fullRect.x + half + 2 * zoom, fullRect.y, half - 2 * zoom, fullRect.height};

        DrawRectangleRounded(xRect, 0.3f, 4, Color{10, 10, 18, 255});
        DrawRectangleRounded(yRect, 0.3f, 4, Color{10, 10, 18, 255});

        DrawTextEx(GetFontDefault(), TextFormat("X: %.2f", p.vec2Value.x), Vector2{xRect.x + 4, xRect.y + 1}, 10.0f * zoom, 1, WHITE);
        DrawTextEx(GetFontDefault(), TextFormat("Y: %.2f", p.vec2Value.y), Vector2{yRect.x + 4, yRect.y + 1}, 10.0f * zoom, 1, WHITE);

        return 34; // label row + boxes row
    }

    return 20;
}

bool HandleParamDragging(World &world)
{
    bool hitAnyParamThisFrame = false;

    // checked first so the same click doesn't also select/connect a node - see handleInputs()
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        for (Node &n : world.nodes)
        {
            for (size_t i = 0; i < n.params.size(); i++)
            {
                if (!CheckCollisionPointRec(GetMousePosition(), n.params[i].rect))
                    continue;

                hitAnyParamThisFrame = true;

                if (n.params[i].type == InputType::Color)
                {
                    // clicking the swatch toggles its popup open/closed - the popup does its own
                    // dragging/hit-testing for the wheel and sliders, see DrawColorPopup
                    bool alreadyOpenForThis = colorPopupNodeIdx == static_cast<int>(n.id) && colorPopupParamIdx == static_cast<int>(i);
                    colorPopupNodeIdx = alreadyOpenForThis ? -1 : static_cast<int>(n.id);
                    colorPopupParamIdx = alreadyOpenForThis ? -1 : static_cast<int>(i);
                }
                else
                {
                    draggingParamNodeIdx = static_cast<int>(n.id);
                    draggingParamIdx = static_cast<int>(i);

                    if (n.params[i].type == InputType::Vec2)
                    {
                        // which half (X/Y) got clicked - locked in for the whole drag so wobbling
                        // across the middle mid-drag doesn't switch which component is being edited
                        float half = n.params[i].rect.width / 2.0f;
                        float relX = GetMousePosition().x - n.params[i].rect.x;
                        draggingVec2Component = (relX < half) ? 0 : 1;
                    }
                }
            }
        }
    }

    if (draggingParamNodeIdx != -1)
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            NodeParam &p = world.nodes[draggingParamNodeIdx].params[draggingParamIdx];

            if (p.type == InputType::Slider)
            {
                float t = Clamp((GetMousePosition().x - p.rect.x) / p.rect.width, 0.0f, 1.0f);
                p.value = p.min + t * (p.max - p.min);
            }
            else if (p.type == InputType::DragFloat)
            {
                // relative drag instead of absolute positioning - DragFloat params default to
                // +/-FLT_MAX bounds, so mapping through (max - min) like Slider does would overflow to inf/NaN
                p.value += GetMouseDelta().x * 0.05f;
                p.value = Clamp(p.value, p.min, p.max);
            }
            else if (p.type == InputType::Vec2)
            {
                // same relative-drag style as DragFloat, just applied to whichever component was clicked
                if (draggingVec2Component == 0)
                    p.vec2Value.x += GetMouseDelta().x * 0.05f;
                else
                    p.vec2Value.y += GetMouseDelta().x * 0.05f;
            }
        }
        else
        {
            draggingParamNodeIdx = -1;
            draggingParamIdx = -1;
            draggingVec2Component = -1;
        }
    }

    return hitAnyParamThisFrame || draggingParamNodeIdx != -1;
}

// -------------------- color popup --------------------

// hue/saturation wheel (value fixed at 1.0 - the popup dims it to show the actual value), generated
// once and cached; alpha is zeroed outside the circle so it reads as round against the popup background
static Texture2D GetColorWheelTexture()
{
    static Texture2D tex{};
    static bool loaded = false;
    if (!loaded)
    {
        int size = 128;
        Image img = GenImageColor(size, size, BLANK);
        float center = size / 2.0f;

        for (int y = 0; y < size; y++)
        {
            for (int x = 0; x < size; x++)
            {
                float dx = (x + 0.5f) - center;
                float dy = (y + 0.5f) - center;
                float dist = sqrtf(dx * dx + dy * dy) / center;

                if (dist <= 1.0f)
                {
                    float hue = atan2f(dy, dx) * RAD2DEG;
                    if (hue < 0)
                        hue += 360.0f;

                    ImageDrawPixel(&img, x, y, ColorFromHSV(hue, dist, 1.0f));
                }
            }
        }

        tex = LoadTextureFromImage(img);
        UnloadImage(img);
        loaded = true;
    }
    return tex;
}

// draws one slider row, hit-tests/drags it against the shared draggingColorPopupSlider lock, and
// returns the (possibly updated) value - used for both the HSV and RGB slider sets in DrawColorPopup
static float DrawColorSlider(Rectangle rect, const char *label, float value, float minV, float maxV, int sliderIndex)
{
    DrawRectangleRounded(rect, 0.3f, 4, Color{20, 20, 28, 255});

    float t = Clamp((value - minV) / (maxV - minV), 0.0f, 1.0f);
    Rectangle fill = rect;
    fill.width *= t;
    DrawRectangleRounded(fill, 0.3f, 4, Color{80, 120, 200, 255});

    const char *txt = TextFormat("%s: %.0f", label, value);
    DrawTextEx(GetFontDefault(), txt, Vector2{rect.x + 4, rect.y + 2}, 12, 1, WHITE);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), rect))
        draggingColorPopupSlider = sliderIndex;

    if (draggingColorPopupSlider == sliderIndex && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        float newT = Clamp((GetMousePosition().x - rect.x) / rect.width, 0.0f, 1.0f);
        return minV + newT * (maxV - minV);
    }

    return value;
}

// Blender-style color picker popup: wheel (dimmed by V) + preview + HSV/RGB slider toggle + OK.
// Positioned directly over the swatch that opened it (see the Color case in DrawNodeParam).
void DrawColorPopup(World &world)
{
    if (colorPopupNodeIdx == -1)
        return;
    if (colorPopupNodeIdx >= static_cast<int>(world.nodes.size()))
    {
        colorPopupNodeIdx = -1;
        return;
    }

    NodeParam &p = world.nodes[colorPopupNodeIdx].params[colorPopupParamIdx];

    Rectangle popup{p.rect.x, p.rect.y, 260, 230};
    DrawRectangleRounded(popup, 0.05f, 8, Color{35, 35, 45, 255});
    DrawRectangleRoundedLinesEx(popup, 0.05f, 8, 1.5f, Color{80, 80, 95, 255});

    Vector3 hsv = ColorToHSV(p.colorValue);

    // wheel (hue/saturation), dimmed by an overlay proportional to (1 - value)
    Rectangle wheelRect{popup.x + 10, popup.y + 10, 100, 100};
    Texture2D wheel = GetColorWheelTexture();
    DrawTexturePro(wheel, Rectangle{0, 0, static_cast<float>(wheel.width), static_cast<float>(wheel.height)},
                   wheelRect, Vector2{0, 0}, 0, WHITE);
    DrawRectangleRounded(wheelRect, 1.0f, 16, Color{0, 0, 0, static_cast<unsigned char>((1.0f - hsv.z) * 255)});

    Vector2 wheelCenter{wheelRect.x + wheelRect.width / 2.0f, wheelRect.y + wheelRect.height / 2.0f};
    float markerAngle = hsv.x * DEG2RAD;
    float markerRadius = hsv.y * (wheelRect.width / 2.0f);
    Vector2 markerPos{wheelCenter.x + cosf(markerAngle) * markerRadius, wheelCenter.y + sinf(markerAngle) * markerRadius};
    DrawCircleLines(static_cast<int>(markerPos.x), static_cast<int>(markerPos.y), 4, BLACK);
    DrawCircleLines(static_cast<int>(markerPos.x), static_cast<int>(markerPos.y), 3, WHITE);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), wheelRect))
        draggingColorWheel = true;

    if (draggingColorWheel && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        float dx = GetMousePosition().x - wheelCenter.x;
        float dy = GetMousePosition().y - wheelCenter.y;

        float hue = atan2f(dy, dx) * RAD2DEG;
        if (hue < 0)
            hue += 360.0f;

        hsv.x = hue;
        hsv.y = Clamp(sqrtf(dx * dx + dy * dy) / (wheelRect.width / 2.0f), 0.0f, 1.0f);
        p.colorValue = ColorFromHSV(hsv.x, hsv.y, hsv.z);
    }

    // result preview, to the right of the wheel
    Rectangle previewRect{popup.x + 120, popup.y + 10, 130, 60};
    DrawRectangleRec(previewRect, p.colorValue);
    DrawRectangleLinesEx(previewRect, 1, BLACK);

    // HSV/RGB mode toggle, below the preview
    Rectangle hsvBtn{popup.x + 120, popup.y + 78, 60, 22};
    Rectangle rgbBtn{popup.x + 188, popup.y + 78, 62, 22};
    DrawRectangleRounded(hsvBtn, 0.3f, 4, !colorPopupUseRGB ? Color{90, 110, 160, 255} : Color{55, 55, 65, 255});
    DrawRectangleRounded(rgbBtn, 0.3f, 4, colorPopupUseRGB ? Color{90, 110, 160, 255} : Color{55, 55, 65, 255});
    DrawText("HSV", static_cast<int>(hsvBtn.x + 12), static_cast<int>(hsvBtn.y + 5), 12, WHITE);
    DrawText("RGB", static_cast<int>(rgbBtn.x + 12), static_cast<int>(rgbBtn.y + 5), 12, WHITE);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (CheckCollisionPointRec(GetMousePosition(), hsvBtn))
            colorPopupUseRGB = false;
        else if (CheckCollisionPointRec(GetMousePosition(), rgbBtn))
            colorPopupUseRGB = true;
    }

    // 3 sliders - HSV or RGB depending on the toggle above
    float sliderY = popup.y + 120;
    Rectangle sliderRect0{popup.x + 10, sliderY, 240, 20};
    Rectangle sliderRect1{popup.x + 10, sliderY + 24, 240, 20};
    Rectangle sliderRect2{popup.x + 10, sliderY + 48, 240, 20};

    if (!colorPopupUseRGB)
    {
        hsv.x = DrawColorSlider(sliderRect0, "H", hsv.x, 0.0f, 360.0f, 0);
        hsv.y = DrawColorSlider(sliderRect1, "S", hsv.y, 0.0f, 1.0f, 1);
        hsv.z = DrawColorSlider(sliderRect2, "V", hsv.z, 0.0f, 1.0f, 2);
        p.colorValue = ColorFromHSV(hsv.x, hsv.y, hsv.z);
    }
    else
    {
        float r = DrawColorSlider(sliderRect0, "R", p.colorValue.r, 0.0f, 255.0f, 0);
        float g = DrawColorSlider(sliderRect1, "G", p.colorValue.g, 0.0f, 255.0f, 1);
        float b = DrawColorSlider(sliderRect2, "B", p.colorValue.b, 0.0f, 255.0f, 2);
        p.colorValue.r = static_cast<unsigned char>(r);
        p.colorValue.g = static_cast<unsigned char>(g);
        p.colorValue.b = static_cast<unsigned char>(b);
    }

    // OK button, bottom-left, confirms and closes; clicking outside the popup closes it too
    Rectangle okBtn{popup.x + 10, popup.y + 200, 60, 22};
    DrawRectangleRounded(okBtn, 0.3f, 4, Color{70, 150, 90, 255});
    DrawText("OK", static_cast<int>(okBtn.x + 18), static_cast<int>(okBtn.y + 5), 12, WHITE);

    bool clickedOK = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), okBtn);
    bool clickedOutside = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !CheckCollisionPointRec(GetMousePosition(), popup);

    if (clickedOK || clickedOutside)
    {
        colorPopupNodeIdx = -1;
        colorPopupParamIdx = -1;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        draggingColorWheel = false;
        draggingColorPopupSlider = -1;
    }
}
