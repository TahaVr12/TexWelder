#include <iostream>
#include <vector>

#include "../libs/raylib-6.0/src/raylib.h"
#include "../libs/raylib-6.0/src/raymath.h"
#include "drawhelpers.h"

#include "classes/node.h"
#include "classes/structs.h"
#include "classes/extramenus.h"
#include "classes/paramwidgets.h"
#include "extras/importantmenus.h"

#include "nodes/procedrual/perlinnoise.h"
#include "nodes/procedrual/voronoi.h"

#include "nodes/math/add.h"
#include "nodes/math/subtract.h"
#include "nodes/math/multiply.h"
#include "nodes/math/divide.h"

#include "nodes/math/mix.h"
#include "nodes/math/makeBW.h"
#include "nodes/math/smoothstep.h"

#include "nodes/input/value.h"
#include "nodes/input/color.h"

#include "nodes/output/shownode.h"

float mousewheelsens = 5.0f;

Vector2 screenRes = {0, 0};
Vector2 screenCenter = {0, 0};

Vector2 gridOffset = {0, 0};
float gridScale = 100;

World world;
AddMenu addmenu;

bool drawAddMenu = false;

bool isMovingNode = false;
bool isMakingConnection = false;

// LMB drag to move vars
bool isDraggingSelectedNodes = false;
bool nodeDragExceededThreshold = false;
Vector2 nodeDragStartMousePos{0, 0};
int nodeDragClickedIdx = -1;
bool nodeDragShiftWasHeld = false;

bool isAdjustingTexturePanel = false;

float sideMenuX = GetScreenWidth() - 500;

Vector2 LastMousePos{0, 0};

