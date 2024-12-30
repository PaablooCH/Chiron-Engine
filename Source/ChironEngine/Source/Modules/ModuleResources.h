#pragma once
#include "Module.h"

#include "ModuleFileSystem.h"

#include "DataModels/FileSystem/UID/UID.h"
#include "DataModels/FileSystem/Json/Json.h"
#include "Enums/AssetType.h"

#include "ThreadPool/ThreadPool.h"

#include "Defines/FileSystemDefine.h"

class Asset;
class MaterialImporter;
class MeshImporter;
class ModelImporter;
class TextureImporter;

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
    std::future<std::shared_ptr<A>> RequestAsset(const std::string path);

    // Search resource by UID
    template<class A = Asset>
    std::future<std::shared_ptr<A>> SearchAsset(UID uid);

private:
    void ImportAsset(const std::shared_ptr<Asset>& asset);
    void LoadAsset(const std::shared_ptr<Asset>& asset);

    std::shared_ptr<Asset> LoadBinary(UID uid);

    // ------------- CREATORS ----------------------

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

    std::unique_ptr<ThreadPool> _threadPool;
};

template<class A>
inline std::future<std::shared_ptr<A>> ModuleResources::RequestAsset(const std::string path)
{
    std::shared_ptr<std::promise<std::shared_ptr<A>>> promise = std::make_shared<std::promise<std::shared_ptr<A>>>();
    auto future = promise->get_future();
    _threadPool->AddTask(
        [this, path, promise]() {
            try {
                std::shared_ptr<Asset> shared;
                AssetType type = GetTypeByExtension(path);
                if (type == AssetType::UNKNOWN)
                {
                    LOG_ERROR("Extension not supported.");
                    promise->set_value(nullptr);
                    return;
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
                        shared = (it->second).lock();
                        promise->set_value(std::dynamic_pointer_cast<A>(shared));
                        return;
                    }
                    std::string libraryPath = GetLibraryPath(uid, type);

                    auto libraryDate = ModuleFileSystem::GetModificationDate(libraryPath.c_str());
                    auto metaDate = ModuleFileSystem::GetModificationDate(metaPath.c_str());

                    if (metaDate <= libraryDate)
                    {
                        shared = CreateAssetOfType(type, uid, filePath, libraryPath);
                        LoadAsset(shared);
                        promise->set_value(std::dynamic_pointer_cast<A>(shared));
                        return;
                    }
                }
                // If anything previous works import it again
                if (!ModuleFileSystem::ExistsFile(filePath.c_str()))
                {
                    CHIRON_TODO("copy the file outside the project into the project");
                    promise->set_value(nullptr);
                    return;
                }
                shared = CreateNewAsset(filePath, type);
                ImportAsset(shared);
                promise->set_value(std::dynamic_pointer_cast<A>(shared));
            }
            catch (std::future_error const& e)
            {
                LOG_ERROR("Error during asset request Future error: {}", e.what());
            }
            catch (std::exception const& e)
            {
                LOG_ERROR("Error during asset request Standard exception: {}", e.what());
                promise->set_exception(std::current_exception());
            }
            catch (...)
            {
                LOG_ERROR("Error during asset request Unknown exception");
            }
        });
    return future;
}

template<class A>
inline std::future<std::shared_ptr<A>> ModuleResources::SearchAsset(UID uid)
{
    std::shared_ptr<std::promise<std::shared_ptr<A>>> promise = std::make_shared<std::promise<std::shared_ptr<A>>>();
    auto future = promise->get_future();
    _threadPool->AddTask(
        [this, uid, promise]() {
            try {
                std::shared_ptr<Asset> shared;
                auto it = _assets.find(uid);
                if (it != _assets.end() && !(it->second).expired())
                {
                    shared = (it->second).lock();
                    promise->set_value(std::dynamic_pointer_cast<A>(shared));
                    return;
                }
                shared = LoadBinary(uid);
                if (shared)
                {
                    promise->set_value(std::dynamic_pointer_cast<A>(shared));
                    return;
                }
                LOG_WARNING("Couldn't find or load {} file.", uid);
                promise->set_value(nullptr);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("Error during asset request: {}", e.what());
                promise->set_exception(std::current_exception());
            }
        });
    return future;
}