#pragma once

class Resource
{
public:
	inline bool IsValid();

	void Reset();

	bool CheckFormatSupport(D3D12_FORMAT_SUPPORT1 formatSupport) const;
	bool CheckFormatSupport(D3D12_FORMAT_SUPPORT2 formatSupport) const;

	// ------------- GETTERS ----------------------

	inline ID3D12Resource* GetResource() const;
	inline const std::wstring& GetName() const;

	/*virtual D3D12_CPU_DESCRIPTOR_HANDLE GetCPURenderTargetView() const = 0;
	virtual D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDepthStencilView() const = 0;
	virtual D3D12_CPU_DESCRIPTOR_HANDLE GetCPUShaderResourceView(uint32_t mips = 0) const = 0;
	virtual D3D12_CPU_DESCRIPTOR_HANDLE GetCPUUnorderedAccessView(uint32_t mips = 0) const = 0;*/

	// ------------- SETTERS ----------------------

	void SetResource(ComPtr<ID3D12Resource> resource);
	inline void SetName(std::wstring name);

protected:
	Resource();
	Resource(const D3D12_RESOURCE_DESC& resourceDesc, const std::wstring& name = L"",
		const D3D12_CLEAR_VALUE* clearValue = nullptr);
	Resource(ComPtr<ID3D12Resource> resource);
	Resource(const Resource& copy);

	virtual ~Resource();

	Resource& operator=(const Resource& other) = delete;
	Resource& operator=(Resource&& other) = delete;

private:
	void CheckFeatureSupport();

protected:
	ComPtr<ID3D12Resource> _resource;
	std::wstring _name;
private:
	D3D12_FEATURE_DATA_FORMAT_SUPPORT _featureSupport;
	std::unique_ptr<D3D12_CLEAR_VALUE> _clearValue;
};

inline bool Resource::IsValid()
{
	return _resource;
}

inline ID3D12Resource* Resource::GetResource() const
{
	return _resource.Get();
}

inline const std::wstring& Resource::GetName() const
{
	return _name;
}

inline void Resource::SetName(std::wstring name)
{
	_name = name;
	if (_resource)
	{
		_resource->SetName(_name.c_str());
	}
}
