#pragma once

#include "../../classes/node.h"
#include "../../classes/structs.h"

inline Node MakeColorNode()
{
    Node n;
    n.title = "Color";
    n.type = NodeType::Input;

    n.outputs = {IO{"out"}};

    n.rDownCorner = {85, 80};

    n.fragShaderPath = "nodes/shaders/color.fs";

    n.params = {
        {"Color", InputType::Color}};
    n.params[0].colorValue = Color{25, 255, 255, 255};

    return n;
}
