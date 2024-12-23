#pragma once

class Resource
{
public:
    inline bool IsValid() const;

    void Reset();

    bool CheckFormatSupport(D3D12_FORMAT_SUPPORT1 formatSupport) const;
    bool CheckFormatSupport(D3D12_FORMAT_SUPPORT2 formatSupport) const;

    bool Load();
    bool Unload();

    // ------------- GETTERS ----------------------

    inline ID3D12Resource* GetResource() const;
    inline const std::string& GetName() const;

    /*virtual D3D12_CPU_DESCRIPTOR_HANDLE GetCPURenderTargetView() const = 0;
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDepthStencilView() const = 0;
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetCPUShaderResourceView(uint32_t mips = 0) const = 0;
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetCPUUnorderedAccessView(uint32_t mips = 0) const = 0;*/

    // ------------- SETTERS ----------------------

    void SetResource(ComPtr<ID3D12Resource> resource);
    inline void SetName(std::string name);

protected:
    Resource();
    Resource(const D3D12_RESOURCE_DESC& resourceDesc, const std::string& name = "", bool load = false,
        const D3D12_CLEAR_VALUE* clearValue = nullptr);
    Resource(ComPtr<ID3D12Resource> resource);
    Resource(const Resource& copy);

    virtual ~Resource();

    Resource& operator=(const Resource& other) = delete;
    Resource& operator=(Resource&& other) = delete;

    virtual bool InternalLoad() { return true; };
    virtual bool InternalUnload() { return false; };

private:
    void CheckFeatureSupport();

protected:
    ComPtr<ID3D12Resource> _resource;
    std::string _name;

    ID3D12Device5* _device;
private:
    D3D12_FEATURE_DATA_FORMAT_SUPPORT _featureSupport;
    std::unique_ptr<D3D12_CLEAR_VALUE> _clearValue;
    D3D12_RESOURCE_DESC _resourceDesc;

    bool _loaded;
};

inline bool Resource::IsValid() const
{
    return _loaded;
}

inline ID3D12Resource* Resource::GetResource() const
{
    return _resource.Get();
}

inline const std::string& Resource::GetName() const
{
    return _name;
}

inline void Resource::SetName(std::string name)
{
    _name = name;
    if (_resource)
    {
        std::wstring w = Chiron::Utils::StringToWString(_name);
        _resource->SetName(w.c_str());
    }
}
