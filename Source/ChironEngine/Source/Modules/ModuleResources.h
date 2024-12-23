#pragma once
#include "Module.h"

#include "ModuleFileSystem.h"

#include "DataModels/FileSystem/UID/UID.h"
#include "DataModels/FileSystem/Json/Json.h"
#include "Enums/AssetType.h"

#include "Defines/FileSystemDefine.h"

class TextureImporter;
class MaterialImporter;
class MeshImporter;
class ModelImporter;
class Asset;

class ModuleResources : public Module
{
public:
    ModuleResources();
    ~ModuleResources();

    bool Init() override;
    bool Start() override;
    bool CleanUp() override;

    // Request resource and Import if is necessary
    template<class A = Asset>
    const std::shared_ptr<A> RequestAsset(const std::string path);

    // Search resource
    template<class A = Asset>
    const std::shared_ptr<A> SearchAsset(UID uid);

private:
    void ImportAsset(const char* filePath, const std::shared_ptr<Asset>& asset);
    void LoadAsset(const std::shared_ptr<Asset>& asset);
    
    std::shared_ptr<Asset> LoadBinary(UID uid);

    std::shared_ptr<Asset> CreateNewAsset(const std::string& assetPath, AssetType type);
    std::shared_ptr<Asset> CreateAssetOfType(AssetType type, UID uid, const std::string& assetPath, const std::string& libraryPath);

    void CreateMetaOfAsset(const std::shared_ptr<Asset>& asset);

    // ------------- GETTERS ----------------------

    std::string GetLibraryPath(UID uid, AssetType type);
    std::string GetLibraryPathByType(AssetType type);
    std::string GetAssetPathByType(AssetType type);
    AssetType GetTypeByExtension(const std::string& path);
    AssetType GetTypeByLibraryPath(const std::string& path);
    AssetType GetTypeByFolderName(std::string& pathWithOutFile);

    void CreateAssetsAndLibraryFolders();

private:
    std::map<UID, std::weak_ptr<Asset>> _assets;

    std::unique_ptr<TextureImporter> _textureImporter;
    std::unique_ptr<MaterialImporter> _materialImporter;
    std::unique_ptr<MeshImporter> _meshImporter;
    std::unique_ptr<ModelImporter> _modelImporter;
};

template<class A>
inline const std::shared_ptr<A> ModuleResources::RequestAsset(const std::string path)
{
    AssetType type = GetTypeByExtension(path);
    if (type == AssetType::UNKNOWN)
    {
        LOG_ERROR("Extension not supported.");
        return nullptr;
    }

    std::string filePath = GetAssetPathByType(type) + ModuleFileSystem::GetFile(path.c_str());
    std::string metaPath = filePath + META_EXT;

    if (ModuleFileSystem::ExistsFile(metaPath.c_str()))
    {
        rapidjson::Document doc;
        Json json = Json(doc);
        ModuleFileSystem::LoadJson(metaPath.c_str(), json);

        UID uid = json["uid"];

        auto it = _assets.find(uid);
        if (it != _assets.end() && !(it->second).expired())
        {
            auto asset = (it->second).lock();
            return std::move(std::dynamic_pointer_cast<A>(asset));
        }
        std::string libraryPath = GetLibraryPath(uid, type);

        auto libraryDate = ModuleFileSystem::GetModificationDate(libraryPath.c_str());
        auto metaDate = ModuleFileSystem::GetModificationDate(metaPath.c_str());

        if (metaDate <= libraryDate)
        {
            auto asset = CreateAssetOfType(type, uid, filePath, libraryPath);
            LoadAsset(asset);
            return std::move(std::dynamic_pointer_cast<A>(asset));
        }
    }
    // If anything previous works import it again
    if (!ModuleFileSystem::ExistsFile(filePath.c_str()))
    {
        CHIRON_TODO("copy the file outside the project into the project");
    }
    auto asset = CreateNewAsset(filePath, type);
    ImportAsset(filePath.c_str(), asset);
    return std::move(std::dynamic_pointer_cast<A>(asset));
}

template<class A>
inline const std::shared_ptr<A> ModuleResources::SearchAsset(UID uid)
{
    std::shared_ptr<Asset> shared;
    auto it = _assets.find(uid);
    if (it != _assets.end() && !(it->second).expired())
    {
        shared = (it->second).lock();
        return std::move(std::dynamic_pointer_cast<A>(shared));
    }
    shared = LoadBinary(uid);
    if (shared)
    {
        return std::move(std::dynamic_pointer_cast<A>(shared));
    }
    LOG_WARNING("Couldn't find or load {} file.", uid);
    return nullptr;
}
