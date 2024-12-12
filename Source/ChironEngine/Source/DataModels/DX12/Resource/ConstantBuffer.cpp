#include "Pch.h"
#include "ConstantBuffer.h"

#include "Application.h"

#include "Modules/ModuleID3D12.h"

#include "DataModels/DX12/DescriptorAllocator/DescriptorAllocator.h"

ConstantBuffer::ConstantBuffer(const D3D12_RESOURCE_DESC& resourceDesc, size_t sizeInBytes, const std::string& name) :
    Resource(resourceDesc, name), _sizeInBytes(sizeInBytes)
{
}

ConstantBuffer::~ConstantBuffer()
{
}

void ConstantBuffer::CreateView()
{
    auto id3d12 = App->GetModule<ModuleID3D12>();

    auto descriptorAllocator = id3d12->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    _constantBufferView = descriptorAllocator->Allocate();

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = _resource->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = static_cast<UINT>(Chiron::Utils::AlignUp(_sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT));

    // Create CBV
    _device->CreateConstantBufferView(&cbvDesc, _constantBufferView.GetCPUDescriptorHandle());
}

void ConstantBuffer::InternalLoad()
{
    CreateView();
}