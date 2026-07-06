#pragma once

#include <iostream>
#include <vector>

#include "../../libs/raylib-6.0/src/raylib.h"
#include "structs.h"

class Node
{
public:
    size_t id = 0;

    std::string title = "Node";
    unsigned int titleSize = 13;
    unsigned int titleBarLenghtPX = 20;

    unsigned int segments = 5;

    unsigned int ioPadding = 4;
    unsigned int ioOffsetFromTop = 10;
    unsigned int ioOffsetFromEdge = 10;

    float rounding = 0.1f;

    float outlineThick = 1.5f;

    Vector2 position = {0, 0};
    Vector2 rDownCorner = {100, 140}; // offset for oppopsite corner of positions corner
    Vector2 titlePadding = {13, 6};

    Color baseColor = Color{30, 30, 40, 250};
    Color titleColor = Color{100, 100, 115, 255};

    Color unSelectedOutlineColor = Color{68, 65, 67, 175};
    Color selectedOutlineColor = ORANGE;

    bool isSelected = false;

    std::vector<IO> inputs;
    std::vector<IO> outputs;

    NodeType type = NodeType::Generic;
    std::vector<NodeParam> params;

    // shader-node stuff
    std::string fragShaderPath;
    Shader shader{};
    bool shaderLoaded = false;

    Vector2 textureSize = {512, 512};
    RenderTexture2D renderTexture{};
    bool textureReady = false;

    void Draw(Vector2 screenCenter, Vector2 offset, float gridScale);
    void addPos(const Vector2 &posToAdd);
};