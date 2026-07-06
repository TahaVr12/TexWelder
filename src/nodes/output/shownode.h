#pragma once

#include "../../classes/node.h"
#include "../../classes/structs.h"

inline Node MakeShowNode()
{
    Node n;
    n.title = "Show";
    n.type = NodeType::Show;
    n.rDownCorner = {50, 35};

    n.inputs = {IO{"input"}};

    return n;
}
