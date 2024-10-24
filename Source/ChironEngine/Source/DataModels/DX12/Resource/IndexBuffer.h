#pragma once
#include "Resource.h"

class IndexBuffer : public Resource
{
public:
    IndexBuffer(const D3D12_RESOURCE_DESC& resourceDesc, size_t numIndices, const DXGI_FORMAT& indexFormat,
        const std::string& name = "");
    IndexBuffer(const IndexBuffer& copy);

    ~IndexBuffer() override;

    // ------------- GETTERS ----------------------

    inline const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const;
    inline const size_t& GetNumIndices() const;

private:
    IndexBuffer();

private:
    size_t _numIndices;
    DXGI_FORMAT _format;
    D3D12_INDEX_BUFFER_VIEW _indexBufferView;
};

inline const D3D12_INDEX_BUFFER_VIEW& IndexBuffer::GetIndexBufferView() const
{
    return _indexBufferView;
}

inline const size_t& IndexBuffer::GetNumIndices() const
{
    return _numIndices;
}
