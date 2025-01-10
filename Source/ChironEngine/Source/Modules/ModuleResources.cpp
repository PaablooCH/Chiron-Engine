#include "Pch.h"
#include "ModuleResources.h"

#include "DataModels/FileSystem/Importers/MaterialImporter.h"
#include "DataModels/FileSystem/Importers/MeshImporter.h"
#include "DataModels/FileSystem/Importers/ModelImporter.h"
#include "DataModels/FileSystem/Importers/TextureImporter.h"

#include "DataModels/Assets/MaterialAsset.h"
#include "DataModels/Assets/MeshAsset.h"
#include "DataModels/Assets/ModelAsset.h"
#include "DataModels/Assets/TextureAsset.h"

#include "DataModels/FileSystem/UID/UIDGenerator.h"

ModuleResources::ModuleResources()
{
}

ModuleResources::~ModuleResources()
{
}

bool ModuleResources::Init()
{
    _textureImporter = std::make_unique<TextureImporter>();
    _materialImporter = std::make_unique<MaterialImporter>();
    _meshImporter = std::make_unique<MeshImporter>();
    _modelImporter = std::make_unique<ModelImporter>();

    _threadPool = std::make_unique<ThreadPool>(8);

    return true;
}

bool ModuleResources::Start()
{
    CreateAssetsAndLibraryFolders();
    return true;
}

bool ModuleResources::CleanUp()
{
    return true;
}

void ModuleResources::ImportAsset(const std::shared_ptr<Asset>& asset)
{
    if (asset->GetLibraryPath() == "")
    {
        std::string libraryPath = GetLibraryPathByType(asset->GetType()) + std::to_string(asset->GetUID()) + BINARY_EXT;
        asset->SetLibraryPath(libraryPath);
    }
    switch (asset->GetType())
    {
    case AssetType::Material:
        _materialImporter->Import(asset->GetAssetPath().c_str(), std::dynamic_pointer_cast<MaterialAsset>(asset));
        break;
    case AssetType::Mesh:
        _meshImporter->Import(asset->GetAssetPath().c_str(), std::dynamic_pointer_cast<MeshAsset>(asset));
        break;
    case AssetType::Model:
        _modelImporter->Import(asset->GetAssetPath().c_str(), std::dynamic_pointer_cast<ModelAsset>(asset));
        break;
    case AssetType::Texture:
        _textureImporter->Import(asset->GetAssetPath().c_str(), std::dynamic_pointer_cast<TextureAsset>(asset));
        break;
    case AssetType::UNKNOWN:
        break;
    }

    _assets[asset->GetUID()] = asset;
}

void ModuleResources::LoadAsset(const std::shared_ptr<Asset>& asset)
{
    switch (asset->GetType())
    {
    case AssetType::Material:
        _materialImporter->Load(asset->GetLibraryPath().c_str(), std::dynamic_pointer_cast<MaterialAsset>(asset));
        break;
    case AssetType::Mesh:
        _meshImporter->Load(asset->GetLibraryPath().c_str(), std::dynamic_pointer_cast<MeshAsset>(asset));
        break;
    case AssetType::Model:
        _modelImporter->Load(asset->GetLibraryPath().c_str(), std::dynamic_pointer_cast<ModelAsset>(asset));
        break;
    case AssetType::Texture:
        _textureImporter->Load(asset->GetLibraryPath().c_str(), std::dynamic_pointer_cast<TextureAsset>(asset));
        break;
    case AssetType::UNKNOWN:
        LOG_WARNING("Try to load an UNKNOWN asset.");
        break;
    }

    _assets[asset->GetUID()] = asset;
}

std::shared_ptr<Asset> ModuleResources::LoadBinary(UID uid)
{
    std::string binaryFile = std::to_string(uid) + BINARY_EXT;

    std::vector<std::string> filesInLibPath = ModuleFileSystem::ListFilesWithPath(LIB_PATH);
    std::queue<std::string> filesToCheck;
    int i = 0;
    if (!filesInLibPath.empty())
    {
        filesToCheck.push(filesInLibPath[i++]);
    }

    while (!filesToCheck.empty())
    {
        std::string path = filesToCheck.front();
        filesToCheck.pop();
        if (ModuleFileSystem::IsDirectory(path.c_str()))
        {
            path += "/";
            std::vector<std::string> filesInsideDirectory = ModuleFileSystem::ListFilesWithPath(path.c_str());

            for (const auto& file : filesInsideDirectory)
            {
                filesToCheck.push(file);
            }
        }
        else
        {
            auto fileName = ModuleFileSystem::GetFile(path.c_str());
            if (fileName == binaryFile)
            {
                auto type = GetTypeByLibraryPath(path);
                auto asset = CreateAssetOfType(type, uid, "", path);
                LoadAsset(asset);
                return asset;
            }
        }

        if (filesToCheck.empty() && i < filesInLibPath.size())
        {
            filesToCheck.push(filesInLibPath[i++]);
        }
    }
    return nullptr;
}

std::shared_ptr<Asset> ModuleResources::CreateNewAsset(const std::string& assetPath, AssetType type)
{
    UID uid = Chiron::UIDGenerator::GenerateUID();
    std::string libraryPath = GetLibraryPath(uid, type);

    auto asset = CreateAssetOfType(type, uid, assetPath, libraryPath);
    CreateMetaOfAsset(asset);
    return asset;
}

