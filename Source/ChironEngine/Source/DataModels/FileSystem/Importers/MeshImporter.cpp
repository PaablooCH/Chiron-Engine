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
    char* fileBuffer;
    ModuleFileSystem::LoadFile(filePath, fileBuffer);
    
    char* fileBufferOriginal = fileBuffer;

    mesh->SetName(ModuleFileSystem::GetFile(filePath));

    unsigned int header[2];
    memcpy(header, fileBuffer, sizeof(header));

    UINT numVertices = header[0];
    UINT numIndices = header[1];

    fileBuffer += sizeof(header);

    // -------------- VERTEX ---------------------

    std::vector<Vertex> triangleVertices(reinterpret_cast<Vertex*>(fileBuffer),
        reinterpret_cast<Vertex*>(fileBuffer) + numVertices);
    fileBuffer += sizeof(Vertex) * numVertices;

    const UINT vertexBufferSize = static_cast<UINT>(triangleVertices.size() * sizeof(Vertex));

    std::string newFileName = "Vertex " + mesh->GetName();
    mesh->SetVertexBuffer(CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
        triangleVertices, newFileName);

    // -------------- INDEX ---------------------

    std::vector<UINT> indexBufferData(reinterpret_cast<UINT*>(fileBuffer),
        reinterpret_cast<UINT*>(fileBuffer) + numIndices);
    fileBuffer += sizeof(UINT) * numIndices;

    const UINT indexBufferSize = static_cast<UINT>(indexBufferData.size() * sizeof(UINT));

    newFileName = "Index " + mesh->GetName();
    mesh->SetIndexBuffer(CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize), indexBufferData,
        DXGI_FORMAT_R32_UINT, newFileName);

    Save(mesh);

    delete[] fileBufferOriginal;
}

void MeshImporter::Load(const char* libraryPath, const std::shared_ptr<MeshAsset>& mesh)
{
    if (!ModuleFileSystem::ExistsFile(libraryPath))
    {
        // ------------- META ----------------------

        std::string metaPath = mesh->GetAssetPath() + META_EXT;
        rapidjson::Document doc;
        Json meta = Json(doc);
        ModuleFileSystem::LoadJson(metaPath.c_str(), meta);

        // ------------- REIMPORT FILE ----------------------

        std::string assetPath = meta["assetPath"];
        Import(assetPath.c_str(), mesh);

        return;
    }

    char* fileBuffer;
    ModuleFileSystem::LoadFile(libraryPath, fileBuffer);

    char* fileBufferOriginal = fileBuffer;

    unsigned int header[3];
    memcpy(header, fileBuffer, sizeof(header));
    fileBuffer += sizeof(header);

    mesh->SetName(std::string(fileBuffer, header[0]));
    fileBuffer += header[0];

    UINT numVertices = header[0];
    UINT numIndices = header[1];

    fileBuffer += sizeof(header);

    // -------------- VERTEX ---------------------

    std::vector<Vertex> triangleVertices(reinterpret_cast<Vertex*>(fileBuffer),
        reinterpret_cast<Vertex*>(fileBuffer) + numVertices);
    fileBuffer += sizeof(Vertex) * numVertices;

    const UINT vertexBufferSize = static_cast<UINT>(triangleVertices.size() * sizeof(Vertex));

    std::string newFileName = "Vertex " + mesh->GetName();
    mesh->SetVertexBuffer(CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
        triangleVertices, newFileName);

    // -------------- INDEX ---------------------

    std::vector<UINT> indexBufferData(reinterpret_cast<UINT*>(fileBuffer),
        reinterpret_cast<UINT*>(fileBuffer) + numIndices);
    fileBuffer += sizeof(UINT) * numIndices;

    const UINT indexBufferSize = static_cast<UINT>(indexBufferData.size() * sizeof(UINT));

    newFileName = "Index " + mesh->GetName();
    mesh->SetIndexBuffer(CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize), indexBufferData,
        DXGI_FORMAT_R32_UINT, newFileName);

    delete[] fileBufferOriginal;
}

void MeshImporter::Save(const std::shared_ptr<MeshAsset>& mesh)
{
    auto& triangle = mesh->GetTriangleVertices();
    auto& index = mesh->GetIndexData();

                        //Vertex                                      //Indices
    size_t size = static_cast<UINT>(sizeof(Vertex) * triangle.size()) + (sizeof(UINT) * index.size());

    unsigned int header[3] = { static_cast<unsigned int>(mesh->GetName().size()), 
        static_cast<unsigned int>(triangle.size()), static_cast<unsigned int>(index.size()) };
    size += sizeof(header);

    size += sizeof(char) * static_cast<unsigned int>(mesh->GetName().size());

    char* fileBuffer = new char[size] {};
    char* cursor = fileBuffer;

    unsigned int bytes = sizeof(header);
    memcpy(cursor, header, bytes);
    cursor += bytes;

    bytes = sizeof(char) * static_cast<unsigned int>(mesh->GetName().size());
    memcpy(cursor, &mesh->GetName()[0], bytes);
    cursor += bytes;

    // -------------- VERTEX ---------------------

    bytes = static_cast<UINT>(sizeof(Vertex) * triangle.size());
    memcpy(cursor, triangle.data(), bytes);
    cursor += bytes;

    // -------------- INDEX ---------------------

    bytes = sizeof(UINT) * static_cast<UINT>(index.size());
    memcpy(cursor, index.data(), bytes);
    cursor += bytes;

    ModuleFileSystem::SaveFile(mesh->GetLibraryPath().c_str(), fileBuffer, size);

    delete[] fileBuffer;
}
