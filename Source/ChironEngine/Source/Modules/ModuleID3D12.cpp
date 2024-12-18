#include "Pch.h"
#include "ModuleID3D12.h"

#include "Application.h"

#include "ModuleWindow.h"

#include "DataModels/DX12/CommandList/CommandList.h"
#include "DataModels/DX12/CommandQueue/CommandQueue.h"
#include "DataModels/DX12/DescriptorAllocator/DescriptorAllocator.h"
#include "DataModels/DX12/Resource/Texture.h"

#if OPTICK
    #include "Optick/optick.h"
#endif // OPTICK

ModuleID3D12::ModuleID3D12() : _currentBuffer(0), _vSync(true), _tearingSupported(false), _supportsRT(false), _bufferFenceValues()
{
}

ModuleID3D12::~ModuleID3D12()
{
}

bool ModuleID3D12::Init()
{
    bool ok = CreateFactory();
    ok = ok && CreateDevice();
    ok = ok && CreateCommandQueues();
    ok = ok && CreateSwapChain();

    if (ok)
    {
        InitDescriptorAllocator();
        ObtainRTVFromSwapChain();
        _currentBuffer = _swapChain->GetCurrentBackBufferIndex();
    }
#ifdef DEBUG
    PrintMessages();
#endif // DEBUG

    return ok;
}

bool ModuleID3D12::CleanUp()
{
    _commandQueueDirect.reset();
    _commandQueueCompute.reset();
    _commandQueueCopy.reset();
    _swapChain = nullptr;
    _descriptorAllocators.clear();
    for (int i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
    {
        _renderBuffers[i].reset();
    }

#ifdef DEBUG
    _debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
#endif // DEBUG

    return true;
}

UpdateStatus ModuleID3D12::PostUpdate()
{
#if OPTICK
    OPTICK_CATEGORY("PostUpdateID3D12", Optick::Category::None);
#endif // DEBUG
#ifdef DEBUG
    PrintMessages();
#endif // DEBUG

    return UpdateStatus::UPDATE_CONTINUE;
}

uint64_t ModuleID3D12::ExecuteCommandList(std::shared_ptr<CommandList>& commandList)
{
    switch (commandList->GetType())
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        return _commandQueueDirect->ExecuteCommandList(std::move(commandList));
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        return _commandQueueCompute->ExecuteCommandList(std::move(commandList));
    case D3D12_COMMAND_LIST_TYPE_COPY:
        return _commandQueueCopy->ExecuteCommandList(std::move(commandList));
    default:
        throw std::invalid_argument("Incorrect queue type.");
    }
}

void ModuleID3D12::SaveCurrentBufferFenceValue(const uint64_t& fenceValue)
{
    _bufferFenceValues[_currentBuffer] = fenceValue;
}

void ModuleID3D12::ToggleVSync()
{
    _vSync = !_vSync;
}

