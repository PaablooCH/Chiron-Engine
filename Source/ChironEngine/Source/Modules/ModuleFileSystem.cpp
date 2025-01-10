#include "Pch.h"
#include "ModuleFileSystem.h"

#include <filesystem>
#include "PhysFS/physfs.h"

ModuleFileSystem::ModuleFileSystem()
{
}

ModuleFileSystem::~ModuleFileSystem()
{
}

bool ModuleFileSystem::Init()
{
    PHYSFS_init(nullptr);
    PHYSFS_mount(".", nullptr, 0);
    //PHYSFS_mount("..", nullptr, 0);
    PHYSFS_setWriteDir(".");

    return true;
}

bool ModuleFileSystem::CleanUp()
{
    int deinitResult = PHYSFS_deinit();
    return deinitResult != 0;
}

const std::string ModuleFileSystem::GetFile(const char* path)
{
    std::string sPath(path);

    size_t lastSlash = sPath.find_last_of("/\\");
    if (lastSlash != std::string::npos)
    {
        return sPath.substr(lastSlash + 1);
    }
    else
    {
        return path;
    }
}

const std::string ModuleFileSystem::GetFileExtension(const char* path)
{
    std::string sPath(path);
    size_t dotPosition = sPath.find_last_of('.');
    size_t slashPosition = sPath.find_last_of("/");
    if (dotPosition != std::string::npos && (slashPosition == std::string::npos || dotPosition > slashPosition))
    {
        return sPath.substr(dotPosition);
    }
    else
    {
        return ""; // No extension found or dot is in a directory name
    }
}

const std::string ModuleFileSystem::GetFileName(const std::string& path)
{
    std::string result = "";

    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos)
    {
        result = path.substr(lastSlash + 1);
    }
    else
    {
        result = path;
    }

    size_t lastDot = result.find_last_of(".");
    if (lastDot != std::string::npos)
    {
        result = result.substr(0, lastDot);
    }
    return result;
}

const std::string ModuleFileSystem::GetPathWithoutFile(const std::string& path)
{
    if (path.empty())
    {
        return "";
    }

    // Find the last directory separator
    size_t lastSeparator = path.find_last_of("/\\");
    if (lastSeparator != std::string::npos)
    {
        return path.substr(0, lastSeparator + 1); // Include the separator
    }

    // If no separator is found, return an empty string (no directory path)
    return "";
}

std::vector<std::string> ModuleFileSystem::SplitPath(const std::string& path)
{
    std::vector<std::string> directories;
    for (const auto& part : std::filesystem::path(path)) 
    {
        directories.push_back(part.string());
    }
    return directories;
}

bool ModuleFileSystem::DeleteDirectory(const char* path)
{
    const char* realDir = PHYSFS_getRealDir(path);
    if (!realDir) 
    {
        LOG_ERROR("Error: Directory does not exist in PhysFS.");
        return false;
    }

    try 
    {
        std::filesystem::path fullPath = std::filesystem::path(realDir) / path;
        if (std::filesystem::exists(fullPath)) 
        {
            std::filesystem::remove_all(fullPath);
            LOG_INFO("Directory deleted successfully.");
            PHYSFS_mount(".", nullptr, 0);
            return true;
        }
        else 
        {
            LOG_ERROR("Error: Directory does not exist in the real filesystem.");
            return false;
        }
    }
    catch (const std::filesystem::filesystem_error& e) 
    {
        LOG_ERROR("Filesystem error: {}", e.what());
        return false;
    }
}

