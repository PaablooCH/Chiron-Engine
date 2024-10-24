#include "Pch.h"
#include "MeshAsset.h"

#include "DataModels/DX12/Resource/IndexBuffer.h"
#include "DataModels/DX12/Resource/VertexBuffer.h"

MeshAsset::MeshAsset() : Asset(AssetType::Mesh)
{
}

MeshAsset::~MeshAsset()
{
}

void MeshAsset::SetIndexBuffer(const D3D12_RESOURCE_DESC& resourceDesc, size_t numIndices, const DXGI_FORMAT& indexFormat,
    const std::string& name)
{
    _indexBuffer = std::make_unique<IndexBuffer>(resourceDesc, numIndices, indexFormat, name);
}

void MeshAsset::SetVertexBuffer(const D3D12_RESOURCE_DESC& resourceDesc, size_t numVertices, const std::string& name)
{
    _vertexBuffer = std::make_unique<VertexBuffer>(resourceDesc, numVertices, sizeof(Vertex), name);
}