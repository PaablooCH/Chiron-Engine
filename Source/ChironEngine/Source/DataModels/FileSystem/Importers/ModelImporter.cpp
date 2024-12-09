#include "Pch.h"
#include "ModelImporter.h"

#include "Application.h"

#include "Modules/ModuleID3D12.h"
#include "Modules/ModuleFileSystem.h"
#include "Modules/ModuleResources.h"

#include "DataModels/Assets/MaterialAsset.h"
#include "DataModels/Assets/MeshAsset.h"
#include "DataModels/Assets/ModelAsset.h"
#include "DataModels/Assets/TextureAsset.h"

#include "DataModels/DX12/CommandList/CommandList.h"
#include "DataModels/DX12/Resource/IndexBuffer.h"
#include "DataModels/DX12/Resource/VertexBuffer.h"

#include "Defines/FileSystemDefine.h"

#include "assimp/cimport.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

ModelImporter::ModelImporter()
{
}

ModelImporter::~ModelImporter()
{
}

void ModelImporter::Import(const char* filePath, const std::shared_ptr<ModelAsset>& model)
{
    LOG_INFO("Import Model from {}", filePath);

    const aiScene* scene =
        aiImportFile(filePath, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);
    if (scene)
    {
        ImportNode(scene, filePath, model, scene->mRootNode, -1, Matrix::Identity);
        aiReleaseImport(scene);
    }
    else
    {
        LOG_ERROR("Error loading {}: {}", filePath, aiGetErrorString());
    }
}

void ModelImporter::Load(const char* fileBuffer, const std::shared_ptr<ModelAsset>& resource)
{
}

void ModelImporter::Save(const std::shared_ptr<ModelAsset>& model)
{
}

