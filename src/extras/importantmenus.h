#pragma once

#include "../../libs/raylib-6.0/src/raylib.h"
#include "../classes/structs.h"
#include "../classes/node.h"

// -------------- top menu shit -------------------------
bool isFileMenuOpen = false;

inline void FileButton(float topMenuHeightpx)
{
    Button Bfile;
    Bfile.text = "File";
    Bfile.rect = Rectangle{5, 2, 50, topMenuHeightpx - 4};

    if (Bfile.isHovering() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Bfile.buttonColor = {100, 100, 100, 255};
        isFileMenuOpen = !isFileMenuOpen;
    }
    else if (!Bfile.isHovering() && (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)))
    {
        isFileMenuOpen = false;
    }
    else if (Bfile.isHovering())
    {
        Bfile.buttonColor = {70, 70, 70, 255};
    }

    if (isFileMenuOpen)
    {
        DrawRectangleGradientV(15, topMenuHeightpx, 175, 300, Color{40, 40, 60, 255}, Color{45, 50, 70, 235});
    }

    Bfile.Draw();
}

inline void DrawTopMenu(unsigned int topMenuHeightpx)
{
    DrawRectangleGradientV(0, 0, GetScreenWidth(), topMenuHeightpx, Color{30, 30, 45, 255}, Color{20, 20, 30, 252});
    FileButton(topMenuHeightpx);
}

// ---------------- texture Show Menu ----------------

inline void DrawSideMenu(float xExtend, World &world)
{
    DrawRectangle(static_cast<int>(xExtend), 0, GetScreenWidth(), GetScreenHeight(), BLACK);
    DrawLine(static_cast<int>(xExtend), 0, static_cast<int>(xExtend), GetScreenHeight(), Color{255, 255, 255, 245});

    Button refreshButton;
    refreshButton.text = "Recompile Shaders (Ctrl+R)";
    refreshButton.fontSize = 12;
    refreshButton.rect = Rectangle{xExtend + 10, 40, GetScreenWidth() - xExtend - 20, 28};
    refreshButton.buttonColor = refreshButton.isHovering() ? Color{90, 90, 90, 255} : Color{55, 55, 55, 255};

    if (refreshButton.isHovering() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        world.ReloadShaders();

    refreshButton.Draw();

    for (Node &n : world.nodes)
    {
        if (n.type != NodeType::Show)
            continue;

        Texture2D *tex = n.inputs.empty() ? nullptr : GetInputTexture(world, n, 0);
        if (tex != nullptr)
        {
            float panelWidth = GetScreenWidth() - xExtend - 20;
            float scale = panelWidth / static_cast<float>(tex->width);

            Rectangle src{0, 0, static_cast<float>(tex->width), -static_cast<float>(tex->height)}; // render textures are stored flipped
            Rectangle dst{xExtend + 10, 80, tex->width * scale, tex->height * scale};

            DrawTexturePro(*tex, src, dst, Vector2{0, 0}, 0, WHITE);
        }
        break; // only the first Show node found gets displayed for now
    }
}