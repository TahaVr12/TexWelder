#pragma once

#include "../../classes/node.h"
#include "../../classes/structs.h"

inline Node MakeMixLerpNode()
{
    Node n;
    n.title = "Mix/Lerp";
    n.type = NodeType::Math;

    n.inputs = {IO{"t"}, IO{"a"}, IO{"b"}};
    n.outputs = {IO{"out"}};

    n.rDownCorner = {75, 70};

    n.fragShaderPath = "src/nodes/shaders/mix.fs";

    return n;
}
