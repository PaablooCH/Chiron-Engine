#pragma once
#include "Resource.h"

#include "DataModels/DX12/DescriptorAllocator/DescriptorAllocation.h"

class ConstantBuffer : public Resource
{
public:
    ConstantBuffer(const D3D12_RESOURCE_DESC& resourceDesc, size_t sizeInBytes, const std::string& name = "", bool load = false);
    ~ConstantBuffer();

private:
    void CreateView();

    bool InternalLoad() override;

private:
    size_t _sizeInBytes;

    mutable DescriptorAllocation _constantBufferView;
};