void ModuleID3D12::ResizeBuffers()
{
    Flush();

    // ------------- SWAP-CHAIN ---------------------------

    for (int i = 0; i < NUM_FRAMES_IN_FLIGHT; ++i)
    {
        // Any references to the back buffers must be released before the swap chain can be resized.
        _renderBuffers[i].reset();
        _renderBuffers[i] = nullptr;
    }

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    Chiron::Utils::ThrowIfFailed(_swapChain->GetDesc(&swapChainDesc)); // Get the current descr to apply it to the newer.
    Chiron::Utils::ThrowIfFailed(_swapChain->ResizeBuffers(0, 0, 0, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

    _currentBuffer = _swapChain->GetCurrentBackBufferIndex();

    // ------------- RENDER TARGET VIEW ---------------------------

    ObtainRTVFromSwapChain();
}

void ModuleID3D12::PresentAndSwapBuffer()
{
    UINT syncInterval = _vSync ? 1 : 0;
    UINT presentFlags = _tearingSupported && !_vSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    Chiron::Utils::ThrowIfFailed(_swapChain->Present(syncInterval, presentFlags));

    _currentBuffer = _swapChain->GetCurrentBackBufferIndex();
    _commandQueueDirect->WaitForFenceValue(_bufferFenceValues[_currentBuffer]);
}

void ModuleID3D12::Flush()
{
    _commandQueueDirect->Flush();
    _commandQueueCompute->Flush();
    _commandQueueCopy->Flush();
}

void ModuleID3D12::WaitForFenceValue(D3D12_COMMAND_LIST_TYPE type, uint64_t fenceValue)
{
    switch (type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        _commandQueueDirect->WaitForFenceValue(fenceValue);
        break;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        _commandQueueCompute->WaitForFenceValue(fenceValue);
        break;
    case D3D12_COMMAND_LIST_TYPE_COPY:
        _commandQueueCopy->WaitForFenceValue(fenceValue);
        break;
    default:
        throw std::invalid_argument("Incorrect queue type.");
    }
}

ID3D12CommandQueue* ModuleID3D12::GetID3D12CommandQueue(D3D12_COMMAND_LIST_TYPE type) const
{
    switch (type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        return _commandQueueDirect->GetCommandQueue();
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        return _commandQueueCompute->GetCommandQueue();
    case D3D12_COMMAND_LIST_TYPE_COPY:
        return _commandQueueCopy->GetCommandQueue();
    default:
        assert(false && "Incorrect queue type.");
        break;
    }
    return nullptr;
}

std::shared_ptr<CommandList> ModuleID3D12::GetCommandList(D3D12_COMMAND_LIST_TYPE type) const
{
    switch (type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        return _commandQueueDirect->GetCommandList();
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        return _commandQueueCompute->GetCommandList();
    case D3D12_COMMAND_LIST_TYPE_COPY:
        return _commandQueueCopy->GetCommandList();
    default:
        throw std::invalid_argument("Incorrect queue type.");
    }
}

bool ModuleID3D12::CreateFactory()
{
    UINT dxgiFactoryFlags = 0;
#ifdef DEBUG
    // Create a Debug Controller to track errors
    ComPtr<ID3D12Debug> debugInterface;
    ComPtr<ID3D12Debug1> debugController;
    Chiron::Utils::ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    Chiron::Utils::ThrowIfFailed(debugInterface->QueryInterface(IID_PPV_ARGS(&debugController)));
    debugController->EnableDebugLayer();
    debugController->SetEnableGPUBasedValidation(true);
    //debugController->SetEnableSynchronizedCommandQueueValidation(true);

    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    HRESULT result = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&_factory));

    return SUCCEEDED(result);
}

bool ModuleID3D12::CreateDevice()
{
    ComPtr<IDXGIAdapter1> adapter1;
    ComPtr<IDXGIAdapter4> adapter4;
    ComPtr<ID3D12Device5> device5;

    bool ok = false;
    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != _factory->EnumAdapters1(adapterIndex, &adapter1); ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter1->GetDesc1(&desc);

        // Don't select the Basic Render Driver adapter.
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }

        ComPtr<ID3D12Device5> tmpDevice;

        // Check if the adapter supports Direct3D 12, and use that for the rest
        // of the application
        if (SUCCEEDED(D3D12CreateDevice(_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&tmpDevice))))
        {
            ok = SUCCEEDED(adapter1.As(&adapter4));
            device5 = tmpDevice;
        }
    }

    if (ok)
    {
        BOOL tearing = FALSE;
        _factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearing, sizeof(tearing));

        _tearingSupported = tearing == TRUE;

        D3D12_FEATURE_DATA_D3D12_OPTIONS5 features5{};
        HRESULT hr = device5->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &features5, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS5));
        if (SUCCEEDED(hr) && features5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
        {
            _supportsRT = true;
        }

        _adapter = adapter4;
        _device = device5;
        _device->SetName(L"Device");

#ifdef DEBUG
        // Get debug device
        ok = ok && SUCCEEDED(_device->QueryInterface(IID_PPV_ARGS(&_debugDevice)));
        ok = ok && SUCCEEDED(_device->QueryInterface(IID_PPV_ARGS(&_infoQueue)));
#endif
    }

    return ok;
}

bool ModuleID3D12::CreateCommandQueues()
{
    _commandQueueDirect = std::make_unique<CommandQueue>(D3D12_COMMAND_LIST_TYPE_DIRECT);
    _commandQueueCompute = std::make_unique<CommandQueue>(D3D12_COMMAND_LIST_TYPE_COMPUTE);
    _commandQueueCopy = std::make_unique<CommandQueue>(D3D12_COMMAND_LIST_TYPE_COPY);

    return true;
}

