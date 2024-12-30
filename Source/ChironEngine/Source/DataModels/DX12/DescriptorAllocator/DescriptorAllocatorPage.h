#pragma once

#include "DescriptorAllocation.h"

class DescriptorAllocatorPage : public std::enable_shared_from_this<DescriptorAllocatorPage>
{
public:
    DescriptorAllocatorPage(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap, const std::wstring& name);

    // ------------- MEMORY ----------------------

    // Allocate contiguous descriptors from the CPU
    DescriptorAllocation Allocate(uint32_t numDescriptors);
    bool HasSpace(uint32_t numDescriptors) const;

    // Add a contiguous space of memory to be released later
    void Free(DescriptorAllocation&& descriptorHandle, uint64_t frameNumber);

    // Release all the memory freed <= frame
    void ReleaseStaleDescriptors(uint64_t frameNumber);

    // ------------- GETTERS ----------------------

    inline std::shared_ptr<DescriptorAllocatorPage> GetSharedPtr();
    inline ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() const;
    inline D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const;
    uint32_t NumFreeHandles() const;

private:

    // Compute the offset of the descriptor handle from the start of the heap.
    uint32_t ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle) const;

    // ------------- MEMORY ----------------------

    // Adds a new block to the free list.
    void AddNewBlock(uint32_t offset, uint32_t numDescriptors);

    // Free a block of descriptors and merge, if possible, contiguous free blocks in the free list.
    void FreeBlock(uint32_t offset, uint32_t numDescriptors);

private:

    struct FreeBlockInfo;

    // Type of the map that keeps memory blocks sorted by their offsets
    using TFreeBlocksByOffsetMap = std::map<uint32_t, FreeBlockInfo>;

    // Type of the map that keeps memory blocks sorted by their sizes
    using TFreeBlocksBySizeMap = std::multimap<uint32_t, TFreeBlocksByOffsetMap::iterator>;

    struct FreeBlockInfo
    {
        FreeBlockInfo(uint32_t size) : size(size)
        {
        }

        uint32_t size;
        TFreeBlocksBySizeMap::iterator blockBySizeIt;
    };

    struct StaleDescriptorInfo
    {
        StaleDescriptorInfo(uint32_t offset, uint32_t size, uint64_t frame) : offset(offset), size(size),
            frameNumber(frame)
        {
        }

        // Offset within the descriptor heap.
        uint32_t offset;
        // Number of descriptors
        uint32_t size;
        // Frame that the descriptor was freed.
        uint64_t frameNumber;
    };

    TFreeBlocksByOffsetMap _freeBlocksByOffset;
    TFreeBlocksBySizeMap _freeBlocksBySize;
    // Descriptors returned to be released later
    std::queue<StaleDescriptorInfo> _staleDescriptors;

    // ------------- DESCRIPTOR INFO ----------------------

    ComPtr<ID3D12DescriptorHeap> _descriptorHeap;
    CD3DX12_CPU_DESCRIPTOR_HANDLE _baseCPUDescriptor;
    CD3DX12_GPU_DESCRIPTOR_HANDLE _baseGPUDescriptor;
    D3D12_DESCRIPTOR_HEAP_TYPE _heapType;
    uint32_t _descriptorSize;
    std::wstring _name;

    // Max num of descriptors
    uint32_t _numDescriptorsPerHeap;
    // Current free descriptors
    uint32_t _numFreeHandles;

    std::mutex _mutex;
};

inline std::shared_ptr<DescriptorAllocatorPage> DescriptorAllocatorPage::GetSharedPtr()
{
    return shared_from_this();
}

inline ComPtr<ID3D12DescriptorHeap> DescriptorAllocatorPage::GetDescriptorHeap() const
{
    return _descriptorHeap;
}

inline D3D12_DESCRIPTOR_HEAP_TYPE DescriptorAllocatorPage::GetHeapType() const
{
    return _heapType;
}

inline uint32_t DescriptorAllocatorPage::NumFreeHandles() const
{
    return _numFreeHandles;
}
