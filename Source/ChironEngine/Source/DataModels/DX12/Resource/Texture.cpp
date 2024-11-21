#include "Pch.h"
#include "Texture.h"

#include "Application.h"

#include "Modules/ModuleID3D12.h"

#include "DataModels/DX12/ResourceStateTracker/ResourceStateTracker.h"
#include "DataModels/DX12/DescriptorAllocator/DescriptorAllocator.h"

Texture::Texture() : Resource()
{
}

Texture::Texture(const D3D12_RESOURCE_DESC& resourceDesc, const std::string& name,
    const D3D12_CLEAR_VALUE* clearValue) : Resource(resourceDesc, name, clearValue)
{
    CreateViews();
}

Texture::Texture(ComPtr<ID3D12Resource> resource) : Resource(resource)
{
    CreateViews();
}

Texture::Texture(const Texture& copy) : Resource(copy)
{
    CreateViews();
}

Texture::~Texture()
{
}

//Texture& Texture::operator=(const Texture& other)
//{
//	Resource::operator=(other);
//
//	CreateViews();
//
//	return *this;
//}
//
//Texture& Texture::operator=(Texture&& other) noexcept
//{
//	Resource::operator=(other);
//
//	CreateViews();
//
//	return *this;
//}

void Texture::Resize(uint32_t width, uint32_t height, uint32_t depthOrArraySize)
{
    if (_resource)
    {
        ResourceStateTracker::RemoveGlobalResourceState(_resource.Get());

        CD3DX12_RESOURCE_DESC textureDesc(_resource->GetDesc());

        textureDesc.Width = width > 1U ? width : 1U;
        textureDesc.Height = height > 1U ? height : 1U;
        textureDesc.DepthOrArraySize = depthOrArraySize > 1U ? depthOrArraySize : 1U;

        CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
        Chiron::Utils::ThrowIfFailed(_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &textureDesc,
            D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&_resource)));

        SetName(_name);

        ResourceStateTracker::AddGlobalResourceState(_resource.Get(), D3D12_RESOURCE_STATE_COMMON);

        CreateViews();
    }
}

