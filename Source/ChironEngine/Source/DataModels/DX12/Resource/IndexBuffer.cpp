#include "Pch.h"
#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(const D3D12_RESOURCE_DESC& resourceDesc, size_t numIndices, const DXGI_FORMAT& indexFormat,
    const std::string& name /* = "" */, bool load /* = false */) : Resource(resourceDesc, name, load),
    _numIndices(numIndices), _format(indexFormat)
{
    int stride = _format == DXGI_FORMAT_R32_UINT ? 4 : 2;

    _indexBufferView.SizeInBytes = static_cast<UINT>(numIndices * stride);
    _indexBufferView.Format = indexFormat;
}

IndexBuffer::IndexBuffer(const IndexBuffer& copy) : Resource(copy), _numIndices(copy._numIndices), _format(copy._format),
_indexBufferView(copy._indexBufferView)
{
}

IndexBuffer::~IndexBuffer()
{
}

bool IndexBuffer::InternalLoad()
{
    _indexBufferView.BufferLocation = _resource->GetGPUVirtualAddress();
    return true;
}