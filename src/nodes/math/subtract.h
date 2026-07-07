#pragma once

#include "../../classes/node.h"
#include "../../classes/structs.h"

inline Node MakeSubtractNode()
{
    Node n;
    n.title = "Subtract";
    n.type = NodeType::Math;

    n.inputs = {IO{"a"}, IO{"b"}};
    n.outputs = {IO{"out"}};

    n.rDownCorner = {75, 70};

    n.fragShaderPath = "src/nodes/shaders/subtract.fs";
    /*
        n.params = {
            {"Value", 1.0f, 0.0f, 10.0f}};
    */

    return n;
}
