#include "extramenus.h"
#include "../drawhelpers.h"

Rectangle CategoryRect::PopupRect() const
{
    float buttonHeight = scale.y + 6;
    float gap = 4;
    return Rectangle{
        pos.x + scale.x + 4, pos.y,
        scale.x, static_cast<float>(buttonsOnHover.size()) * (buttonHeight + gap) + gap};
}

// draws just the category row itself
void CategoryRect::DrawButton()
{
    hoverRectBox = PopupRect();

    DrawRectangleRec(ButtonRect(), color);
    DrawText(text.c_str(), pos.x + 6, pos.y + scale.y / 2 - 5, 10, WHITE);
}

// draws the popup (list of buttons) that are only called for whichever ONE category is active
bool CategoryRect::DrawPopup(World &world, Vector2 screenCenter, Vector2 offset, float gridScale)
{
    float buttonHeight = scale.y + 6;
    float gap = 4;

    bool isPressingLMB = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool nodeAdded = false;

    DrawRectangleRec(hoverRectBox, color);

    float buttonOffset = gap;
    for (AddObj ao : buttonsOnHover)
    {
        ao.rect = {
            hoverRectBox.x + gap, hoverRectBox.y + buttonOffset,
            hoverRectBox.width - 2 * gap, buttonHeight};

        DrawRectangleRec(ao.rect, ao.color);
        DrawText(ao.text.c_str(), ao.rect.x + 6, ao.rect.y + buttonHeight / 2 - 5, 10, WHITE);

        if (CheckCollisionPointRec(GetMousePosition(), ao.rect) && isPressingLMB)
        {
            ao.nodeToInstantiate.position = ScreenToWorld(GetMousePosition(), screenCenter, offset, gridScale);
            ao.nodeToInstantiate.id = world.nodes.size();
            world.nodes.emplace_back(ao.nodeToInstantiate);
            nodeAdded = true;
        }

        buttonOffset += buttonHeight + gap;
    }

    return nodeAdded;
}

bool AddMenu::Draw(World &world, Vector2 screenCenter, Vector2 offset, float gridScale)
{
    Rectangle menuRect{position.x, position.y, scale.x, scale.y};

    DrawRectangleRounded(menuRect, 0.15f, 4, baseColor);

    bool mouseOverAnything = CheckCollisionPointRec(GetMousePosition(), menuRect);

    // layout passings
    float yOffset = 0;
    for (CategoryRect &cr : categories)
    {
        cr.pos = {position.x + 5, position.y + 5 + yOffset};
        cr.scale = {scale.x - 10, 30};
        yOffset += cr.scale.y + 5;
    }

    // exclusivity passings
    bool stillOverActivePopup = activeCategoryIdx != -1 && activeCategoryIdx < static_cast<int>(categories.size()) &&
                                (CheckCollisionPointRec(GetMousePosition(), categories[activeCategoryIdx].PopupRect()) ||
                                 CheckCollisionPointRec(GetMousePosition(), categories[activeCategoryIdx].BridgeRect()));

    if (!stillOverActivePopup)
    {
        int hoveredButtonIdx = -1;
        for (size_t i = 0; i < categories.size(); i++)
        {
            if (CheckCollisionPointRec(GetMousePosition(), categories[i].ButtonRect()))
            {
                hoveredButtonIdx = static_cast<int>(i);
                break;
            }
        }
        activeCategoryIdx = hoveredButtonIdx;
    }

    for (CategoryRect &cr : categories)
        cr.DrawButton();

    bool nodeAdded = false;
    if (activeCategoryIdx != -1)
    {
        categories[activeCategoryIdx].isHovering = true;
        if (categories[activeCategoryIdx].DrawPopup(world, screenCenter, offset, gridScale))
            nodeAdded = true;
        mouseOverAnything = true;
    }

    bool clickedOutside = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !mouseOverAnything;

    return nodeAdded || clickedOutside;
}
