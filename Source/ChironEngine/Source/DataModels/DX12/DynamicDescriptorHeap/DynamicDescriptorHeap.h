#pragma once

class CommandList;
class RootSignature;

class DynamicDescriptorHeap
{
public:
    DynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32_t numDescriptorsPerHeap = 1024);

    ~DynamicDescriptorHeap();

    // ------------- MEMORY GPU ----------------------

    // Stages a contiguous range of CPU visible descriptors. Descriptors are copied to the GPU when the
    // CommitStagedDescriptors function is called.
    void StageDescriptors(uint32_t rootParameterIndex, uint32_t offset, uint32_t numDescriptors,
        const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptors);

    // Copy all of the staged descriptors to the GPU visible descriptor heap and bind the descriptor heap and
    // the descriptor tables to the command list.
    void CommitStagedDescriptorsForDraw(CommandList& commandList);
    void CommitStagedDescriptorsForDispatch(CommandList& commandList);

    // Copies a single CPU visible descriptor to a GPU visible descriptor heap.
    D3D12_GPU_DESCRIPTOR_HANDLE CopyDescriptor(CommandList& comandList, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor);

    // Determine which root parameters contain descriptor tables and the number of descriptors needed for each table.
    void ParseRootSignature(const RootSignature* rootSignature);

    // Reset used descriptors. Call only if any descriptors has finished executing on the command queue.
    void Reset();

private:
    // Copy all of the staged descriptors to the GPU visible descriptor heap and bind the descriptor heap and
    // the descriptor tables to the command list. The passed-in function object is used to set the GPU visible descriptors
    // on the command list. Two possible functions are:
    // Before a draw: ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable
    // Before a dispatch: ID3D12GraphicsCommandList::SetComputeRootDescriptorTable
    void CommitStagedDescriptors(CommandList& commandList,
        std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc);

    ComPtr<ID3D12DescriptorHeap> RequestDescriptorHeap();
    ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap() const;

    // Compute the number of stale descriptors that need to be copied to GPU visible descriptor heap.
    uint32_t ComputeStaleDescriptorCount() const;

private:
    // Same values as in RootSignature class.
    static const uint32_t MaxDescriptorTables = 32;

    // Represents a descriptor table entry in the root signature.
    struct DescriptorTableCache
    {
        DescriptorTableCache()
            : NumDescriptors(0)
            , BaseDescriptor(nullptr)
        {
        }

        // Reset the table cache.
        void Reset()
        {
            NumDescriptors = 0;
            BaseDescriptor = nullptr;
        }

        // The number of descriptors in this descriptor table.
        uint32_t NumDescriptors;
        // The pointer to the descriptor in the descriptor handle cache.
        D3D12_CPU_DESCRIPTOR_HANDLE* BaseDescriptor;
    };

    // Valid values are CBV_SRV_UAV and SAMPLER.
    D3D12_DESCRIPTOR_HEAP_TYPE _descriptorHeapType;

    uint32_t _numDescriptorsPerHeap;

    // Descriptor size, vendor-dependent.
    uint32_t _descriptorHandleIncrementSize;

    // Descriptor total memory space.
    std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE[]> _descriptorHandleCache;

    // Store all the root signature's tables.
    DescriptorTableCache _descriptorTableCache[MaxDescriptorTables];

    // Each bit represents the index in the root signature that contains a descriptor table.
    uint32_t _descriptorTableBitMask;
    // Each bit represents a descriptor table in the root signature that has changed since the last time the
    // descriptors were copied.
    uint32_t _staleDescriptorTableBitMask;

    using DescriptorHeapPool = std::queue<ComPtr<ID3D12DescriptorHeap>>;

    DescriptorHeapPool _descriptorHeapPool;
    DescriptorHeapPool _availableDescriptorHeaps;

    ComPtr<ID3D12DescriptorHeap> _currentDescriptorHeap;
    CD3DX12_GPU_DESCRIPTOR_HANDLE _currentGPUDescriptorHandle;
    CD3DX12_CPU_DESCRIPTOR_HANDLE _currentCPUDescriptorHandle;

    uint32_t _numFreeHandles;

    ID3D12Device5* _device;
};
