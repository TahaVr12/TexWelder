#pragma once

#include "../../classes/node.h"
#include "../../classes/structs.h"

inline Node MakePerlinNoiseNode()
{
    Node n;
    n.title = "Perlin Noise";
    n.type = NodeType::Noise;

    n.outputs = {IO{"output"}};

    n.fragShaderPath = "nodes/shaders/perlinnoise.fs";

    n.params = {
        {"Scale", InputType::DragFloat, 4.0f},
        {"Octaves", InputType::DragFloat, 4.0f},
        {"Brightness", InputType::DragFloat, 1.0f},
        {"Seed", InputType::DragFloat, 0.0f}};

    return n;
}
