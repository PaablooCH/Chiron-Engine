#include "Pch.h"
#include "ModuleFileSystem.h"

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
    PHYSFS_mount("..", nullptr, 0);
    PHYSFS_setWriteDir(".");

    return true;
}

bool ModuleFileSystem::CleanUp()
{
    int deinitResult = PHYSFS_deinit();
    return deinitResult != 0;
}

const std::string ModuleFileSystem::GetFileExtension(const char* path)
{
    std::string sPath(path);
    size_t dotPosition = sPath.find_last_of('.');
    size_t slashPosition = sPath.find_last_of("/");
    if (dotPosition != std::string::npos && (slashPosition == std::string::npos || dotPosition > slashPosition)) {
        return sPath.substr(dotPosition);
    }
    else {
        return ""; // No extension found or dot is in a directory name
    }
}

const std::string ModuleFileSystem::GetFileName(const std::string& path)
{
    std::string result = "";

    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        result = path.substr(lastSlash + 1);
    }
    else {
        result = path;
    }

    size_t lastDot = result.find_last_of(".");
    if (lastDot != std::string::npos) {
        result = result.substr(0, lastDot);
    }
    return result;
}

const std::string ModuleFileSystem::GetPathWithoutFile(const std::string& path)
{
    if (path.empty()) {
        return "";
    }

    // Find the last directory separator
    size_t lastSeparator = path.find_last_of("/\\");
    if (lastSeparator != std::string::npos) {
        return path.substr(0, lastSeparator + 1); // Include the separator
    }

    // If no separator is found, return an empty string (no directory path)
    return "";
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

bool ModuleFileSystem::CopyFileC(const char* sourcePath, const char* destPath)
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

bool ModuleFileSystem::IsDirectory(const char* path)
{
    return PHYSFS_isDirectory(path);
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