void Texture::CreateViews()
{
    if (_resource)
    {
        auto id3d12 = App->GetModule<ModuleID3D12>();

        auto resourceDesc = _resource->GetDesc();

        if ((resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0 && CompatibleWithRTV())
        {
            auto descriptorAllocator = id3d12->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            _renderTargetView = descriptorAllocator->Allocate();
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = CreateRTVDesc(resourceDesc);
            _device->CreateRenderTargetView(_resource.Get(), &rtvDesc, _renderTargetView.GetCPUDescriptorHandle());
        }
        if ((resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0 && CompatibleWithDSV())
        {
            auto descriptorAllocator = id3d12->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
            _depthStencilView = descriptorAllocator->Allocate();
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = CreateDSVDesc(resourceDesc);
            _device->CreateDepthStencilView(_resource.Get(), &dsvDesc, _depthStencilView.GetCPUDescriptorHandle());
        }
        if ((resourceDesc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) == 0 && CompatibleWithSRV())
        {
            auto descriptorAllocator = id3d12->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            _shaderResourceView = descriptorAllocator->Allocate();
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = CreateSRVDesc(resourceDesc);
            _device->CreateShaderResourceView(_resource.Get(), &srvDesc, _shaderResourceView.GetCPUDescriptorHandle());
        }
        if ((resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) != 0 && CompatibleWithUAV()
            && resourceDesc.DepthOrArraySize == 1)
        {
            auto descriptorAllocator = id3d12->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            _unorderedAccessView = descriptorAllocator->Allocate(resourceDesc.MipLevels);
            for (int i = 0; i < resourceDesc.MipLevels; i++)
            {
                auto uavDesc = CreateUAVDesc(resourceDesc, i);
                _device->CreateUnorderedAccessView(_resource.Get(), nullptr, &uavDesc, _unorderedAccessView.GetCPUDescriptorHandle());
            }
        }
    }
}

void Texture::SetTexture(ComPtr<ID3D12Resource> texture)
{
    SetResource(texture);
    CreateViews();
}

DXGI_FORMAT Texture::GetUAVCompatibleFormat(DXGI_FORMAT format)
{
    DXGI_FORMAT uavFormat = format;

    switch (format)
    {
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8X8_TYPELESS:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        uavFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        break;
    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
        uavFormat = DXGI_FORMAT_R32_FLOAT;
        break;
    }

    return uavFormat;
}

D3D12_RENDER_TARGET_VIEW_DESC Texture::CreateRTVDesc(const D3D12_RESOURCE_DESC& resourceDesc)
{
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = resourceDesc.Format;

    switch (resourceDesc.Dimension)
    {
    case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
        if (resourceDesc.DepthOrArraySize > 1)
        {
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
            rtvDesc.Texture1DArray.MipSlice = 0;
            rtvDesc.Texture1DArray.FirstArraySlice = 0;
            rtvDesc.Texture1DArray.ArraySize = resourceDesc.DepthOrArraySize;
        }
        else
        {
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
            rtvDesc.Texture1D.MipSlice = 0;
        }
        break;

    case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
        if (resourceDesc.DepthOrArraySize > 1)
        {
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
            rtvDesc.Texture2DArray.MipSlice = 0;
            rtvDesc.Texture2DArray.FirstArraySlice = 0;
            rtvDesc.Texture2DArray.ArraySize = resourceDesc.DepthOrArraySize;
            rtvDesc.Texture2DArray.PlaneSlice = 0;
        }
        else
        {
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = 0;
            rtvDesc.Texture2D.PlaneSlice = 0;
        }
        break;

    case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
        rtvDesc.Texture3D.MipSlice = 0;
        rtvDesc.Texture3D.FirstWSlice = 0;
        rtvDesc.Texture3D.WSize = resourceDesc.DepthOrArraySize;
        break;

    default:
        throw std::invalid_argument("Unsupported resource dimension.");
    }

    return rtvDesc;
}

D3D12_DEPTH_STENCIL_VIEW_DESC Texture::CreateDSVDesc(const D3D12_RESOURCE_DESC& resourceDesc)
{
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = resourceDesc.Format;

    switch (resourceDesc.Dimension)
    {
    case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
        if (resourceDesc.DepthOrArraySize > 1)
        {
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
            dsvDesc.Texture1DArray.MipSlice = 0;
            dsvDesc.Texture1DArray.FirstArraySlice = 0;
            dsvDesc.Texture1DArray.ArraySize = resourceDesc.DepthOrArraySize;
        }
        else
        {
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
            dsvDesc.Texture1D.MipSlice = 0;
        }
        break;

    case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
        if (resourceDesc.DepthOrArraySize > 1)
        {
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            dsvDesc.Texture2DArray.MipSlice = 0;
            dsvDesc.Texture2DArray.FirstArraySlice = 0;
            dsvDesc.Texture2DArray.ArraySize = resourceDesc.DepthOrArraySize;
        }
        else
        {
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = 0;
        }
        break;

    default:
        throw std::invalid_argument("Unsupported resource dimension.");
    }

    return dsvDesc;
}

D3D12_SHADER_RESOURCE_VIEW_DESC Texture::CreateSRVDesc(const D3D12_RESOURCE_DESC& resourceDesc)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = resourceDesc.Format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    switch (resourceDesc.Dimension)
    {
    case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
        if (resourceDesc.DepthOrArraySize > 1)
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
            srvDesc.Texture1DArray.MostDetailedMip = 0;
            srvDesc.Texture1DArray.MipLevels = resourceDesc.MipLevels;
            srvDesc.Texture1DArray.FirstArraySlice = 0;
            srvDesc.Texture1DArray.ArraySize = resourceDesc.DepthOrArraySize;
            srvDesc.Texture1DArray.ResourceMinLODClamp = 0.0f;
        }
        else
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
            srvDesc.Texture1D.MostDetailedMip = 0;
            srvDesc.Texture1D.MipLevels = resourceDesc.MipLevels;
            srvDesc.Texture1D.ResourceMinLODClamp = 0.0f;
        }
        break;

    case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
        if (resourceDesc.DepthOrArraySize > 1)
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            srvDesc.Texture2DArray.MostDetailedMip = 0;
            srvDesc.Texture2DArray.MipLevels = resourceDesc.MipLevels;
            srvDesc.Texture2DArray.FirstArraySlice = 0;
            srvDesc.Texture2DArray.ArraySize = resourceDesc.DepthOrArraySize;
            srvDesc.Texture2DArray.PlaneSlice = 0;
            srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
        }
        else
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = resourceDesc.MipLevels;
            srvDesc.Texture2D.PlaneSlice = 0;
            srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        }
        break;

    case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
        srvDesc.Texture3D.MostDetailedMip = 0;
        srvDesc.Texture3D.MipLevels = resourceDesc.MipLevels;
        srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
        break;

    default:
        throw std::invalid_argument("Unsupported resource dimension.");
    }

    return srvDesc;
}

D3D12_UNORDERED_ACCESS_VIEW_DESC Texture::CreateUAVDesc(const D3D12_RESOURCE_DESC& resourceDesc, UINT mipSlice,
    UINT arraySlice, UINT planeSlice)
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = resourceDesc.Format;

    switch (resourceDesc.Dimension)
    {
    case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
        if (resourceDesc.DepthOrArraySize > 1)
        {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
            uavDesc.Texture1DArray.MipSlice = mipSlice;
            uavDesc.Texture1DArray.FirstArraySlice = arraySlice;
            uavDesc.Texture1DArray.ArraySize = resourceDesc.DepthOrArraySize;
        }
        else
        {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
            uavDesc.Texture1D.MipSlice = mipSlice;
        }
        break;

    case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
        if (resourceDesc.DepthOrArraySize > 1)
        {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
            uavDesc.Texture2DArray.MipSlice = mipSlice;
            uavDesc.Texture2DArray.FirstArraySlice = arraySlice;
            uavDesc.Texture2DArray.ArraySize = resourceDesc.DepthOrArraySize;
            uavDesc.Texture2DArray.PlaneSlice = planeSlice;
        }
        else
        {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Texture2D.MipSlice = mipSlice;
            uavDesc.Texture2D.PlaneSlice = planeSlice;
        }
        break;

    case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
        uavDesc.Texture3D.MipSlice = mipSlice;
        uavDesc.Texture3D.FirstWSlice = 0;
        uavDesc.Texture3D.WSize = resourceDesc.DepthOrArraySize;
        break;

    default:
        throw std::invalid_argument("Unsupported resource dimension.");
    }

    return uavDesc;
}