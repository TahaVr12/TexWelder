#pragma once

#include <iostream>
#include <vector>

#include "node.h"
#include "../../libs/raylib-6.0/src/raylib.h"

struct AddObj
{
    Rectangle rect;
    Color color = {65, 65, 65, 255};

    std::string text = "Add Node";

    Node nodeToInstantiate;
};

struct CategoryRect
{

    Vector2 pos;
    Vector2 scale = {60, 25};
    Color color = {50, 50, 50, 250};

    std::string text = "Category";

    bool isHovering = false;

    Rectangle hoverRectBox = {
        scale.x + pos.x + 2, pos.y - scale.y - 2,
        scale.x + 2, 2 * scale.y + 4};

    std::vector<AddObj> buttonsOnHover{}; // i didnt know what to name this lol

    Rectangle ButtonRect() const { return Rectangle{pos.x, pos.y, scale.x, scale.y}; }
    Rectangle PopupRect() const;
    Rectangle BridgeRect() const { return Rectangle{pos.x + scale.x, pos.y, 4, scale.y}; } // fills the small gap PopupRect leaves before it
    void DrawButton();
    bool DrawPopup(World &world, Vector2 screenCenter, Vector2 offset, float gridScale); // returns true if a node was instantiated
};

struct AddMenu
{
    Vector2 position;
    Vector2 scale = {70 * 2, 100 * 2};

    bool isHovering = false;
    int activeCategoryIdx = -1; // only one category's popup may be open at a time - see AddMenu::Draw

    std::vector<CategoryRect> categories{};

    Color baseColor = DARKGRAY;

    bool Draw(World &world, Vector2 screenCenter, Vector2 offset, float gridScale); // returns true once the menu should close
};
