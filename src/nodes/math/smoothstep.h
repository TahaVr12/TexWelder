#pragma once

#include "../../classes/node.h"
#include "../../classes/structs.h"

inline Node MakeSmoothstepNode()
{
    Node n;
    n.title = "Smoothstep";
    n.titleSize = 12;
    n.type = NodeType::Math;

    n.inputs = {IO{"input"}, IO{"edge0"}, IO{"edge1"}};
    n.outputs = {IO{"out"}};

    n.rDownCorner = {75, 75};

    n.fragShaderPath = "nodes/shaders/smoothstep.fs";

    return n;
}
