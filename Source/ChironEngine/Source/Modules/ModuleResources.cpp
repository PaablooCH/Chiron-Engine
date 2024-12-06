#include "Pch.h"
#include "ModuleResources.h"

#include "ModuleFileSystem.h"

#include "DataModels/FileSystem/Importers/MaterialImporter.h"
#include "DataModels/FileSystem/Importers/MeshImporter.h"
#include "DataModels/FileSystem/Importers/ModelImporter.h"
#include "DataModels/FileSystem/Importers/TextureImporter.h"

#include "DataModels/Assets/MaterialAsset.h"
#include "DataModels/Assets/MeshAsset.h"
#include "DataModels/Assets/ModelAsset.h"
#include "DataModels/Assets/TextureAsset.h"

#include "Defines/FileSystemDefine.h"

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

void ModuleResources::Import(const char* filePath, const std::shared_ptr<Asset>& asset)
{
    rapidjson::Document doc;
    Json json = Json(doc);

    json["uid"] = asset->GetUID();
    json["type"] = AssetTypeUtils::ToString(asset->GetType());
    rapidjson::StringBuffer buffer = json.ToBuffer();
    std::string metaPath = ASSETS_PATH + AssetTypeUtils::GetFolder(asset->GetType()) + '/' + 
        ModuleFileSystem::GetFileName(filePath) + ModuleFileSystem::GetFileExtension(filePath) + META_EXT;
    ModuleFileSystem::SaveFile(metaPath.c_str(), buffer.GetString(), (unsigned int)buffer.GetSize());

    switch (asset->GetType())
    {
    case AssetType::Material:
        _materialImporter->Import(filePath, std::dynamic_pointer_cast<MaterialAsset>(asset));
        break;
    case AssetType::Mesh:
        _meshImporter->Import(filePath, std::dynamic_pointer_cast<MeshAsset>(asset));
        break;
    case AssetType::Model:
        _modelImporter->Import(filePath, std::dynamic_pointer_cast<ModelAsset>(asset));
        break;
    case AssetType::Texture:
        _textureImporter->Import(filePath, std::dynamic_pointer_cast<TextureAsset>(asset));
        break;
    default:
        break;
    }
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
