#include "Pch.h"
#include "ModelImporter.h"

#include "Application.h"

#include "Modules/ModuleID3D12.h"
#include "Modules/ModuleResources.h"

#include "DataModels/Assets/MaterialAsset.h"
#include "DataModels/Assets/MeshAsset.h"
#include "DataModels/Assets/ModelAsset.h"
#include "DataModels/Assets/TextureAsset.h"

#include "DataModels/DX12/CommandList/CommandList.h"
#include "DataModels/DX12/Resource/IndexBuffer.h"
#include "DataModels/DX12/Resource/VertexBuffer.h"

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
        model->SetName(ModuleFileSystem::GetFileName(filePath));
        ImportNode(scene, filePath, model, scene->mRootNode, -1, Matrix::Identity);

        Save(model);

        aiReleaseImport(scene);
    }
    else
    {
        LOG_ERROR("Error loading {}: {}", filePath, aiGetErrorString());
    }
}

void ModelImporter::Load(const char* libraryPath, const std::shared_ptr<ModelAsset>& model)
{
    if (!ModuleFileSystem::ExistsFile(libraryPath))
    {
        // ------------- META ----------------------

        std::string metaPath = model->GetAssetPath() + META_EXT;
        rapidjson::Document doc;
        Json meta = Json(doc);
        ModuleFileSystem::LoadJson(metaPath.c_str(), meta);

        // ------------- REIMPORT FILE ----------------------

        std::string assetPath = meta["assetPath"];
        Import(assetPath.c_str(), model);

        return;
    }

    char* fileBuffer;
    ModuleFileSystem::LoadFile(libraryPath, fileBuffer);
    char* oringinalBuffer = fileBuffer;

    // ------------- BINARY ----------------------

    unsigned int header[2];
    unsigned int bytes = sizeof(header);
    memcpy(header, fileBuffer, bytes);
    fileBuffer += bytes;

    model->SetName(std::string(fileBuffer, header[0]));
    fileBuffer += header[0];

    std::vector<std::unique_ptr<Node>> nodes;
    nodes.reserve(header[1]);

    for (unsigned int i = 0; i < header[1]; ++i)
    {
        std::unique_ptr<Node> node = std::make_unique<Node>();

        unsigned int nodeHeader[2];
        bytes = sizeof(nodeHeader);
        memcpy(nodeHeader, fileBuffer, bytes);
        fileBuffer += bytes;

        node->name = std::string(fileBuffer, nodeHeader[0]);
        fileBuffer += nodeHeader[0];

        memcpy(&node->transform, fileBuffer, sizeof(Matrix));
        fileBuffer += sizeof(Matrix);

        memcpy(&node->parent, fileBuffer, sizeof(int));
        fileBuffer += sizeof(int);

        node->meshMaterial.reserve(nodeHeader[1]);

        // NOT ENGINE
        std::vector<UID> meshesUIDs(nodeHeader[1]);
        memcpy(meshesUIDs.data(), fileBuffer, sizeof(UID) * nodeHeader[1]);
        fileBuffer += sizeof(UID) * nodeHeader[1];

        std::vector<UID> materialsUIDs(nodeHeader[1]);
        memcpy(materialsUIDs.data(), fileBuffer, sizeof(UID) * nodeHeader[1]);
        fileBuffer += sizeof(UID) * nodeHeader[1];

        auto moduleResource = App->GetModule<ModuleResources>();
        for (int i = 0; i < meshesUIDs.size(); ++i)
        {
            std::shared_ptr<MeshAsset> mesh = moduleResource->SearchAsset<MeshAsset>(meshesUIDs[i]);
            std::shared_ptr<MaterialAsset> material = moduleResource->SearchAsset<MaterialAsset>(materialsUIDs[i]);
            node->meshMaterial.emplace_back(mesh, material);
        }

        nodes.push_back(std::move(node));
    }
    model->SetNodes(nodes);

    delete[] oringinalBuffer;
}

