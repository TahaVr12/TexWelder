#pragma once

#include "../../classes/node.h"
#include "../../classes/structs.h"

inline Node MakeMakeBWNode()
{
    Node n;
    n.title = "Make BW";
    n.type = NodeType::Math;

    n.inputs = {IO{"input"}};
    n.outputs = {IO{"out"}};

    n.rDownCorner = {75, 70};

    n.fragShaderPath = "src/nodes/shaders/makeBW.fs";

    return n;
}
