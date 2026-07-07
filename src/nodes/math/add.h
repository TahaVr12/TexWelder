#pragma once

#include "../../classes/node.h"
#include "../../classes/structs.h"

inline Node MakeAddNode()
{
    Node n;
    n.title = "Add";
    n.type = NodeType::Math;

    n.inputs = {IO{"a"}, IO{"b"}};
    n.outputs = {IO{"out"}};

    n.rDownCorner = {75, 70};

    n.fragShaderPath = "src/nodes/shaders/add.fs";

    return n;
}
