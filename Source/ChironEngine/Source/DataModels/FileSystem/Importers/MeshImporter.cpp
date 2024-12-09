#include "Pch.h"
#include "MeshImporter.h"

#include "Application.h"

#include "Modules/ModuleID3D12.h"
#include "Modules/ModuleFileSystem.h"

#include "DataModels/Assets/MeshAsset.h"

#include "DataModels/DX12/CommandList/CommandList.h"
#include "DataModels/DX12/Resource/IndexBuffer.h"
#include "DataModels/DX12/Resource/VertexBuffer.h"

#include "Defines/FileSystemDefine.h"

MeshImporter::MeshImporter()
{
}

MeshImporter::~MeshImporter()
{
}

void MeshImporter::Import(const char* filePath, const std::shared_ptr<MeshAsset>& mesh)
{
    char* filebuffer;
    ModuleFileSystem::LoadFile(filePath, filebuffer);
    Load(filebuffer, mesh);
    Save(mesh);
    delete filebuffer;
}

void MeshImporter::Load(const char* fileBuffer, const std::shared_ptr<MeshAsset>& mesh)
{
    auto d3d12 = App->GetModule<ModuleID3D12>();
    auto copyCommandList = d3d12->GetCommandList(D3D12_COMMAND_LIST_TYPE_COPY);
    auto directCommandList = d3d12->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    unsigned int header[2];
    memcpy(header, fileBuffer, sizeof(header));

    UINT numVertices = header[0];
    UINT numIndices = header[1];

    fileBuffer += sizeof(header);

    // -------------- VERTEX ---------------------

    Vertex* vertexPointer = new Vertex[numVertices];
    unsigned int bytes = sizeof(Vertex) * numVertices;
    memcpy(vertexPointer, fileBuffer, bytes);
    std::vector<Vertex> triangleVertices(vertexPointer, vertexPointer + numVertices);

    const UINT vertexBufferSize = static_cast<UINT>(triangleVertices.size() * sizeof(Vertex));

    std::string newFileName = "Vertex " + mesh->GetName();
    mesh->SetVertexBuffer(CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
        triangleVertices.size(), newFileName);

    D3D12_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pData = triangleVertices.data();
    subresourceData.RowPitch = vertexBufferSize;
    subresourceData.SlicePitch = vertexBufferSize;
    copyCommandList->UpdateBufferResource(mesh->GetVertexBuffer(), 0, 1, &subresourceData);

    fileBuffer += bytes;  

    // -------------- INDEX ---------------------

    UINT* indexPointer = new UINT[numIndices];
    bytes = sizeof(UINT) * numIndices;
    memcpy(indexPointer, fileBuffer, bytes);
    std::vector<UINT> indexBufferData(indexPointer, indexPointer + numIndices);
    const UINT indexBufferSize = static_cast<UINT>(indexBufferData.size() * sizeof(UINT));

    newFileName = "Index " + mesh->GetName();
    mesh->SetIndexBuffer(CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize), indexBufferData.size(),
        DXGI_FORMAT_R32_UINT, newFileName);

    D3D12_SUBRESOURCE_DATA subresourceData2 = {};
    subresourceData2.pData = indexBufferData.data();
    subresourceData2.RowPitch = indexBufferSize;
    subresourceData2.SlicePitch = indexBufferSize;
    copyCommandList->UpdateBufferResource(mesh->GetIndexBuffer(), 0, 1, &subresourceData2);

    // Change states
    directCommandList->TransitionBarrier(mesh->GetIndexBuffer(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    directCommandList->TransitionBarrier(mesh->GetVertexBuffer(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

    auto queueType = copyCommandList->GetType();
    uint64_t initFenceValue = d3d12->ExecuteCommandList(copyCommandList);
    //d3d12->WaitForFenceValue(queueType, initFenceValue);
    CHIRON_TODO("Watch out this");

    queueType = directCommandList->GetType();
    initFenceValue = d3d12->ExecuteCommandList(directCommandList);
    //d3d12->WaitForFenceValue(queueType, initFenceValue);
    CHIRON_TODO("Watch out this");

    delete[] vertexPointer;
    delete[] indexPointer;
}

void MeshImporter::Save(const std::shared_ptr<MeshAsset>& mesh)
{
    auto vertexBuffer = mesh->GetVertexBuffer();
    auto indexBuffer = mesh->GetIndexBuffer();
                        //Vertex
    unsigned int size = static_cast<UINT>(vertexBuffer->GetVertexStride() * vertexBuffer->GetNumVertex()) + 
        //Indices
        (sizeof(UINT) * indexBuffer->GetNumIndices());

    unsigned int header[2] = { static_cast<unsigned int>(vertexBuffer->GetNumVertex()), 
        static_cast<unsigned int>(indexBuffer->GetNumIndices()) };
    size += sizeof(header);

    char* filebuffer = new char[size] {};
    char* cursor = filebuffer;

    unsigned int bytes = sizeof(header);
    memcpy(cursor, header, bytes);
    cursor += bytes;

    // -------------- VERTEX ---------------------

    bytes = static_cast<UINT>(vertexBuffer->GetVertexStride() * vertexBuffer->GetNumVertex());

    void* pData = nullptr;
    HRESULT hr = vertexBuffer->GetResource()->Map(0, nullptr, &pData);
    if (SUCCEEDED(hr))
    {
        const void* vertexData = static_cast<const void*>(pData);

        memcpy(cursor, vertexData, bytes);

        vertexBuffer->GetResource()->Unmap(0, nullptr);
    }
    else
    {
        LOG_ERROR("Couldn't map the mesh resource to save all the vertexs.");
    }
    cursor += bytes;

    // -------------- INDEX ---------------------

    bytes = sizeof(UINT) * static_cast<UINT>(indexBuffer->GetNumIndices());

    pData = nullptr;
    hr = indexBuffer->GetResource()->Map(0, nullptr, &pData);
    if (SUCCEEDED(hr))
    {
        const void* indexData = static_cast<const void*>(pData);
        //size_t dataSize = indexBuffer->GetIndexBufferView().SizeInBytes;

        memcpy(cursor, indexData, bytes);

        indexBuffer->GetResource()->Unmap(0, nullptr);
    }
    else
    {
        LOG_ERROR("Couldn't map the mesh resource to save all the indices.");
    }

    ModuleFileSystem::SaveFile((MESHES_LIB_PATH + std::to_string(mesh->GetUID()) + GENERAL_BINARY_EXTENSION).c_str(), filebuffer, size);

    delete[] filebuffer;
}