bool ModuleFileSystem::MoveDirectory(const char* sourcePath, const char* destinationPath)
{
    const char* realSource = PHYSFS_getRealDir(sourcePath);
    if (!realSource) 
    {
        LOG_ERROR("Error: Unable to locate real source directory: {}", PHYSFS_getLastError());
        return false;
    }

    try 
    {
        std::filesystem::path realSourcePath = std::filesystem::path(realSource) / sourcePath;
        std::filesystem::path realDestinationPath = std::filesystem::path(PHYSFS_getWriteDir()) / destinationPath;

        if (!std::filesystem::exists(realSourcePath)) 
        {
            LOG_ERROR("Error: Source directory does not exist in the filesystem.");
            return false;
        }

        if (std::filesystem::exists(realDestinationPath)) 
        {
            LOG_ERROR("Error: Destination already exists in the filesystem.");
            return false;
        }

        std::filesystem::rename(realSourcePath, realDestinationPath);
        LOG_INFO("Directory moved successfully from {} to {}", realSourcePath.string(), realDestinationPath.string());

        if (!PHYSFS_mount(realDestinationPath.string().c_str(), nullptr, 1))
        {
            LOG_ERROR("Warning: Unable to mount destination into PhysFS: {}", PHYSFS_getLastError());
        }
        if (!PHYSFS_unmount(realSourcePath.string().c_str()))
        {
            LOG_WARNING("Unable to unmount old source path: {}", PHYSFS_getLastError());
        }

        return true;
    }
    catch (const std::filesystem::filesystem_error& e) 
    {
        LOG_ERROR("Filesystem error: {}", e.what());
        return false;
    }
}

bool ModuleFileSystem::CopyFileC(const char* sourcePath, const char* destPath)
{
    try 
    {
        std::filesystem::path destination = destPath;
        std::filesystem::path source = sourcePath;
        std::filesystem::copy(source, destination, std::filesystem::copy_options::overwrite_existing);
        LOG_INFO("File copied successfully");
        if (!PHYSFS_mount(destination.parent_path().string().c_str(), nullptr, 1))
        {
            LOG_ERROR("Warning: Unable to mount destination into PhysFS: {}", PHYSFS_getLastError());
            return false;
        }
        return true;
    }
    catch (const std::filesystem::filesystem_error& e) 
    {
        LOG_ERROR("Error: {}", e.what());
    }
}

std::string ModuleFileSystem::TrimPathToDesired(const std::string& fullPath, const std::string& desiredStart)
{
    std::filesystem::path path(fullPath);
    std::filesystem::path result;

    bool found = false;
    for (const auto& part : path) 
    {
        if (part == desiredStart) 
        {
            found = true;
        }
        if (found) 
        {
            result /= part;
        }
    }

    if (!found) 
    {
        LOG_ERROR("Desired part not found in the path.");
    }

    return result.generic_string();
}

bool ModuleFileSystem::SaveFile(const char* filePath, const void* buffer, size_t size, bool append /*= false */)
{
    PHYSFS_File* handle;
    if (append)
    {
        if (OpenFile(filePath, OpenFileMethod::APPEND, handle))
        {
            if (PHYSFS_writeBytes(handle, buffer, size) < static_cast<PHYSFS_sint64>(size))
            {
                LOG_ERROR("Physfs has error {{}} when try to append {}", PHYSFS_getLastError(), filePath);
                PHYSFS_close(handle);
                return false;
            }
            PHYSFS_close(handle);
            return true;
        }
    }
    else
    {
        if (OpenFile(filePath, OpenFileMethod::WRITE, handle))
        {
            if (PHYSFS_writeBytes(handle, buffer, size) < static_cast<PHYSFS_sint64>(size))
            {
                LOG_ERROR("Physfs has error {{}} when try to write {}", PHYSFS_getLastError(), filePath);
                PHYSFS_close(handle);
                return false;
            }
            PHYSFS_close(handle);
            return true;
        }
    }
    return false;
}

