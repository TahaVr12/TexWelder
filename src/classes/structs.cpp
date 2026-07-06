#include "structs.h"
#include "node.h"
#include "../drawhelpers.h"
#include "../../libs/raylib-6.0/src/rlgl.h"
#include <string>

void IO::Draw()
{
    DrawCircleV(position, radius, color);
    DrawTextEx(GetFontDefault(), text.c_str(), Vector2{position.x + textoffset, position.y - radius}, textSize, 1, textColor);
}

void World::Draw(Vector2 screenCenter, Vector2 offset, float gridScale)
{
    for (Node &n : nodes)
    {
        n.Draw(screenCenter, offset, gridScale);
    }
    for (Connection &c : connections)
    {
        c.Draw(screenCenter, offset, gridScale);
    }
}

bool Button::isHovering()
{
    if (CheckCollisionPointRec(GetMousePosition(), rect))
    {
        return true;
    }
    return false;
}

void Button::Draw()
{
    DrawRectangleRec(rect, buttonColor);
    DrawText(text.c_str(), static_cast<int>(static_cast<float>(rect.x + rect.width) * 0.2f), static_cast<int>(static_cast<float>(rect.y + rect.height) * 0.2f), fontSize, TextColor);
}

void Connection::Draw(Vector2 screenCenter, Vector2 offset, float gridScale)
{
    Vector2 inScreen = WorldToScreen(inPos, screenCenter, offset, gridScale);
    Vector2 outScreen = WorldToScreen(outPos, screenCenter, offset, gridScale);
    DrawLineBezier(inScreen, outScreen, inRadius / 2.7f, color);
}

Texture2D *GetInputTexture(World &world, Node &n, unsigned int inputIndex)
{
    for (Connection &c : world.connections)
    {
        if (c.finishConnection && c.outNodeIdx == n.id && c.outAddr == inputIndex)
        {
            Node &src = world.nodes[c.inNodeIdx];
            if (src.textureReady)
                return &src.renderTexture.texture;
        }
    }
    return nullptr;
}

// LoadRenderTexture always creates an 8-bit-per-channel RGBA target, which clamps every pixel to
// 0..1 the moment a shader writes it - a node computing something brighter than 1.0 (e.g. adding
// two bright textures together) would get silently clipped before any later node ever saw the real
// value. This mirrors raylib's own LoadRenderTexture (rtextures.c) but with a 32-bit float color
// attachment instead, so values pass through node chains unclamped ("HDR").
static RenderTexture2D LoadRenderTextureHDR(int width, int height)
{
    RenderTexture2D target{};
    target.id = rlLoadFramebuffer();

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        target.texture.id = rlLoadTexture(nullptr, width, height, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;
        target.texture.mipmaps = 1;

        target.depth.id = rlLoadTextureDepth(width, height, true);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.mipmaps = 1;

        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);

        rlFramebufferComplete(target.id);
        rlDisableFramebuffer();
    }

    return target;
}

static void makeSureTexture(Node &n)
{
    if (!n.textureReady)
    {
        n.renderTexture = LoadRenderTextureHDR(static_cast<int>(n.textureSize.x), static_cast<int>(n.textureSize.y));
        n.textureReady = true;
    }
}

static Texture2D GetFullQuadTexture()
{
    static Texture2D tex{};
    static bool loaded = false;
    if (!loaded)
    {
        Image img = GenImageColor(1, 1, WHITE);
        tex = LoadTextureFromImage(img);
        UnloadImage(img);
        loaded = true;
    }
    return tex;
}

static Texture2D GetBlackTexture()
{
    static Texture2D tex{};
    static bool loaded = false;
    if (!loaded)
    {
        Image img = GenImageColor(1, 1, BLACK);
        tex = LoadTextureFromImage(img);
        UnloadImage(img);
        loaded = true;
    }
    return tex;
}

static void ComputeNode(Node &n, World &world)
{
    if (n.fragShaderPath.empty())
        return;

    if (!n.shaderLoaded)
    {
        n.shader = LoadShader(nullptr, n.fragShaderPath.c_str());
        n.shaderLoaded = true;
    }

    makeSureTexture(n);

    BeginTextureMode(n.renderTexture);
    ClearBackground(BLACK);
    BeginShaderMode(n.shader);

    for (size_t i = 0; i < n.inputs.size(); i++)
    {
        Texture2D *inputTex = GetInputTexture(world, n, static_cast<unsigned int>(i));
        Texture2D boundTex = (inputTex != nullptr) ? *inputTex : GetBlackTexture();

        std::string uniformName = "inputTexture" + std::to_string(i);
        int loc = GetShaderLocation(n.shader, uniformName.c_str());
        if (loc != -1)
            SetShaderValueTexture(n.shader, loc, boundTex);
    }

    for (NodeParam &p : n.params)
    {
        int loc = GetShaderLocation(n.shader, p.name.c_str());
        if (loc == -1)
            continue;

        if (p.type == InputType::Color)
        {
            float rgb[3] = {p.colorValue.r / 255.0f, p.colorValue.g / 255.0f, p.colorValue.b / 255.0f};
            SetShaderValue(n.shader, loc, rgb, SHADER_UNIFORM_VEC3);
        }
        else if (p.type == InputType::Vec2)
        {
            float xy[2] = {p.vec2Value.x, p.vec2Value.y};
            SetShaderValue(n.shader, loc, xy, SHADER_UNIFORM_VEC2);
        }
        else
        {
            SetShaderValue(n.shader, loc, &p.value, SHADER_UNIFORM_FLOAT);
        }
    }

    Texture2D quadTex = GetFullQuadTexture();
    Rectangle src{0, 0, static_cast<float>(quadTex.width), static_cast<float>(quadTex.height)};
    Rectangle dst{0, 0, n.textureSize.x, n.textureSize.y};
    DrawTexturePro(quadTex, src, dst, Vector2{0, 0}, 0, WHITE);

    EndShaderMode();
    EndTextureMode();
}

void World::Compute()
{
    for (Node &n : nodes)
    {
        ComputeNode(n, *this);
    }
}

void World::ReloadShaders()
{
    for (Node &n : nodes)
    {
        if (n.fragShaderPath.empty())
            continue;

        if (n.shaderLoaded)
            UnloadShader(n.shader);

        n.shaderLoaded = false;
    }
}

void World::RemoveNode(size_t index)
{
    Node &n = nodes[index];
    if (n.shaderLoaded)
        UnloadShader(n.shader);
    if (n.textureReady)
        UnloadRenderTexture(n.renderTexture);

    for (size_t i = 0; i < connections.size();)
    {
        if (connections[i].inNodeIdx == index || connections[i].outNodeIdx == index)
            connections.erase(connections.begin() + i);
        else
            i++;
    }

    nodes.erase(nodes.begin() + index);

    for (size_t i = index; i < nodes.size(); i++)
        nodes[i].id = i;

    for (Connection &c : connections)
    {
        if (c.inNodeIdx > index)
            c.inNodeIdx--;
        if (c.outNodeIdx > index)
            c.outNodeIdx--;
    }
}