void handleInputs()
{
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
    {
        gridOffset.x += GetMouseDelta().x;
        gridOffset.y += GetMouseDelta().y;
    }

    if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) && IsKeyPressed(KEY_A)) // adding nodes
    {
        drawAddMenu = true;
        addmenu.position = GetMousePosition();
    }

    gridScale += GetMouseWheelMove() * mousewheelsens;
    gridScale = Clamp(gridScale, 10.0f, 400.0f);

    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_R))
    {
        world.ReloadShaders();
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        for (size_t i = 0; i < world.connections.size(); i++)
        {
            Connection &c = world.connections[i];
            if (!c.finishConnection)
                continue;

            Vector2 inScreen = WorldToScreen(c.inPos, screenCenter, gridOffset, gridScale);
            Vector2 outScreen = WorldToScreen(c.outPos, screenCenter, gridOffset, gridScale);

            if (CheckCollisionPointLine(GetMousePosition(), inScreen, outScreen, 8))
            {
                world.connections.erase(world.connections.begin() + i);
                break;
            }
        }
    }

    if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_X))
    {
        for (int i = static_cast<int>(world.nodes.size()) - 1; i >= 0; i--)
        {
            if (world.nodes[i].isSelected)
                world.RemoveNode(static_cast<size_t>(i));
        }

        // node indices shifted around under these, so fuh any in progress interaction referencing them
        ResetParamInteractionState();
        isMakingConnection = false;
        isDraggingSelectedNodes = false;
        nodeDragClickedIdx = -1;
    }

    bool paramConsumedClick = HandleParamDragging(world);

    // selection shit

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !paramConsumedClick)
    {
        if (!isMovingNode) // node selection / LMB drag start
        {
            bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

            // only the topmost node under the cursor should respond to the click
            int hitIdx = -1;
            for (int i = static_cast<int>(world.nodes.size()) - 1; i >= 0; i--)
            {
                Node &n = world.nodes[i];
                bool hit = CheckCollisionPointRec(GetMousePosition(),
                                                  Rectangle{
                                                      WorldToScreen(n.position, screenCenter, gridOffset, gridScale).x,
                                                      WorldToScreen(n.position, screenCenter, gridOffset, gridScale).y,
                                                      n.rDownCorner.x * (gridScale / 100),
                                                      n.rDownCorner.y * (gridScale / 100)});
                if (hit)
                {
                    hitIdx = i;
                    break;
                }
            }

            if (hitIdx != -1)
            {
                bool wasPreselected = world.nodes[hitIdx].isSelected;

                if (!wasPreselected)
                {
                    if (!shift)
                        for (Node &n : world.nodes)
                            n.isSelected = false;
                    world.nodes[hitIdx].isSelected = true;
                }
                else
                {
                    nodeDragClickedIdx = hitIdx;
                    nodeDragShiftWasHeld = shift;
                }

                isDraggingSelectedNodes = true;
                nodeDragStartMousePos = GetMousePosition();
                nodeDragExceededThreshold = false;
            }
            else if (!shift)
            {
                for (Node &n : world.nodes)
                    n.isSelected = false;
            }
        }
        else
        {
            isMovingNode = false;
        }

        size_t unfinishedConnections = 0;
        for (Connection &c : world.connections) // for the function below for not adding more connections if there are unfinished ones
        {
            if (!c.finishConnection)
            {
                unfinishedConnections++;
            }
        }

        bool wasAlreadyMakingConnection = isMakingConnection;
        bool connectionFinishedThisClick = false;

        for (Node &n : world.nodes)
        {
            if (isMakingConnection)
            {
                for (u_int16_t i = 0; i < n.inputs.size(); i++)
                {
                    if (CheckCollisionPointCircle(GetMousePosition(), n.inputs[i].position, n.inputs[i].radius))
                    {
                        for (Connection &c : world.connections)
                        {
                            if (!c.finishConnection)
                            {
                                c.outNodeIdx = n.id;
                                c.outPos = ScreenToWorld(n.inputs[i].position, screenCenter, gridOffset, gridScale);
                                c.outAddr = i;
                                c.outRadius = n.inputs[i].radius;
                                c.finishConnection = true;
                            }
                        }
                        connectionFinishedThisClick = true;
                    }
                }
            }
            else
            {
                if (unfinishedConnections == 0) // this is for not adding more connections if there are unfinished ones ;-)
                {
                    for (u_int16_t i = 0; i < n.outputs.size(); i++)
                    {

                        if (CheckCollisionPointCircle(GetMousePosition(), n.outputs[i].position, n.outputs[i].radius))
                        {
                            Connection connection;
                            connection.inNodeIdx = n.id;
                            connection.inPos = ScreenToWorld(n.outputs[i].position, screenCenter, gridOffset, gridScale);
                            connection.inAddr = i;
                            connection.outPos = ScreenToWorld(GetMousePosition(), screenCenter, gridOffset, gridScale);
                            connection.finishConnection = false;
                            world.connections.emplace_back(connection);

                            isMakingConnection = true;
                        }
                    }
                }
            }
        }

        if (wasAlreadyMakingConnection)
        {
            if (connectionFinishedThisClick)
            {
                isMakingConnection = false;
            }
            else
            {
                if (!world.connections.empty() && !world.connections.back().finishConnection)
                    world.connections.pop_back();
                isMakingConnection = false;
            }
        }
    }
    for (Connection &c : world.connections) // update unfinished connection output thingies
    {
        if (!c.finishConnection)
        {
            c.outPos = ScreenToWorld(GetMousePosition(), screenCenter, gridOffset, gridScale);
        }
        else
        {
            c.inPos = ScreenToWorld(world.nodes[c.inNodeIdx].outputs[c.inAddr].position, screenCenter, gridOffset, gridScale);
            c.outPos = ScreenToWorld(world.nodes[c.outNodeIdx].inputs[c.outAddr].position, screenCenter, gridOffset, gridScale);
        }
    }

    // LMB drag to move
    if (isDraggingSelectedNodes)
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            if (!nodeDragExceededThreshold && Vector2Distance(GetMousePosition(), nodeDragStartMousePos) > 4.0f)
                nodeDragExceededThreshold = true;
        }
        else
        {
            if (!nodeDragExceededThreshold && nodeDragClickedIdx != -1 && nodeDragClickedIdx < static_cast<int>(world.nodes.size()))
            {
                if (!nodeDragShiftWasHeld)
                    for (Node &n : world.nodes)
                        n.isSelected = false;
                world.nodes[nodeDragClickedIdx].isSelected = false;
            }

            isDraggingSelectedNodes = false;
            nodeDragClickedIdx = -1;
        }
    }

    // ---------------- this is the part with the moving nodes----------------------
    if (IsKeyPressed(KEY_G))
    {
        isMovingNode = !isMovingNode;
    }
    if (isMovingNode || (isDraggingSelectedNodes && nodeDragExceededThreshold))
    {
        Vector2 worldMouseDelta = {ScreenToWorld(GetMousePosition(), screenCenter, gridOffset, gridScale).x - ScreenToWorld(LastMousePos, screenCenter, gridOffset, gridScale).x,
                                   ScreenToWorld(GetMousePosition(), screenCenter, gridOffset, gridScale).y - ScreenToWorld(LastMousePos, screenCenter, gridOffset, gridScale).y};

        for (Node &n : world.nodes)
        {
            if (n.isSelected)
            {
                n.addPos(worldMouseDelta);
            }
        }
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        if (CheckCollisionPointRec(GetMousePosition(), Rectangle{
                                                           sideMenuX,
                                                           0,
                                                           sideMenuX + 10, // 10 pixel grab thing
                                                           static_cast<float>(GetRenderHeight())}))
        {
            isAdjustingTexturePanel = true;
        }
    }
    else
    {
        isAdjustingTexturePanel = false;
    }

    if (isAdjustingTexturePanel) // for adjusting texture panel
    {
        sideMenuX += GetMouseDelta().x;
    }

    LastMousePos = GetMousePosition();
}