std::shared_ptr<Asset> ModuleResources::CreateAssetOfType(AssetType type, UID uid, const std::string& assetPath, const std::string& libraryPath)
{
    std::shared_ptr<Asset> asset;
    switch (type)
    {
    case AssetType::Material:
        asset = std::make_shared<MaterialAsset>(uid, assetPath, libraryPath);
        break;
    case AssetType::Mesh:
        asset = std::make_shared<MeshAsset>(uid, assetPath, libraryPath);
        break;
    case AssetType::Model:
        asset = std::make_shared<ModelAsset>(uid, assetPath, libraryPath);
        break;
    case AssetType::Texture:
        asset = std::make_shared<TextureAsset>(uid, assetPath, libraryPath);
        break;
    case AssetType::UNKNOWN:
        LOG_WARNING("Try to create an Asset with UNKNOWN type.");
        break;
    }

    if (asset)
    {
        asset->SetName(ModuleFileSystem::GetFileName(assetPath));
        _assets[uid] = asset;
    }
    return asset;
}

void ModuleResources::CreateMetaOfAsset(const std::shared_ptr<Asset>& asset)
{
    std::string filePath = asset->GetAssetPath();
    rapidjson::Document doc;
    Json json = Json(doc);

    json["uid"] = asset->GetUID();
    json["type"] = AssetTypeUtils::ToString(asset->GetType());
    json["assetPath"] = asset->GetAssetPath();
    rapidjson::StringBuffer buffer = json.ToBuffer();
    std::string metaPath = filePath + META_EXT;
    ModuleFileSystem::SaveFile(metaPath.c_str(), buffer.GetString(), (unsigned int)buffer.GetSize());
}

std::string ModuleResources::GetLibraryPath(UID uid, AssetType type)
{
    std::string path = GetLibraryPathByType(type);
    path += std::to_string(uid) + BINARY_EXT;
    return path;
}

AssetType ModuleResources::GetTypeByExtension(const std::string& path)
{
    std::string fileExtension = ModuleFileSystem::GetFileExtension(path.c_str());
    std::string normalizedExtension = "";

    for (int i = 0; i < fileExtension.size(); ++i)
    {
        normalizedExtension.push_back(tolower(fileExtension[i]));
    }

    if (normalizedExtension == FBX_EXT || normalizedExtension == OBJ_EXT || normalizedExtension == GLTF_EXT)
    {
        return AssetType::Model;
    }
    else if (normalizedExtension == JPG_EXT || normalizedExtension == PNG_EXT ||
        normalizedExtension == TIF_EXT || normalizedExtension == DDS_EXT ||
        normalizedExtension == TGA_EXT || normalizedExtension == HDR_EXT)
    {
        return AssetType::Texture;
    }
    else if (normalizedExtension == MAT_EXT)
    {
        return AssetType::Material;
    }
    else if (normalizedExtension == MESH_EXT)
    {
        return AssetType::Mesh;
    }

    return AssetType::UNKNOWN;
}

std::string ModuleResources::GetLibraryPathByType(AssetType type)
{
    switch (type)
    {
    case AssetType::Material:
        return MATERIALS_LIB_PATH;
    case AssetType::Mesh:
        return MESHES_LIB_PATH;
    case AssetType::Model:
        return MODELS_LIB_PATH;
    case AssetType::Texture:
        return TEXTURES_LIB_PATH;
    case AssetType::UNKNOWN:
        LOG_WARNING("Try to get the UNKNOWN library folder.");
        break;
    };
    return "";
}

AssetType ModuleResources::GetTypeByLibraryPath(const std::string& path)
{
    std::string pathWithOutFile = ModuleFileSystem::GetPathWithoutFile(path);
    std::string::size_type libPathPos = pathWithOutFile.find(LIB_PATH);

    if (libPathPos != std::string::npos)
    {
        pathWithOutFile.erase(libPathPos, std::string(LIB_PATH).length());
        pathWithOutFile.pop_back();
    }

    return GetTypeByFolderName(pathWithOutFile);
}

AssetType ModuleResources::GetTypeByFolderName(std::string& folderName)
{
    if (folderName == "Models")
    {
        return AssetType::Model;
    }
    if (folderName == "Textures")
    {
        return AssetType::Texture;
    }
    if (folderName == "Meshes")
    {
        return AssetType::Mesh;
    }
    if (folderName == "Materials")
    {
        return AssetType::Material;
    }
    return AssetType::UNKNOWN;
}

void ModuleResources::CreateAssetsAndLibraryFolders()
{
    if (!ModuleFileSystem::IsDirectory(ASSETS_FOLDER))
    {
        ModuleFileSystem::CreateDirectoryC(ASSETS_FOLDER);
    }
    if (!ModuleFileSystem::IsDirectory(LIB_FOLDER))
    {
        ModuleFileSystem::CreateDirectoryC(LIB_FOLDER);
    }

    std::vector<std::string> folders = {
        AssetTypeUtils::GetFolder(AssetType::Material), AssetTypeUtils::GetFolder(AssetType::Texture),
        AssetTypeUtils::GetFolder(AssetType::Model), AssetTypeUtils::GetFolder(AssetType::Mesh)
    };

    for (auto& folder : folders)
    {
        std::string assetsFolderOfType = ASSETS_PATH + folder;
        if (!ModuleFileSystem::IsDirectory(assetsFolderOfType.c_str()))
        {
            ModuleFileSystem::CreateDirectoryC(assetsFolderOfType.c_str());
        }

        std::string libraryFolderOfType = LIB_PATH + folder;
        if (!ModuleFileSystem::IsDirectory(libraryFolderOfType.c_str()))
        {
            ModuleFileSystem::CreateDirectoryC(libraryFolderOfType.c_str());
        }
    }
}