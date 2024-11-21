#include "Pch.h"
#include "DefaultProgram.h"

#include "Structs/ModelAttributes.h"

DefaultProgram::DefaultProgram(const std::string& name) : Program(name, true)
{
    InitRootSignature();
    InitPipelineState();
}

DefaultProgram::~DefaultProgram()
{
}

void DefaultProgram::InitRootSignature()
{
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    // It's a good idea to sort parameters by frequency of change.
    CD3DX12_ROOT_PARAMETER1 rootParameters[3]{};

    // ------------- CONSTANT BUFFER ----------------------

    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);

    // ------------- INLINE CONSTANT BUFFER ----------------------

    rootParameters[1].InitAsConstants(sizeof(ModelAttributes) / 4, 1);

    // ------------- DESCRIPTOR TABLE ----------------------

    CD3DX12_DESCRIPTOR_RANGE1 srv(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
        D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);

    rootParameters[2].InitAsDescriptorTable(1, &srv, D3D12_SHADER_VISIBILITY_PIXEL);

    // ------------- STATIC SAMPLER ----------------------

    CD3DX12_STATIC_SAMPLER_DESC staticSampler = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_ANISOTROPIC,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 16,
        D3D12_COMPARISON_FUNC_NEVER, D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE, 0.0f, D3D12_FLOAT32_MAX,
        D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription{};
    rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 1, &staticSampler, rootSignatureFlags);

    CreateRootSignature(rootSignatureDescription.Desc_1_1);
    _rootSignature->GetID3D12RootSignature()->SetName(L"Default Root Signature");
}

void DefaultProgram::InitPipelineState()
{
    UINT compileFlags = 0;
#ifdef DEBUG
    // Enable better shader debugging with the graphics debugging tools.
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

    ComPtr<ID3DBlob> errorBuff;
    if (FAILED(D3DCompileFromFile(L"Shaders/default.hlsl", nullptr, nullptr, "VSmain", "vs_5_1", compileFlags, 0,
        &_vertexShader, &errorBuff)))
    {
        OutputDebugStringA((char*)errorBuff->GetBufferPointer());
    }

    if (errorBuff)
    {
        errorBuff->Release();
    }

    errorBuff.Reset();

    if (FAILED(D3DCompileFromFile(L"Shaders/default.hlsl", nullptr, nullptr, "PSmain", "ps_5_1", compileFlags, 0,
        &_pixelShader, &errorBuff)))
    {
        OutputDebugStringA((char*)errorBuff->GetBufferPointer());
    }

    if (errorBuff)
    {
        errorBuff->Release();
    }

    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
    inputLayoutDesc.NumElements = sizeof(inputElementDescs) / sizeof(D3D12_INPUT_ELEMENT_DESC);
    inputLayoutDesc.pInputElementDescs = inputElementDescs;

    // Specify depth and stencil values
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthStencilDesc.StencilEnable = TRUE;
    depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

    depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

    depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

    D3D12_RASTERIZER_DESC rasterizeState{};
    rasterizeState.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizeState.CullMode = D3D12_CULL_MODE_BACK;
    rasterizeState.FrontCounterClockwise = TRUE;
    rasterizeState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizeState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizeState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizeState.DepthClipEnable = TRUE;
    rasterizeState.MultisampleEnable = FALSE;
    rasterizeState.AntialiasedLineEnable = FALSE;
    rasterizeState.ForcedSampleCount = 0;
    rasterizeState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = inputLayoutDesc;
    psoDesc.pRootSignature = _rootSignature->GetID3D12RootSignature();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(_vertexShader.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(_pixelShader.Get());
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc = { 1, 0 };
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.RasterizerState = rasterizeState;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.NumRenderTargets = 1;
    psoDesc.DepthStencilState = depthStencilDesc;

    CreateGraphicPipelineState(&psoDesc);
    _pipelineState->SetName(L"Default Pipeline");
}