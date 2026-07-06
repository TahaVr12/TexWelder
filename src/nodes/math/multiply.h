#pragma once

#include "../../classes/node.h"
#include "../../classes/structs.h"

inline Node MakeMultiplyNode()
{
    Node n;
    n.title = "Multiply";
    n.type = NodeType::Math;

    n.inputs = {IO{"a"}, IO{"b"}};
    n.outputs = {IO{"out"}};

    n.rDownCorner = {75, 70};

    n.fragShaderPath = "nodes/shaders/multiply.fs";

    return n;
}