int ModuleFileSystem::LoadFile(const char* filePath, char*& buffer)
{
    PHYSFS_File* handle;
    if (OpenFile(filePath, OpenFileMethod::READ, handle))
    {
        auto size = PHYSFS_fileLength(handle);
        buffer = new char[size + 1] {};
        if (PHYSFS_readBytes(handle, buffer, size) < size)
        {
            LOG_ERROR("Physfs has error {{}} when try to read {}", PHYSFS_getLastError(), filePath);
            PHYSFS_close(handle);
            return -1;
        }
        PHYSFS_close(handle);
        return static_cast<int>(size);
    }
    return -1;
}

int ModuleFileSystem::LoadJson(const char* filePath, Json& json)
{
    if (!ModuleFileSystem::ExistsFile(filePath))
    {
        return -1;
    }
    char* buffer;
    int size = LoadFile(filePath, buffer);

    json.ToJson(buffer);

    delete buffer;

    return size;
}

bool ModuleFileSystem::CopyFilePHYSFS(const char* sourcePath, const char* destPath)
{
    if (!ExistsFile(sourcePath))
    {
        LOG_ERROR("Source file {} doesn't exist.", sourcePath);
    }
    char* buffer = nullptr;
    int size = LoadFile(sourcePath, buffer);
    if (size == -1)
    {
        return false;
    }
    SaveFile(destPath, buffer, size);
    delete buffer;
    return true;
}

bool ModuleFileSystem::DeleteFileC(const char* path)
{
    return PHYSFS_delete(path);
}

bool ModuleFileSystem::ExistsFile(const char* path)
{
    return PHYSFS_exists(path);
}

bool ModuleFileSystem::CreateDirectoryC(const char* directoryName)
{
    return PHYSFS_mkdir(directoryName);
}

bool ModuleFileSystem::CreateUniqueDirectory(std::string& directoryName)
{
    UniqueName(directoryName);
    return CreateDirectoryC(directoryName.c_str());
}

bool ModuleFileSystem::IsDirectory(const char* path)
{
    return PHYSFS_isDirectory(path);
}

long long ModuleFileSystem::GetModificationDate(const char* path)
{
    if (!ExistsFile(path))
    {
        return 0;
    }
    PHYSFS_Stat fileStats;
    PHYSFS_stat(path, &fileStats);
    return fileStats.modtime;
}

std::vector<std::string> ModuleFileSystem::ListFiles(const char* directoryPath)
{
    std::vector<std::string> files;
    char** rc = PHYSFS_enumerateFiles(directoryPath);
    char** i;
    for (i = rc; *i != NULL; ++i)
    {
        files.push_back(*i);
    }
    PHYSFS_freeList(rc);
    return files;
}

std::vector<std::string> ModuleFileSystem::ListFilesWithPath(const char* directoryPath)
{
    std::vector<std::string> files = ListFiles(directoryPath);
    for (int i = 0; i < files.size(); ++i)
    {
        files[i] = directoryPath + files[i];
    }
    return files;
}

void ModuleFileSystem::UniqueName(std::string& directoryName)
{
    std::string uniqueName = directoryName;
    int counter = 1;

    while (true)
    {
        if (!PHYSFS_exists(uniqueName.c_str()))
        {
            break;
        }

        std::ostringstream oss;
        oss << directoryName << "_" << counter++;
        uniqueName = oss.str();
    }

    directoryName = uniqueName;
}

bool ModuleFileSystem::OpenFile(const char* filePath, OpenFileMethod method, PHYSFS_File*& result)
{
    switch (method)
    {
    case ModuleFileSystem::OpenFileMethod::APPEND:
        result = PHYSFS_openAppend(filePath);
        break;
    case ModuleFileSystem::OpenFileMethod::READ:
        result = PHYSFS_openRead(filePath);
        break;
    case ModuleFileSystem::OpenFileMethod::WRITE:
        result = PHYSFS_openWrite(filePath);
        break;
    }
    if (result == nullptr)
    {
        LOG_ERROR("Physfs has error {{}} when try to open {}", PHYSFS_getLastError(), filePath);
        PHYSFS_close(result);
        return false;
    }
    return true;
}