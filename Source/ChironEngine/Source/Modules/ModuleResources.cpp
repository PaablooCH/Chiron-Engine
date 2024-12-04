#include "Pch.h"
#include "ModuleResources.h"

#include "ModuleFileSystem.h"

#include "DataModels/FileSystem/Importers/ModelImporter.h"
#include "DataModels/FileSystem/Importers/TextureImporter.h"

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
    switch (asset->GetType())
    {
    case AssetType::Model:
        _modelImporter->Import(filePath, std::dynamic_pointer_cast<ModelAsset>(asset));
        break;
    case AssetType::Texture:
        _textureImporter->Import(filePath, std::dynamic_pointer_cast<TextureAsset>(asset));
        break;
    case AssetType::Mesh:
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
        AssetTypeUtils::GetFolders(AssetType::Material), AssetTypeUtils::GetFolders(AssetType::Texture),
        AssetTypeUtils::GetFolders(AssetType::Model), AssetTypeUtils::GetFolders(AssetType::Mesh)
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