void ModelImporter::Save(const std::shared_ptr<ModelAsset>& model)
{
    // ------------- META ----------------------

    std::string metaPath = model->GetAssetPath() + META_EXT;
    rapidjson::Document doc;
    Json meta = Json(doc);
    ModuleFileSystem::LoadJson(metaPath.c_str(), meta);
    auto meshes = meta["MeshesAssetPaths"];
    auto mat = meta["MatAssetPaths"];
    unsigned int countMeshes = 0;
    unsigned int countMat = 0;

    // ------------- BINARY ----------------------

                        //transform         //parent    //name and vector lenght header
    unsigned int size = (sizeof(Matrix) + sizeof(int) + (sizeof(unsigned int) * 2)) * static_cast<unsigned int>(model->GetNodes().size());

    for (auto& node : model->GetNodes())
    {
        size += sizeof(UID) * 2 * static_cast<unsigned int>(node->meshMaterial.size());
        size += sizeof(char) * static_cast<unsigned int>(node->name.size());
    }

    unsigned int header[2] = { static_cast<unsigned int>(model->GetName().size()), static_cast<unsigned int>(model->GetNodes().size()) };
    size += sizeof(header);
    size += sizeof(char) * static_cast<unsigned int>(model->GetName().size());

    char* fileBuffer = new char[size] {};
    char* cursor = fileBuffer;

    unsigned int bytes = sizeof(header);
    memcpy(cursor, header, bytes);
    cursor += bytes;

    bytes = sizeof(char) * static_cast<unsigned int>(model->GetName().size());
    memcpy(cursor, &model->GetName()[0], bytes);
    cursor += bytes;

    for (auto& node : model->GetNodes())
    {
        unsigned int nodeHeader[2] = { static_cast<unsigned int>(node->name.size()),
                                       static_cast<unsigned int>(node->meshMaterial.size()) };

        bytes = sizeof(nodeHeader);
        memcpy(cursor, nodeHeader, bytes);
        cursor += bytes;

        bytes = sizeof(char) * static_cast<unsigned int>(node->name.size());
        memcpy(cursor, &(node->name[0]), bytes);
        cursor += bytes;

        bytes = sizeof(Matrix);
        memcpy(cursor, &(node->transform), bytes);
        cursor += bytes;

        bytes = sizeof(int);
        memcpy(cursor, &(node->parent), bytes);
        cursor += bytes;

        for (int i = 0; i < node->meshMaterial.size(); ++i)
        {
            // ------------- META ----------------------

            meshes[countMeshes] = (MESHES_PATH + node->meshMaterial[i].first->GetName());
            ++countMeshes;

            // ------------- BINARY ----------------------

            UID meshUID = node->meshMaterial[i].first->GetUID();
            memcpy(cursor, &meshUID, sizeof(UID));
            cursor += sizeof(UID);
        }

        for (int i = 0; i < node->meshMaterial.size(); ++i)
        {
            // ------------- META ----------------------

            mat[countMat] = (MATERIALS_PATH + node->meshMaterial[i].second->GetName());
            ++countMat;

            // ------------- BINARY ----------------------

            UID materialUID = node->meshMaterial[i].second->GetUID();
            memcpy(cursor, &materialUID, sizeof(UID));
            cursor += sizeof(UID);
        }
    }

    // ------------- META ----------------------

    rapidjson::StringBuffer buffer = meta.ToBuffer();
    ModuleFileSystem::SaveFile(metaPath.c_str(), buffer.GetString(), (unsigned int)buffer.GetSize());

    // ------------- BINARY ----------------------

    ModuleFileSystem::SaveFile(model->GetLibraryPath().c_str(), fileBuffer, size);

    delete[] fileBuffer;
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
        Node* modelNode = new Node();
        modelNode->name = name;
        modelNode->parent = parentIdx;
        modelNode->transform = transform * accTransform;

        LOG_INFO("Node name: {}", name);
        if (node->mParent)
        {
            LOG_INFO("Parent node name: {}", node->mParent->mName.C_Str());
        }

        for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            LOG_INFO("Importing mesh {}", name);
            LOG_INFO("Importing material {}", material->GetName().C_Str());

            std::shared_ptr<MeshAsset> meshAsset = ImportMesh(mesh, name, i);
            std::shared_ptr<MaterialAsset> materialAsset = ImportMaterial(material, filePath, i);

            std::pair<std::shared_ptr<MeshAsset>, std::shared_ptr<MaterialAsset>> meshMat = std::make_pair(meshAsset, materialAsset);
            modelNode->meshMaterial.push_back(meshMat);
        }

        model->AddNode(modelNode);
        int newParentId = static_cast<int>(model->GetNodes().size()) - 1;

        for (unsigned int i = 0; i < node->mNumChildren; ++i)
        {
            ImportNode(scene, filePath, model, node->mChildren[i], newParentId, Matrix::Identity);
        }
    }
}