void ModelImporter::ImportNode(const aiScene* scene, const char* filePath, const std::shared_ptr<ModelAsset>& model, const aiNode* node,
    int parentIdx, const Matrix& accTransform)
{
    std::string name = node->mName.C_Str();
    Matrix transform = (*(Matrix*)&node->mTransformation);

    if (name.find("$AssimpFbx$") != std::string::npos)
    {
        for (unsigned int i = 0; i < node->mNumChildren; ++i)
        {
            const Matrix& newAcctransform = accTransform * transform;

            ImportNode(scene, filePath, model, node->mChildren[i], parentIdx, newAcctransform);
        }
    }
    else
    {
        LOG_INFO("Node name: {}", name);
        if (node->mParent)
        {
            LOG_INFO("Parent node name: {}", node->mParent->mName.C_Str());
        }
        LOG_INFO("Node parentIdx: {}", parentIdx);

        Vector3 pos;
        Quaternion rot;
        Vector3 scale;

        transform.Decompose(scale, rot, pos);

        LOG_INFO("Transform:\n\tpos: ({}, {}, {})\trot: ({}, {}, {})\t scale: ({}, {}, {})",
            pos.x,
            pos.y,
            pos.z,
            DirectX::XMConvertToDegrees(rot.ToEuler().x),
            DirectX::XMConvertToDegrees(rot.ToEuler().y),
            DirectX::XMConvertToDegrees(rot.ToEuler().z),
            scale.x,
            scale.y,
            scale.z);

        // loading meshes and materials
        auto d3d12 = App->GetModule<ModuleID3D12>();
        CHIRON_TODO("Move this to meshImporter");
        auto copyCommandList = d3d12->GetCommandList(D3D12_COMMAND_LIST_TYPE_COPY);
        auto directCommandList = d3d12->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

        for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            LOG_INFO("Importing mesh {}", name);
            LOG_INFO("Importing material {}", material->GetName().C_Str());

            std::shared_ptr<MeshAsset> meshAsset = ImportMesh(mesh, name, i, copyCommandList);
            std::shared_ptr<MaterialAsset> materialAsset = ImportMaterial(material, filePath, i);

            CHIRON_TODO("Move this to meshImporter");
            // Change states
            directCommandList->TransitionBarrier(meshAsset->GetIndexBuffer(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
            directCommandList->TransitionBarrier(meshAsset->GetVertexBuffer(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

            model->AddMaterial(materialAsset);
            model->AddMesh(meshAsset);
        }

        // Copy data to resources
        auto queueType = copyCommandList->GetType();
        uint64_t initFenceValue = d3d12->ExecuteCommandList(copyCommandList);
        d3d12->WaitForFenceValue(queueType, initFenceValue);

        initFenceValue = d3d12->ExecuteCommandList(directCommandList);
        d3d12->WaitForFenceValue(D3D12_COMMAND_LIST_TYPE_DIRECT, initFenceValue);

        for (unsigned int i = 0; i < node->mNumChildren; ++i)
        {
            ImportNode(scene, filePath, model, node->mChildren[i], 0, Matrix::Identity);
        }
    }
}

std::shared_ptr<MeshAsset> ModelImporter::ImportMesh(const aiMesh* mesh, const std::string& fileName, int iteration,
    const std::shared_ptr<CommandList>& copyCommandList)
{
    std::shared_ptr<MeshAsset> resourceMesh = std::make_shared<MeshAsset>();

    // -------------- VERTEX ---------------------

    std::vector<Vertex> triangleVertices;
    triangleVertices.reserve(mesh->mNumVertices);
    for (UINT i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex newVertex;

        newVertex.vertices = Vector3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

        if (mesh->HasTextureCoords(0))
        {
            newVertex.texCoords = Vector2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }

        if (mesh->HasNormals())
        {
            newVertex.normals = Vector3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        }

        if (mesh->HasTangentsAndBitangents())
        {
            newVertex.tangents = Vector3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
            newVertex.biTangents = Vector3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        }
        triangleVertices.push_back(newVertex);
    }
    const UINT vertexBufferSize = static_cast<UINT>(triangleVertices.size() * sizeof(Vertex));

    std::string newFileName = "Vertex " + resourceMesh->GetName();
    resourceMesh->SetVertexBuffer(CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
        triangleVertices.size(), newFileName);

    D3D12_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pData = triangleVertices.data();
    subresourceData.RowPitch = vertexBufferSize;
    subresourceData.SlicePitch = vertexBufferSize;
    copyCommandList->UpdateBufferResource(resourceMesh->GetVertexBuffer(), 0, 1, &subresourceData);

    // -------------- INDEX ---------------------

    UINT numIndices = mesh->mNumFaces * 3;

    std::vector<UINT> indexBufferData;
    indexBufferData.reserve(numIndices);
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        indexBufferData.push_back(mesh->mFaces[i].mIndices[0]);
        indexBufferData.push_back(mesh->mFaces[i].mIndices[1]);
        indexBufferData.push_back(mesh->mFaces[i].mIndices[2]);
    }
    const UINT indexBufferSize = static_cast<UINT>(indexBufferData.size() * sizeof(UINT));

    newFileName = "Index " + resourceMesh->GetName();
    resourceMesh->SetIndexBuffer(CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize), indexBufferData.size(),
        DXGI_FORMAT_R32_UINT, newFileName);

    D3D12_SUBRESOURCE_DATA subresourceData2 = {};
    subresourceData2.pData = indexBufferData.data();
    subresourceData2.RowPitch = indexBufferSize;
    subresourceData2.SlicePitch = indexBufferSize;
    copyCommandList->UpdateBufferResource(resourceMesh->GetIndexBuffer(), 0, 1, &subresourceData2);

    // ------------- SAVE MESH FILE ----------------------

                        //Vertex                                //Indices
    unsigned int size = (sizeof(Vertex) * mesh->mNumVertices) + (sizeof(UINT) * (mesh->mNumFaces * 3));

    unsigned int header[2] = { static_cast<unsigned int>(mesh->mNumVertices), mesh->mNumFaces * 3 };
    size += sizeof(header);

    char* fileBuffer = new char[size] {};
    char* cursor = fileBuffer;

    unsigned int bytes = sizeof(header);
    memcpy(cursor, header, bytes);
    cursor += bytes;

    bytes = sizeof(Vertex) * mesh->mNumVertices;
    memcpy(cursor, triangleVertices.data(), bytes);
    cursor += bytes;

    bytes = sizeof(UINT) * (mesh->mNumFaces * 3);
    memcpy(cursor, indexBufferData.data(), bytes);
    cursor += bytes;

    ModuleFileSystem::SaveFile((MESHES_PATH + resourceMesh->GetName()).c_str(), fileBuffer, size);

    delete[] fileBuffer;

    return resourceMesh;
}

std::shared_ptr<MaterialAsset> ModelImporter::ImportMaterial(const aiMaterial* material, const std::string& filePath, int iteration)
{
    std::shared_ptr<MaterialAsset> materialAsset = std::make_shared<MaterialAsset>();

    auto resources = App->GetModule<ModuleResources>();

    aiString file;

    if (material->GetTexture(aiTextureType_DIFFUSE, 0, &file) == AI_SUCCESS)
    {
        std::string baseTexturePath = "";

        CheckPathMaterial(filePath.c_str(), file, baseTexturePath);

        if (baseTexturePath != "")
        {
            std::shared_ptr<TextureAsset> textureAsset = std::make_shared<TextureAsset>(TextureType::DIFFUSE);

            resources->Import(baseTexturePath.c_str(), textureAsset);
            materialAsset->SetBaseTexture(textureAsset);
        }
    }

    if (material->GetTexture(aiTextureType_NORMALS, 0, &file) == AI_SUCCESS)
    {
        std::string normalMapPath = "";

        CheckPathMaterial(filePath.c_str(), file, normalMapPath);

        if (normalMapPath != "")
        {
            std::shared_ptr<TextureAsset> textureAsset = std::make_shared<TextureAsset>(TextureType::NORMAL_MAP);

            resources->Import(normalMapPath.c_str(), textureAsset);
            materialAsset->SetNormalMap(textureAsset);
        }
    }

    if (material->GetTexture(aiTextureType_LIGHTMAP, 0, &file) == AI_SUCCESS)
    {
        std::string ambientOcclusionPath = "";

        CheckPathMaterial(filePath.c_str(), file, ambientOcclusionPath);

        if (ambientOcclusionPath != "")
        {
            std::shared_ptr<TextureAsset> textureAsset = std::make_shared<TextureAsset>(TextureType::OCCLUSION);

            resources->Import(ambientOcclusionPath.c_str(), textureAsset);

            materialAsset->SetAmbientOcclusion(textureAsset);
        }
    }

    if (material->GetTexture(aiTextureType_METALNESS, 0, &file) == AI_SUCCESS)
    {
        std::string propertyTexturePath = "";

        CheckPathMaterial(filePath.c_str(), file, propertyTexturePath);

        if (propertyTexturePath != "")
        {
            std::shared_ptr<TextureAsset> textureAsset = std::make_shared<TextureAsset>(TextureType::METALLIC);

            resources->Import(propertyTexturePath.c_str(), textureAsset);
            materialAsset->SetPropertyTexture(textureAsset);
        }
    }

    if (material->GetTexture(aiTextureType_EMISSIVE, 0, &file) == AI_SUCCESS)
    {
        std::string emissiveTexturePath = "";

        CheckPathMaterial(filePath.c_str(), file, emissiveTexturePath);

        if (emissiveTexturePath != "")
        {
            std::shared_ptr<TextureAsset> textureAsset = std::make_shared<TextureAsset>(TextureType::EMISSIVE);

            resources->Import(emissiveTexturePath.c_str(), textureAsset);
            materialAsset->SetEmissiveTexture(textureAsset);
        }
    }

    return materialAsset;
}

void ModelImporter::CheckPathMaterial(const char* filePath, const aiString& file, std::string& dataBuffer)
{
    // No exists in its file
    if (!ModuleFileSystem::ExistsFile(file.data))
    {
        std::string name = ModuleFileSystem::GetFileName(file.data);
        name += ModuleFileSystem::GetFileExtension(file.data);

        std::string modelPath = ModuleFileSystem::GetPathWithoutFile(filePath);
        // No exists in its model path
        if (!ModuleFileSystem::ExistsFile((modelPath + name).c_str()))
        {
            // No exists in the engine folder
            if (!ModuleFileSystem::ExistsFile((TEXTURES_PATH + name).c_str()))
            {
                LOG_ERROR("Texture not found!!!");
            }
            else
            {
                // Exists in the engine folder
                dataBuffer = TEXTURES_PATH + name;
            }
        }
        else
        {
            // Exists in its model path
            dataBuffer = modelPath + name;
        }
    }
    else
    {
        // Exists in its file
        dataBuffer = file.data;
    }
}