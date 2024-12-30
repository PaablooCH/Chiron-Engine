#include "Pch.h"
#include "MeshAsset.h"

#include "Application.h"

#include "Modules/ModuleID3D12.h"

#include "DataModels/DX12/CommandList/CommandList.h"
#include "DataModels/DX12/Resource/IndexBuffer.h"
#include "DataModels/DX12/Resource/VertexBuffer.h"

MeshAsset::MeshAsset() : Asset(AssetType::Mesh)
{
}

MeshAsset::MeshAsset(UID uid, const std::string& assetPath, const std::string& libraryPath) :
    Asset(uid, assetPath, libraryPath, AssetType::Mesh)
{
}

MeshAsset::~MeshAsset()
{
}

void MeshAsset::SetIndexBuffer(const D3D12_RESOURCE_DESC& resourceDesc, std::vector<UINT>& indexBufferData,
    const DXGI_FORMAT& indexFormat, const std::string& name)
{
    _indexBuffer = std::make_unique<IndexBuffer>(resourceDesc, indexBufferData.size(), indexFormat, name);
    _indexBufferData = indexBufferData;
}

void MeshAsset::SetVertexBuffer(const D3D12_RESOURCE_DESC& resourceDesc, std::vector<Vertex>& triangleVertices, const std::string& name)
{
    _vertexBuffer = std::make_unique<VertexBuffer>(resourceDesc, triangleVertices.size(), sizeof(Vertex), name);
    _triangleVertices = triangleVertices;
}

bool MeshAsset::InternalLoad()
{
    bool result = true;
    bool commandsUsed = false;

    auto id3d12 = App->GetModule<ModuleID3D12>();
    auto copyCommandList = id3d12->GetCommandList(D3D12_COMMAND_LIST_TYPE_COPY);
    auto directCommandList = id3d12->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    if (_vertexBuffer)
    {
        result = result && _vertexBuffer->Load();

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = _triangleVertices.data();
        subresourceData.RowPitch = _vertexBuffer->GetResource()->GetDesc().Width;
        subresourceData.SlicePitch = _vertexBuffer->GetResource()->GetDesc().Width;
        copyCommandList->UpdateBufferResource(_vertexBuffer.get(), 0, 1, &subresourceData);
        directCommandList->TransitionBarrier(_vertexBuffer.get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        commandsUsed = true;
    }

    if (_indexBuffer)
    {
        result = result && _indexBuffer->Load();

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = _indexBufferData.data();
        subresourceData.RowPitch = _indexBuffer->GetResource()->GetDesc().Width;
        subresourceData.SlicePitch = _indexBuffer->GetResource()->GetDesc().Width;
        copyCommandList->UpdateBufferResource(_indexBuffer.get(), 0, 1, &subresourceData);

        directCommandList->TransitionBarrier(_indexBuffer.get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        commandsUsed = true;
    }

    if (commandsUsed)
    {
        auto signal = id3d12->ExecuteCommandList(copyCommandList);
        //id3d12->WaitForFenceValue(D3D12_COMMAND_LIST_TYPE_COPY, signal);
        signal = id3d12->ExecuteCommandList(directCommandList);
        //id3d12->WaitForFenceValue(D3D12_COMMAND_LIST_TYPE_DIRECT, signal);
    }

    return result;
}

bool MeshAsset::InternalUnload()
{
    bool result = false;
    if (_vertexBuffer)
    {
        result = result || _vertexBuffer->Unload();
    }

    if (_indexBuffer)
    {
        result = result || _indexBuffer->Unload();
    }
    return result;
}