std::shared_ptr<MeshAsset> ModelImporter::ImportMesh(const aiMesh* mesh, const std::string& fileName, int iteration)
{
    std::string meshPath = MESHES_PATH + fileName + "_" + std::to_string(iteration) + MESH_EXT;

    if (ModuleFileSystem::ExistsFile(meshPath.c_str()))
    {
        std::shared_ptr<MeshAsset> meshAsset = App->GetModule<ModuleResources>()->RequestAsset<MeshAsset>(meshPath);

        return meshAsset;
    }

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

    ModuleFileSystem::SaveFile(meshPath.c_str(), fileBuffer, size);

    std::shared_ptr<MeshAsset> meshAsset = App->GetModule<ModuleResources>()->RequestAsset<MeshAsset>(meshPath);

    delete[] fileBuffer;

    return meshAsset;
}

std::shared_ptr<MaterialAsset> ModelImporter::ImportMaterial(const aiMaterial* material, const std::string& filePath, int iteration)
{
    auto resources = App->GetModule<ModuleResources>();

    std::string matPath = MATERIALS_PATH + ModuleFileSystem::GetFileName(filePath) + "_" +
        std::to_string(iteration) + MAT_EXT;
    if (ModuleFileSystem::ExistsFile(matPath.c_str()))
    {
        std::shared_ptr<MaterialAsset> materialAsset = resources->RequestAsset<MaterialAsset>(matPath);

        return materialAsset;
    }

    aiString file;

    rapidjson::Document doc;
    Json json = Json(doc);
    json["BaseTexturePath"] = "";
    json["NormalMapPath"] = "";
    json["AmbientOcclusionPath"] = "";
    json["PropertyTexturePath"] = "";
    json["EmissiveTexturePath"] = "";

    if (material->GetTexture(aiTextureType_DIFFUSE, 0, &file) == AI_SUCCESS)
    {
        std::string baseTexturePath = "";
        CheckPathMaterial(filePath.c_str(), file, baseTexturePath);
        json["BaseTexturePath"] = baseTexturePath;
    }

    if (material->GetTexture(aiTextureType_NORMALS, 0, &file) == AI_SUCCESS)
    {
        std::string normalMapPath = "";
        CheckPathMaterial(filePath.c_str(), file, normalMapPath);
        json["NormalMapPath"] = normalMapPath;
    }

    if (material->GetTexture(aiTextureType_LIGHTMAP, 0, &file) == AI_SUCCESS)
    {
        std::string ambientOcclusionPath = "";
        CheckPathMaterial(filePath.c_str(), file, ambientOcclusionPath);
        json["AmbientOcclusionPath"] = ambientOcclusionPath;
    }

    if (material->GetTexture(aiTextureType_METALNESS, 0, &file) == AI_SUCCESS)
    {
        std::string propertyTexturePath = "";
        CheckPathMaterial(filePath.c_str(), file, propertyTexturePath);
        json["PropertyTexturePath"] = propertyTexturePath;
    }

    if (material->GetTexture(aiTextureType_EMISSIVE, 0, &file) == AI_SUCCESS)
    {
        std::string emissiveTexturePath = "";
        CheckPathMaterial(filePath.c_str(), file, emissiveTexturePath);
        json["EmissiveTexturePath"] = emissiveTexturePath;
    }

    // ------------- SAVE MATERIAL FILE ----------------------

    auto filebuffer = json.ToBuffer();

    ModuleFileSystem::SaveFile(matPath.c_str(),
        filebuffer.GetString(), filebuffer.GetSize());

    std::shared_ptr<MaterialAsset> materialAsset = resources->RequestAsset<MaterialAsset>(matPath);

    return materialAsset;
}

void ModelImporter::CheckPathMaterial(const char* filePath, const aiString& file, std::string& dataBuffer)
{
    CHIRON_TODO("instead of check the path, search for the file and copy it into texture asset.");
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