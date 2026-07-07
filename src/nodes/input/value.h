#pragma once

#include "../../classes/node.h"
#include "../../classes/structs.h"

inline Node MakeValueNode()
{
    Node n;
    n.title = "Value";
    n.type = NodeType::Input;

    n.outputs = {IO{"out"}};

    n.rDownCorner = {75, 70};

    n.fragShaderPath = "src/nodes/shaders/value.fs";

    n.params = {
        {"Value", InputType::DragFloat, 1.0f}};

    return n;
}
