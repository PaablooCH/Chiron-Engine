#include "Pch.h"
#include "CommandQueue.h"

#include "Application.h"

#include "Modules/ModuleID3D12.h"

#include "DataModels/DX12/CommandList/CommandList.h"
#include "DataModels/DX12/ResourceStateTracker/ResourceStateTracker.h"

CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE type) : _type(type), _fenceValue(0)
{
    auto device = App->GetModule<ModuleID3D12>()->GetDevice();

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = _type;
    Chiron::Utils::ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue)));

    Chiron::Utils::ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence)));
    _fenceEvent = Chiron::Utils::CreateEventHandle();

    switch (type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        _commandQueue->SetName(L"Direct Command Queue");
        _fence->SetName(L"Direct Fence");
        break;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        _commandQueue->SetName(L"Compute Command Queue");
        _fence->SetName(L"Compute Fence");
        break;
    case D3D12_COMMAND_LIST_TYPE_COPY:
        _commandQueue->SetName(L"Copy Command Queue");
        _fence->SetName(L"Copy Fence");
        break;
    }
}

CommandQueue::~CommandQueue()
{
    Flush();
    ::CloseHandle(_fenceEvent);
}

uint64_t CommandQueue::ExecuteCommandList(std::shared_ptr<CommandList> commandList)
{
    if (!IsValidCommandListForQueue(commandList->GetType()))
    {
        LOG_ERROR("CommandList type doesn't match the commandQueue type.");
        return 0;
    }
    std::vector<ID3D12CommandList*> commandListsToPush;
    commandListsToPush.reserve(2);

    ResourceStateTracker::Lock();

    auto pendingBarriersList = GetCommandList();
    bool hasPendingBarriers = commandList->Close(pendingBarriersList.get());

    ResourceStateTracker::Unlock();

    pendingBarriersList->Close();

    if (hasPendingBarriers)
    {
        commandListsToPush.push_back(pendingBarriersList->GetGraphicsCommandList().Get());
    }

    commandListsToPush.push_back(commandList->GetGraphicsCommandList().Get());

    std::lock_guard<std::mutex> lock(_mutex);

    _commandQueue->ExecuteCommandLists(static_cast<UINT>(commandListsToPush.size()), commandListsToPush.data());
    uint64_t fenceValue = Signal();

    auto computeCommandList = commandList->GetComputeCommandList();
    if (computeCommandList)
    {
        auto computeQueue = App->GetModule<ModuleID3D12>()->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
        computeQueue->WaitForCommandQueue(this);
        computeQueue->ExecuteCommandList(computeCommandList);
    }

    _commandListWaiting.emplace(CommandListWaiting{ commandList, fenceValue });
    _commandListWaiting.emplace(CommandListWaiting{ pendingBarriersList, fenceValue });

    return fenceValue;
}

void CommandQueue::WaitForCommandQueue(CommandQueue* commandQueue)
{
    _commandQueue->Wait(commandQueue->_fence.Get(), commandQueue->_fenceValue);
}

void CommandQueue::WaitForFenceValue(uint64_t fenceValue)
{
    if (!IsFenceComplete(fenceValue))
    {
        Chiron::Utils::ThrowIfFailed(_fence->SetEventOnCompletion(fenceValue, _fenceEvent));
        ::WaitForSingleObject(_fenceEvent, INFINITE);
    }
}

void CommandQueue::Flush()
{
    WaitForFenceValue(Signal());
}

std::shared_ptr<CommandList> CommandQueue::GetCommandList()
{
    std::shared_ptr<CommandList> commandList;

    std::lock_guard<std::mutex> lock(_mutex);

    if (!_commandListWaiting.empty() && IsFenceComplete(_commandListWaiting.front().fenceValue))
    {
        CommandListWaiting cmdWaiting;

        Chiron::Utils::TryFrontPop(_commandListWaiting, cmdWaiting);
        commandList = cmdWaiting.commandList;
        commandList->Reset();
    }
    else
    {
        commandList = CreateCommandList();
    }

    return commandList;
}

bool CommandQueue::IsValidCommandListForQueue(D3D12_COMMAND_LIST_TYPE commandListType) {
    switch (_type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        return (commandListType == D3D12_COMMAND_LIST_TYPE_DIRECT ||
            commandListType == D3D12_COMMAND_LIST_TYPE_COMPUTE ||
            commandListType == D3D12_COMMAND_LIST_TYPE_COPY);
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        return (commandListType == D3D12_COMMAND_LIST_TYPE_COMPUTE);
    case D3D12_COMMAND_LIST_TYPE_COPY:
        return (commandListType == D3D12_COMMAND_LIST_TYPE_COPY);
    default:
        return false;
    }
}

std::shared_ptr<CommandList> CommandQueue::CreateCommandList() const
{
    return std::make_shared<CommandList>(_type);
}

uint64_t CommandQueue::Signal()
{
    uint64_t fenceValue = ++_fenceValue;
    Chiron::Utils::ThrowIfFailed(_commandQueue->Signal(_fence.Get(), fenceValue));
    return fenceValue;
}

bool CommandQueue::IsFenceComplete(uint64_t fenceValue)
{
    return _fence->GetCompletedValue() >= fenceValue;
}