#pragma once
#include "Resource.h"

class VertexBuffer : public Resource
{
public:
    VertexBuffer(const D3D12_RESOURCE_DESC& resourceDesc, size_t numVertices, size_t vertexStride,
        const std::string& name = "");
    VertexBuffer(const VertexBuffer& copy);

    ~VertexBuffer() override;

    // ------------- GETTERS ----------------------

    inline const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const;

private:
    VertexBuffer();

private:
    size_t _numVertices;
    size_t _vertexStride;
    D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;
};

inline const D3D12_VERTEX_BUFFER_VIEW& VertexBuffer::GetVertexBufferView() const
{
    return _vertexBufferView;
}