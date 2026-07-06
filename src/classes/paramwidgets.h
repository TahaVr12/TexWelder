#pragma once

#include "structs.h"

// returns how much vertical space (in unzoomed units) this param used, since Vec2 needs more room
// than a single-row Slider/DragFloat/Color - callers should advance their layout cursor by it
int DrawNodeParam(NodeParam &p, Vector2 topLeft, float zoom, float rDownCornerX, int paramOffsetY);

bool HandleParamDragging(World &world);

// call once per frame from the main draw loop after everything else, renders on top of all nodes
void DrawColorPopup(World &world);

// clears all param dragging/popup state, called whenever node indices shift (like World::RemoveNode)
void ResetParamInteractionState();
