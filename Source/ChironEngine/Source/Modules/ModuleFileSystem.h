#pragma once
#include "Module.h"

class TextureImporter;
class ModelImporter;
class Asset;

struct PHYSFS_File;

class ModuleFileSystem : public Module
{
public:
    ModuleFileSystem();
    ~ModuleFileSystem() override;

    bool Init() override;
    bool CleanUp() override;

    void Import(const char* filePath, const std::shared_ptr<Asset>& asset);

    static const std::string GetFileExtension(const char* path);
    static const std::string GetFileName(const std::string& path);
    static const std::string GetPathWithoutFile(const std::string& path);

    // ------------- PHYSFS METHODS ----------------------

    static bool SaveFile(const void* buffer, const char* filePath, size_t size);
    static bool LoadFile(const char* filePath, char*& buffer);
    static bool DeleteFileC(const char* path);
    static bool ExistsFile(const char* path);
    static bool CreateDirectoryC(const char* directoryName);
    static bool IsDirectory(const char* path);

private:
    enum class OpenFileMethod
    {
        APPEND,
        READ,
        WRITE
    };
    static bool OpenFile(const char* filePath, OpenFileMethod method, PHYSFS_File*& result);

private:
    std::unique_ptr<TextureImporter> _textureImporter;
    std::unique_ptr<ModelImporter> _modelImporter;
};
