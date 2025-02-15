#pragma once

class DDRenderInterfaceCoreD3D12;

class DebugDrawPass
{
public:
    DebugDrawPass(ID3D12Device2* device, ID3D12CommandQueue* uploadQueue);
    ~DebugDrawPass();

    void record(ID3D12GraphicsCommandList* commandList, uint32_t width, uint32_t height, const Matrix& view, const Matrix& proj);

private:
    static DDRenderInterfaceCoreD3D12* implementation;
};
