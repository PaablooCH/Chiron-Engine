#include "Pch.h"
#include "ModuleRender.h"

#include "Application.h"

#include "ModuleCamera.h"
#include "ModuleID3D12.h"
#include "ModuleProgram.h"
#include "ModuleWindow.h"

#include "DataModels/Camera/Camera.h"


#include "DataModels/DX12/CommandList/CommandList.h"
#include "DataModels/DX12/RootSignature/RootSignature.h"
#include "DataModels/DX12/Resource/Texture.h"

#include "DataModels/Programs/Program.h"

#include "Structs/ViewProjection.h"

#include "DebugDrawPass.h"

#if OPTICK
    #include "Optick/optick.h"
#endif // OPTICK

ModuleRender::ModuleRender() : _scissor(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX)), _sceneTexture(nullptr),
_depthStencilTexture(nullptr)
{
}

ModuleRender::~ModuleRender()
{
}

bool ModuleRender::Init()
{
    auto d3d12 = App->GetModule<ModuleID3D12>();

    auto commandQueue = d3d12->GetID3D12CommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    _debugDraw = std::make_unique<DebugDrawPass>(d3d12->GetDevice(), commandQueue);

    CreateTextures();

    return true;
}

UpdateStatus ModuleRender::PreUpdate()
{
#if OPTICK
    OPTICK_CATEGORY("PreUpdateRender", Optick::Category::Rendering);
#endif // DEBUG
    auto d3d12 = App->GetModule<ModuleID3D12>();

    _drawCommandList = d3d12->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    // Clear Viewport
    FLOAT clearColor[] = { 0.4f, 0.4f, 0.4f, 1.0f }; // Set color

    // send the clear command into the list
    _drawCommandList->ClearRenderTargetView(_sceneTexture.get(), clearColor, 0);
    _drawCommandList->ClearDepthStencilView(_depthStencilTexture.get(), D3D12_CLEAR_FLAG_DEPTH, 1.0, 0, 0);

    return UpdateStatus::UPDATE_CONTINUE;
}

UpdateStatus ModuleRender::Update()
{
#if OPTICK
    OPTICK_CATEGORY("UpdateRender", Optick::Category::Rendering);
#endif // DEBUG
    auto d3d12 = App->GetModule<ModuleID3D12>();
    auto programs = App->GetModule<ModuleProgram>();
    auto window = App->GetModule<ModuleWindow>();
    auto moduleCamera = App->GetModule<ModuleCamera>();

    Program* defaultP = programs->GetProgram(ProgramType::DEFAULT);

    _drawCommandList->UseProgram(defaultP);

    _drawCommandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    unsigned width;
    unsigned height;
    window->GetWindowSize(width, height);

    D3D12_VIEWPORT viewport{};
    viewport.TopLeftX = viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);

    _drawCommandList->SetViewports(1, viewport);
    _drawCommandList->SetScissorRects(1, _scissor);

    auto camera = moduleCamera->GetCamera();
    Matrix view = camera->GetViewMatrix();
    Matrix proj = camera->GetProjMatrix();

    ViewProjection vp;
    vp.view = view.Transpose();
    vp.proj = proj.Transpose();
    _drawCommandList->SetGraphicsDynamicConstantBuffer(0, vp);

    auto rtv = _sceneTexture->GetRenderTargetView().GetCPUDescriptorHandle();
    auto dsv = _depthStencilTexture->GetDepthStencilView().GetCPUDescriptorHandle();
    _drawCommandList->SetRenderTargets(1, &rtv, FALSE, &dsv);


    // ------------- DEBUG DRAW ----------------------

    dd::xzSquareGrid(-500.0f, 500.0f, 0.0f, 1.0f, dd::colors::LightGray);
    dd::axisTriad(Chiron::Utils::ddConvert(Matrix::Identity), 0.5f, 1000.0f);

    _debugDraw->record(_drawCommandList->GetGraphicsCommandList().Get(), width, height, view, proj);

    // ------------- CLOSE COMMANDLIST ----------------------

    d3d12->ExecuteCommandList(_drawCommandList);

    return UpdateStatus::UPDATE_CONTINUE;
}

UpdateStatus ModuleRender::PostUpdate()
{
    return UpdateStatus::UPDATE_CONTINUE;
}

bool ModuleRender::CleanUp()
{
    _sceneTexture.reset();
    _drawCommandList.reset();
    _depthStencilTexture.reset();
    _debugDraw.reset();
    _drawCommandList = nullptr;

    return true;
}

void ModuleRender::ResizeBuffers(unsigned newWidth, unsigned newHeight)
{
    FLOAT clearColor[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    memcpy(clearValue.Color, clearColor, sizeof(clearColor));

    D3D12_RESOURCE_DESC textureDesc =
        CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, newWidth, newHeight, 1, 1, 1, 0,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
    _sceneTexture = std::make_unique<Texture>(textureDesc, "Scene Texture", &clearValue);

    _depthStencilTexture = App->GetModule<ModuleID3D12>()->
        CreateDepthStencil("Scene Depth Stencil Texture", newWidth, newHeight);
}

void ModuleRender::CreateTextures()
{
    unsigned width;
    unsigned height;
    App->GetModule<ModuleWindow>()->GetWindowSize(width, height);

    FLOAT clearColor[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    memcpy(clearValue.Color, clearColor, sizeof(clearColor));

    D3D12_RESOURCE_DESC textureDesc =
        CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1, 1, 0,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
    _sceneTexture = std::make_unique<Texture>(textureDesc, "Scene Texture", &clearValue);

    _depthStencilTexture = App->GetModule<ModuleID3D12>()->CreateDepthStencil("Scene Depth Stencil Texture", width, height);
}