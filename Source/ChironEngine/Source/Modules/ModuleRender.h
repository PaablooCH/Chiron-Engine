#pragma once
#include "Module.h"

class CommandList;
class DebugDrawPass;
class ModelAsset;
class Texture;

class ModuleRender : public Module
{
public:
    ModuleRender();
    ~ModuleRender();

    bool Init() override;
    UpdateStatus PreUpdate() override;
    UpdateStatus Update() override;
    UpdateStatus PostUpdate() override;
    bool CleanUp() override;

    void ResizeBuffers(unsigned newWidth, unsigned newHeight);

    // ------------- GETTERS ----------------------

    inline const Texture* GetSceneTexture() const;

    std::shared_ptr<ModelAsset> model;
private:
    void CreateTextures();

private:
    std::unique_ptr<DebugDrawPass> _debugDraw;

    std::unique_ptr<Texture> _sceneTexture;
    std::unique_ptr<Texture> _depthStencilTexture;

    std::shared_ptr<CommandList> _drawCommandList;

    D3D12_RECT _scissor;
};

inline const Texture* ModuleRender::GetSceneTexture() const
{
    return _sceneTexture.get();
}