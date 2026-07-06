#pragma once

#include <iostream>
#include <vector>
#include <limits>

#include "../../libs/raylib-6.0/src/raylib.h"

class Node;

enum class NodeType
{
    Generic,
    Input,
    Noise,
    Math,
    Show
};

enum class InputType
{
    Slider,
    DragFloat,
    Color,
    Vec2
};

struct NodeParam
{
    std::string name;
    InputType type;
    float value = 0.0f;
    float min = -std::numeric_limits<float>::max();
    float max = std::numeric_limits<float>::max();

    Color colorValue = WHITE;         // only used when type == InputType::Color
    Vector2 vec2Value = {0.0f, 0.0f}; // only used when type == InputType::Vec2

    Rectangle rect; // slider rect (or the combined swatch/X+Y rect, for Color/Vec2)
};

struct IO
{
    std::string text = "output/input";

    Vector2 position;
    Color color = RED;

    Color textColor = GRAY;

    int textoffset = -10;
    float textSize = 10.2f; // same as vv
    float radius = 4.0f;    // this is supposed to be a constant but cant cause i have to make it scale with zoom, DO NOT CHANGE THIS VALUE UNLESS YOU CHANGE IT ON node.cpp TOO

    void Draw();
};

struct Connection
{
    size_t inNodeIdx, outNodeIdx;
    unsigned int inAddr, outAddr; // i mean by 'Addr' is like in.outputs[inAddr] or out.inputs[outAddr]
    Color color = {10, 81, 172, 200};

    Vector2 inPos, outPos;               // these both are for collission dedections
    float inRadius = 20, outRadius = 20; // ^^^^^

    bool finishConnection = false;

    void Draw(Vector2 screenCenter, Vector2 offset, float gridScale);
};

struct World
{
    std::vector<Node> nodes;
    std::vector<Connection> connections;

    void Draw(Vector2 screenCenter, Vector2 offset, float gridScale);
    void Compute();       // runs every shader node's fragment shader into its RenderTexture2D >:)
    void ReloadShaders(); // forces every shader node to reload its .fs from disk on the next Compute()

    // deletes nodes[index] and any connection touching it, and re indexes everything after it
    void RemoveNode(size_t index);
};

// walks connections to find whatever feeds n.inputs[inputIndex]; ,returns nullptr if nothing (valid) is connected
Texture2D *GetInputTexture(World &world, Node &n, unsigned int inputIndex);

// ----------- helper shit ---------------

struct Button
{
    Rectangle rect;
    Color buttonColor = DARKGRAY;
    Color TextColor = WHITE;
    std::string text;
    float fontSize = 3;

    bool isHovering();
    void Draw();
};