int main(void)
{

#pragma region category shit add menu
    CategoryRect ProcedrualCat;
    AddObj PerlinNoiseButton;
    AddObj VoronoiButton;

    PerlinNoiseButton.text = "Perlin Noise";
    PerlinNoiseButton.nodeToInstantiate = MakePerlinNoiseNode();

    VoronoiButton.text = "Voronoi";
    VoronoiButton.nodeToInstantiate = MakeVoronoiNode();

    ProcedrualCat.text = "Procedrual";
    ProcedrualCat.buttonsOnHover = {PerlinNoiseButton, VoronoiButton};

    // Input category

    CategoryRect InputCat;
    AddObj ValueButton;
    AddObj ColorButton;

    ValueButton.text = "Value";
    ValueButton.nodeToInstantiate = MakeValueNode();

    ColorButton.text = "Color";
    ColorButton.nodeToInstantiate = MakeColorNode();

    InputCat.text = "Inputs";
    InputCat.buttonsOnHover = {
        ValueButton, ColorButton};

    // Math category

    CategoryRect MathCat;
    AddObj AddButton;
    AddObj SubtractButton;
    AddObj MultiplyButton;
    AddObj DivideButton;

    AddObj MixLerpButton;
    AddObj BWButton;
    AddObj SmoothstepButton;

    AddButton.text = "Add";
    AddButton.nodeToInstantiate = MakeAddNode();

    SubtractButton.text = "Subtract";
    SubtractButton.nodeToInstantiate = MakeSubtractNode();

    MultiplyButton.text = "Multiply";
    MultiplyButton.nodeToInstantiate = MakeMultiplyNode();

    DivideButton.text = "Divide";
    DivideButton.nodeToInstantiate = MakeDivideNode();

    MixLerpButton.text = "Mix/Lerp";
    MixLerpButton.nodeToInstantiate = MakeMixLerpNode();

    BWButton.text = "Make BW";
    BWButton.nodeToInstantiate = MakeMakeBWNode();

    SmoothstepButton.text = "Smoothstep";
    SmoothstepButton.nodeToInstantiate = MakeSmoothstepNode();

    MathCat.text = "Math";
    MathCat.buttonsOnHover = {AddButton, SubtractButton, MultiplyButton, DivideButton, MixLerpButton, BWButton, SmoothstepButton};

    // Output category
    CategoryRect OutCat;
    AddObj showButton;

    showButton.text = "Show";
    showButton.nodeToInstantiate = MakeShowNode();

    OutCat.text = "Output";
    OutCat.buttonsOnHover = {showButton};

    addmenu.categories = {InputCat, MathCat, ProcedrualCat, OutCat};

#pragma endregion

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "testing ts");

    // SetTargetFPS(144);

    sideMenuX = GetScreenWidth() - 250;

    while (!WindowShouldClose())
    {

        if (OnScreenResChange(screenRes))
        {
            sideMenuX = screenRes.x - 250;
        }

        screenRes = {static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};
        screenCenter = {screenRes.x / 2.0f, screenRes.y / 2.0f};

        handleInputs();
        world.Compute();

        BeginDrawing();
        ClearBackground(BLACK);

        // raylib/rlgl thing: the first 2d draw call issued right after a custom-shader RenderTexture
        // pass (see World::Compute) renders corrupted and drawing something invisible first absorbs whatever is corrupted ig
        DrawPixel(-10, -10, BLANK);

        drawGrid(gridScale, 2, screenRes, gridOffset, Color{255, 255, 255, 65});

        world.Draw(screenCenter, gridOffset, gridScale);

        DrawSideMenu(sideMenuX, world);

        DrawTopMenu(30);

        if (drawAddMenu)
        {
            if (addmenu.Draw(world, screenCenter, gridOffset, gridScale))
                drawAddMenu = false;
        }

        DrawColorPopup(world); // on top of everything else, including the add-menu

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
