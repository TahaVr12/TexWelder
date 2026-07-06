#include "node.h"
#include "paramwidgets.h"
#include "../drawhelpers.h"

void Node::Draw(Vector2 screenCenter, Vector2 offset, float gridScale)
{
    float zoom = gridScale / BASE_GRID_SCALE;

    Vector2 topLeft = WorldToScreen(position, screenCenter, offset, gridScale);
    Rectangle rect{topLeft.x, topLeft.y, rDownCorner.x * zoom, rDownCorner.y * zoom};

    DrawRectangleRounded(rect, rounding, segments, baseColor); // draw base window/rectangle

    // mr title bar vvv
    DrawRectangleRounded(Rectangle{topLeft.x, topLeft.y, rDownCorner.x * zoom, titleBarLenghtPX * zoom}, rounding * (titleBarLenghtPX / 5.0f), segments, DARKGRAY);

    DrawTextEx(GetFontDefault(), title.c_str(), Vector2{topLeft.x + titlePadding.x, topLeft.y + titlePadding.y}, titleSize * zoom, 1, titleColor);

    // selection shittings vv
    if (isSelected)
        DrawRectangleRoundedLinesEx(rect, rounding, segments, outlineThick * zoom, selectedOutlineColor);
    else
        DrawRectangleRoundedLinesEx(rect, rounding, segments, outlineThick * zoom, unSelectedOutlineColor);

    int offsetY = 0;

    for (IO &in : inputs)
    {
        offsetY += 18;
        in.position = Vector2{topLeft.x + ioOffsetFromEdge * zoom, topLeft.y + (ioOffsetFromTop + offsetY) * zoom};

        in.textoffset = ioOffsetFromEdge;
        in.textSize = 10.2f * zoom;

        in.radius = 4.0f * zoom;

        in.Draw();
    }

    offsetY = 0;
    for (IO &out : outputs)
    {
        offsetY += 18;
        out.position = Vector2{(topLeft.x + rDownCorner.x * zoom) - ioOffsetFromEdge * zoom, topLeft.y + (ioOffsetFromTop + offsetY) * zoom};

        out.textoffset = ioOffsetFromEdge;
        out.textSize = 10.2f * zoom;
        out.radius = 4.0f * zoom;

        out.Draw();
    }

    size_t ioRows = inputs.size() > outputs.size() ? inputs.size() : outputs.size();
    int paramOffsetY = ioOffsetFromTop + static_cast<int>(ioRows) * 18 + 10;

    for (NodeParam &p : params)
    {
        paramOffsetY += DrawNodeParam(p, topLeft, zoom, rDownCorner.x, paramOffsetY);
    }
}

void Node::addPos(const Vector2 &posToAdd)
{
    position.x += posToAdd.x;
    position.y += posToAdd.y;
}