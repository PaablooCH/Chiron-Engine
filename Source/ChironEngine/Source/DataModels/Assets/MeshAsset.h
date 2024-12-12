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
    ~MeshAsset() override;

    // ------------- GETTERS ----------------------

    inline IndexBuffer* GetIndexBuffer() const;
    inline VertexBuffer* GetVertexBuffer() const;
    inline std::string GetAssetPath() const override;
    inline std::string GetLibraryPath() const override;

    // ------------- SETTERS ----------------------

    void SetIndexBuffer(const D3D12_RESOURCE_DESC& resourceDesc, size_t numIndices, const DXGI_FORMAT& indexFormat,
        const std::string& name = "");
    void SetVertexBuffer(const D3D12_RESOURCE_DESC& resourceDesc, size_t numVertices, const std::string& name = "");

private:
    std::unique_ptr<IndexBuffer> _indexBuffer;
    std::unique_ptr<VertexBuffer> _vertexBuffer;
};

inline IndexBuffer* MeshAsset::GetIndexBuffer() const
{
    return _indexBuffer.get();
}

inline VertexBuffer* MeshAsset::GetVertexBuffer() const
{
    return _vertexBuffer.get();
}

inline std::string MeshAsset::GetAssetPath() const
{
    return MESHES_PATH + GetName();
}

inline std::string MeshAsset::GetLibraryPath() const
{
    return MESHES_LIB_PATH + std::to_string(GetUID()) + GENERAL_BINARY_EXTENSION;
}
