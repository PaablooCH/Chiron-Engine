#pragma once
#include "Asset.h"

class IndexBuffer;
class VertexBuffer;

struct Vertex
{
    Vector3 vertices;
    Vector2 texCoords;
    Vector3 normals;
    Vector3 tangents;
    Vector3 biTangents;
};

class MeshAsset : public Asset
{
public:
    MeshAsset();
    MeshAsset(UID uid, const std::string& assetPath, const std::string& libraryPath);
    ~MeshAsset() override;

    // ------------- GETTERS ----------------------

    inline IndexBuffer* GetIndexBuffer();
    inline const std::vector<UINT>& GetIndexData();

    inline VertexBuffer* GetVertexBuffer();
    inline const std::vector<Vertex>& GetTriangleVertices();

    // ------------- SETTERS ----------------------

    void SetIndexBuffer(const D3D12_RESOURCE_DESC& resourceDesc, std::vector<UINT>& indexBufferData, const DXGI_FORMAT& indexFormat,
        const std::string& name = "");
    void SetVertexBuffer(const D3D12_RESOURCE_DESC& resourceDesc, std::vector<Vertex>& triangleVertices,
        const std::string& name = "");

private:
    bool InternalLoad() override;
    bool InternalUnload() override;

private:
    std::unique_ptr<IndexBuffer> _indexBuffer;
    std::unique_ptr<VertexBuffer> _vertexBuffer;

    std::vector<UINT> _indexBufferData;
    std::vector<Vertex> _triangleVertices;
};

inline IndexBuffer* MeshAsset::GetIndexBuffer()
{
    if (!IsValid())
    {
        Load();
    }
    return _indexBuffer.get();
}

inline const std::vector<UINT>& MeshAsset::GetIndexData()
{
    return _indexBufferData;
}

inline VertexBuffer* MeshAsset::GetVertexBuffer()
{
    if (!IsValid())
    {
        Load();
    }
    return _vertexBuffer.get();
}

inline const std::vector<Vertex>& MeshAsset::GetTriangleVertices()
{
    return _triangleVertices;
}
