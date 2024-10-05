#pragma once
#include "Resource.h"

#include "DataModels/DX12/DescriptorAllocator/DescriptorAllocation.h"

class ConstantBuffer : public Resource
{
public:
    ConstantBuffer(const D3D12_RESOURCE_DESC& resourceDesc, size_t sizeInBytes, const std::wstring& name = L"");
    ~ConstantBuffer();

private:
    void CreateView();

private:
    size_t _sizeInBytes;

    mutable DescriptorAllocation _constantBufferView;
};