bool ModuleID3D12::CreateSwapChain()
{
    unsigned width;
    unsigned height;

    auto windowModule = App->GetModule<ModuleWindow>();
    windowModule->GetWindowSize(width, height);
    HWND hwnd = windowModule->GetWindowId();

    if (_swapChain != nullptr)
    {
        // Create Render Target Attachments from swapchain
        _swapChain->ResizeBuffers(NUM_FRAMES_IN_FLIGHT, width, height,
            DXGI_FORMAT_R8G8B8A8_UNORM, 0);
        return true;
    }
    else
    {
        // Create swapchain
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = 0;
        swapChainDesc.Height = 0;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc = { 1, 0 };
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = NUM_FRAMES_IN_FLIGHT;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT |
            (_tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);

        ComPtr<IDXGISwapChain1> newSwapchain1;
        Chiron::Utils::ThrowIfFailed(_factory->CreateSwapChainForHwnd(_commandQueueDirect->GetCommandQueue(), hwnd,
            &swapChainDesc, nullptr, nullptr, &newSwapchain1));

        Chiron::Utils::ThrowIfFailed(newSwapchain1.As(&_swapChain));

        // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen will be handled manually.
        Chiron::Utils::ThrowIfFailed(_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

        return true;
    }

    return false;
}

std::unique_ptr<Texture> ModuleID3D12::CreateDepthStencil(const std::string& name)
{
    unsigned width;
    unsigned height;
    App->GetModule<ModuleWindow>()->GetWindowSize(width, height);

    return CreateDepthStencil(name, width, height);
}

std::unique_ptr<Texture> ModuleID3D12::CreateDepthStencil(const std::string& name, unsigned width, unsigned height)
{
    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil = { 1.0f, 0 };
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0,
        D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

    return std::make_unique<Texture>(desc, name, true, &clearValue);
}

void ModuleID3D12::ObtainRTVFromSwapChain()
{
    // ------------- RTV ---------------------------

    // Create a RTV for each frame.
    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
    {
        ComPtr<ID3D12Resource> backBuffer = nullptr;
        Chiron::Utils::ThrowIfFailed(_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        _renderBuffers[i] = std::make_unique<Texture>(backBuffer);
        _renderBuffers[i]->SetName(("Render Buffer " + std::to_string(i)).c_str());
    }
}

void ModuleID3D12::InitDescriptorAllocator()
{
    _descriptorAllocators.reserve(D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES);

    for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
    {
        _descriptorAllocators.push_back(std::make_unique<DescriptorAllocator>(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i)));
    }
}

void ModuleID3D12::PrintMessages()
{
#ifdef DEBUG
    static std::set<std::string> shownMessages;
    const UINT64 numMessages = _infoQueue->GetNumStoredMessages();
    for (UINT64 i = 0; i < numMessages; ++i)
    {
        SIZE_T messageLength = 0;
        // Get Message Lenght
        _infoQueue->GetMessage(i, nullptr, &messageLength);

        std::vector<char> messageData(messageLength);
        D3D12_MESSAGE* message = reinterpret_cast<D3D12_MESSAGE*>(messageData.data());

        // Get Message
        _infoQueue->GetMessage(i, message, &messageLength);

        if (shownMessages.insert(message->pDescription).second)
        {
            switch (message->Severity)
            {
            case D3D12_MESSAGE_SEVERITY_CORRUPTION:
                LOG_FATAL("{}", message->pDescription);
                break;
            case D3D12_MESSAGE_SEVERITY_ERROR:
                LOG_ERROR("{}", message->pDescription);
                break;
            case D3D12_MESSAGE_SEVERITY_WARNING:
                LOG_WARNING("{}", message->pDescription);
                break;
            case D3D12_MESSAGE_SEVERITY_INFO:
                LOG_INFO("{}", message->pDescription);
                break;
            case D3D12_MESSAGE_SEVERITY_MESSAGE:
                LOG_TRACE("{}", message->pDescription);
                break;
            }
        }
    }

    _infoQueue->ClearStoredMessages();
#endif // DEBUG
}