#pragma once
#include "Module.h"

#include "DataModels/FileSystem/Json/Json.h"

struct PHYSFS_File;

class ModuleFileSystem : public Module
{
public:
    ModuleFileSystem();
    ~ModuleFileSystem() override;

    bool Init() override;
    bool CleanUp() override;

    static const std::string GetFile(const char* path);
    static const std::string GetFileExtension(const char* path);
    static const std::string GetFileName(const std::string& path);
    static const std::string GetPathWithoutFile(const std::string& path);
    static std::vector<std::string> SplitPath(const std::string& path);
    static bool DeleteDirectory(const char* path);
    static bool MoveDirectory(const char* sourcePath, const char* destinationPath);
    static bool CopyFileC(const char* sourcePath, const char* destPath);
    static std::string TrimPathToDesired(const std::string& fullPath, const std::string& desiredStart);

    // ------------- PHYSFS METHODS ----------------------

    static bool SaveFile(const char* filePath, const void* buffer, size_t size, bool append = false);
    static int LoadFile(const char* filePath, char*& buffer);
    static int LoadJson(const char* filePath, Json& json);
    static bool CopyFilePHYSFS(const char* sourcePath, const char* destPath);
    static bool DeleteFileC(const char* path);
    static bool ExistsFile(const char* path);
    static bool CreateDirectoryC(const char* directoryName);
    static bool CreateUniqueDirectory(std::string& directoryName);
    static bool IsDirectory(const char* path);
    static long long GetModificationDate(const char* path);
    static std::vector<std::string> ListFiles(const char* directoryPath);
    static std::vector<std::string> ListFilesWithPath(const char* directoryPath);

private:
    enum class OpenFileMethod
    {
        APPEND,
        READ,
        WRITE
    };
    static void UniqueName(std::string& directoryName);
    static bool OpenFile(const char* filePath, OpenFileMethod method, PHYSFS_File*& result);
};
