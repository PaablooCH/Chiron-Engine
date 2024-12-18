#include "Pch.h"
#include "Resource.h"

#include "Application.h"

#include "Modules/ModuleID3D12.h"

#include "DataModels/DX12/ResourceStateTracker/ResourceStateTracker.h"

Resource::Resource() : _resource(nullptr), _name(""), _device(nullptr), _featureSupport(), _clearValue(nullptr), 
_resourceDesc(), _loaded(false)
{
}

Resource::Resource(const D3D12_RESOURCE_DESC& resourceDesc, const std::string& name /* = "" */, bool load /* = false */, 
    const D3D12_CLEAR_VALUE* clearValue /* = nullptr */) : _resourceDesc(resourceDesc), _loaded(load)
{
    if (clearValue)
    {
        _clearValue = std::make_unique<D3D12_CLEAR_VALUE>(*clearValue);
    }

    _device = App->GetModule<ModuleID3D12>()->GetDevice();

    if (_loaded)
    {
        CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        Chiron::Utils::ThrowIfFailed(_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &_resourceDesc,
            D3D12_RESOURCE_STATE_COMMON, _clearValue.get(), IID_PPV_ARGS(&_resource)));

        ResourceStateTracker::AddGlobalResourceState(_resource.Get(), D3D12_RESOURCE_STATE_COMMON);
    }

    SetName(name);
    CheckFeatureSupport();
}

Resource::Resource(ComPtr<ID3D12Resource> resource) : _resource(resource), _name(""), _loaded(true)
{
    _device = App->GetModule<ModuleID3D12>()->GetDevice();
    _resourceDesc = resource->GetDesc();
    ResourceStateTracker::AddGlobalResourceState(_resource.Get(), D3D12_RESOURCE_STATE_COMMON);
    CheckFeatureSupport();
}

Resource::Resource(const Resource& copy) : _resource(copy._resource), _name(copy._name), _device(copy._device),
_featureSupport(copy._featureSupport), _resourceDesc(copy._resourceDesc), _loaded(copy._loaded)
{
    if (copy._clearValue)
    {
        _clearValue = std::make_unique<D3D12_CLEAR_VALUE>(*copy._clearValue);
    }
}

Resource::~Resource()
{
    ResourceStateTracker::RemoveGlobalResourceState(_resource.Get());
}

//Resource& Resource::operator=(const Resource& other)
//{
//	if (&other != this)
//	{
//		D3D12_RESOURCE_DESC resourceDesc = other._resource->GetDesc();
//		CreateCommittedResource(resourceDesc);
//		SetName(other._name);
//		_featureSupport = other._featureSupport;
//		_clearValue = std::make_unique<D3D12_CLEAR_VALUE>(other._clearValue);
//	}
//	return *this;
//}
//
//Resource& Resource::operator=(Resource&& other) noexcept
//{
//	if (&other != this)
//	{
//		D3D12_RESOURCE_DESC resourceDesc = other._resource->GetDesc();
//		CreateCommittedResource(resourceDesc);
//		SetName(other._name);
//		_featureSupport = other._featureSupport;
//		_clearValue = std::make_unique<D3D12_CLEAR_VALUE>(other._clearValue);
//
//		other.Reset();
//	}
//	return *this;
//}

void Resource::Reset()
{
    _resource.Reset();
    _name.clear();
    _featureSupport = { DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
}

bool Resource::CheckFormatSupport(D3D12_FORMAT_SUPPORT1 formatSupport) const
{
    return (_featureSupport.Support1 & formatSupport) != 0;
}

bool Resource::CheckFormatSupport(D3D12_FORMAT_SUPPORT2 formatSupport) const
{
    return (_featureSupport.Support2 & formatSupport) != 0;
}

bool Resource::Load()
{
    if (!_loaded)
    {
        CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        Chiron::Utils::ThrowIfFailed(_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &_resourceDesc,
            D3D12_RESOURCE_STATE_COMMON, _clearValue.get(), IID_PPV_ARGS(&_resource)));
        
        SetName(_name);
        ResourceStateTracker::AddGlobalResourceState(_resource.Get(), D3D12_RESOURCE_STATE_COMMON);

        _loaded = InternalLoad();
    }
    return _loaded;
}

bool Resource::Unload()
{
    if (_loaded)
    {
        ResourceStateTracker::RemoveGlobalResourceState(_resource.Get());
        _resource.Reset();

        _loaded = InternalUnload();
    }
    return _loaded;
}

void Resource::SetResource(ComPtr<ID3D12Resource> resource)
{
    ResourceStateTracker::RemoveGlobalResourceState(_resource.Get());
    _resource = resource;
    _resourceDesc = _resource->GetDesc();
    _loaded = true;
    ResourceStateTracker::AddGlobalResourceState(_resource.Get(), D3D12_RESOURCE_STATE_COMMON);
    CheckFeatureSupport();
}

void Resource::CheckFeatureSupport()
{
    _featureSupport.Format = _resourceDesc.Format;
    Chiron::Utils::ThrowIfFailed(_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &_featureSupport,
        sizeof(_featureSupport)));
}