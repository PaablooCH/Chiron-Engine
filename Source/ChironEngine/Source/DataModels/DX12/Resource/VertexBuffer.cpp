#include "Pch.h"
#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(const D3D12_RESOURCE_DESC& resourceDesc, size_t numVertices, size_t vertexStride,
    const std::string& name) : Resource(resourceDesc, name, nullptr),
    _numVertices(numVertices), _vertexStride(vertexStride)
{
    _vertexBufferView.BufferLocation = _resource->GetGPUVirtualAddress();
    _vertexBufferView.SizeInBytes = static_cast<UINT>(_numVertices * _vertexStride);
    _vertexBufferView.StrideInBytes = static_cast<UINT>(_vertexStride);
}

VertexBuffer::VertexBuffer(const VertexBuffer& copy) : Resource(copy), _numVertices(copy._numVertices),
_vertexStride(copy._vertexStride), _vertexBufferView(copy._vertexBufferView)
{
}

VertexBuffer::~VertexBuffer()
{
}