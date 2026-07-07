#pragma once

#include "../../classes/node.h"
#include "../../classes/structs.h"

inline Node MakeVoronoiNode()
{
    Node n;
    n.title = "Voronoi";
    n.type = NodeType::Noise;

    n.outputs = {IO{"output"}};

    n.rDownCorner = {100, 165};

    n.fragShaderPath = "src/nodes/shaders/voronoi.fs";

    n.params = {
        {"Pos", InputType::Vec2},
        {"Scale", InputType::Vec2},
        {"Jitter", InputType::DragFloat, 1.0f},
        {"Phase", InputType::DragFloat, 0.0f},
        {"Seed", InputType::DragFloat, 0.0f}};

    n.params[1].vec2Value = {5.0f, 5.0f}; // scale

    return n;
}
